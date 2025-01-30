#!/bin/bash

# Find all .c and .h files and apply clang-format
find . -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +

echo "All C and header files have been formatted."