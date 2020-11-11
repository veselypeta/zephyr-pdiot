#pragma once

struct AccelerometerData
{
    float x;
    float y;
    float z;
};

AccelerometerData fetch_data(const struct device *sensor);