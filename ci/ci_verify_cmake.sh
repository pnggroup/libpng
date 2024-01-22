#!/usr/bin/env bash
set -e

# Copyright (c) 2019-2024 Cosmin Truta.
#
# Use, modification and distribution are subject to the MIT License.
# Please see the accompanying file LICENSE_MIT.txt
#
# SPDX-License-Identifier: MIT

# shellcheck source="ci/lib/ci.lib.sh"
source "$(dirname "$0")/lib/ci.lib.sh"
cd "$CI_TOPLEVEL_DIR"

CI_SRC_DIR="$CI_TOPLEVEL_DIR"
CI_OUT_DIR="$CI_TOPLEVEL_DIR/out"
CI_BUILD_DIR="$CI_OUT_DIR/ci_verify_cmake.$CI_TARGET_SYSTEM.$CI_TARGET_ARCH.build"
CI_INSTALL_DIR="$CI_OUT_DIR/ci_verify_cmake.$CI_TARGET_SYSTEM.$CI_TARGET_ARCH.install"

# Keep the following relative paths in sync with the absolute paths.
# We use them for the benefit of native Windows tools that might be
# otherwise confused by the path encoding used by Bash-on-Windows.
CI_BUILD_TO_SRC_RELDIR="../.."
CI_BUILD_TO_INSTALL_RELDIR="../ci_verify_cmake.$CI_TARGET_SYSTEM.$CI_TARGET_ARCH.install"

function ci_init_build {
    # Ensure that the mandatory variables are initialized.
    CI_CMAKE="${CI_CMAKE:-cmake}"
    CI_CTEST="${CI_CTEST:-ctest}"
    CI_CMAKE_BUILD_TYPE="${CI_CMAKE_BUILD_TYPE:-Release}"
    [[ -x $(command -v ninja) ]] &&
        CI_CMAKE_GENERATOR="${CI_CMAKE_GENERATOR:-Ninja}"
    if [[ $CI_CMAKE_GENERATOR == "Visual Studio"* ]]
    then
        # Clean up incidental mixtures of Windows and Bash-on-Windows
        # environment variables, to avoid confusing MSBuild.
        [[ $TEMP && ( $Temp || $temp ) ]] && unset TEMP
        [[ $TMP && ( $Tmp || $tmp ) ]] && unset TMP
        # Ensure that CI_CMAKE_GENERATOR_PLATFORM is initialized for this generator.
        [[ $CI_CMAKE_GENERATOR_PLATFORM ]] ||
            ci_err_internal "missing \$CI_CMAKE_GENERATOR_PLATFORM"
    fi
}

