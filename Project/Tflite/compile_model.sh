#!/bin/bash

# This file serves as a helper script for the conversion from a TFLITE model
#   to a C++ header file (also as a bash coding exercise hehe - it's fun!)
# It also adds a directive that is helpful for the Arduino code and a comment
#   with more information and some metadata to the file.
# Additionally I decided to have it include all the definitions the model uses as well.
# I believe it fit's better to have them all in one place,
#  like this the model file by itself is basically shippable.
# It uses Linux built in tools and commands and thus needs to be run on Linux
#   or (as in my case) WSL - Windows won't work here.

SCRIPT_PATH="./tflite_model.py"
MODEL_PATH="./output/model.tflite"
OUTPUT_PATH="./../TinyML/model.hpp"
TEMP_FILE=$(mktemp)

main() {

  if [ ! -f "$SCRIPT_PATH" ]; then
    echo "Error: Python script file '$SCRIPT_PATH' not found."
    exit 1
  fi

  # Run python script
  python3 "$SCRIPT_PATH"

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
  # Additionaly add byte size and timestamp as metadata in the comment
  # Also add definitions for labels and as strings and as enum to the file
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

  Additionally, the labels are included as an enum and a string array to work with.
  
  Model size: $MODEL_SIZE bytes (HEX dump only - excluding definitions)
  Generated on: $TIMESTAMP (GMT+08)
*/


const char *LABELS[] = {
  "no_contact",
  "perfect_posture",
  "leaning_back",
  "slouching",
  "leaning_forward",
  "leaning_left",
  "leaning_right"
};

enum Positions {
  NO_CONTACT = 0,
  PERFECT_POSTURE = 1,
  LEANING_BACK = 2,
  SLOUCHING = 3,
  LEANING_FORWARD = 4,
  LEANING_LEFT = 5,
  LEANING_RIGHT = 6
};


EOF
    cat "$OUTPUT_PATH"
  } >"$TEMP_FILE" && mv "$TEMP_FILE" "$OUTPUT_PATH"

  echo "Successfully added directive and info text to the beginning of the model header file."
}

main
