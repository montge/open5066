#!/bin/bash
# Generate license.c from COPYING files

OUTPUT_FILE="$1"

# Start the C string
printf 'char* license = "' > "${OUTPUT_FILE}"

# Add copyright notice
printf 'Copyright (c) 2006 Sampo Kellomaki (sampo@iki.fi), All Rights Reserved.\\n' >> "${OUTPUT_FILE}"

# Process COPYING file - escape backslashes and quotes, add \n at end of each line
sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/$/\\n/' COPYING | tr -d '\n' >> "${OUTPUT_FILE}"

# Process COPYING_sis5066_h file
sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/$/\\n/' COPYING_sis5066_h | tr -d '\n' >> "${OUTPUT_FILE}"

# Close the string
printf '";\n' >> "${OUTPUT_FILE}"
