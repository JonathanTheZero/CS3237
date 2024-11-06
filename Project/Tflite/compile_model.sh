#!/bin/bash

# This file serves as a helper script for the conversion from a TFLITE model
#   to a C++ header file (also as a bash coding exercise hehe - it's fun!)
# It also adds a directive that is helpful for the Arduino code and a comment
#   with more information and some metadata to the file
# It uses Linux built in tools and commands and thus needs to be run on Linux
#   or (as in my case) WSL - Windows won't work here

MODEL_PATH="./output/model.tflite"
OUTPUT_PATH="./../TinyML/model.hpp"
TEMP_FILE=$(mktemp)

main() {
  if [ ! -f "$MODEL_PATH" ]; then
    echo "Error: Model file '$MODEL_PATH' not found."
    exit 1
  fi

  # Convert model to C++ header using xxd
  xxd -i "$MODEL_PATH" >"$OUTPUT_PATH"

  if [ $? -ne 0 ]; then
    echo "Error: Failed to convert model to C++ header."
    exit 1
  fi
  echo "Model compiled and saved to output path: $OUTPUT_PATH"

  # Add `alignas(16)` directive in front of the array to ensure that the array is aligned on a 16-byte boundary
  # This is important for performance and to prevent some issues on ARM microcontroller architectures.
  sed -i "1s/^/alignas(16) /" "$OUTPUT_PATH"
  if [ $? -ne 0 ]; then
    echo "Error: Failed to add alignment directive."
    exit 1
  fi

  TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
  MODEL_SIZE=$(stat -c%s "$MODEL_PATH")

  # Add the comment from above + some extra auto-generated information to the HEX dump
  # Credit for the info text goes to the library lol - it is quite useful to include it here
  # Also add byte size and timestamp as metadata in the comment
  {
    cat <<EOF
/* 
  This header file contains the definition of the unsigned char array that represents 
  the binary format of the TensorFlow Lite model to be run by the Arduino sketch. 
  This TensorFlow Lite model was generated on an external development system,
  where the original Deep Neural Network model was created and trained. 
  The trained model was saved in .tflite format and its hexadecimal dump was used to build 
  the following declaration with initialization of the model array. 
  The alignas(16) directive is used to ensure that the array is aligned on a 16-byte boundary, 
  which is important for performance and to prevent some issues on ARM microcontroller architectures.
  
  Model size: $MODEL_SIZE bytes
  Generated on: $TIMESTAMP
*/
EOF
    cat "$OUTPUT_PATH"
  } >"$TEMP_FILE" && mv "$TEMP_FILE" "$OUTPUT_PATH"

  echo "Successfully added directive and info text to the beginning of the model."
}

main
