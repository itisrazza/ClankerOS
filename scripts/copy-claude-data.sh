#!/usr/bin/env bash

#
# Copy Claude data into repo.
# (this one was actually written by yours truly)
#

set -eu

path="$(dirname "$0")/.."
path="$(realpath "$path")"

claude_project_path="$(echo "$path" | sed 's/\//-/g')"

cp "$HOME/.claude/projects/$claude_project_path"/* "$path/docs/outputs"
