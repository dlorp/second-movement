#!/usr/bin/env python3
"""
Measure actual compiled flash size for each watch face.
Builds minimal firmware configs with one target face + essential faces.
"""

import json
import os
import re
import subprocess
import sys
from pathlib import Path

# Essential faces needed for functional firmware
# Note: sleep_tracker_face and smart_alarm_face are referenced directly in movement.c
ESSENTIAL_FACES = [
    'settings_face',
    'set_time_face',
    'sleep_tracker_face',
    'smart_alarm_face',
]

# Face dependencies - some faces require other faces to be included
FACE_DEPENDENCIES = {
    'finetune_face': ['nanosec_face'],  # finetune_face calls nanosec_* functions
}

# Faces that require hardware feature flags
HARDWARE_DEPENDENT_FACES = {
    'irda_upload_face': ['HAS_IR_SENSOR'],
    'light_sensor_face': ['HAS_IR_SENSOR'],
}

def load_registry():
    """Load face registry."""
    with open('builder/face_registry.json', 'r') as f:
        return json.load(f)

def generate_config(target_face_id, registry):
    """Generate movement_config.h, movement_faces.h, and watch-faces.mk with minimal face set."""
    # Find the target face
    target_face = next((f for f in registry['faces'] if f['id'] == target_face_id), None)
    if not target_face:
        raise ValueError(f"Face not found: {target_face_id}")
    
    # Build face list: target + dependencies + essentials
    face_ids = [target_face_id]
    
    # Add any required dependencies
    if target_face_id in FACE_DEPENDENCIES:
        for dep in FACE_DEPENDENCIES[target_face_id]:
            if dep not in face_ids:
                face_ids.append(dep)
    
    # Add essentials
    for essential in ESSENTIAL_FACES:
        if essential not in face_ids:
            face_ids.append(essential)
    
    # Generate movement_config.h
    config = """#ifndef MOVEMENT_CONFIG_H_
#define MOVEMENT_CONFIG_H_

#include "movement_faces.h"

const watch_face_t watch_faces[] = {
"""
    
    for face_id in face_ids:
        config += f"    {face_id},\n"
    
    config += """};

#define MOVEMENT_NUM_FACES (sizeof(watch_faces) / sizeof(watch_face_t))
#define MOVEMENT_SECONDARY_FACE_INDEX 0

#define SIGNAL_TUNE_DEFAULT
#define MOVEMENT_DEFAULT_RED_COLOR 0x0
#define MOVEMENT_DEFAULT_GREEN_COLOR 0xF
#define MOVEMENT_DEFAULT_BLUE_COLOR 0x0
#define MOVEMENT_DEFAULT_24H_MODE false
#define MOVEMENT_DEFAULT_BUTTON_SOUND true
#define MOVEMENT_DEFAULT_BUTTON_VOLUME WATCH_BUZZER_VOLUME_SOFT
#define MOVEMENT_DEFAULT_SIGNAL_VOLUME WATCH_BUZZER_VOLUME_LOUD
#define MOVEMENT_DEFAULT_ALARM_VOLUME WATCH_BUZZER_VOLUME_LOUD
#define MOVEMENT_DEFAULT_TIMEOUT_INTERVAL 0
#define MOVEMENT_DEFAULT_LOW_ENERGY_INTERVAL 2
#define MOVEMENT_DEFAULT_LED_DURATION 1
#define MOVEMENT_DEBOUNCE_TICKS 0

#endif // MOVEMENT_CONFIG_H_
"""
    
    # Generate movement_faces.h with only the needed face headers
    faces_h = """#ifndef MOVEMENT_FACES_H_
#define MOVEMENT_FACES_H_

#include "watch.h"

"""
    for face_id in face_ids:
        face = next((f for f in registry['faces'] if f['id'] == face_id), None)
        if face and 'header' in face:
            faces_h += f"#include \"{face['header'][2:]}\"\n"  # Remove ./ prefix
    
    faces_h += "\n#endif // MOVEMENT_FACES_H_\n"
    
    # Generate watch-faces.mk with only the needed faces
    faces_mk = "SRCS += \\\n"
    for face_id in face_ids:
        # Find the face in the registry to get its source path
        face = next((f for f in registry['faces'] if f['id'] == face_id), None)
        if face and 'source' in face:
            faces_mk += f"  {face['source']} \\\n"
    
    # Remove trailing backslash
    faces_mk = faces_mk.rstrip(" \\\n") + "\n"
    
    # Determine if we need hardware flags
    hardware_flags = []
    for face_id in face_ids:
        if face_id in HARDWARE_DEPENDENT_FACES:
            hardware_flags.extend(HARDWARE_DEPENDENT_FACES[face_id])
    
    return config, faces_h, faces_mk, list(set(hardware_flags))

