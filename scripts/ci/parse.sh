#!/bin/bash

set -e

LOG_ROOT="ci_logs"
REPORT_ROOT="$LOG_ROOT/memory_usage"
SUMMARY_LOG="$LOG_ROOT/memory_report.log"
SUMMARY_CSV="$LOG_ROOT/memory_report.csv"

GENERATE_CSV=0

usage() {
    echo "Usage: $0 [--csv]"
    echo
    echo "Without options: generate table .log reports."
    echo "With --csv: generate table .log reports and CSV reports."
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --csv)
            GENERATE_CSV=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "[parse_memory.sh] ERROR: Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

config_order() {
    case "$1" in
        min) echo 1 ;;
        suspend) echo 2 ;;
        retention) echo 3 ;;
        max) echo 4 ;;
        max_all) echo 5 ;;
        *) echo 99 ;;
    esac
}

safe_filename() {
    local value="$1"
    value="${value//\//__}"
    value="${value//\\/__}"
    echo "$value"
}

table_from_tsv() {
    local input_file="$1"

    awk -F '\t' '
    BEGIN {
        n = 12

        header[1] = "SOC"
        header[2] = "Sample"
        header[3] = "Config"
        header[4] = "Flash"
        header[5] = "RAM"
        header[6] = "Text"
        header[7] = "Code"
        header[8] = "Rodata"
        header[9] = "Data"
        header[10] = "BSS"
        header[11] = "Dec"
        header[12] = "Hex"

        for (i = 1; i <= n; i++) {
            width[i] = length(header[i])
        }
    }

    {
        rows[NR] = $0

        for (i = 1; i <= n; i++) {
            if (length($i) > width[i]) {
                width[i] = length($i)
            }
        }
    }

    function repeat(ch, count, result, i) {
        result = ""

        for (i = 0; i < count; i++) {
            result = result ch
        }

        return result
    }

    function print_row(values, i) {
        for (i = 1; i <= n; i++) {
            printf "%*s", width[i], values[i]

            if (i < n) {
                printf "  "
            }
        }

        printf "\n"
    }

    END {
        if (NR == 0) {
            exit
        }

        print_row(header)

        for (i = 1; i <= n; i++) {
            printf "%s", repeat("-", width[i])

            if (i < n) {
                printf "  "
            }
        }

        printf "\n"

        for (row = 1; row <= NR; row++) {
            split(rows[row], values, FS)
            print_row(values)
        }
    }
    ' "$input_file"
}

csv_from_tsv() {
    local input_file="$1"

    {
        echo "SOC,Sample,Config,Flash,RAM,Text,Code,Rodata,Data,BSS,Dec,Hex"

        awk -F '\t' '
        {
            print $1 "," $2 "," $3 "," $4 "," $5 "," $6 "," $7 "," $8 "," $9 "," $10 "," $11 "," $12
        }
        ' "$input_file"
    }
}

if [ ! -d "$LOG_ROOT" ]; then
    echo "[parse_memory.sh] ERROR: logs directory not found: $LOG_ROOT"
    exit 1
fi

mkdir -p "$REPORT_ROOT"

ROWS_TMP="$(mktemp)"
SORTED_ROWS_TMP="$(mktemp)"

cleanup() {
    rm -f "$ROWS_TMP" "$SORTED_ROWS_TMP"
}

trap cleanup EXIT

while IFS= read -r log_file; do
    rel_path="${log_file#$LOG_ROOT/}"

    soc="${rel_path%%/*}"
    rest="${rel_path#*/}"

    if [ "$soc" = "$rel_path" ]; then
        continue
    fi

    config_file="${rest##*/}"
    config="${config_file%.log}"
    sample="${rest%/*}"

    if [ -z "$soc" ] || [ -z "$sample" ] || [ -z "$config" ]; then
        continue
    fi

    size_line="$(awk '
        /^[[:space:]]*[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9a-fA-F]+[[:space:]].*Telink\.elf[[:space:]]*$/ {
            print $1, $2, $3, $4, $5, $6, $7
            exit
        }
    ' "$log_file")"

    if [ -z "$size_line" ]; then
        echo "[parse_memory.sh] warning: no memory data in $log_file"
        continue
    fi

    read -r text code rodata data bss dec hex_value <<< "$size_line"

    flash=$((text + data))
    ram=$((data + bss))
    order="$(config_order "$config")"

    printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" \
        "$soc" \
        "$sample" \
        "$order" \
        "$config" \
        "$flash" \
        "$ram" \
        "$text" \
        "$code" \
        "$rodata" \
        "$data" \
        "$bss" \
        "$dec" \
        "$hex_value" >> "$ROWS_TMP"

done < <(
    find "$LOG_ROOT" \
        -type f \
        -name "*.log" \
        ! -path "$REPORT_ROOT/*" \
        ! -name "memory_report.log" \
        | sort
)

if [ ! -s "$ROWS_TMP" ]; then
    echo "[parse_memory.sh] ERROR: no memory data found"
    exit 1
fi

sort -t "$(printf '\t')" -k1,1 -k2,2 -k3,3n -k4,4 "$ROWS_TMP" | \
    awk -F '\t' 'BEGIN { OFS = FS } { print $1, $2, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13 }' \
    > "$SORTED_ROWS_TMP"

{
    echo "Memory usage report, bytes"
    echo "========================================="
    echo
    table_from_tsv "$SORTED_ROWS_TMP"
    echo
    echo "Separate reports: $REPORT_ROOT"
    echo
} > "$SUMMARY_LOG"

echo "[parse_memory.sh] summary report written to $SUMMARY_LOG"

awk -F '\t' '!seen[$1 FS $2]++ { print $1 FS $2 }' "$SORTED_ROWS_TMP" | \
while IFS="$(printf '\t')" read -r soc sample; do
    sample_file="$(safe_filename "$sample")"

    report_dir="$REPORT_ROOT/$soc"
    report_log="$report_dir/$sample_file.log"
    report_csv="$report_dir/$sample_file.csv"

    mkdir -p "$report_dir"

    sample_rows_tmp="$(mktemp)"

    awk -F '\t' -v soc="$soc" -v sample="$sample" '
        $1 == soc && $2 == sample {
            print
        }
    ' "$SORTED_ROWS_TMP" > "$sample_rows_tmp"

    {
        echo "Memory usage report for $soc/$sample, bytes"
        echo "========================================="
        echo
        table_from_tsv "$sample_rows_tmp"
        echo
    } > "$report_log"

    echo "[parse_memory.sh] sample report written to $report_log"

    if [ "$GENERATE_CSV" -eq 1 ]; then
        csv_from_tsv "$sample_rows_tmp" > "$report_csv"
        echo "[parse_memory.sh] sample CSV report written to $report_csv"
    fi

    rm -f "$sample_rows_tmp"
done

if [ "$GENERATE_CSV" -eq 1 ]; then
    csv_from_tsv "$SORTED_ROWS_TMP" > "$SUMMARY_CSV"
    echo "[parse_memory.sh] summary CSV report written to $SUMMARY_CSV"
fi
