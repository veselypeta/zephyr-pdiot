#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include "accelerometer.h"

AccelerometerData fetch_data(const struct device *sensor)
{
	static unsigned int count;
	struct sensor_value accel[3];
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == -EBADMSG)
	{
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER))
		{
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}
	if (rc == 0)
	{
		rc = sensor_channel_get(sensor,
								SENSOR_CHAN_ACCEL_XYZ,
								accel);
	}
	if (rc < 0)
	{
		return AccelerometerData({-1, -1, -1});
	}

	double scaler = ((double)SENSOR_G) / 1000000;

	float x = sensor_value_to_double(&accel[0]) / scaler;
	float y = sensor_value_to_double(&accel[1]) / scaler;
	float z = sensor_value_to_double(&accel[2]) / scaler;

	return AccelerometerData({x, y, z});
}