def build_firmware(board='sensorwatch_red', display='classic', extra_cflags=None):
    """Build firmware and return flash size."""
    try:
        # Prepare environment with PATH
        env = os.environ.copy()
        env['PATH'] = '/usr/local/opt/arm-none-eabi-gcc@9/bin:/usr/local/bin:/usr/sbin:/usr/bin:/bin:' + env.get('PATH', '')
        
        # Add extra CFLAGS if needed (for hardware flags)
        if extra_cflags:
            cflags = ' '.join(f'-D{flag}' for flag in extra_cflags)
            env['CFLAGS'] = env.get('CFLAGS', '') + ' ' + cflags
        
        # Manually remove build directory to ensure clean state
        # (avoids issues with make clean and parallel build .d dependency race conditions)
        import shutil
        if os.path.exists('build'):
            # Use ignore_errors to handle any locked/busy files
            shutil.rmtree('build', ignore_errors=True)
        # Don't manually create build directory - let make do it via the 'directory' target
        
        # Try building up to 3 times (workaround for assembler race condition)
        for attempt in range(3):
            result = subprocess.run(
                ['make', '-j1', f'BOARD={board}', f'DISPLAY={display}'],
                cwd=os.getcwd(),
                capture_output=True,
                text=True,
                timeout=600,
                env=env
            )
            
            # Check if firmware.elf was created, even if make returned an error
            if os.path.exists('build/firmware.elf'):
                break
        
        if not os.path.exists('build/firmware.elf'):
            print(f"Build failed: {result.stderr}", file=sys.stderr)
            return None
        
        # Extract size by running arm-none-eabi-size directly on the elf file
        size_result = subprocess.run(
            ['arm-none-eabi-size', 'build/firmware.elf'],
            cwd=os.getcwd(),
            capture_output=True,
            text=True,
            env=env
        )
        
        if size_result.returncode != 0:
            print(f"Size extraction failed: {size_result.stderr}", file=sys.stderr)
            return None
        
        # Parse output like: "   text	   data	    bss	    dec	    hex	filename"
        # Second line: " 115612	   2828	   4604	 123044	  1e0a4	build/firmware.elf"
        lines = size_result.stdout.strip().split('\n')
        if len(lines) >= 2:
            parts = lines[1].split()
            if len(parts) >= 3:
                text_size = int(parts[0])
                data_size = int(parts[1])
                total_size = text_size + data_size  # Flash usage (text + data)
                return total_size
        
        print(f"Could not parse size output: {size_result.stdout}", file=sys.stderr)
        return None
            
    except subprocess.TimeoutExpired:
        print(f"Build timeout", file=sys.stderr)
        return None
    except Exception as e:
        print(f"Build error: {e}", file=sys.stderr)
        return None

def measure_face_size(face_id, registry):
    """Measure flash size for a single face."""
    print(f"Measuring {face_id}...", file=sys.stderr)
    
    # Generate config with just this face + essentials + dependencies
    config, faces_h, faces_mk, hardware_flags = generate_config(face_id, registry)
    
    # Write config
    with open('movement_config.h', 'w') as f:
        f.write(config)
    
    # Write movement_faces.h
    with open('movement_faces.h', 'w') as f:
        f.write(faces_h)
    
    # Write watch-faces.mk
    with open('watch-faces.mk', 'w') as f:
        f.write(faces_mk)
    
    # Build and get size (with hardware flags if needed)
    size = build_firmware(extra_cflags=hardware_flags if hardware_flags else None)
    
    if size is None:
        print(f"  FAILED", file=sys.stderr)
        return None
    
    print(f"  {size} bytes", file=sys.stderr)
    return size

