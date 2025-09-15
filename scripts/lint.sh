#!/bin/bash

FILES_TO_FORMAT_WITH_CLANG_FORMAT="tracing-.+/.+\.(cpp|c|h|hpp)$"

if [[ "$1" == "--only-changed" ]]; then
    FILES=$(git diff --cached --name-only --diff-filter=ACMR | sed 's| |\\ |g')
    [ -z "$FILES" ] && exit 0 # Exit if no staged files
else
    FILES=$(git ls-files)
fi

# Check if clang-format is installed
if command -v clang-format &>/dev/null; then
    echo "$FILES" | grep -E "$FILES_TO_FORMAT_WITH_CLANG_FORMAT"  | xargs -P0 -I{} clang-format -i {} >/dev/null 2>&1
fi
