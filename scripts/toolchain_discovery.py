#!/usr/bin/env python3
"""
Toolchain discovery script for UniSDK.
This script finds the appropriate toolchain path for a given SoC series.
If toolchain is not found, it can optionally download it.

Search order when TELINK_TOOLCHAIN_PATH is set (toolchain_root):
  1. {toolchain_root}/settings.json  — version-to-path mapping
  2. {toolchain_root}/toolchains/<version>/<subdir>  — convention path
  3. VS Code settings  (fallback)

Search order when TELINK_TOOLCHAIN_PATH is NOT set:
  1. ~/.telink/toolchains/<version>/<subdir>  — default download location
  2. VS Code settings  (fallback)

If all searches fail:
  - Non-TTY (CI): error exit
  - TTY (local): interactive download, auto-write to settings.json
"""

import argparse
import os
import sys
import platform
import subprocess
import yaml
import json
import shutil
import tempfile
import zipfile
import tarfile
from pathlib import Path
from urllib.request import urlretrieve

try:
    import py7zr
    _HAS_PY7ZR = True
except ImportError:
    _HAS_PY7ZR = False

SETTINGS_FILENAME = 'settings.json'
CONVENTION_ROOT = 'toolchains'

def load_soc_yaml(soc_yaml_path):
    """Load soc.yaml and return mapping."""
    try:
        with open(soc_yaml_path, 'r', encoding='utf-8') as f:
            data = yaml.safe_load(f)
        return data
    except Exception as e:
        print(f"Error loading {soc_yaml_path}: {e}", file=sys.stderr)
        return None

def get_toolchain_version_for_soc(soc_series, soc_data, toolchain_type='andes'):
    """Return the toolchain version required for the given SoC series."""
    if not soc_data:
        return None
    key = 'toolchain' if toolchain_type == 'andes' else 'toolchain_zephyr'
    series = soc_data.get('soc_series', {})
    if soc_series in series:
        return series[soc_series].get(key)
    return None

def get_toolchain_info(version, soc_data):
    """Return info dict for a toolchain version."""
    versions = soc_data.get('toolchain_versions', {})
    if version in versions:
        info = versions[version].copy()
        info.setdefault('prefix', 'riscv32-elf-')
        info.setdefault('subdir', '')
        return info
    return {'prefix': 'riscv32-elf-', 'subdir': 'bin'}

def validate_toolchain_path(path, subdir='', prefix="riscv32-elf-"):
    """Check that the toolchain path contains required binaries."""
    exe_suffix = ".exe" if platform.system() == "Windows" else ""
    toolchain_dir = os.path.join(path, subdir)
    bin_dir = os.path.join(toolchain_dir, 'bin')
    if not os.path.exists(bin_dir):
        return False
    gcc = os.path.join(bin_dir, f"{prefix}gcc{exe_suffix}")
    return os.path.exists(gcc)