def measure_baseline(registry):
    """Measure baseline size (just essential faces)."""
    print("Measuring baseline (essential faces only)...", file=sys.stderr)
    
    # Generate movement_config.h
    config = """#ifndef MOVEMENT_CONFIG_H_
#define MOVEMENT_CONFIG_H_

#include "movement_faces.h"

const watch_face_t watch_faces[] = {
"""
    
    for face_id in ESSENTIAL_FACES:
        config += f"    {face_id},\n"
    
    config += """};

#define MOVEMENT_NUM_FACES (sizeof(watch_faces) / sizeof(watch_face_t))
#define MOVEMENT_SECONDARY_FACE_INDEX 0

#define SIGNAL_TUNE_DEFAULT
#define MOVEMENT_DEFAULT_RED_COLOR 0x0
#define MOVEMENT_DEFAULT_GREEN_COLOR 0xF
#define MOVEMENT_DEFAULT_BLUE_COLOR 0x0
#define MOVEMENT_DEFAULT_24H_MODE false
#define MOVEMENT_DEFAULT_BUTTON_SOUND true
#define MOVEMENT_DEFAULT_BUTTON_VOLUME WATCH_BUZZER_VOLUME_SOFT
#define MOVEMENT_DEFAULT_SIGNAL_VOLUME WATCH_BUZZER_VOLUME_LOUD
#define MOVEMENT_DEFAULT_ALARM_VOLUME WATCH_BUZZER_VOLUME_LOUD
#define MOVEMENT_DEFAULT_TIMEOUT_INTERVAL 0
#define MOVEMENT_DEFAULT_LOW_ENERGY_INTERVAL 2
#define MOVEMENT_DEFAULT_LED_DURATION 1
#define MOVEMENT_DEBOUNCE_TICKS 0

#endif // MOVEMENT_CONFIG_H_
"""
    
    with open('movement_config.h', 'w') as f:
        f.write(config)
    
    # Generate movement_faces.h with only essential faces
    faces_h = """#ifndef MOVEMENT_FACES_H_
#define MOVEMENT_FACES_H_

#include "watch.h"

"""
    for face_id in ESSENTIAL_FACES:
        face = next((f for f in registry['faces'] if f['id'] == face_id), None)
        if face and 'header' in face:
            faces_h += f"#include \"{face['header'][2:]}\"\n"  # Remove ./ prefix
    
    faces_h += "\n#endif // MOVEMENT_FACES_H_\n"
    
    with open('movement_faces.h', 'w') as f:
        f.write(faces_h)
    
    # Generate watch-faces.mk with only essential faces
    faces_mk = "SRCS += \\\n"
    for face_id in ESSENTIAL_FACES:
        face = next((f for f in registry['faces'] if f['id'] == face_id), None)
        if face and 'source' in face:
            faces_mk += f"  {face['source']} \\\n"
    faces_mk = faces_mk.rstrip(" \\\n") + "\n"
    
    with open('watch-faces.mk', 'w') as f:
        f.write(faces_mk)
    
    size = build_firmware()
    print(f"  Baseline: {size} bytes", file=sys.stderr)
    return size

def main():
    if len(sys.argv) < 2:
        print("Usage: measure_face_sizes.py <face_id> [face_id ...]", file=sys.stderr)
        sys.exit(1)
    
    face_ids = sys.argv[1:]
    registry = load_registry()
    
    # Measure baseline first
    baseline = measure_baseline(registry)
    if baseline is None:
        print("Failed to measure baseline", file=sys.stderr)
        sys.exit(1)
    
    results = {}
    
    # Measure each face
    for face_id in face_ids:
        if face_id in ESSENTIAL_FACES:
            # Essential faces are always included, use baseline
            results[face_id] = baseline
        else:
            total_size = measure_face_size(face_id, registry)
            if total_size is not None:
                # Face size = total - baseline
                # BUT if the face has dependencies, we need to subtract those too
                face_size = total_size - baseline
                
                # If this face required dependencies, subtract their sizes too
                if face_id in FACE_DEPENDENCIES:
                    for dep_id in FACE_DEPENDENCIES[face_id]:
                        if dep_id in results:
                            face_size -= results[dep_id]
                
                results[face_id] = face_size
    
    # Output JSON results
    print(json.dumps(results, indent=2))

if __name__ == '__main__':
    main()
