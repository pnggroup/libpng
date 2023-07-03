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

CI_SHELLCHECK="$(command -v shellcheck || true)"
CI_YAMLLINT="$(command -v yamllint || true)"
CI_LINT_COUNTER=0

function ci_lint_ci_config_files {
    ci_info "linting: CI config files"
    local MY_FILE
    if [[ -x $CI_YAMLLINT ]]
    then
        ci_spawn "$CI_YAMLLINT" --version
        for MY_FILE in "$CI_TOPLEVEL_DIR"/.*.yml
        do
            ci_spawn "$CI_YAMLLINT" --strict "$MY_FILE" ||
                CI_LINT_COUNTER=$((CI_LINT_COUNTER + 1))
        done
    else
        ci_warn "program not found: 'yamllint'; skipping checks"
    fi
}

function ci_lint_ci_scripts {
    ci_info "linting: CI scripts"
    local MY_FILE
    if [[ -x $CI_SHELLCHECK ]]
    then
        ci_spawn "$CI_SHELLCHECK" --version
        for MY_FILE in "$CI_SCRIPT_DIR"/*.sh
        do
            ci_spawn "$CI_SHELLCHECK" -x "$MY_FILE" ||
                CI_LINT_COUNTER=$((CI_LINT_COUNTER + 1))
        done
    else
        ci_warn "program not found: 'shellcheck'; skipping checks"
    fi
}

function ci_lint_ci_scripts_license {
    ci_info "linting: CI scripts license"
    ci_spawn grep -F "Boost Software License" ci/LICENSE_BSL_1_0.txt || {
        ci_warn "bad or missing CI license file: '$CI_SCRIPT_DIR/LICENSE_BSL_1_0.txt'"
        CI_LINT_COUNTER=$((CI_LINT_COUNTER + 1))
    }
}

function main {
    [[ $# -eq 0 ]] || {
        ci_info "note: this program accepts environment options only"
        ci_err "unsupported command argument: '$1'"
    }
    ci_lint_ci_config_files
    ci_lint_ci_scripts
    ci_lint_ci_scripts_license
    if [[ $CI_LINT_COUNTER -eq 0 ]]
    then
        ci_info "success!"
        exit 0
    else
        ci_info "failed on $CI_LINT_COUNTER file(s)"
        exit 1
    fi
}

main "$@"
