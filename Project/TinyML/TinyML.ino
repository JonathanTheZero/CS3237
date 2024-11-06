#include <ArduTFLite.h>
#include "model.hpp"


// The Tensor Arena memory area is used by TensorFlow Lite to store input, output and intermediate tensors
// It must be defined as a global array of byte (or u_int8 which is the same type on Arduino)
// The Tensor Arena size must be defined by trials and errors. We use here a quite large value.
// The alignas(16) directive is used to ensure that the array is aligned on a 16-byte boundary,
// this is important for performance and to prevent some issues on ARM microcontroller architectures.
constexpr int kTensorArenaSize = 8 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
const int inputLength = 8;  // Number of input values
const char* LABELS[] = {
  "no_contact",
  "leaning_back",
  "slouching",
  "backward_lean"
};


void setup() {
  // Initialize serial communications and wait for Serial Monitor to be opened
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("Sine(x) function inference example.");
  Serial.println("Initializing TensorFlow Lite Micro Interpreter...");
  if (!modelInit(__output_model_tflite, tensor_arena, kTensorArenaSize)) {
    Serial.println("Model initialization failed!");
    while (true)
      ;
  }
  Serial.println("Model initialization done.");
  Serial.println();

   float inputData[inputLength] = {19047,17051,18310,16747,0,1489,870,0};

  // Load data into the model's input tensor
  for (int i = 0; i < inputLength; i++) {
    modelSetInput(inputData[i], i);
  }

  // Run inference
  if (!modelRunInference()) {
    Serial.println("Inference failed!");
    return;
  }

  // Print output probabilities
  for (int i = 0; i < sizeof(LABELS) / sizeof(LABELS[0]); i++) {
    Serial.print(i);
    Serial.print(LABELS[i]);
    Serial.print(": ");
    Serial.print(modelGetOutput(i) * 100, 2);
    Serial.println("%");
  }
}


void loop() {
}