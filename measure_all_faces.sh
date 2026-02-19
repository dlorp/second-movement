#!/bin/bash
# Measure all watch faces systematically

# Get all face IDs from registry
FACES=$(cat builder/face_registry.json | jq -r '.faces[].id' | sort)

# Skip hardware-specific faces that can't build on default board
SKIP_FACES=("irda_upload_face" "light_sensor_face")

# Collect measurable faces
MEASURABLE=()
for face in $FACES; do
    skip=false
    for skip_face in "${SKIP_FACES[@]}"; do
        if [ "$face" == "$skip_face" ]; then
            skip=true
            break
        fi
    done
    if [ "$skip" == "false" ]; then
        MEASURABLE+=("$face")
    fi
done

echo "Measuring ${#MEASURABLE[@]} faces (skipping ${#SKIP_FACES[@]} hardware-specific faces)..." >&2

# Run measurement
python3 measure_face_sizes.py "${MEASURABLE[@]}"
