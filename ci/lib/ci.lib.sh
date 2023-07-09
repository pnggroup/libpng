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

# Initialize the global constants CI_{HOST,TARGET}_{SYSTEM,MACHINE}.
CI_HOST_SYSTEM="${CI_HOST_SYSTEM:-"$(uname -s | tr 'A-Z/\.-' 'a-z____')"}"
CI_HOST_MACHINE="${CI_HOST_MACHINE:-"$(uname -m | tr 'A-Z/\.-' 'a-z____')"}"
CI_TARGET_SYSTEM="${CI_TARGET_SYSTEM:-"$CI_HOST_SYSTEM"}"
CI_TARGET_MACHINE="${CI_TARGET_MACHINE:-"$CI_HOST_MACHINE"}"

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
    test "$@" || ci_err_internal "failed:" test "$@"
}

function ci_spawn {
    printf >&2 "%s: executing:" "$CI_SCRIPT_NAME"
    printf >&2 " %q" "$@"
    printf >&2 "\\n"
    "$@"
}

# Ensure that the initialization is correct.
ci_assert "$CI_TOPLEVEL_DIR/ci/lib/ci.lib.sh" -ef "${BASH_SOURCE[0]}"
ci_assert "$CI_SCRIPT_DIR/$CI_SCRIPT_NAME" -ef "$0"
ci_assert -n "$CI_HOST_SYSTEM" -a -n "$CI_HOST_MACHINE"
ci_assert -n "$CI_TARGET_SYSTEM" -a -n "$CI_TARGET_MACHINE"
