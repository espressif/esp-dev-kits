#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
#
# Scheduled CI helper: fetch the product API, rewrite every ordering-info.inc,
# and push a branch with a merge request if anything changed.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

BRANCH="ci/ordering-info-refresh"
TARGET="${CI_COMMIT_REF_NAME:-${CI_DEFAULT_BRANCH:-master}}"
SSH_HOST="${CI_SERVER_SHELL_SSH_HOST:-${CI_SERVER_HOST:-gitlab.espressif.cn}}"
# This server does not serve git over port 22.
SSH_PORT="${CI_SERVER_SHELL_SSH_PORT:-27227}"

python tools/ordering_info/ordering_info.py apply

git add docs/en/*/ordering-info.inc docs/zh_CN/*/ordering-info.inc || true

if git diff --cached --quiet; then
    echo "No ordering-information changes."
    exit 0
fi

git checkout -B "$BRANCH"
git -c user.name="esp-dev-kits CI" -c user.email="gitlab-ci@espressif.com" \
    commit -m "docs: refresh ordering information from the product API"

if [ -n "${CI_PROJECT_PATH:-}" ]; then
    if [ -n "${ORDERING_INFO_TOKEN:-}" ]; then
        SERVER="${CI_SERVER_HOST:-gitlab.espressif.cn}${CI_SERVER_PORT:+:${CI_SERVER_PORT}}"
        # Keep the token out of the log.
        git remote set-url origin \
            "https://oauth2:${ORDERING_INFO_TOKEN}@${SERVER}/${CI_PROJECT_PATH}.git"
        echo "Pushing to https://${SERVER}/${CI_PROJECT_PATH}.git with an access token"
    else
        mkdir -p ~/.ssh
        {
            echo "Host ${SSH_HOST}"
            echo "	Port ${SSH_PORT}"
            echo "	StrictHostKeyChecking no"
        } >> ~/.ssh/config
        git remote set-url origin "ssh://git@${SSH_HOST}:${SSH_PORT}/${CI_PROJECT_PATH}.git"
        echo "Pushing to ssh://${SSH_HOST}:${SSH_PORT}/${CI_PROJECT_PATH}.git with an SSH key"
    fi
fi

PUSH_OPTIONS=(
    -o merge_request.create
    -o "merge_request.target=${TARGET}"
    -o merge_request.title="docs: refresh ordering information from the product API"
    -o merge_request.description="Scheduled API refresh. Review the updated ordering-info.inc files."
)
if [ -n "${SYNC_MR_ASSIGNEE:-}" ]; then
    PUSH_OPTIONS+=(-o "merge_request.assign=${SYNC_MR_ASSIGNEE}")
fi

git push --force origin "HEAD:refs/heads/${BRANCH}" "${PUSH_OPTIONS[@]}"
