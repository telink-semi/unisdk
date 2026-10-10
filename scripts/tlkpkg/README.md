# tlkpkg

Package tooling for UniSDK: export (pack) MCU cores, modules and the SDK
itself as `.tlkpkg` archives, publish them to the registry server
(`tl_unisdk_registry_server`), and install them into an SDK or an
application project.

A `.tlkpkg` is a plain zip file with a `.tlkpkg` extension. Everything is
driven through one CLI, `registry_client.py`.

## Folder layout

Each kind folder holds only what is specific to that kind. `common/` holds
only the code those kind files use - shared helpers and the data formats they
read and write. Code used only by the CLI (the registry server client,
install planning, version bumps) stays at the top level next to it.

```
scripts/tlkpkg/
  registry_client.py     the CLI: search / install-* / export-* / publish-* / version / tokens
  server_api.py          registry server access: token, catalog, download, publish (CLI only)
  resolver.py            install planning: which versions to install, dependencies first (CLI only)
  release.py             version bumps, release commits and tags (CLI only)
  mcu/                   mcu packages
    mcu_manifest.py        manifest.yaml schema (McuManifest)
    mcu_discovery.py       cores / SoC series / boards found under packages/
    mcu_export.py          pack packages/<CORE> into <CORE>.tlkpkg
    mcu_install.py         install a <CORE>.tlkpkg into an exported SDK (from a file, the registry or git)
    mcu_verify.py          check installed files against the packed hashes
  modules/               module packages (ported third-party libraries)
    module_manifest.py     manifest.yaml schema (ModuleManifest)
    module_export.py       pack modules/<dir> into <name>.tlkpkg
    module_install.py      install a <name>.tlkpkg into an application project (from a file, the registry or git)
    module_verify.py       check installed files and git dependencies
  sdk/                   sdk packages
    sdk_export.py          export this checkout as a clean sdk package (SdkManifest)
    sdk_install.py         extract an sdk .tlkpkg into a new directory - standard library only
  common/                used by the kind files (mcu/, modules/, sdk/)
    export.py              selection checks, stage + zip an archive
    install.py             download, extract, copy, compatibility checks, git catalog entries
    integrity.py           sha256 hashing and diffs
    manifest.py            manifest.yaml plumbing
    component.py           tlkpkg.yaml - a component's source manifest
    versioning.py          semver / npm-style ranges
    installed.py           what's installed (records + source checkout), for compatibility checks
    registry.py            install records (packages/<kind>/<name>.yaml)
    project_deps.py        a project's tlkpkg-dependencies.yaml
    dependency.py          git dependency cloning
  sdk_version.py         the SDK's own version (also used by the firmware build)
  list_installed.py, new_project.py, add_makefile.py, list_samples.py,
  kconfig_web.py, pinmux_web.py, toolchain_manager.py   VS Code extension backends
```

## Package layouts

### mcu

A package is scoped to one **core**, not one SoC series: if a core backs
more than one series (B92 backs both TLSR922X and TLSR952X), they ship
together in a single `<CORE>.tlkpkg`, each with their own boards.

The archive has the layout of `packages/<CORE>/` in the source checkout and
installs there as is (all but `manifest.yaml`):

```
<CORE>.tlkpkg
  manifest.yaml
  Kconfig.core
  drivers/
  properties/         tlk_*.h generated from the yaml
  registers/          tlk_*.h generated from the yaml
  soc/<SERIES>/       one per series, its properties/ and registers/ likewise
  boards/<BOARD>/     one per board of those series
  kconfig/<SERIES>/   parsed Kconfig without a board, plus <BOARD>/ per board
  libraries/          libtelink-unisdk-<SERIES>[-zephyr].a per toolchain
```

Cores, series and boards are discovered from each package's Kconfig files
(`mcu/mcu_discovery.py`): `packages/<CORE>/Kconfig.core`,
`soc/<SERIES>/Kconfig.soc` (which declares the `TLK_SOC_<variant>`s) and
`boards/<BOARD>/Kconfig.board` (whose `depends on TLK_SOC_*` line says
which SoCs it supports). `packages/Kconfig.mcu` globs the same
`*/Kconfig.core` files, so the Kconfig menus and the tooling can't disagree.

What an mcu package leaves out:

- **yaml the build converts to headers** - `properties/` and `registers/` of
  the core and of each series, and `properties/` of each board, ship the
  generated `tlk_*.h` *instead of* the yaml. The yaml's other use, the
  `${uart.count}` / `for i in range ...` macro expansion in
  `Kconfig.build`'s per-chip menus, is done at export time into
  `kconfig/<SERIES>/` and `kconfig/<SERIES>/<BOARD>/`; cmake's SDK mode reads
  that instead. `pinmux/*.yaml` (SoC and
  board) is kept - the pinmux tool reads it on every build.
- **private `_*.c` sources** (e.g. `drivers/_tlk_rf.c`) and
  **`soc/<SERIES>/crypto/src/`** - compiled only in dev mode, into
  `libtelink-unisdk-<SERIES>[-zephyr].a`, so they ship only inside that
  library.
- **`ci_configs/`, `tlkpkg.yaml`, `.git`** - not SDK content.

`manifest.yaml`:

```yaml
package:
  name: B92
  version: 0.1.0                 # from packages/B92/tlkpkg.yaml
  vendor: Telink
  description: B92 core (TLSR922X, TLSR952X)
  sdk: ^0.3.0                    # compatible SDK versions (npm-style range)
core:
  name: B92
soc_series:
  - name: TLSR952X
    socs:                        # from soc/<SERIES>/Kconfig.soc (informational)
      - name: TLSR9528A
        case: QFN88
    boards:
      - name: TLSR9528A_EVK
        socs: [TLSR9528A]        # from boards/<BOARD>/Kconfig.board
integrity: {...}                 # sha256 per installed file, checked by mcu_verify.py
dependencies:                    # only when tlkpkg.yaml declares any
  some_module: "^1.0.0"
```

