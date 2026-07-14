#!/bin/bash
#
# sync_to_github.sh — Sync UniSDK files + build artifacts to GitLab SDK repo
#
# This script:
#   1. Clones the GitLab SDK repo (sdk_app/unisdk/unisdk.git)
#   2. Clears existing content
#   3. Copies files listed in scripts/ci/sync_manifest.txt
#   4. Copies .a static library files from CI build artifacts
#   5. Commits and pushes to the GitLab SDK repo
#
# Environment variables:
#   SDK_REPO_URL          [required]  GitLab SDK repo URL (http://${SDK_DEPLOY_TOKEN_USER}:${SDK_DEPLOY_TOKEN_PASSWORD}@192.168.48.36/sdk_app/unisdk/unisdk.git)
#   CI_PROJECT_DIR        [required]  Path to the src repo (set by GitLab CI)
#   SDK_REPO_BRANCH       [optional]  Branch to push to (default: CI_COMMIT_REF_NAME or "main")
#   ARTIFACTS_DIR         [optional]  Path to .a artifacts (default: $CI_PROJECT_DIR/ci_libs)
#   GIT_USER_NAME         [optional]  Git user name for commit
#   GIT_USER_EMAIL        [optional]  Git user email for commit
# =============================================================================

set -euo pipefail

# === Argument parsing ===
SKIP_LIBS=false
while [[ $# -gt 0 ]]; do
    case "$1" in
        --skip-libs) SKIP_LIBS=true; shift ;;
        *) echo "[ERROR] Unknown argument: $1"; exit 1 ;;
    esac
done

# === Required environment ===
SDK_REPO_URL="${SDK_REPO_URL:?SDK_REPO_URL is not set}"
CI_PROJECT_DIR="${CI_PROJECT_DIR:?CI_PROJECT_DIR is not set}"
SDK_REPO_BRANCH="${SDK_REPO_BRANCH:-${CI_COMMIT_REF_NAME:-main}}"
ARTIFACTS_DIR="${ARTIFACTS_DIR:-$CI_PROJECT_DIR/ci_libs}"
GENERATED_HEADERS_DIR="${GENERATED_HEADERS_DIR:-$CI_PROJECT_DIR/ci_generated_headers}"
KCONFIG_DIR="${KCONFIG_DIR:-$CI_PROJECT_DIR/ci_kconfig}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SDK_CLONE_DIR="ci_sdk_repo"
MANIFEST="${MANIFEST:-$CI_PROJECT_DIR/scripts/ci/sync_manifest.txt}"
COMMIT_MSG_FILE="$CI_PROJECT_DIR/ci_commit_msg.txt"

echo "============================================"
echo " UniSDK → GitLab SDK Repo Sync"
echo "============================================"
echo "Target:    $SDK_REPO_URL ($SDK_REPO_BRANCH)"
echo "Source:    $CI_PROJECT_DIR"
echo "Artifacts: $ARTIFACTS_DIR"
echo "Gen Headers: $GENERATED_HEADERS_DIR"
echo "Kconfig:    $KCONFIG_DIR"
echo "Manifest:  $MANIFEST"
echo "============================================"

# -------------------------------------------------------
# 1. Validate artifacts and manifest
# -------------------------------------------------------
echo "[1] Checking artifacts and manifest..."

# Count .a files in the artifacts directory
A_COUNT=0
if [ -d "$ARTIFACTS_DIR" ]; then
    A_COUNT=$(find "$ARTIFACTS_DIR" -name '*.a' -type f 2>/dev/null | wc -l)
fi
echo "  Found $A_COUNT .a library files in $ARTIFACTS_DIR"

if [ -f "$MANIFEST" ]; then
    echo "  Manifest found: $MANIFEST"
else
    echo "[ERROR] Manifest file not found: $MANIFEST"
    exit 1
fi

# -------------------------------------------------------
# 2. Clone the GitLab SDK repo — use same branch as src; create if not exists
# -------------------------------------------------------
echo "[2] Cloning GitLab SDK repo (branch: $SDK_REPO_BRANCH)..."
rm -rf "$SDK_CLONE_DIR"
git clone --depth 1 --no-single-branch "$SDK_REPO_URL" "$SDK_CLONE_DIR"
cd "$SDK_CLONE_DIR"
# Explicitly fetch the target branch (shallow clone only fetches default branch)
git fetch origin "$SDK_REPO_BRANCH" 2>/dev/null || true
if git rev-parse -q --verify "origin/$SDK_REPO_BRANCH" >/dev/null 2>&1; then
    git checkout -b "$SDK_REPO_BRANCH" "origin/$SDK_REPO_BRANCH"
    echo "  Checked out existing branch '$SDK_REPO_BRANCH'."
