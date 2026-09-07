# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

include(python)

# Header holding the SDK version and the application version (git tag/commit,
# or a frozen release VERSION file for the SDK) used by the .mcu_info
# binary-signature block (core/common/src/tlk_mcu_info.c).
set(GIT_VERSION_H "${CMAKE_BINARY_DIR}/tlk_git_version.h")

# add_custom_target() (unlike add_custom_command()) has no OUTPUT to check
# timestamps against, so it always re-runs - that is what we want here: the
# git commit/dirty state can change without any tracked input file changing.
# The generator script only rewrites GIT_VERSION_H when its content actually
# changed, so an unchanged version does not force a rebuild of its includers.
add_custom_target(generate_git_version ALL
    COMMAND ${PYTHON_EXECUTABLE}
            ${TELINK_BASE}/scripts/generators/git_version_gen.py
            --output ${GIT_VERSION_H}
            --sdk-repo ${TELINK_BASE}
            --sdk-version-file ${TELINK_BASE}/VERSION
            --sdk-max-len ${CONFIG_TLK_BINARY_SIGNATURE_SDK_VERSION_SIZE}
            --app-repo ${APPLICATION_SOURCE_DIR}
            --app-max-len ${CONFIG_TLK_BINARY_SIGNATURE_APP_VERSION_SIZE}
    BYPRODUCTS ${GIT_VERSION_H}
    COMMENT "Generating SDK/App git version header"
)
