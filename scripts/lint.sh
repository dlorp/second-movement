#!/bin/bash
# Run clang-tidy on second-movement codebase
# Usage: ./scripts/lint.sh [file or directory]

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if clang-tidy is installed
if ! command -v clang-tidy &> /dev/null; then
    echo -e "${RED}Error: clang-tidy not found${NC}"
    echo "Install with: brew install llvm"
    exit 1
fi

# Determine what to check
if [ $# -eq 0 ]; then
    # No arguments - check all C files (exclude tests)
    FILES=$(find watch-faces lib -name "*.c" -o -name "*.h" | \
            grep -v "test/unity" | \
            grep -v "test/watch_tcc" || true)
    echo -e "${GREEN}Running clang-tidy on all C files${NC}"
else
    # Check specific file or directory
    if [ -d "$1" ]; then
        FILES=$(find "$1" -name "*.c" -o -name "*.h" || true)
        echo -e "${GREEN}Running clang-tidy on directory: $1${NC}"
    elif [ -f "$1" ]; then
        FILES="$1"
        echo -e "${GREEN}Running clang-tidy on file: $1${NC}"
    else
        echo -e "${RED}Error: $1 is not a file or directory${NC}"
        exit 1
    fi
fi

if [ -z "$FILES" ]; then
    echo -e "${YELLOW}No C files found to check${NC}"
    exit 0
fi

# Count files
FILE_COUNT=$(echo "$FILES" | wc -l | xargs)
echo -e "${YELLOW}Checking $FILE_COUNT files...${NC}"
echo

# Run clang-tidy
WARNINGS=0
for file in $FILES; do
    echo -e "${YELLOW}▸ $file${NC}"
    
    if clang-tidy "$file" -- \
        -I. \
        -Iwatch-faces/complication \
        -Ilib \
        -DHAS_IR_SENSOR \
        -D__ARM_ARCH_6M__ 2>&1 | grep -i "warning:\|error:" || true; then
        WARNINGS=$((WARNINGS + 1))
    fi
done

echo
if [ $WARNINGS -eq 0 ]; then
    echo -e "${GREEN}✓ No warnings found!${NC}"
    exit 0
else
    echo -e "${YELLOW}⚠ Found warnings in $WARNINGS files${NC}"
    echo -e "${YELLOW}Note: Warnings are informational, not blocking${NC}"
    exit 0
fi
