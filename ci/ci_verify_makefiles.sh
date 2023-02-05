#!/usr/bin/env bash
set -e

# ci_verify_makefiles.sh
# Continuously integrate libpng using the legacy makefiles.
#
# Copyright (c) 2019-2023 Cosmin Truta.
#
# This software is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h.

CI_SCRIPTNAME="$(basename "$0")"
CI_SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
CI_SRCDIR="$(dirname "$CI_SCRIPTDIR")"

function ci_info {
    printf >&2 "%s: %s\\n" "$CI_SCRIPTNAME" "$*"
}

function ci_err {
    printf >&2 "%s: error: %s\\n" "$CI_SCRIPTNAME" "$*"
    exit 2
}

function ci_spawn {
    printf >&2 "%s: executing:" "$CI_SCRIPTNAME"
    printf >&2 " %q" "$@"
    printf >&2 "\\n"
    "$@"
}

function ci_init_makefiles_build {
    CI_SYSTEM_NAME="$(uname -s)"
    CI_MACHINE_NAME="$(uname -m)"
    CI_MAKE="${CI_MAKE:-make}"
    case "$CI_CC" in
    ( *clang* )
        CI_MAKEFILES="${CI_MAKEFILES:-"scripts/makefile.clang"}" ;;
    ( *gcc* )
        CI_MAKEFILES="${CI_MAKEFILES:-"scripts/makefile.gcc"}" ;;
    ( * )
        CI_MAKEFILES="${CI_MAKEFILES:-"scripts/makefile.std"}" ;;
    esac
}

function ci_trace_makefiles_build {
    ci_info "## START OF CONFIGURATION ##"
    ci_info "system name: $CI_SYSTEM_NAME"
    ci_info "machine hardware name: $CI_MACHINE_NAME"
    ci_info "source directory: $CI_SRCDIR"
    ci_info "environment option: \$CI_MAKEFILES: '$CI_MAKEFILES'"
    ci_info "environment option: \$CI_MAKE: '$CI_MAKE'"
    ci_info "environment option: \$CI_MAKE_FLAGS: '$CI_MAKE_FLAGS'"
    ci_info "environment option: \$CI_MAKE_VARS: '$CI_MAKE_VARS'"
    ci_info "environment option: \$CI_CC: '$CI_CC'"
    ci_info "environment option: \$CI_CC_FLAGS: '$CI_CC_FLAGS'"
    ci_info "environment option: \$CI_CPP: '$CI_CPP'"
    ci_info "environment option: \$CI_CPP_FLAGS: '$CI_CPP_FLAGS'"
    ci_info "environment option: \$CI_AR: '$CI_AR'"
    ci_info "environment option: \$CI_RANLIB: '$CI_RANLIB'"
    ci_info "environment option: \$CI_LD: '$CI_LD'"
    ci_info "environment option: \$CI_LD_FLAGS: '$CI_LD_FLAGS'"
    ci_info "environment option: \$CI_LIBS: '$CI_LIBS'"
    ci_info "environment option: \$CI_SANITIZERS: '$CI_SANITIZERS'"
    ci_info "environment option: \$CI_NO_TEST: '$CI_NO_TEST'"
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

function ci_cleanup_old_makefiles_build {
    # Any old makefile-based build will most likely leave a mess
    # of object files behind if interrupted, e.g., via Ctrl+C.
    # There may be other files behind, depending on what makefile
    # had been used. We cannot easily enumerate all of those.
    # Fortunately, for a clean makefiles-based build, it should be
    # sufficient to remove the old object files only.
    [[ -z $(find "$CI_SRCDIR" -maxdepth 1 -name "*.o") ]] ||
        ci_spawn rm -f "$CI_SRCDIR"/*.o
    [[ -z $(find "$CI_SRCDIR" -maxdepth 1 -name "*.obj") ]] ||
        ci_spawn rm -f "$CI_SRCDIR"/*.obj
}

function ci_build_makefiles {
    ci_info "## START OF BUILD ##"
    # Initialize ALL_CC_FLAGS and ALL_LD_FLAGS as strings.
    local ALL_CC_FLAGS="$CI_CC_FLAGS"
    local ALL_LD_FLAGS="$CI_LD_FLAGS"
    [[ $CI_SANITIZERS ]] && {
        ALL_CC_FLAGS="-fsanitize=$CI_SANITIZERS ${ALL_CC_FLAGS:-"-O2"}"
        ALL_LD_FLAGS="-fsanitize=$CI_SANITIZERS $ALL_LD_FLAGS"
    }
    # Initialize ALL_MAKE_FLAGS and ALL_MAKE_VARS as arrays.
    local ALL_MAKE_FLAGS=($CI_MAKE_FLAGS)
    local ALL_MAKE_VARS=()
    [[ $CI_CC ]] && ALL_MAKE_VARS+=(CC="$CI_CC")
    [[ $ALL_CC_FLAGS ]] && ALL_MAKE_VARS+=(CFLAGS="$ALL_CC_FLAGS")
    [[ $CI_CPP ]] && ALL_MAKE_VARS+=(CPP="$CI_CPP")
    [[ $CI_CPP_FLAGS ]] && ALL_MAKE_VARS+=(CPPFLAGS="$CI_CPP_FLAGS")
    [[ $CI_AR ]] && ALL_MAKE_VARS+=(
        AR="${CI_AR:-ar}"
        AR_RC="${CI_AR:-ar} rc"
    )
    [[ $CI_RANLIB ]] && ALL_MAKE_VARS+=(RANLIB="$CI_RANLIB")
    [[ $CI_LD ]] && ALL_MAKE_VARS+=(LD="$CI_LD")
    [[ $ALL_LD_FLAGS ]] && ALL_MAKE_VARS+=(LDFLAGS="$ALL_LD_FLAGS")
    [[ $CI_LIBS ]] && ALL_MAKE_VARS+=(LIBS="$CI_LIBS")
    ALL_MAKE_VARS+=($CI_MAKE_VARS)
    # Build!
    cd "$CI_SRCDIR"
    local MY_MAKEFILE
    for MY_MAKEFILE in $CI_MAKEFILES
    do
        ci_info "using makefile: $MY_MAKEFILE"
        ci_spawn "$CI_MAKE" -f "$MY_MAKEFILE" \
                            "${ALL_MAKE_FLAGS[@]}" \
                            "${ALL_MAKE_VARS[@]}"
        [[ $CI_NO_TEST ]] ||
            ci_spawn "$CI_MAKE" -f "$MY_MAKEFILE" \
                                "${ALL_MAKE_FLAGS[@]}" \
                                "${ALL_MAKE_VARS[@]}" \
                                test
        [[ $CI_NO_CLEAN ]] ||
            ci_spawn "$CI_MAKE" -f "$MY_MAKEFILE" \
                                "${ALL_MAKE_FLAGS[@]}" \
                                "${ALL_MAKE_VARS[@]}" \
                                clean
    done
    ci_info "## END OF BUILD ##"
}

function main {
    [[ $# -eq 0 ]] || {
        ci_info "note: this program accepts environment options only"
        ci_err "unexpected command arguments: '$*'"
    }
    ci_init_makefiles_build
    ci_trace_makefiles_build
    ci_cleanup_old_makefiles_build
    ci_build_makefiles
}

main "$@"
