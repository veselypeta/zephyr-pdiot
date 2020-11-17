#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "gyro_lite_model.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    int inference_count = 0;

    // Create an area of memory to use for input, output, and intermediate arrays.
    // Minimum arena size, at the time of writing. After allocating tensors
    // you can retrieve this value by invoking interpreter.arena_used_bytes().
    // TODO - try and reduce
    const int kTensorArenaSize = 30 * 1024;
    uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void tflite_model_setup()
{

    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(gyroscope_model_1_layer_small_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "model provided is schema version %d not equal to supported version %d",
                             model->version(),
                             TFLITE_SCHEMA_VERSION);
        return;
    }

    // Setup Resolver
    static tflite::MicroMutableOpResolver<10> resolver;
    resolver.AddConv2D();
    resolver.AddMaxPool2D();
    resolver.AddFullyConnected();
    resolver.AddSoftmax();
    resolver.AddRelu();
    resolver.AddAdd();
    resolver.AddReshape();

    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    inference_count = 0;
}

float *tflite_model_predict(float *buffer)
{

    // place input into input tensor
    float *x = interpreter->typed_input_tensor<float>(0);
    memcpy(x, buffer, 150);

    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "INVOKE failed on interpreter");
        return NULL;
    }

    float *y = interpreter->typed_output_tensor<float>(0);

    printf("----- OUTPUT ----- \n");
    printf("%f , %f , %f , total=%f\n", y[0], y[1], y[2], y[0] + y[1] + y[2]);

    return y;
}