else
    git checkout -b "$SDK_REPO_BRANCH"
    echo "  Branch '$SDK_REPO_BRANCH' not found in GitLab SDK repo, creating it..."
fi
cd "$CI_PROJECT_DIR"
echo "  Clone complete."

# -------------------------------------------------------
# 3. Clear existing content (except .git)
# -------------------------------------------------------
echo "[3] Clearing existing content..."
cd "$SDK_CLONE_DIR"
find . -mindepth 1 -not -path './.git/*' -not -name '.git' -not -name '.gitignore' -not -name '.gitattributes' -delete 2>/dev/null || true
cd "$CI_PROJECT_DIR"
echo "  Done."

# -------------------------------------------------------
# 4. Copy files listed in manifest
# -------------------------------------------------------
echo "[4] Copying files from manifest..."

COPY_COUNT=0

# Function to copy a single file
copy_file() {
    local src="$1"
    local rel="$2"
    local dest="$SDK_CLONE_DIR/$rel"
    mkdir -p "$(dirname "$dest")"
    cp "$src" "$dest"
    echo "  + $rel"
    COPY_COUNT=$((COPY_COUNT + 1))
}

while IFS= read -r line || [ -n "$line" ]; do
    # Strip inline comments and trim whitespace
    line="${line%%#*}"
    # Trim leading/trailing whitespace
    line="${line#"${line%%[![:space:]]*}"}"
    line="${line%"${line##*[![:space:]]}"}"
    [[ -z "$line" ]] && continue

    # Strip leading ./ if present
    line="${line#./}"

    if [[ "$line" == *\** ]]; then
        # Glob pattern — find matching files
        while IFS= read -r -d '' match; do
            rel="${match#$CI_PROJECT_DIR/}"
            copy_file "$match" "$rel"
        done < <(find "$CI_PROJECT_DIR" -path "${CI_PROJECT_DIR}/${line}" -type f -print0 2>/dev/null || true)
    else
        # Exact file path
        src="$CI_PROJECT_DIR/$line"
        if [ -f "$src" ]; then
            copy_file "$src" "$line"
        else
            echo "  [WARN] Not found (skipped): $line"
        fi
    fi
done < "$MANIFEST"

echo "  Copied $COPY_COUNT files from manifest."

# -------------------------------------------------------
# 5. Copy .a library files from build artifacts
# -------------------------------------------------------
if [ "$SKIP_LIBS" = false ]; then
    echo "[5] Copying .a library files to libraries/..."

    if [ -d "$ARTIFACTS_DIR" ]; then
        # Copy .a files preserving SOC subdirectory structure
        A_SYNCED=0
        while IFS= read -r -d '' f; do
            rel="${f#$ARTIFACTS_DIR/}"
            dest="$SDK_CLONE_DIR/libraries/$rel"
            mkdir -p "$(dirname "$dest")"
            cp "$f" "$dest"
            echo "  + libraries/$rel"
            A_SYNCED=$((A_SYNCED + 1))
        done < <(find "$ARTIFACTS_DIR" -name '*.a' -type f -print0 2>/dev/null || true)
        echo "  Copied $A_SYNCED .a files."
    else
        echo "  [ERROR] Artifacts directory not found: $ARTIFACTS_DIR (required for sync with libraries)"
        exit 1
    fi
else
    echo "[5] Skipping library copy (--skip-libs mode)"
fi

# -------------------------------------------------------
# 6. Copy generated header files (from YAML files via headers_gen.cmake)
# -------------------------------------------------------
echo "[6] Copying generated header files..."
GENH_SYNCED=0
if [ -d "$GENERATED_HEADERS_DIR" ]; then
    # Copy generated headers preserving directory structure
    while IFS= read -r -d '' f; do
        # Remove the ci_generated_headers/SOC/ prefix
        rel="${f#$GENERATED_HEADERS_DIR/}"
        # Remove SOC directory prefix (e.g., TL321X/)
        rel="${rel#*/}"
        dest="$SDK_CLONE_DIR/$rel"
        mkdir -p "$(dirname "$dest")"
        cp "$f" "$dest"
        echo "  + $rel"
        GENH_SYNCED=$((GENH_SYNCED + 1))
    done < <(find "$GENERATED_HEADERS_DIR" -name '*.h' -type f -print0 2>/dev/null || true)
    echo "  Copied $GENH_SYNCED generated header files."
else
    echo "  [WARN] Generated headers directory not found: $GENERATED_HEADERS_DIR"
fi

# -------------------------------------------------------
# 7. Copy preprocessed Kconfig files (organized by Core)
# -------------------------------------------------------
echo "[7] Copying preprocessed Kconfig files to kconfig/..."
KCONF_SYNCED=0
if [ -d "$KCONFIG_DIR" ]; then
    while IFS= read -r -d '' f; do
        rel="${f#$KCONFIG_DIR/}"
        dest="$SDK_CLONE_DIR/kconfig/$rel"
        mkdir -p "$(dirname "$dest")"
        cp "$f" "$dest"
        echo "  + kconfig/$rel"
        KCONF_SYNCED=$((KCONF_SYNCED + 1))
    done < <(find "$KCONFIG_DIR" -type f -print0 2>/dev/null || true)
    echo "  Copied $KCONF_SYNCED Kconfig files."
else
    echo "  [WARN] Kconfig directory not found: $KCONFIG_DIR"
fi

# -------------------------------------------------------
# 8. Commit and push
# -------------------------------------------------------
cd "$SDK_CLONE_DIR"

# Generate commit message: original src commit message + sync info
GIT_SHA="${CI_COMMIT_SHA:-unknown}"
GIT_REF="${CI_COMMIT_REF_NAME:-unknown}"
GIT_TITLE="${CI_COMMIT_TITLE:-}"
GIT_MESSAGE="${CI_COMMIT_MESSAGE:-}"
CI_PIPELINE_URL="${CI_PIPELINE_URL:-}"

{
    echo "${GIT_TITLE}"
    echo ""
    if [ -n "$GIT_MESSAGE" ] && [ "$GIT_MESSAGE" != "$GIT_TITLE" ]; then
        echo "$GIT_MESSAGE"
        echo ""
    fi
    echo "---"
    echo "UniSDK GitLab Sync"
    echo "Source: $GIT_SHA ($GIT_REF)"
    echo "Files synced: $COPY_COUNT"
    if [ -n "$CI_PIPELINE_URL" ]; then
        echo "Pipeline: $CI_PIPELINE_URL"
    fi
} > "$COMMIT_MSG_FILE"

# Use original commit author from src repo
GIT_AUTHOR="${CI_COMMIT_AUTHOR:-}"
if [ -n "$GIT_AUTHOR" ]; then
    GIT_USER_NAME="${GIT_AUTHOR%% <*}"
    GIT_USER_EMAIL="${GIT_AUTHOR#*<}"
    GIT_USER_EMAIL="${GIT_USER_EMAIL%>}"
elif [ -n "${CI_COMMIT_SHA:-}" ]; then
    GIT_USER_NAME="$(cd "$CI_PROJECT_DIR" && git log -1 --format='%an' 2>/dev/null || echo 'UniSDK CI')"
    GIT_USER_EMAIL="$(cd "$CI_PROJECT_DIR" && git log -1 --format='%ae' 2>/dev/null || echo 'unisdk-ci@telink-semi.com')"
fi
git config user.name "${GIT_USER_NAME:-UniSDK CI}"
git config user.email "${GIT_USER_EMAIL:-unisdk-ci@telink-semi.com}"

# Check if there are any changes
if git diff --quiet && git diff --cached --quiet && [ -z "$(git status --porcelain)" ]; then
    echo "[6] No changes to commit — GitLab SDK repo is up to date."
    echo "============================================"
    exit 0
fi

git add -A
git commit -F "$COMMIT_MSG_FILE"

echo "[6] Pushing to $SDK_REPO_URL ($SDK_REPO_BRANCH)..."
git push origin "$SDK_REPO_BRANCH"

echo "============================================"
echo " Sync complete! Pushed to GitLab SDK repo."
echo "============================================"
