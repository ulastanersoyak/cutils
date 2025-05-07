#!/bin/bash

if [ -d "includes" ]; then
    echo "Formatting files in includes directory..."
    find includes -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cpp" \) -exec clang-format -i {} \;
else
    echo "Warning: includes directory not found."
fi

if [ -d "src" ]; then
    echo "Formatting files in src directory..."
    find src -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cpp" \) -exec clang-format -i {} \;
else
    echo "Warning: src directory not found."
fi

echo "Formatting complete!"
