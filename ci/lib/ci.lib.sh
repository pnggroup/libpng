# Copyright (c) 2019-2023 Cosmin Truta.
#
# Use, modification and distribution are subject
# to the Boost Software License, Version 1.0.
# See the accompanying file LICENSE_BSL_1_0.txt
# or visit http://www.boost.org/LICENSE_1_0.txt
#
# SPDX-License-Identifier: BSL-1.0

test -f "$BASH_SOURCE" ||
    echo >&2 "warning: this module requires Bash version 3 or newer"
test "${#BASH_SOURCE[@]}" -gt 1 ||
    echo >&2 "warning: this module should be sourced from a Bash script"

# Reset the locale to avoid surprises from locale-dependent commands.
export LC_ALL=C
export LANG=C
export LANGUAGE=C

# Reset CDPATH to avoid surprises from the "cd" command.
export CDPATH=""

# Initialize the global constants CI_SCRIPT_{NAME,DIR} and CI_TOPLEVEL_DIR.
CI_SCRIPT_NAME="$(basename -- "$0")"
CI_SCRIPT_DIR="$(cd "$(dirname -- "$0")" && pwd)"
CI_TOPLEVEL_DIR="$(cd "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"

# Initialize the global constants CI_BUILD_{...} for the host build platform.
CI_BUILD_ARCH="${CI_BUILD_ARCH:-"$(uname -m | tr 'A-Z/\.-' 'a-z____')"}"
CI_BUILD_SYSTEM="${CI_BUILD_SYSTEM:-"$(uname -s | tr 'A-Z/\.-' 'a-z____')"}"

# Initialize the global constants CI_TARGET_{...} for the target platform.
if [[ $CI_TARGET_ARCH && $CI_TARGET_SYSTEM && $CI_TARGET_ABI ]]
then
    CI_TARGET_TRIPLET="${CI_TARGET_TRIPLET:-$CI_TARGET_ARCH-$CI_TARGET_SYSTEM-$CI_TARGET_ABI}"
elif [[ $CI_TARGET_TRIPLET ]]
then
    CI_TARGET_ARCH="${CI_TARGET_ARCH:-${CI_TARGET_TRIPLET%%-*}}"
    CI_TARGET_DOUBLET_IMPL="${CI_TARGET_TRIPLET#*-}"
    CI_TARGET_SYSTEM="${CI_TARGET_SYSTEM:-${CI_TARGET_DOUBLET_IMPL%%-*}}"
    CI_TARGET_ABI="${CI_TARGET_ABI:-${CI_TARGET_DOUBLET_IMPL#*-}}"
fi
CI_TARGET_ARCH="${CI_TARGET_ARCH:-"$CI_BUILD_ARCH"}"
CI_TARGET_SYSTEM="${CI_TARGET_SYSTEM:-"$CI_BUILD_SYSTEM"}"

function ci_info {
    printf >&2 "%s: %s\\n" "$CI_SCRIPT_NAME" "$*"
}

function ci_warn {
    printf >&2 "%s: warning: %s\\n" "$CI_SCRIPT_NAME" "$*"
}

function ci_err {
    printf >&2 "%s: error: %s\\n" "$CI_SCRIPT_NAME" "$*"
    exit 2
}

function ci_err_internal {
    printf >&2 "%s: internal error: %s\\n" "$CI_SCRIPT_NAME" "$*"
    printf >&2 "ABORTED\\n"
    # Exit with the conventional SIGABRT code.
    exit 134
}

function ci_assert {
    # Use the "test" built-in command instead of the "[[ ]]" syntax,
    # to ensure the a-priori expansion of all assertion arguments.
    # (Consistently, both "ci_assert" and "test" have a command-like behavior.)
    [[ $# -ge 2 ]] ||
        ci_err_internal "failed: ci_assert: bad or missing operands"
    local label="$1"
    shift
    test "$@" ||
        ci_err_internal "failed: $label:" test "$@"
}

function ci_spawn {
    printf >&2 "%s: executing:" "$CI_SCRIPT_NAME"
    printf >&2 " %q" "$@"
    printf >&2 "\\n"
    "$@"
}

# Ensure that the initialization is correct.
ci_assert "checking CI_TOPLEVEL_DIR" \
          "$CI_TOPLEVEL_DIR/ci/lib/ci.lib.sh" -ef "${BASH_SOURCE[0]}"
ci_assert "checking CI_SCRIPT_DIR and CI_SCRIPT_NAME" \
          "$CI_SCRIPT_DIR/$CI_SCRIPT_NAME" -ef "$0"
ci_assert "checking CI_BUILD_ARCH and CI_BUILD_SYSTEM" \
          -n "$CI_BUILD_ARCH" -a -n "$CI_BUILD_SYSTEM"
ci_assert "checking CI_TARGET_ARCH and CI_TARGET_SYSTEM" \
          -n "$CI_TARGET_ARCH" -a -n "$CI_TARGET_SYSTEM"
ci_assert "checking CI_TARGET_TRIPLET" \
          x"$CI_TARGET_TRIPLET" = x"" -o \
          x"$CI_TARGET_TRIPLET" = x"$CI_TARGET_ARCH-$CI_TARGET_SYSTEM-$CI_TARGET_ABI"
ci_assert "checking if CI_NO_TEST is boolean" \
          $((CI_NO_TEST)) -eq $((!!CI_NO_TEST))
ci_assert "checking if CI_NO_INSTALL is boolean" \
          $((CI_NO_INSTALL)) -eq $((!!CI_NO_INSTALL))
ci_assert "checking if CI_NO_CLEAN is boolean" \
          $((CI_NO_CLEAN)) -eq $((!!CI_NO_CLEAN))
