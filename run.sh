#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <input_image> [output_directory]"
    exit 1
fi

input_image="$1"
output_directory="${2:-$(dirname "$input_image")}"

if [[ ! -f "$input_image" ]]; then
    echo "Error: Input image does not exist: $input_image"
    exit 1
fi

mkdir -p "$output_directory"

input_absolute="$(cd "$(dirname "$input_image")" && pwd)/$(basename "$input_image")"
output_absolute="$(cd "$output_directory" && pwd)"

docker run --rm \
    -v "$input_absolute:/input/image:ro" \
    -v "$output_absolute:/output" \
    saber-edge-detector \
    /input/image \
    /output/sobel_output.png