def load_settings(toolchain_root):
    """Load settings.json from toolchain root directory."""
    path = os.path.join(toolchain_root, SETTINGS_FILENAME)
    if os.path.exists(path):
        try:
            with open(path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except Exception:
            return {}
    return {}

def save_settings(toolchain_root, settings):
    """Save settings.json to toolchain root directory."""
    path = os.path.join(toolchain_root, SETTINGS_FILENAME)
    try:
        os.makedirs(toolchain_root, exist_ok=True)
        with open(path, 'w', encoding='utf-8') as f:
            json.dump(settings, f, indent=2)
    except Exception as e:
        print(f"Warning: failed to save {path}: {e}", file=sys.stderr)


def find_toolchain_in_settings(toolchain_root, version, prefix):
    """Look up toolchain path from settings.json."""
    settings = load_settings(toolchain_root)
    entry = settings.get('toolchains', {}).get(version, {})
    path = entry.get('path')
    if path and validate_toolchain_path(path, '', prefix):
        return path, f"settings.json:{path}"
    return None, None

def find_toolchain_in_convention(toolchain_root, version, subdir, prefix):
    """Check convention path: {root}/toolchains/<version>/<subdir>"""
    candidate = os.path.join(toolchain_root, CONVENTION_ROOT, version, subdir)
    if validate_toolchain_path(candidate, '', prefix):
        return candidate, f"convention:{candidate}"
    return None, None

def find_toolchain_default_dir(version, subdir, prefix):
    """Check default download directory: ~/.telink/toolchains/<version>/<subdir>"""
    root = os.path.join(os.path.expanduser("~"), ".telink", CONVENTION_ROOT)
    candidate = os.path.join(root, version, subdir)
    if validate_toolchain_path(candidate, '', prefix):
        return candidate, f"default:{candidate}"
    return None, None

def find_toolchain_vscode(toolchain_version, info):
    """Try to find toolchain path from VSCode settings.

    The Telink VS Code extension (extension ID 'tlk') stores toolchain paths
    under keys like 'tlk.<description>' where <description> matches the
    'description' field in soc.yaml toolchain_versions.
    """
    system = platform.system()
    if system == "Windows":
        appdata = os.getenv('APPDATA')
        if appdata:
            settings_path = os.path.join(appdata, 'Code', 'User', 'settings.json')
            if os.path.exists(settings_path):
                try:
                    with open(settings_path, 'r', encoding='utf-8') as f:
                        settings = json.load(f)
                    description = info.get('description')
                    if description:
                        tlk_key = f"tlk.{description}"
                        path = settings.get(tlk_key)
                        if path and os.path.exists(path):
                            return path, f"vscode:{tlk_key}"
                    install_path = settings.get('tlk.installPath')
                    if install_path:
                        installdir = info.get('installdir', toolchain_version)
                        candidate = os.path.join(install_path, "toolchains", installdir)
                        if os.path.exists(candidate):
                            return candidate, "vscode:tlk.installPath"
                    path = settings.get('telink.toolchain.path')
                    if path and os.path.exists(path):
                        return path, 'vscode:telink.toolchain.path'
                    path = settings.get('telink.toolchainPath')
                    if path and os.path.exists(path):
                        return path, 'vscode:telink.toolchainPath'
                except Exception:
                    pass
    elif system == "Linux" or system == "Darwin":
        home = os.path.expanduser("~")
        settings_path = os.path.join(home, '.config', 'Code', 'User', 'settings.json')
        if os.path.exists(settings_path):
            try:
                with open(settings_path, 'r', encoding='utf-8') as f:
                    settings = json.load(f)
                description = info.get('description')
                if description:
                    tlk_key = f"tlk.{description}"
                    path = settings.get(tlk_key)
                    if path and os.path.exists(path):
                        return path, f"vscode:{tlk_key}"
                install_path = settings.get('tlk.installPath')
                if install_path:
                    installdir = info.get('installdir', toolchain_version)
                    candidate = os.path.join(install_path, "toolchains", installdir)
                    if os.path.exists(candidate):
                        return candidate, "vscode:tlk.installPath"
                path = settings.get('telink.toolchain.path')
                if path and os.path.exists(path):
                    return path, 'vscode:telink.toolchain.path'
                path = settings.get('telink.toolchainPath')
                if path and os.path.exists(path):
                    return path, 'vscode:telink.toolchainPath'
            except Exception:
                pass
    return None, None


def _get_platform_download_url(download_url):
    """Select the download URL for the current platform."""
    system = platform.system()
    machine = platform.machine().lower()

    if system == "Windows":
        return download_url.get("windows")
    elif system == "Linux":
        if machine in ("aarch64",):
            return download_url.get("linux_aarch64") or download_url.get("linux")
        return download_url.get("linux_x86_64") or download_url.get("linux")
    elif system == "Darwin":
        if machine in ("arm64", "aarch64"):
            return download_url.get("mac_aarch64")
        return download_url.get("mac_x64")

    return None


_console_out = None

def _console_msg(msg, end='\n'):
    """Print a message directly to terminal console (bypasses CMake pipe capture).
    Falls back to stderr if console is not available."""
    global _console_out
    if _console_out is None:
        try:
            _console_out = open('CONOUT$', 'w') if platform.system() == 'Windows' else open('/dev/tty', 'w')
        except Exception:
            _console_out = False
    if _console_out:
        print(msg, end=end, file=_console_out, flush=True)
    else:
        print(msg, end=end, file=sys.stderr, flush=True)

def _console_input(prompt):
    """Read input from user."""
    _console_msg(prompt, end='')
    if sys.stdin.isatty():
        try:
            return input()
        except Exception:
            return ''
    try:
        if platform.system() == 'Windows':
            with open('CONIN$', 'r') as con:
                return con.readline().strip()
        else:
            with open('/dev/tty', 'r') as con:
                return con.readline().strip()
    except Exception:
        return ''

def _download_progress(block_count, block_size, total_size):
    """Progress callback for urlretrieve."""
    downloaded = block_count * block_size
    if total_size > 0:
        percent = downloaded * 100 / total_size
        _console_msg(f"\r  {downloaded / 1024 / 1024:.1f} MB / {total_size / 1024 / 1024:.1f} MB ({percent:.0f}%)", end='')
    else:
        _console_msg(f"\r  {downloaded / 1024 / 1024:.1f} MB ...", end='')


def _extract_archive(archive_path, extract_dir):
    """Extract archive to extract_dir, dispatching by format."""
    if archive_path.endswith('.zip'):
        with zipfile.ZipFile(archive_path, 'r') as zipf:
            zipf.extractall(extract_dir)
    elif archive_path.endswith('.tar.gz') or archive_path.endswith('.tgz'):
        with tarfile.open(archive_path, 'r:gz') as tar:
            tar.extractall(extract_dir, filter='data')
    elif archive_path.endswith('.tar.xz'):
        with tarfile.open(archive_path, 'r:xz') as tar:
            tar.extractall(extract_dir, filter='data')
    elif archive_path.endswith('.7z'):
        if not _HAS_PY7ZR:
            raise RuntimeError(
                ".7z extraction requires py7zr. Install it with: pip install py7zr"
            )
        with py7zr.SevenZipFile(archive_path, 'r') as z7z:
            z7z.extractall(extract_dir)
    else:
        raise RuntimeError(f"Unsupported archive format: {archive_path}")


def _move_to_target(src_dir, target_dir):
    """Detect single wrapping directory and move contents to target_dir.

    If src_dir contains exactly one directory entry (a single wrapping
    directory), its contents are moved up to target_dir — effectively
    stripping the platform-specific wrapper.  Otherwise, everything in
    src_dir is moved as-is.
    """
    entries = os.listdir(src_dir)
    if len(entries) == 1 and os.path.isdir(os.path.join(src_dir, entries[0])):
        # Single wrapping directory — strip it
        wrapping = os.path.join(src_dir, entries[0])
        for item in os.listdir(wrapping):
            shutil.move(os.path.join(wrapping, item), target_dir)
    else:
        # No single wrapping directory — move everything as-is
        for item in entries:
            shutil.move(os.path.join(src_dir, item), target_dir)


def download_toolchain(version, info, target_dir=None):
    """Download and extract toolchain for given version.
    Downloads into target_dir (version-level directory) by default.
    Returns the version-level directory (parent of subdir).
    """
    raw_url = info.get('download_url')
    download_url = _get_platform_download_url(raw_url) if raw_url else None
    if not download_url:
        print(f"Error: No download URL configured for toolchain version {version}.", file=sys.stderr)
        return None

    if target_dir is None:
        root = os.path.join(os.path.expanduser("~"), ".telink", CONVENTION_ROOT)
        target_dir = os.path.join(root, version)
    if os.path.exists(target_dir):
        _console_msg(f"Toolchain already exists at {target_dir}")
        return target_dir

    with tempfile.TemporaryDirectory() as tmpdir:
        archive_name = os.path.basename(download_url.split('?')[0])
        archive_path = os.path.join(tmpdir, archive_name)
        _console_msg(f"Downloading {version} ...")
        try:
            urlretrieve(download_url, archive_path, reporthook=_download_progress)
            _console_msg("")
        except Exception as e:
            _console_msg(f"\nDownload failed: {e}")
            return None

        # Extract to a temporary staging directory first, so that the
        # target_dir only sees the final structure (no partial extraction).
        staging = os.path.join(tmpdir, 'staging')
        os.makedirs(staging, exist_ok=True)
        _console_msg(f"Extracting {archive_name} ...")
        try:
            _extract_archive(archive_path, staging)
        except Exception as e:
            _console_msg(f"\nExtraction failed: {e}")
            return None

        # Move extracted content to target_dir, stripping any single
        # platform-specific wrapping directory (e.g. "toolchain_v5_v54x/"
        # or "toolchain_linux-x86_64_riscv64-zephyr-elf/").
        os.makedirs(target_dir, exist_ok=True)
        _console_msg(f"Installing to {target_dir}...")
        try:
            _move_to_target(staging, target_dir)
        except Exception as e:
            _console_msg(f"\nFailed to install toolchain: {e}")
            return None

        return target_dir


def discover_toolchain(soc_series, soc_data, toolchain_root=None, toolchain_type='andes'):
    """Main discovery logic.

    Args:
        soc_series: SoC series name (e.g. TL321X)
        soc_data: Parsed soc.yaml data
        toolchain_root: Optional root directory for toolchain storage.
            When set, searches {root}/settings.json and {root}/toolchains/<version>/<subdir>.
            When not set (None), searches ~/.telink/toolchains/<version>/<subdir>.
        toolchain_type: Toolchain type ('andes' or 'zephyr').

    Returns:
        (path, version, prefix, source) on success, or (None, version, prefix, None) on failure.
    """
    toolchain_version = get_toolchain_version_for_soc(soc_series, soc_data, toolchain_type)
    if not toolchain_version:
        raise ValueError(f"No toolchain version defined for SoC series {soc_series}")

    info = get_toolchain_info(toolchain_version, soc_data)
    prefix = info['prefix']
    subdir = info['subdir']

    # ── Search chain ──────────────────────────────────────────

    # 1. If toolchain_root is set: check settings.json
    if toolchain_root:
        path, source = find_toolchain_in_settings(toolchain_root, toolchain_version, prefix)
        if path:
            _console_msg(f"Found toolchain in settings.json: {path}")
            return path, toolchain_version, prefix, source

    # 2. If toolchain_root is set: check convention path {root}/toolchains/<version>/<subdir>
    if toolchain_root:
        path, source = find_toolchain_in_convention(toolchain_root, toolchain_version, subdir, prefix)
        if path:
            _console_msg(f"Found toolchain at convention path: {path}")
            return path, toolchain_version, prefix, source

    # 3. If toolchain_root is set: try using it directly as the toolchain path
    #    (useful for Zephyr SDK which doesn't follow the Andes convention structure)
    if toolchain_root:
        if validate_toolchain_path(toolchain_root, subdir, prefix):
            _console_msg(f"Found toolchain at toolchain root: {toolchain_root}")
            return toolchain_root, toolchain_version, prefix, "toolchain-root"

    # 4. No toolchain_root: check default dir ~/.telink/toolchains/<version>/<subdir>
    if not toolchain_root:
        path, source = find_toolchain_default_dir(toolchain_version, subdir, prefix)
        if path:
            _console_msg(f"Found toolchain at default directory: {path}")
            return path, toolchain_version, prefix, source

    # 5. Always: VS Code settings as fallback
    vscode_path, vscode_source = find_toolchain_vscode(toolchain_version, info)
    if vscode_path:
        for try_subdir in [subdir, '']:
            if validate_toolchain_path(vscode_path, try_subdir, prefix):
                result_path = os.path.join(vscode_path, try_subdir) if try_subdir else vscode_path
                _console_msg(f"Found toolchain via VS Code: {result_path}")
                return result_path, toolchain_version, prefix, f"vscode:{vscode_source}"

    # ── Not found ──────────────────────────────────────────
    if not sys.stdin.isatty():
        # CI / non-interactive
        print(f"Toolchain for version {toolchain_version} not found.", file=sys.stderr)
        print("Searched in:", file=sys.stderr)
        if toolchain_root:
            print(f"  - {os.path.join(toolchain_root, SETTINGS_FILENAME)}", file=sys.stderr)
            print(f"  - {os.path.join(toolchain_root, CONVENTION_ROOT, toolchain_version, subdir)}", file=sys.stderr)
            print(f"  - {toolchain_root} (direct)", file=sys.stderr)
        else:
            print(f"  - {os.path.join(os.path.expanduser('~'), '.telink', CONVENTION_ROOT, toolchain_version, subdir)}", file=sys.stderr)
        print("  - VS Code settings", file=sys.stderr)
        print(file=sys.stderr)
        print("Please set TELINK_TOOLCHAIN_PATH or install the toolchain to the convention path.", file=sys.stderr)
        return None, toolchain_version, prefix, None

    # ── Interactive download ──────────────────────────────
    _console_msg("")
    _console_msg(f"Toolchain '{toolchain_version}' not found. Would you like to download it?")
    answer = _console_input("  Download now? (y/N): ").lower()
    if answer not in ('y', 'yes'):
        _console_msg("Download cancelled.")
        return None, toolchain_version, prefix, None

    if toolchain_root:
        default_target = os.path.join(toolchain_root, CONVENTION_ROOT, toolchain_version)
    else:
        default_target = os.path.join(os.path.expanduser("~"), ".telink", CONVENTION_ROOT, toolchain_version)

    path_answer = _console_input(f"  Use default path '{default_target}'? (Y/n): ").lower()
    if path_answer in ('n', 'no'):
        custom_dir = _console_input("  Enter download path: ")
        target_dir = os.path.abspath(custom_dir) if custom_dir else default_target
    else:
        target_dir = default_target

    _console_msg(f"Downloading to {target_dir} ...")
    downloaded = download_toolchain(toolchain_version, info, target_dir=target_dir)
    if not downloaded:
        _console_msg("Download failed.")
        return None, toolchain_version, prefix, None

    final_path = os.path.join(downloaded, subdir)
    if not validate_toolchain_path(final_path, '', prefix):
        _console_msg("Download completed but toolchain validation failed.")
        return None, toolchain_version, prefix, None

    _console_msg("Download complete!")

    # Auto-write to settings.json if toolchain_root is set
    if toolchain_root:
        try:
            settings = load_settings(toolchain_root)
            if 'toolchains' not in settings:
                settings['toolchains'] = {}
            settings['toolchains'][toolchain_version] = {
                'path': final_path,
                'version': toolchain_version,
            }
            save_settings(toolchain_root, settings)
            _console_msg(f"  Written to {os.path.join(toolchain_root, SETTINGS_FILENAME)}")
        except Exception as e:
            _console_msg(f"  Warning: could not write settings.json: {e}")

    return final_path, toolchain_version, prefix, "downloaded"


def main():
    parser = argparse.ArgumentParser(description="Discover toolchain path for given SoC series.")
    parser.add_argument("--soc-series", required=True, help="SoC series (e.g., TL321X)")
    parser.add_argument("--soc-yaml", default=None, help="Path to soc.yaml (default: <script_dir>/../soc/soc.yaml)")
    parser.add_argument("--toolchain-root", default=None,
                        help="Toolchain root directory (default: value of TELINK_TOOLCHAIN_PATH env var)")
    parser.add_argument("--toolchain-type", default="andes", choices=["andes", "zephyr"],
                        help="Toolchain type (default: andes)")
    parser.add_argument("--output", choices=["path", "json"], default="path",
                        help="Output format: path (just the path) or json (structured)")
    args = parser.parse_args()

    # Determine toolchain_root: CLI arg > env var
    toolchain_root = args.toolchain_root or os.environ.get('TELINK_TOOLCHAIN_PATH')

    # Determine soc.yaml path
    if args.soc_yaml is None:
        script_dir = Path(__file__).parent
        default_yaml = script_dir.parent / "soc" / "soc.yaml"
        soc_yaml_path = str(default_yaml)
    else:
        soc_yaml_path = args.soc_yaml

    if not os.path.exists(soc_yaml_path):
        print(f"Error: soc.yaml not found at {soc_yaml_path}", file=sys.stderr)
        sys.exit(1)

    soc_data = load_soc_yaml(soc_yaml_path)
    if not soc_data:
        sys.exit(1)

    try:
        path, version, prefix, found_by = discover_toolchain(
            args.soc_series, soc_data, toolchain_root=toolchain_root,
            toolchain_type=args.toolchain_type
        )
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    if path:
        if args.output == "json":
            info = get_toolchain_info(version, soc_data)
            result = {
                "toolchain_path": path,
                "toolchain_version": version,
                "prefix": prefix,
                "found_by": found_by,
                "extra_flags": info.get("extra_flags", [])
            }
            print(json.dumps(result, indent=2))
        else:
            print(path)
        sys.exit(0)
    else:
        if args.output == "json":
            result = {
                "toolchain_path": None,
                "toolchain_version": version,
                "error": f"Toolchain for version {version} not found."
            }
            print(json.dumps(result, indent=2))
        else:
            print(f"Toolchain for version {version} not found.", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()