function ci_trace_build {
    ci_info "## START OF CONFIGURATION ##"
    ci_info "build arch: $CI_BUILD_ARCH"
    ci_info "build system: $CI_BUILD_SYSTEM"
    [[ "$CI_TARGET_SYSTEM.$CI_TARGET_ARCH" != "$CI_BUILD_SYSTEM.$CI_BUILD_ARCH" ]] && {
        ci_info "target arch: $CI_TARGET_ARCH"
        ci_info "target system: $CI_TARGET_SYSTEM"
    }
    ci_info "source directory: $CI_SRC_DIR"
    ci_info "build directory: $CI_BUILD_DIR"
    ci_info "install directory: $CI_INSTALL_DIR"
    ci_info "environment option: \$CI_CMAKE: '$CI_CMAKE'"
    ci_info "environment option: \$CI_CMAKE_GENERATOR: '$CI_CMAKE_GENERATOR'"
    ci_info "environment option: \$CI_CMAKE_GENERATOR_PLATFORM: '$CI_CMAKE_GENERATOR_PLATFORM'"
    ci_info "environment option: \$CI_CMAKE_BUILD_TYPE: '$CI_CMAKE_BUILD_TYPE'"
    ci_info "environment option: \$CI_CMAKE_BUILD_FLAGS: '$CI_CMAKE_BUILD_FLAGS'"
    ci_info "environment option: \$CI_CMAKE_TOOLCHAIN_FILE: '$CI_CMAKE_TOOLCHAIN_FILE'"
    ci_info "environment option: \$CI_CMAKE_VARS: '$CI_CMAKE_VARS'"
    ci_info "environment option: \$CI_CTEST: '$CI_CTEST'"
    ci_info "environment option: \$CI_CTEST_FLAGS: '$CI_CTEST_FLAGS'"
    ci_info "environment option: \$CI_CC: '$CI_CC'"
    ci_info "environment option: \$CI_CC_FLAGS: '$CI_CC_FLAGS'"
    ci_info "environment option: \$CI_AR: '$CI_AR'"
    ci_info "environment option: \$CI_RANLIB: '$CI_RANLIB'"
    ci_info "environment option: \$CI_SANITIZERS: '$CI_SANITIZERS'"
    ci_info "environment option: \$CI_NO_TEST: '$CI_NO_TEST'"
    ci_info "environment option: \$CI_NO_INSTALL: '$CI_NO_INSTALL'"
    ci_info "environment option: \$CI_NO_CLEAN: '$CI_NO_CLEAN'"
    ci_info "executable: \$CI_CMAKE: $(command -V "$CI_CMAKE")"
    ci_info "executable: \$CI_CTEST: $(command -V "$CI_CTEST")"
    [[ $CI_CC ]] &&
        ci_info "executable: \$CI_CC: $(command -V "$CI_CC")"
    [[ $CI_AR ]] &&
        ci_info "executable: \$CI_AR: $(command -V "$CI_AR")"
    [[ $CI_RANLIB ]] &&
        ci_info "executable: \$CI_RANLIB: $(command -V "$CI_RANLIB")"
    ci_info "## END OF CONFIGURATION ##"
}

function ci_cleanup_old_build {
    if [[ -e $CI_BUILD_DIR || -e $CI_INSTALL_DIR ]]
    then
        ci_info "## START OF PRE-BUILD CLEANUP ##"
        ci_spawn rm -fr "$CI_BUILD_DIR"
        ci_spawn rm -fr "$CI_INSTALL_DIR"
        ci_info "## END OF PRE-BUILD CLEANUP ##"
    fi
}