### module

The archive is `modules/<dir>/` itself (samples in it included) and installs
there as is (all but `manifest.yaml`):

```
<name>.tlkpkg
  manifest.yaml          name, version, SDK range, dependencies (from tlkpkg.yaml), integrity
  CMakeLists.txt, Kconfig, ...   the port; no tlkpkg.yaml, .git or git dependency checkouts (repo/)
```

### sdk

Like mcu and module packages, the archive is the SDK as it installs, plus
`manifest.yaml` (install copies everything else into `--target-root`):

```
<name>.tlkpkg
  manifest.yaml          name, version, integrity
  version.yaml           type: SDK
  CMakeLists.txt, Kconfig.build, cmake/, core/, scripts/, ...
```

Archives packed before versioning have no `package.sdk` ("compatibility
unknown") and no `dependencies:`; a legacy module archive's
`module.upstream: {url, revision}` is still read as its single git
dependency.

## Versioning

Versions are strict semver, ranges npm-style (`versioning.py`, on top of
`semantic_version`'s `NpmSpec` - listed in the SDK's `requirements.txt`):

- `MAJOR.MINOR.PATCH[-prerelease][+build]`; ordering is semver
  precedence, and **build metadata is ignored** for ordering and range
  matching (an SDK dev checkout at `0.3.0+5.gabc1234` counts as `0.3.0`).
- Ranges: `^`, `~`, `>=`/`<`..., x-ranges, hyphen ranges, `||`. **`^0.x`
  is strict, like npm**: `^0.3.0` means `>=0.3.0 <0.4.0`.
- A prerelease only satisfies a range naming a prerelease of the same
  `MAJOR.MINOR.PATCH` (`0.4.0-rc.1` does not satisfy `>=0.3.0`).
- A package with no SDK range, or an SDK whose version is unknown, is
  **"compatibility unknown"**: installable with a warning, never treated
  as compatible.

### `tlkpkg.yaml` - a component's source manifest

Every packaged component commits one next to its sources - the
`package.json` equivalent: `packages/<CORE>/tlkpkg.yaml` for an mcu
package, `modules/<module>/tlkpkg.yaml` for a module (see
`component.py`). The exporters copy its fields into the archive's
`manifest.yaml`; exporting a component without one is an error.

```yaml
name: tinyusb                    # registry package name
version: 0.1.0                   # strict semver - required
description: Telink port of tinyUSB
sdk: "^0.3.0"                    # required - npm-style range of compatible SDK versions
dependencies:
  B92: "^0.1.0"                  # a string: another registry package, as a range
  upstream:                      # a mapping with `git`: a pinned git checkout
    git: https://github.com/hathach/tinyusb.git
    rev: 3af1bec1a9161ee8dec29487831f7ac7ade9e189
    path: repo                   # optional, relative to the installed component (default: repo)
package_repo: https://github.com/telink-semi/unisdk_module_package_tinyusb   # optional - export-<kind> --git
```

The SDK's own package repo goes in a `tlkpkg.yaml` at the SRC root, which
holds only that (its version stays in `version.yaml`), and isn't exported:

```yaml
name: unisdk
package_repo: https://github.com/telink-semi/unisdk   # export-sdk --git
```

Git dependencies are only supported for modules (`export-mcu` refuses
them); several git dependencies each need a distinct `path`. The only
tools that rewrite this file are `registry_client.py version` and
`publish-* --bump` (see "Publishing"), which change just the `version:`
line.

### The SDK's own version

`sdk_version.py` is the one place deciding "which UniSDK version is this
checkout". The SDK version lives in the **committed** `<sdk_root>/version.yaml`:

```yaml
version: 0.4.0   # strict semver
type: SRC        # SRC - the source repo (publishes packages); SDK - an export (installs them)
```

`version:` is only rewritten by `registry_client.py version --sdk` /
`publish-sdk --bump`; anything but strict semver is an error. `type:` tells
tools (the VS Code extension) whether the checkout publishes or installs
packages.

1. In a source checkout that is its own git root, git adds build metadata
   relative to the release tag `v<version>`: exactly on the tag and clean
   -> `0.4.0`; 5 commits past it -> `0.4.0+5.gabc1234[.dirty]`; tag not
   created (yet) -> `0.4.0+gabc1234[.dirty]`. Build metadata never affects
   compatibility.
2. Anywhere else (an export, or an SDK vendored inside an application's
   repo) the file is taken as-is.
3. No `version.yaml` (or no `version:` in it): unknown - shown as
   `0.0.0+unknown`.

`export-sdk` writes rule 1's result (and `type: SDK`) for HEAD into the
export's `version.yaml`, so an export still identifies the exact commit it
came from. `python scripts/tlkpkg/sdk_version.py [--sdk-root DIR] [--json]`
prints `{semver, display, source}`.

## Commands

### Export

```bash
python scripts/tlkpkg/registry_client.py export-mcu                                      # every core -> export/mcu/
python scripts/tlkpkg/registry_client.py export-mcu --core B92 --skip TLSR922X --toolchain andes
python scripts/tlkpkg/registry_client.py export-module --module tinyUSB                   # -> export/module/
python scripts/tlkpkg/registry_client.py export-sdk                                      # -> export/sdk/unisdk.tlkpkg
python scripts/tlkpkg/registry_client.py export-sdk --path ../dist --name unisdk --force
python scripts/tlkpkg/registry_client.py export-all                                      # all three -> export/<kind>/
python scripts/tlkpkg/registry_client.py export-all --path ../dist --force
```

`export-all` (`--path`, default `export/`, and `--force`) runs `export-sdk`,
`export-module` and `export-mcu` with their own defaults (every module,
every core, both toolchains, package name `unisdk`), each into
`<--path>/<kind>/`. Each exporter's docstring (`mcu/mcu_export.py`,
`modules/module_export.py`, `sdk/sdk_export.py`) lists its steps.

