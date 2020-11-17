#pragma once
#include "tensorflow/lite/micro/micro_interpreter.h"
void tflite_model_setup();

float* tflite_model_predict(float *buffer);
