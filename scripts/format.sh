#!/bin/bash
# formats every c source and header with the repository's .clang-format
set -e
cd "$(dirname "$0")/.."
dirs=()
for dir in include src tests examples; do
  [ -d "$dir" ] && dirs+=("$dir")
done
find "${dirs[@]}" -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +
echo "formatting complete"