Each `export-<kind>` writes into `export/<kind>/` of this checkout unless
`--path` says otherwise (`export/` is git-ignored). When an archive is
already there, it asks whether to skip it or overwrite it (`[S]kip` /
`[o]verwrite`, per archive); `--force` overwrites without asking, and
without a terminal to ask in it's an error.

`export-mcu` flags:

| Flag | Meaning |
|---|---|
| `--core CORE...` | Cores to pack (default: every core under `packages/`) |
| `--soc-series SERIES...` | Only these SoC series (default: every series of the selected cores) |
| `--skip SERIES...` | SoC series to leave out - wins over `--soc-series` |
| `--toolchain andes\|zephyr...` | Prebuilt library variants (default: both) |
| `--path DIR` | Where the `<CORE>.tlkpkg` files go (default: `export/mcu/`) |
| `--force` | Overwrite an archive already exported there, without asking |

`export-mcu` packs **directly from this source checkout** (`packages/<CORE>`
populated as git submodules). For each SoC series and toolchain (Andes and
Zephyr by default; `--toolchain andes` builds only the named ones) it runs

```bash
scripts/tlkpkg/mcu/mcu_build.sh -s <SERIES> -t andes|zephyr
```

which uses the SDK's own CMake build: `make cmake SOC_SERIES=<SERIES>` (no
board), then `make reconfigure BOARD=<BOARD>` for every board of the series,
and each time parses Kconfig (`parse_kconfig`), writes `build.config` from
`mcu/mcu_build.defconfig`, generates the headers (`generate_headers`) and
builds the library (`build_soc_libraries`). That leaves in `build/`:

- `libraries/libtelink-unisdk-<SERIES>[-zephyr].a`;
- `generated/headers/packages/<CORE>/...` - the `tlk_*.h` of the core's, the
  series' and every board's properties/registers yaml, laid out like
  `packages/<CORE>/`;
- `generated/kconfig/<SERIES>/` (no board) and
  `generated/kconfig/<SERIES>/<BOARD>/` per board.

The exporter copies the sources without the converted yaml and lays that
output over them as is - `generated/headers/packages/<CORE>/` has the
package's layout. A Zephyr build uses `TELINK_TOOLCHAIN_PATH_ZEPHYR`
when it is set.

`export-sdk` (`sdk/sdk_export.py`, its docstring lists the steps) packs
HEAD as a clean SDK, `<--path>/<--name>.tlkpkg` (`export/sdk/unisdk.tlkpkg`
by default), at the version in this checkout's `version.yaml`: tracked files
only, no mcu or module package (they install separately), no Kconfig files
the build parses (an SDK uses the parsed copies its MCU package ships), none
of what only an SRC uses (`SRC_ONLY_PATHS`: `.gitlab-ci.yml`, `.gitmodules`,
`.pre-commit-config.yaml`, `docs/`, `tests/`, `scripts/ci/`, `release.py`,
`git_export.py`, `mcu_build.sh`, `mcu_discovery.py`, `mcu_export.py`,
`sdk_export.py` - so in an SDK `export-*`, `publish-*` and `version` say
they need an SRC), a `west.yml` without the projects only reachable inside
the company (`INTERNAL_WEST_PROJECTS`, e.g. `ble_host`; a project still
pointing into the private network stops the export) and
`version.yaml` with `type: SDK`. That type is what marks an SDK, both for
cmake (link the prebuilt libraries instead of building them) and for the
installers. Flags: `--path`, `--name` (default `unisdk`), `--force`.

Selection, versions and publishing are described in "Publishing" below.

### Install

```bash
python scripts/tlkpkg/registry_client.py install-mcu B92 --target-root ../export/unisdk
python scripts/tlkpkg/registry_client.py install-mcu --file export/mcu/B92.tlkpkg --target-root ../export/unisdk --dry-run
python scripts/tlkpkg/registry_client.py install-module --file export/module/tinyusb.tlkpkg --target-root ../my_app
python scripts/tlkpkg/registry_client.py install-sdk --file export/sdk/unisdk.tlkpkg --target-root ../unisdk
```

