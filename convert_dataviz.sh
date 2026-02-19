#!/bin/bash
# Convert Dataism III EPS files to PNG for Phase 3

set -e

cd ~/repos/second-movement/assets/dataviz

echo "Converting 5 smallest Dataism III EPS files to PNG (800px)..."

# Pick 5 smallest files for optimal file size
FILES=(
    "E3931_13.eps"  # 2.4 MB
    "E3931_21.eps"  # 2.9 MB
    "E3931_06.eps"  # 3.3 MB
    "E3931_12.eps"  # 3.3 MB
    "E3931_08.eps"  # 4.9 MB
)

COUNT=1
for f in "${FILES[@]}"; do
    echo "Converting $f..."
    convert "$f" -resize 800x800 -background none -flatten \
        "../../builder/assets/dataviz/dataviz-$(printf '%02d' $COUNT).png"
    ls -lh "../../builder/assets/dataviz/dataviz-$(printf '%02d' $COUNT).png"
    COUNT=$((COUNT + 1))
done

echo ""
echo "✓ Converted 5 data viz graphics"
echo "Total size:"
du -sh ../../builder/assets/dataviz/
