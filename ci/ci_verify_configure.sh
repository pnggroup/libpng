#!/usr/bin/env bash
set -e

# Copyright (c) 2019-2023 Cosmin Truta.
#
# Use, modification and distribution are subject
# to the Boost Software License, Version 1.0.
# See the accompanying file LICENSE_BSL_1_0.txt
# or visit http://www.boost.org/LICENSE_1_0.txt
#
# SPDX-License-Identifier: BSL-1.0

# shellcheck source="ci/lib/ci.lib.sh"
source "$(dirname "$0")/lib/ci.lib.sh"
cd "$CI_TOPLEVEL_DIR"

CI_SRC_DIR="$CI_TOPLEVEL_DIR"
CI_OUT_DIR="$CI_TOPLEVEL_DIR/out"
CI_BUILD_DIR="$CI_OUT_DIR/ci_verify_configure.$CI_TARGET_SYSTEM.$CI_TARGET_MACHINE.build"
CI_INSTALL_DIR="$CI_OUT_DIR/ci_verify_configure.$CI_TARGET_SYSTEM.$CI_TARGET_MACHINE.install"

function ci_init_build {
    CI_MAKE="${CI_MAKE:-make}"
    # Set CI_CC to cc by default, if the cc command is available.
    # The configure script defaults CC to gcc, which is not always a good idea.
    [[ -x $(command -v cc) ]] && CI_CC="${CI_CC:-cc}"
    # Ensure that the CI_ variables that cannot be customized reliably are not initialized.
    [[ ! $CI_CONFIGURE_VARS ]] || ci_err "unsupported: \$CI_CONFIGURE_VARS='$CI_CONFIGURE_VARS'"
    [[ ! $CI_MAKE_VARS ]] || ci_err "unsupported: \$CI_MAKE_VARS='$CI_MAKE_VARS'"
}

function ci_trace_build {
    ci_info "## START OF CONFIGURATION ##"
    ci_info "host system: $CI_HOST_SYSTEM"
    ci_info "host machine hardware: $CI_HOST_MACHINE"
    [[ "$CI_TARGET_SYSTEM" != "$CI_HOST_SYSTEM" ]] &&
        ci_info "target system: $CI_TARGET_SYSTEM"
    [[ "$CI_TARGET_MACHINE" != "$CI_HOST_MACHINE" ]] &&
        ci_info "target machine hardware: $CI_TARGET_MACHINE"
    ci_info "source directory: $CI_SRC_DIR"
    ci_info "build directory: $CI_BUILD_DIR"
    ci_info "install directory: $CI_INSTALL_DIR"
    ci_info "environment option: \$CI_CONFIGURE_FLAGS: '$CI_CONFIGURE_FLAGS'"
    ci_info "environment option: \$CI_MAKE: '$CI_MAKE'"
    ci_info "environment option: \$CI_MAKE_FLAGS: '$CI_MAKE_FLAGS'"
    ci_info "environment option: \$CI_CC: '$CI_CC'"
    ci_info "environment option: \$CI_CC_FLAGS: '$CI_CC_FLAGS'"
    ci_info "environment option: \$CI_CPP: '$CI_CPP'"
    ci_info "environment option: \$CI_CPP_FLAGS: '$CI_CPP_FLAGS'"
    ci_info "environment option: \$CI_AR: '$CI_AR'"
    ci_info "environment option: \$CI_RANLIB: '$CI_RANLIB'"
    ci_info "environment option: \$CI_LD: '$CI_LD'"
    ci_info "environment option: \$CI_LD_FLAGS: '$CI_LD_FLAGS'"
    ci_info "environment option: \$CI_SANITIZERS: '$CI_SANITIZERS'"
    ci_info "environment option: \$CI_NO_TEST: '$CI_NO_TEST'"
    ci_info "environment option: \$CI_NO_INSTALL: '$CI_NO_INSTALL'"
    ci_info "environment option: \$CI_NO_CLEAN: '$CI_NO_CLEAN'"
    ci_info "executable: \$CI_MAKE: $(command -V "$CI_MAKE")"
    [[ $CI_CC ]] &&
        ci_info "executable: \$CI_CC: $(command -V "$CI_CC")"
    [[ $CI_CPP ]] &&
        ci_info "executable: \$CI_CPP: $(command -V "$CI_CPP")"
    [[ $CI_AR ]] &&
        ci_info "executable: \$CI_AR: $(command -V "$CI_AR")"
    [[ $CI_RANLIB ]] &&
        ci_info "executable: \$CI_RANLIB: $(command -V "$CI_RANLIB")"
    [[ $CI_LD ]] &&
        ci_info "executable: \$CI_LD: $(command -V "$CI_LD")"
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
    # Export the configure build environment.
    [[ $CI_CC ]] && ci_spawn export CC="$CI_CC"
    [[ $CI_CC_FLAGS ]] && ci_spawn export CFLAGS="$CI_CC_FLAGS"
    [[ $CI_CPP ]] && ci_spawn export CPP="$CI_CPP"
    [[ $CI_CPP_FLAGS ]] && ci_spawn export CPPFLAGS="$CI_CPP_FLAGS"
    [[ $CI_AR ]] && ci_spawn export AR="$CI_AR"
    [[ $CI_RANLIB ]] && ci_spawn export RANLIB="$CI_RANLIB"
    [[ $CI_LD ]] && ci_spawn export LD="$CI_LD"
    [[ $CI_LD_FLAGS ]] && ci_spawn export LDFLAGS="$CI_LD_FLAGS"
    [[ $CI_SANITIZERS ]] && {
        ci_spawn export CFLAGS="-fsanitize=$CI_SANITIZERS ${CFLAGS:-"-O2"}"
        ci_spawn export LDFLAGS="-fsanitize=$CI_SANITIZERS $LDFLAGS"
    }
    # Build and install.
    ci_spawn mkdir -p "$CI_BUILD_DIR"
    ci_spawn cd "$CI_BUILD_DIR"
    ci_spawn "$CI_SRC_DIR/configure" --prefix="$CI_INSTALL_DIR" $CI_CONFIGURE_FLAGS
    ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS
    [[ $CI_NO_TEST ]] || ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS test
    [[ $CI_NO_INSTALL ]] || ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS install
    [[ $CI_NO_CLEAN ]] || ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS clean
    [[ $CI_NO_CLEAN ]] || ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS distclean
    ci_info "## END OF BUILD ##"
}

function main {
    [[ $# -eq 0 ]] || {
        ci_info "note: this program accepts environment options only"
        ci_err "unsupported command argument: '$1'"
    }
    ci_init_build
    ci_trace_build
    ci_cleanup_old_build
    ci_build
}

main "$@"