`install-<kind> <name>` installs from the registry (see "Installing from a
registry"); `--file` installs a local archive instead, without dependency
resolution.

**mcu** (`mcu/mcu_install.py`) - the archive is copied into
`packages/<CORE>/` as is (minus `manifest.yaml`). cmake's SDK mode uses its
`kconfig/<SERIES>/[<BOARD>/]` and links its `libraries/*.a`.
Nothing needs regenerating: `packages/Kconfig.mcu` picks up the package's
`Kconfig.core`, so the new SoC series/boards show up in the menus at once.

An mcu package only installs into an **exported** SDK (`packages/Kconfig.mcu`
and `version.yaml` `type: SDK`). In a source checkout
`packages/<CORE>` is a git submodule, and copying a package on top would
shadow its tracked files, so that is refused.

**module** (`modules/module_install.py`) - only into an application project, never
into the SDK itself - see "Modules are different" below.

**sdk** (`sdk/sdk_install.py`) - extracts the archive's `sdk/` into a new or
empty `--target-root` (`--repair` wipes a non-empty one first), becoming a
new SDK installation (what the VS Code extension's **Add SDK
Installation**/**Select Active SDK** track). Standard library only, so it
also runs from a bare system Python:
`python scripts/tlkpkg/sdk/sdk_install.py <pkg>.tlkpkg --target-root <dir>`.

For mcu and module installs:

- **Idempotent.** File copies overwrite (reinstalling the same package is a
  no-op diff-wise); `--repair` wipes each installed directory first, to
  clear files a verify flagged as modified.
- `--dry-run` reports every change without touching disk.
- **Compatibility and dependencies are checked first**
  (`common.install.check_versioning()`): refused when the package's
  `package.sdk` range doesn't match the SDK's version, when one of its
  registry `dependencies` isn't installed at a satisfying version, or when
  its version would break an installed package's range on it.
  "Compatibility unknown" only warns; `--force` installs anyway.

The archive is extracted to a throwaway temp directory and discarded once
installed.

### Registry (bookkeeping)

Every non-dry-run mcu/module install writes one record to
`packages/<kind>/<name>.yaml` (`registry.py`) - the manifest (including
`package.sdk`, `dependencies` and `integrity`) plus:

```yaml
kind: mcu
source: https://host:8765/api/packages/B92/0.1.0/download   # what was installed from
installed_at: '2026-08-17T00:02:00Z'
installed_paths:                       # where its files landed, relative to target-root
  - packages/B92
  - packages/B92/soc/TLSR922X
  - packages/B92/boards/TLSR9228A_EVK
  - libraries/libtelink-unisdk-TLSR922X.a
  - kconfig/TLSR922X
```

These records - plus, in an SDK *source* checkout, the committed
`tlkpkg.yaml` of each `packages/<CORE>` submodule and in-tree
`modules/<m>` - are what `common.installed.installed_packages()` reads as
"installed". `list_installed.py` reports them (with `mcu_verify.py` /
`module_verify.py` results) for the VS Code extension. There's no
`uninstall` command yet.

## Publishing

`registry_client.py` also packs and publishes packages to a running
registry server (`tl_unisdk_registry_server/server.py`), rather than just
installing from one - see that server's own
[README](../../tl_unisdk_registry_server/README.md) for the auth model
and endpoint list.

There are three package kinds - `mcu`, `module` (a ported third-party
library) and `sdk` - and one command per kind for each action:

| Command | What it does |
|---|---|
| `search` | List the catalog, each package's SDK range and whether it's compatible with `--sdk-root` (default: this SDK) |
| `install-mcu` / `install-module <name>` | Plan and install, dependencies included (`--version`, `--target-root`, `--sdk-root`, `--dry-run`, `--repair`, `--yes`, `--plan-json`, `--force`) - see "Installing from a registry"; `--file <archive>` installs a local archive instead |
| `install-sdk <name>` | Download an sdk package into a new directory (`--version`, `--target-root`, `--dry-run`, `--repair`, `--file`) - no compatibility/dependency resolution |
| `version <level> --mcu CORE / --module MODULE / --sdk` | Bump a component's `tlkpkg.yaml` or the SDK's `version.yaml`, commit and tag it |
| `export-mcu` / `export-module` | Pack into `--path` (default `export/mcu/`, `export/module/`), one `<name>.tlkpkg` per core/module |
| `export-mcu --git [URL]` / `export-module --git [URL]` / `export-sdk --git [URL]` | Instead of an archive, commit each package - its SDK form, never the SRC - into its package repo (`package_repo` in the `tlkpkg.yaml` - the SDK's at the SRC root - or `URL`, for one `--core`/`--module`), on top of the repo's default branch, tag it `v<version>` and push - see `common/git_export.py`. Refuses uncommitted, untracked or ignored files in the component (for the SDK, built from HEAD: only uncommitted changes), a target that is its own SRC remote, partial cores (`--soc-series`/`--skip`/`--toolchain`) and an existing tag (`--force` moves it); `--dry-run` only clones and compares |
| `export-all` | `export-sdk`, `export-module` and `export-mcu` with their defaults, into `<--path>/<kind>/` (default `export/`; `--force`) |
| `export-sdk` | Pack this source checkout as a clean SDK package, `<--path>/<--name>.tlkpkg` (default `export/sdk/unisdk.tlkpkg`), see `sdk/sdk_export.py` |
| `publish-mcu` / `publish-module` / `publish-sdk` | Export (the same selection flags as `export-*`; `--name` for the sdk) or take `--path <file>.tlkpkg`, publish, and tag the release in this repo (`--bump`, `--no-git-tag`, `--force`) |

`export-mcu`/`publish-mcu` cover **every** core under `packages/` by
default - one `<CORE>.tlkpkg` per core, grouping every SoC series it backs.
`--core` and/or `--soc-series` narrow that down, `--skip` leaves SoC series
out (even ones given in `--soc-series`). Likewise `export-module`/`publish-module` cover every module
under `modules/` that has a `tlkpkg.yaml`; `--module` narrows that down, `--skip`
leaves modules out (even ones given in `--module`). Everything else (name,
version, SDK range, git and registry dependencies) comes from
`modules/<module>/tlkpkg.yaml`. An unknown name in any of these flags is an
error, not silently ignored.
Prebuilt libraries are built for both toolchains (`andes`, `zephyr`) by
default; `--toolchain andes` builds only the named ones.

**Versions** come from each component's `tlkpkg.yaml` (the SDK's
`version.yaml`) - there is no auto-increment.

`publish-<kind>` flags:

| Flag | Meaning |
|---|---|
| `--core` `--soc-series` `--skip` `--toolchain` (mcu), `--module` `--skip` (module), `--name` (sdk) | What to export, as in `export-<kind>` |
| `--path FILE` | Publish this built package (e.g. `export/mcu/B92.tlkpkg`) instead of exporting first - not a folder to search |
| `--bump LEVEL` | Bump the version first: `major`, `minor`, `patch` or `X.Y.Z` (default: publish the version in the yaml; no prereleases for now) |
| `--no-git-tag` | Don't tag the release (and don't commit a `--bump`) |
| `--force` | Overwrite a version that's already on the server (and move an existing tag) |

Without `--path` the package is exported into a temporary directory first,
exactly as `export-<kind>` would. Before anything is changed, the version
must be valid semver, not already published and not older than the
registry's newest (unless `--force`). Then:

1. `--bump`: rewrite the yaml's `version:` and commit it
   (`chore(release): <name> v<version>`).
2. Export and publish.
3. Tag HEAD with the release tag (below) - publishing always tags, bumped or
   not. That needs a clean tree under the component's directory and a tag
   that doesn't exist on another commit yet (`--force` moves it).

`--no-git-tag` skips the commit and the tag (and so the clean-tree check):
a `--bump` only edits the file. With `--path`, the tag is made only when the
package's version matches the yaml's.

`version <major|minor|patch|X.Y.Z> --mcu CORE --module MODULE --sdk` does
the bump on its own: rewrite, commit and tag (`--no-git-tag`: rewrite only).
Push commits and tags yourself.

Whether a package is restricted isn't set from the CLI: tick "Restricted"
next to it on the server's `/admin` page (Packages tab).

Tag names (`release.py`): `v<version>` for the SDK and for a component
that is its own git repo (an mcu core submodule, or a module in its own
repo); `<name>@<version>` for a component inside another repo (a module
still in the SDK tree) - so a component tag can never be mistaken for an
SDK release.

Every version is checked (and every bump computed) before anything is
written, packed or published, so a batch `publish-mcu` never stops halfway
on a version error. The server enforces the same rules itself - HTTP 400
for a non-semver version or a missing/invalid `package.sdk`, 409 for a
version that isn't newer unless the publish passes `?force=true` - so they
also hold for the VS Code extension's **Publish Package...**, which offers
**Overwrite**/**Publish Anyway** on a 409.

```bash
# store the access token issued by the server admin (its /admin page) -
# required to publish at all, and to download/reinstall a restricted package
python scripts/tlkpkg/registry_client.py add-token <token>
python scripts/tlkpkg/registry_client.py remove-token <token>   # must match what's stored

# pack only (no publish)
python scripts/tlkpkg/registry_client.py export-mcu --skip TL321X
python scripts/tlkpkg/registry_client.py export-mcu --core B92 --skip TLSR922X --toolchain andes   # only the Andes prebuilt library
python scripts/tlkpkg/registry_client.py export-module --skip mbedtls

# export the SDK itself to a folder on this machine (+ optionally pack it)
python scripts/tlkpkg/registry_client.py export-sdk --path /path/to/dist
python scripts/tlkpkg/registry_client.py export-sdk --force

# release: bump + commit + tag, like `npm version`
python scripts/tlkpkg/registry_client.py version minor --mcu B92
python scripts/tlkpkg/registry_client.py version 1.0.0 --module tinyUSB --no-git-tag
python scripts/tlkpkg/registry_client.py version minor --sdk          # bumps version.yaml, commits, tags v<next>

# export + publish + tag - at each yaml's version
python scripts/tlkpkg/registry_client.py --server https://registry.example.com:8765 \
    publish-mcu --skip TL321X TLSR922X
python scripts/tlkpkg/registry_client.py --server https://registry.example.com:8765 \
    publish-module --module tinyUSB --bump patch                 # bump + commit, export, publish, tag
python scripts/tlkpkg/registry_client.py --server https://registry.example.com:8765 \
    publish-sdk --bump minor
python scripts/tlkpkg/registry_client.py --server https://registry.example.com:8765 \
    publish-mcu --path export/mcu/B92.tlkpkg                     # an already exported package

# overwrite an existing version (and move its tag)
python scripts/tlkpkg/registry_client.py --server https://registry.example.com:8765 \
    publish-mcu --core B92 --force
```

### Installing from a registry

**Without a registry server.** `search`, `install-mcu`, `install-module` and
`install-sdk` first check that `--server` answers. If it doesn't (none
deployed, or it's down), they read the packages straight from GitHub
instead (`git_registry.py`): the repos of `--git-org` (default
`telink-semi`, env `TLKPKG_GIT_ORG`) named `unisdk` or `unisdk_<...>`,
every `v<semver>` tag whose `manifest.yaml` is a package of that version
(other repos and tags aren't packages and are skipped), installed from git
like a version the server keeps in a package repo. Manifests are cached by
commit in `~/.tlkpkg/git_index_cache.json`. Needs `git` and access to the
repos; the GitHub API allows 60 unauthenticated requests an hour - set
`GITHUB_TOKEN` to lift that. Nothing is restricted without a server, and
publishing still needs one (or goes to git, see the VS Code extension).

`install-mcu`/`install-module` resolve the whole install before touching
anything (`resolver.py`) - like npm, but **flat**, closer to npm's
`peerDependencies` than nested `node_modules`: an SDK (plus the project a
module goes into) holds exactly one copy of each package, so every range
naming a package must be satisfied by that one version, and a conflict
fails the plan instead of installing duplicates.

- The SDK checked against is `--sdk-root` for modules (default: the SDK
  these scripts live in), and `--target-root` itself for mcu packages
  (`--sdk-root`, if given, must equal it). The catalog is fetched with
  `?sdk=<semver>`.
- The requested package gets `--version` if given, else its newest
  version satisfying every range on it (from packages being planned and
  from installed packages depending on it), preferring SDK-compatible
  versions, then "compatibility unknown" ones (with a warning).
  Incompatible versions are refused unless `--force`. Prereleases are only
  picked when requested explicitly or named by a range.
- Each registry dependency not already satisfied by what's installed is
  planned the same way - mcu packages into the SDK, modules into
  `--target-root`. An mcu dependency can't be installed into an SDK
  *source* checkout (those come from git submodules), and anything that
  is part of the source checkout is never installed over.
- Steps run dependencies first; each is an `install`, `upgrade`,
  `downgrade`, `reinstall` (relative to what's already at that target) or
  `keep`.

The plan is printed first, and **confirmed** (`Proceed? [y/N]`) when it
changes more than one package, upgrades/downgrades anything, or carries
warnings - `--yes` skips the prompt, and without a terminal the command
refuses instead of guessing. `--plan-json` prints the plan without
installing - `{"ok": true, "sdk": {semver, display, source, root}, "root":
{name, version}, "steps": [{name, kind, version, action,
installed_version, target, required_by, compatible, reason}], "warnings":
[...]}`, or `{"ok": false, "error": ...}` with exit code 1 - which is what
the VS Code extension runs before asking for confirmation and re-running
with `--yes`. `TLKPKG_TOKEN` in the environment overrides the stored
token.

```bash
python scripts/tlkpkg/registry_client.py search --kind module
python scripts/tlkpkg/registry_client.py install-module tinyusb --target-root ../my_app --plan-json
python scripts/tlkpkg/registry_client.py install-module tinyusb --target-root ../my_app --yes
python scripts/tlkpkg/registry_client.py install-mcu B92 --version 0.1.0 --target-root /path/to/export/unisdk
```

The token is stored in plain text at `~/.tlkpkg/credentials.json`
(`TLKPKG_CONFIG_DIR` overrides the `~/.tlkpkg` directory, e.g. for CI). A
server using its default self-signed TLS certificate needs `--insecure`
on every `registry_client.py` call (skips certificate verification, not
auth) unless you've pointed it at a real CA-issued cert.

## Modules are different

Modules - ported third-party libraries (`modules/tinyUSB`, `modules/mbedtls`) are
packaged too, but only the **port**: the Telink-authored glue layer
(`CMakeLists.txt`, config headers, wrapper `.c` files) under
`modules/<name>`, excluding its git dependency checkouts (e.g.
`modules/<name>/repo`) - the actual third-party source. Those are
declared in `modules/<name>/tlkpkg.yaml`'s `dependencies:` as
`{git, rev, path}` entries and recorded in the manifest rather than
bundled (no west workspace involved). Samples a module keeps in its own
directory ship with it like any other file.

- **Export** (`modules/module_export.py`): `registry_client.py export-module --module tinyUSB`
- **Install** (`modules/module_install.py`): `registry_client.py install-module tinyusb --target-root <app>`
  (or `--file <pkg>.tlkpkg`)
  - the target must be an application project: an SDK (`--sdk-root`, or any directory with a
    `version.yaml`) is refused - the SDK's own `modules/` come from its source checkout only
  - copies the port into `modules/<module_dir>/`
  - registers nothing: the SDK discovers every directory under `modules/` (the SDK's own and
    the application's) that has a `CMakeLists.txt`/`Kconfig`, and the module guards itself on
    its own Kconfig symbol - see "Writing a module" below
  - **fetches each git dependency automatically** - a partial `git clone` straight to the
    pinned `rev`, landing at `modules/<module_dir>/<path>` (default `repo/`; skipped if that
    directory already exists and is non-empty, so it never clobbers a checkout you already
    have) - no west workspace or `west.yml` involved (see `dependency.clone_upstream()`)
  - first refuses a package incompatible with the SDK the target builds against
    (`--sdk-root`, default: the SDK these scripts live in), or whose registry dependencies
    aren't installed in the SDK or the target - see "Install" above; `--force` overrides
  - updates `tlkpkg-dependencies.yaml` at the target root (see "New project" below)

`scripts/tlkpkg/registry_client.py install-module <name>` (and the VS Code
extension, through it) plans the install and runs `modules/module_install.py`
for `kind: module` packages -
see [../../tl_unisdk_registry_server/README.md](../../tl_unisdk_registry_server/README.md).
The archive's `manifest.yaml` carries the module's details under a
top-level `module:` section; the install record lands at
`packages/module/<name>.yaml`.

### Writing a module

Nothing in the SDK lists its modules. Every directory directly under
`modules/` - the SDK's own, and the application's own `modules/` if it has
one - is picked up from disk: `modules/Kconfig` sources each `*/Kconfig`,
and `telink_add_modules()` (`cmake/modules/extensions.cmake`)
`add_subdirectory`s each `*/CMakeLists.txt`. So a module directory (or
its own repo, checked out there) needs:

- `Kconfig` - defines the module's own enable symbol (and its options)
- `CMakeLists.txt` - starts by returning early unless that symbol is set,
  so a module that's present but disabled builds nothing:

  ```cmake
  if(NOT CONFIG_TLK_FLASHDB)
      return()
  endif()
  ```
- `tlkpkg.yaml` - makes it a package (`export-module`/`publish-module`
  only pick up directories that have one), with the upstream project as a
  git dependency checked out at `repo/` (the default `path`, gitignored)

A module that depends on another one should `depends on` its symbol
rather than `select` it - a `select` of a module that isn't present
silently does nothing.

## New project

```bash
python scripts/tlkpkg/new_project.py --name my_app --location G:\Telink --sdk-root .
python scripts/tlkpkg/new_project.py --name my_app --location G:\Telink --sdk-root . --soc TL7218X --board TL7218X_EVK
python scripts/tlkpkg/new_project.py --name my_uart_app --location G:\Telink --sdk-root . --from-sample uart_demo
python scripts/tlkpkg/list_samples.py --sdk-root .
```

Scaffolds `<location>/<name>/` as a standalone application project:

- a minimal `CMakeLists.txt` + `main.c` using the same
  `find_package(Telink) HINTS $ENV{TELINK_BASE}` pattern as
  `samples/simple_demo`, so it builds against the SDK checkout without
  living inside it - or, with `--from-sample <id>` (an id from
  `list_samples.py`, e.g. `uart_demo` or `i2c_demo/ds1307` for a grouped
  sample), copies that sample's own `CMakeLists.txt`/sources/`Kconfig`/
  `README.md` into the project instead (`build/` and other VCS/cache
  clutter excluded - see `copy_sample()`), renaming the copy's
  `project(...)` call to match the new project name. `list_samples.py`
  discovers every sample by walking `samples/` for any directory with its
  own `CMakeLists.txt` - grouped samples (`i2c_demo/ds1307`,
  `system_demo/log_demo`, `tinyusb_demos/usb_device/cdc_demo`) come back
  the same way top-level ones do, no separate handling needed - and prints
  `[{id, name, description}]` as JSON, `description` being the sample's
  README's first non-heading line if it has one. Copying (rather than
  pointing the new project at `samples/<id>` in place) is what lets the
  sample be freely edited afterward without touching the original under
  the SDK checkout. A copied sample's own `Kconfig` needs no extra wiring
  to become configurable either - see "Kconfig editor backend" below;
  `Kconfig.build`'s `osource "$(KCONFIG_APPLICATION_DIR)/Kconfig"` already
  reads whatever `Kconfig` sits at the project's own root, which is
  exactly what the copy puts there.
- a self-contained `Makefile` - `make config`/`make build` run from
  *inside this project*, with no `APP=<path>`/`BUILD_DIR=<path>` juggling
  needed (unlike driving the SDK's own top-level `Makefile` against
  something outside its tree - see "Install" above). It always builds `.`
  into its own `build/`. `TELINK_BASE ?= <sdk_root>` bakes in the path
  recorded at creation time as a default, but a `TELINK_BASE` already in
  the environment (e.g. set by the VS Code extension's auto-activation)
  wins over it. Also passes `-DPython3_EXECUTABLE=<TELINK_BASE>/.venv/...`
  explicitly when that venv exists, rather than relying on
  `cmake/modules/python.cmake`'s `VIRTUAL_ENV`-env-var-based discovery -
  which silently falls back to whatever `python`/`python3` is on `PATH`
  (e.g. a system Python without `kconfiglib`, producing a bare "SOC/BOARD
  validation failed:" cmake error with no further detail) in any shell that
  never activated the venv. `make cmake`/`make build` work the same way
  regardless of terminal activation state now. The SDK's own top-level
  `Makefile` got the identical fix. Likewise `SOC ?=`/`BOARD ?=` bake in `--soc`/`--board` if
  given (empty otherwise) - these only take effect once `make cmake`/`make
  config` runs, which passes them to cmake as `-DSOC=`/`-DBOARD=` and from
  there to `scripts/generators/chip_config_gen.py`. That script (not this one)
  resolves them into `CONFIG_TLK_SOC_SERIES`, `CONFIG_TLK_CORE`,
  `CONFIG_TLK_SOC` and `CONFIG_TLK_BOARD` in `chip.config` - the exact
  same resolution the SDK's own Kconfig menu does (`packages/Kconfig.mcu`'s
  "SoC Series" choice -> `soc/<series>/Kconfig.soc`'s "SoC" choice ->
  the "Board" choice), just pre-selected instead of
  interactive. The VS Code extension's **+ New Project** wizard passes
  neither - its SoC/Board get picked afterwards in the Kconfig editor.
- `.vscode/settings.json` with `python.defaultInterpreterPath` pointing at
  the SDK's `.venv`. The SDK checkout gets its venv auto-activated in new
  integrated terminals for free - VS Code's Python extension auto-discovers
  a `.venv` sitting at a workspace's own root and activates it there by
  default. An external project has no venv of its own for that to find, so
  this tells the extension explicitly which interpreter to use; once set,
  its own auto-activation runs the real `Activate.ps1`-equivalent
  (`VIRTUAL_ENV`, prompt prefix, the works) here too. Merges into any
  existing `settings.json` rather than overwriting it
- `tlkpkg-dependencies.yaml` with one `kind: sdk` entry recording the
  UniSDK version (`sdk_version.py`'s display string, e.g.
  `0.3.0+5.gabc1234`) and path the project was created against

`install-module` extends the same `tlkpkg-dependencies.yaml` with a
`kind: module` entry per module installed afterward (see `project_deps.py`
for the shared upsert-by-name logic) - so the file ends up listing both
"which SDK" and "which modules" a project depends on, e.g.:

```yaml
dependencies:
- name: tinyusb
  kind: module
  version: 0.1.0
  module_dir: tinyUSB
  sdk: ^0.3.0
  dependencies:
    upstream:
      git: https://github.com/hathach/tinyusb.git
      rev: 3af1bec1a9161ee8dec29487831f7ac7ade9e189
      path: repo
  installed_at: '2026-08-17T20:49:29Z'
- name: unisdk
  kind: sdk
  version: 0.3.0
  path: G:\Telink\unisdk
  installed_at: '2026-08-17T20:47:58Z'
```

The VS Code extension's **+ New Project** wizard drives both this script
and `list_samples.py` (for its "Sample" vs "Custom" step) - see
[../../vscode-extension/README.md](../../vscode-extension/README.md).

### Add Makefile

```bash
python scripts/tlkpkg/add_makefile.py --target-root G:\Unisdk_external_tests\app3 --sdk-root .
python scripts/tlkpkg/add_makefile.py --target-root . --sdk-root ../unisdk --force
```

Retrofits `new_project.py`'s self-contained `Makefile` *and*
`.vscode/settings.json` (see above) into a project that predates them -
one created before those existed, or by hand. Refuses to overwrite an
existing `Makefile` unless `--force` is given; `settings.json` is always
merged (safe to re-run regardless of `--force`). Verified for real against
`app3`/`app4`/`app8` under `G:\Unisdk_external_tests\`: `make
cmake`/`make build` run from inside any of them, with no `TELINK_BASE` set
beforehand and no `cd` to the SDK, produce a working `Telink.elf`.

The VS Code extension's **Telink UniSDK: Add Makefile to Project** command
drives this script.

### List installed

```bash
python scripts/tlkpkg/list_installed.py --sdk-root . --project-root ../sample_project
```

Reads `packages/mcu/*.yaml` (an SDK's installed MCU packages) and each
given project's `tlkpkg-dependencies.yaml` (its installed modules
+ which SDK it targets), and prints it all as one JSON object - `--sdk-root`
and each `--project-root` are both optional and repeatable. Each MCU
entry's `series` field is the full `soc_series` -> `socs`/`boards` tree
(normalized across old and new package schema shapes), not just the
flattened name lists in `soc_series`/`boards`. MCU and module entries
carry their `sdk` range; in a source checkout, each `packages/<CORE>`
submodule is listed with its `tlkpkg.yaml` version (its `git describe`
only when it has none). This is what backs the
sidebar's "Installed MCUs"/"Installed Modules" lists; nothing else reads
this output, so its JSON shape isn't a stable API, just whatever those two
callers currently need.

### Kconfig editor backend

```bash
python scripts/tlkpkg/kconfig_web.py export --build-dir build --scope chip
python scripts/tlkpkg/kconfig_web.py export --build-dir build --scope build
python scripts/tlkpkg/kconfig_web.py set --build-dir build --scope chip --values "{\"TLK_SOC_SERIES_TL721X\": \"y\"}"
```

Backs the VS Code extension's Kconfig editor webview (SRC and SDK):
`export` prints the same menu tree `scripts/menuconfig.py` shows as JSON,
`set` applies `{SYMBOL_NAME: value}` assignments and writes the config file.

It only reads a build directory CMake has already configured - it never
configures or parses Kconfig itself:

| Scope | Kconfig | Config file | From `CMakeCache.txt` |
|---|---|---|---|
| `chip` | `Kconfig.chip` | `<build>/chip.config` | `TELINK_BASE` |
| `build` | `Kconfig.build` | `<build>/build.config` | `TELINK_BASE`, `GENERATED_KCONFIG_BASE_DIR` (as `KCONFIG_BINARY_DIR`), `KCONFIG_APPLICATION_DIR` |

The same variables `cmake/modules/kconfig.cmake` passes to menuconfig in its
`config_build` target. Around it the extension runs:

- `make cmake` when the build directory has no cache yet (or, for a sample,
  was configured for another one): it creates the default `chip.config` and
  `build.config`, which CMake tracks from then on;
- in an SRC only, `cmake --build <build> --target parse_kconfig` before the
  Build Config tab - it re-parses Kconfig when the chip changed (an SDK
  ships it parsed in `packages/<CORE>/kconfig/`);
- `cmake --build <build> --target generate_autoconf` after every change: it
  reconfigures, so `GENERATED_KCONFIG_BASE_DIR` in the cache follows a new
  chip, and regenerates `autoconf.h`.

`TELINK_BASE` is set in the environment of those `cmake --build` runs, so a
reconfigure finds the SDK even if CMake has to drop the cache (a series that
needs another compiler).

One `kconfiglib` subtlety worth knowing if you extend this: a symbol
belonging to an *optional* `choice` (like `packages/Kconfig.mcu`'s "Board") has
`assignable == ()` - not individually settable - until the choice itself
is turned on. `kconfig_web.py`'s `set` command calls
`sym.choice.set_value(2)` first whenever it detects this (any optional
choice, not hardcoded to Board), then proceeds with the normal
`sym.set_value()`.

## Known limitations

- **Board SoC dependencies are read as a plain list.** Each board's
  `boards/<BOARD>/Kconfig.board` may use any `depends on` expression as far
  as Kconfig is concerned, but the tooling (`mcu_discovery.discover_boards()`
  -> pack manifests, the new-project picker) only sees which `TLK_SOC_*`
  names it mentions - keep it a `TLK_SOC_A || TLK_SOC_B` list.
- **Zephyr integration is out of scope for now.** `zephyr/Kconfig` has its
  own separate SoC/board enumeration and is not touched by mcu installs.
- **mcu installs can't see application projects.** Installing or
  upgrading an mcu package in an SDK only protects the ranges of
  dependents recorded in that SDK itself (`packages/*/` there, or its
  source tree) - a module installed into a separate application project
  depending on it isn't visible, so an SDK-level upgrade can break it
  without warning.
- **Module dependency fetch needs network + `git`.** `modules/module_install.py`
  shells out to `git clone`/`checkout`/`submodule update` - it will fail
  without network access to the upstream host, and it doesn't go through
  `west` (no west workspace is assumed to exist), so it won't reuse an
  existing west-managed clone elsewhere on disk.
- **Windows long paths.** mbedtls's nested `tf-psa-crypto` submodule has
  file paths that exceed Windows' default 260-character limit, so
  `git submodule update --init --recursive` can fail with `Filename too
  long` there even though the top-level clone succeeded. Run
  `git config --global core.longpaths true` (and enable long paths in
  Windows itself, if not already) before installing `mbedtls` on Windows.