function ci_build {
    ci_info "## START OF BUILD ##"
    ci_spawn "$(command -v "$CI_CMAKE")" --version
    ci_spawn "$(command -v "$CI_CTEST")" --version
    [[ $CI_CMAKE_GENERATOR == *"Ninja"* ]] &&
        ci_spawn "$(command -v ninja)" --version
    # Initialize ALL_CC_FLAGS as a string.
    local ALL_CC_FLAGS="$CI_CC_FLAGS"
    [[ $CI_SANITIZERS ]] &&
        ALL_CC_FLAGS="-fsanitize=$CI_SANITIZERS $ALL_CC_FLAGS"
    # Initialize ALL_CMAKE_VARS, ALL_CMAKE_BUILD_FLAGS and ALL_CTEST_FLAGS as arrays.
    local ALL_CMAKE_VARS=()
    [[ $CI_CMAKE_TOOLCHAIN_FILE ]] &&
        ALL_CMAKE_VARS+=(-DCMAKE_TOOLCHAIN_FILE="$CI_CMAKE_TOOLCHAIN_FILE")
    [[ $CI_CC ]] &&
        ALL_CMAKE_VARS+=(-DCMAKE_C_COMPILER="$CI_CC")
    [[ $ALL_CC_FLAGS ]] &&
        ALL_CMAKE_VARS+=(-DCMAKE_C_FLAGS="$ALL_CC_FLAGS")
    [[ $CI_AR ]] && {
        # Use the full path of CI_AR to work around a CMake error.
        ALL_CMAKE_VARS+=(-DCMAKE_AR="$(command -v "$CI_AR")")
    }
    [[ $CI_RANLIB ]] && {
        # Use the full path of CI_RANLIB to work around a CMake error.
        ALL_CMAKE_VARS+=(-DCMAKE_RANLIB="$(command -v "$CI_RANLIB")")
    }
    ALL_CMAKE_VARS+=(-DCMAKE_BUILD_TYPE="$CI_CMAKE_BUILD_TYPE")
    ALL_CMAKE_VARS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
    ALL_CMAKE_VARS+=($CI_CMAKE_VARS)
    local ALL_CMAKE_BUILD_FLAGS=($CI_CMAKE_BUILD_FLAGS)
    local ALL_CTEST_FLAGS=($CI_CTEST_FLAGS)
    # Export the CMake environment variables.
    [[ $CI_CMAKE_GENERATOR ]] &&
        ci_spawn export CMAKE_GENERATOR="$CI_CMAKE_GENERATOR"
    [[ $CI_CMAKE_GENERATOR_PLATFORM ]] &&
        ci_spawn export CMAKE_GENERATOR_PLATFORM="$CI_CMAKE_GENERATOR_PLATFORM"
    # Build!
    # Use $CI_BUILD_TO_SRC_RELDIR and $CI_BUILD_TO_INSTALL_RELDIR
    # instead of $CI_SRC_DIR and $CI_INSTALL_DIR from this point onwards.
    ci_spawn mkdir -p "$CI_BUILD_DIR"
    ci_spawn cd "$CI_BUILD_DIR"
    [[ $CI_BUILD_TO_SRC_RELDIR -ef $CI_SRC_DIR ]] ||
        ci_err_internal "bad or missing \$CI_BUILD_TO_SRC_RELDIR"
    ci_spawn mkdir -p "$CI_INSTALL_DIR"
    [[ $CI_BUILD_TO_INSTALL_RELDIR -ef $CI_INSTALL_DIR ]] ||
        ci_err_internal "bad or missing \$CI_BUILD_TO_INSTALL_RELDIR"
    # Spawn "cmake ...".
    ci_spawn "$CI_CMAKE" -DCMAKE_INSTALL_PREFIX="$CI_BUILD_TO_INSTALL_RELDIR" \
                         "${ALL_CMAKE_VARS[@]}" \
                         "$CI_BUILD_TO_SRC_RELDIR"
    # Spawn "cmake --build ...".
    ci_spawn "$CI_CMAKE" --build . \
                         --config "$CI_CMAKE_BUILD_TYPE" \
                         "${ALL_CMAKE_BUILD_FLAGS[@]}"
    ci_expr $((CI_NO_TEST)) || {
        # Spawn "ctest" if testing is not disabled.
        ci_spawn "$CI_CTEST" --build-config "$CI_CMAKE_BUILD_TYPE" \
                             "${ALL_CTEST_FLAGS[@]}"
    }
    ci_expr $((CI_NO_INSTALL)) || {
        # Spawn "cmake --build ... --target install" if installation is not disabled.
        ci_spawn "$CI_CMAKE" --build . \
                             --config "$CI_CMAKE_BUILD_TYPE" \
                             --target install \
                             "${ALL_CMAKE_BUILD_FLAGS[@]}"
    }
    ci_expr $((CI_NO_CLEAN)) || {
        # Spawn "make --build ... --target clean" if cleaning is not disabled.
        ci_spawn "$CI_CMAKE" --build . \
                             --config "$CI_CMAKE_BUILD_TYPE" \
                             --target clean \
                             "${ALL_CMAKE_BUILD_FLAGS[@]}"
    }
    ci_info "## END OF BUILD ##"
}

function usage {
    echo "usage: $CI_SCRIPT_NAME"
    exit 0
}

function main {
    local opt
    while getopts ":" opt
    do
        # This ain't a while-loop. It only pretends to be.
        [[ $1 == -[?h]* || $1 == --help ]] && usage
        ci_err "unknown option: '$1'"
    done
    shift $((OPTIND - 1))
    ci_init_build
    ci_trace_build
    [[ $# -eq 0 ]] || {
        ci_info "note: this program accepts environment options only"
        ci_err "unexpected argument: '$1'"
    }
    ci_cleanup_old_build
    ci_build
}

main "$@"
