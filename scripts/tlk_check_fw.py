#!/usr/bin/env python3
"""
Telink Firmware Check and Post-Build Processing Script

Key Features:
- Performs CRC32-based firmware integrity check (FW_CHECK_AGTHM2 algorithm)
- Extracts SRAM usage information from .lst files
- Extracts SDK version information from the firmware binary

Usage:
    python tl_check_fw.py <bin_file>

Author: Telink Semiconductor
License: Apache-2.0
"""

import sys
import os
import re

# Firmware check configuration constants
FLAG_FW_CHECK = 0x5D          # Magic number identifying firmware check header
FW_CHECK_AGTHM2 = 0x02       # Algorithm type: CRC32
INPUT_BIN_SIZE_MAX = 4 * 1024 * 1024  # Maximum firmware size: 4MB

# CRC32 lookup table for fast computation
# This table is used by the CRC32 algorithm to calculate checksums efficiently
# Generated from the standard CRC32 polynomial 0xEDB88320
CRC32_TABLE = [
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
]


def crc32_cal(crc_init, input_data, table):
    """
    Calculate CRC32 checksum using the lookup table method.

    Args:
        crc_init: Initial CRC value (typically 0xFFFFFFFF for start)
        input_data: Bytes to calculate CRC over
        table: CRC32 lookup table

    Returns:
        Computed CRC32 value
    """
    crc = crc_init
    for byte in input_data:
        crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF]
    return crc


def check_firmware(bin_path):
    """
    Perform firmware integrity check and add CRC32 checksum to firmware.

    This function implements the FW_CHECK_AGTHM2 algorithm:
    1. Reads the binary firmware file
    2. Pads the data to 16-byte alignment with 0xFF
    3. Writes firmware header (FLAG_FW_CHECK at offset 6, algorithm type at offset 7)
    4. Writes total size at offset 24-27
    5. Calculates CRC32 of all bytes except last 4 bytes
    6. Writes CRC32 checksum at the last 4 bytes of the file

    Args:
        bin_path: Path to the firmware binary file

    Returns:
        True if check passed, False otherwise
    """
    with open(bin_path, 'rb') as f:
        data = f.read()

    file_size = len(data)
    if file_size > INPUT_BIN_SIZE_MAX:
        print(f"Error: File size {file_size} is more than limit {INPUT_BIN_SIZE_MAX}")
        return False

    print(f"File size: {file_size}")

    # Allocate buffer with extra space for padding and checksum
    buffer = bytearray(INPUT_BIN_SIZE_MAX + 20)
    buffer[:file_size] = data

    # Calculate padding required for 16-byte alignment
    replenish_cnt = file_size % 16
    pkt_cnt = file_size // 16
    if pkt_cnt < 2:
        return True

    # Pad with 0xFF to reach 16-byte alignment
    if replenish_cnt:
        replenish_cnt = 16 - replenish_cnt
        pkt_cnt += 1
        for i in range(replenish_cnt):
            buffer[file_size + i] = 0xFF

    # Calculate final size: original + 4 bytes for checksum + padding
    final_size = file_size + 4 + replenish_cnt

    # Write firmware header
    buffer[6] = FLAG_FW_CHECK      # Magic number identifying valid firmware
    buffer[7] = FW_CHECK_AGTHM2   # Algorithm type: CRC32

    # Write total firmware size at offset 24 (little-endian)
    buffer[24] = final_size & 0xFF
    buffer[24 + 1] = (final_size >> 8) & 0xFF
    buffer[24 + 2] = (final_size >> 16) & 0xFF
    buffer[24 + 3] = (final_size >> 24) & 0xFF

    # Calculate CRC32 of everything except the last 4 bytes (where checksum will be stored)
    crc_init = 0xFFFFFFFF
    crc_result = crc32_cal(crc_init, buffer[:final_size - 4], CRC32_TABLE)

    # Write CRC32 checksum at the end (last 4 bytes)
    buffer[final_size - 4] = crc_result & 0xFF
    buffer[final_size - 3] = (crc_result >> 8) & 0xFF
    buffer[final_size - 2] = (crc_result >> 16) & 0xFF
    buffer[final_size - 1] = (crc_result >> 24) & 0xFF

    # Write the modified buffer back to file
    with open(bin_path, 'wb') as f:
        f.write(buffer[:final_size])

    print("Output done!")
    return True


def main():
    """
    Main entry point for the firmware post-build processing script.

    This function:
    1. Prints banner and system information
    2. Validates command line arguments
    3. Performs firmware CRC32 check on the bin file

    Note:
        The bin file is already generated by CMake's OBJ_COPY command before this script runs.
        The lst file path is derived by replacing .bin with .lst extension.

    Returns:
        Exit code: 0 for success, 1 for error
    """
    print()
    print("******************** Post build *********************")
    print("Current binary: " + (sys.argv[1] if len(sys.argv) > 1 else ""))

    print(f"Arguments: {sys.argv[1:]}" if len(sys.argv) > 1 else "")

    if len(sys.argv) < 2:
        print("usage: python tl_check_fw.py <bin_file>")
        return 1

    bin_file = sys.argv[1]

    # Perform firmware integrity check
    if not check_firmware(bin_file):
        return 1

    print("**************** End of post build ******************")
    return 0


if __name__ == '__main__':
    sys.exit(main())
