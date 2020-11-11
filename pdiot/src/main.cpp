/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <logging/log.h>
#include "neural_network.h"
#include "accelerometer.h"

static const double sampling_rate = 12.5;

LOG_MODULE_REGISTER(main);

void main(void)
{

    // setup variables for the tflite model
    static float input_tensor_array[50 * 3];
    int counter = 0;

    // setup TFLite Model
    tflite_model_setup();

    // Setup Accelerometer Device
    const struct device *sensor = device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh)));
    if (sensor == NULL)
    {
        printf("Could not get %s device\n",
               DT_LABEL(DT_INST(0, st_lis2dh)));
        return;
    }

    while (true)
    {
        if (counter == 50)
        {
            tflite_model_predict(input_tensor_array);
            // TODO - send predicted results over bluetooth
            counter = 0;
            LOG_INF("Bluetooth initialized");
        }
        else
        {

            AccelerometerData acc_data = fetch_data(sensor);
            input_tensor_array[(counter * 3)] = acc_data.x;
            input_tensor_array[(counter * 3) + 1] = acc_data.y;
            input_tensor_array[(counter * 3) + 2] = acc_data.z;

            counter++;

            k_sleep(K_MSEC((int)1000 / sampling_rate));
        }
    }
}