#!/bin/bash
# Generate license.c from COPYING files
# Robust version that handles all edge cases

OUTPUT_FILE="$1"

if [ -z "$OUTPUT_FILE" ]; then
    echo "Usage: $0 <output_file>"
    exit 1
fi

# Create a temporary file for building the license string
TEMP_FILE="${OUTPUT_FILE}.tmp"

{
    echo 'char* license = '

    # Copyright header
    echo '"Copyright (c) 2006 Sampo Kellomaki (sampo@iki.fi), All Rights Reserved.\\n"'

    # Process COPYING file - escape quotes and backslashes, add \n at end
    if [ -f COPYING ]; then
        sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/^/"/' -e 's/$/\\n"/' COPYING
    fi

    # Process COPYING_sis5066_h file
    if [ -f COPYING_sis5066_h ]; then
        sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/^/"/' -e 's/$/\\n"/' COPYING_sis5066_h
    fi

    # Close the string
    echo ';'
} > "$TEMP_FILE"

# Move temp file to output only if successful
if [ $? -eq 0 ]; then
    mv "$TEMP_FILE" "$OUTPUT_FILE"
    exit 0
else
    rm -f "$TEMP_FILE"
    exit 1
fi
