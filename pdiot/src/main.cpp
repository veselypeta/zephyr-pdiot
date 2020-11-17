#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <logging/log.h>
#include "neural_network.h"
#include "accelerometer.h"
#include "tfservice.h"
#include <bluetooth/conn.h>
#include <bluetooth/bluetooth.h>

LOG_MODULE_REGISTER(main);

void bluetoth_setup()
{
    printf("Starting Bluetooth Setup!");
    bt_conn_cb_register(&conn_callbacks);
    int err = bt_enable(bt_ready);
    if (err)
    {
        printk("Bluetooth init failed (err %d", err);
    }
}

void tflite_send_ble_notif()
{
    if (conn)
    {
        if (notify)
        {
            int err = bt_gatt_notify(NULL, &(stsensor_svc.attrs[1]), &tf_buffer, sizeof(tf_buffer[0]) * tf_buffer_len);
        }
    }
}

void copy_data_to_ble_buffer(float *preds)
{
    for (int i = 0; i < tf_buffer_len; i++)
    {
        uint16_t val = preds[i] * 1000;
        tf_buffer[i] = val;
    }
}

void main(void)
{
    bluetoth_setup();

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
            float* res_buf = tflite_model_predict(input_tensor_array);
            copy_data_to_ble_buffer(res_buf);
            tflite_send_ble_notif();
            counter = 0;
        }
        else
        {

            AccelerometerData acc_data = fetch_data(sensor);
            input_tensor_array[(counter * 3)] = acc_data.x;
            input_tensor_array[(counter * 3) + 1] = acc_data.y;
            input_tensor_array[(counter * 3) + 2] = acc_data.z;

            counter++;

            k_sleep(K_MSEC(80 /*12.5 Hz*/));
        }
    }
}
