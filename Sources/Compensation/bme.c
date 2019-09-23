#include "bme.h"
#include <stdint.h>

/*!
 *  @brief This API is used to parse the pressure, temperature and
 *  humidity data and store it in the bme280_uncomp_data structure instance.
 */
void bme280_parse_sensor_data(const uint8_t *reg_data, struct bme280_uncomp_data *uncomp_data)
{
    /* Variables to store the sensor data */
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;

    /* Store the parsed register values for pressure data */
    data_msb = (uint32_t)reg_data[0] << 12;
    data_lsb = (uint32_t)reg_data[1] << 4;
    data_xlsb = (uint32_t)reg_data[2] >> 4;
    uncomp_data->pressure = data_msb | data_lsb | data_xlsb;

    /* Store the parsed register values for temperature data */
    data_msb = (uint32_t)reg_data[3] << 12;
    data_lsb = (uint32_t)reg_data[4] << 4;
    data_xlsb = (uint32_t)reg_data[5] >> 4;
    uncomp_data->temperature = data_msb | data_lsb | data_xlsb;

    /* Store the parsed register values for temperature data */
    data_lsb = (uint32_t)reg_data[6] << 8;
    data_msb = (uint32_t)reg_data[7];
    uncomp_data->humidity = data_msb | data_lsb;
}

/*!
 * @brief This internal API is used to compensate the raw temperature data and
 * return the compensated temperature data in integer data type.
 */
static int32_t compensate_temperature(const struct bme280_uncomp_data *uncomp_data,
                                      struct bme280_calib_data *calib_data)
{
    int32_t var1;
    int32_t var2;
    int32_t temperature;
    int32_t temperature_min = -4000;
    int32_t temperature_max = 8500;

    var1 = (int32_t)((uncomp_data->temperature / 8) - ((int32_t)calib_data->dig_T1 * 2));
    var1 = (var1 * ((int32_t)calib_data->dig_T2)) / 2048;
    var2 = (int32_t)((uncomp_data->temperature / 16) - ((int32_t)calib_data->dig_T1));
    var2 = (((var2 * var2) / 4096) * ((int32_t)calib_data->dig_T3)) / 16384;
    calib_data->t_fine = var1 + var2;
    temperature = (calib_data->t_fine * 5 + 128) / 256;
    if (temperature < temperature_min)
    {
        temperature = temperature_min;
    }
    else if (temperature > temperature_max)
    {
        temperature = temperature_max;
    }

    return temperature;
}


/*!
 * @brief This internal API is used to compensate the raw pressure data and
 * return the compensated pressure data in integer data type with higher
 * accuracy.
 */
static uint32_t compensate_pressure(const struct bme280_uncomp_data *uncomp_data,
                                    const struct bme280_calib_data *calib_data)
{
    int64_t var1;
    int64_t var2;
    int64_t var3;
    int64_t var4;
    uint32_t pressure;
    uint32_t pressure_min = 3000000;
    uint32_t pressure_max = 11000000;

    var1 = ((int64_t)calib_data->t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib_data->dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib_data->dig_P5) * 131072);
    var2 = var2 + (((int64_t)calib_data->dig_P4) * 34359738368);
    var1 = ((var1 * var1 * (int64_t)calib_data->dig_P3) / 256) + ((var1 * ((int64_t)calib_data->dig_P2) * 4096));
    var3 = ((int64_t)1) * 140737488355328;
    var1 = (var3 + var1) * ((int64_t)calib_data->dig_P1) / 8589934592;

    /* To avoid divide by zero exception */
    if (var1 != 0)
    {
        var4 = 1048576 - uncomp_data->pressure;
        var4 = (((var4 * INT64_C(2147483648)) - var2) * 3125) / var1;
        var1 = (((int64_t)calib_data->dig_P9) * (var4 / 8192) * (var4 / 8192)) / 33554432;
        var2 = (((int64_t)calib_data->dig_P8) * var4) / 524288;
        var4 = ((var4 + var1 + var2) / 256) + (((int64_t)calib_data->dig_P7) * 16);
        pressure = (uint32_t)(((var4 / 2) * 100) / 128);
        if (pressure < pressure_min)
        {
            pressure = pressure_min;
        }
        else if (pressure > pressure_max)
        {
            pressure = pressure_max;
        }
    }
    else
    {
        pressure = pressure_min;
    }

    return pressure;
}


/*!
 * @brief This internal API is used to compensate the raw humidity data and
 * return the compensated humidity data in integer data type.
 */
static uint32_t compensate_humidity(const struct bme280_uncomp_data *uncomp_data,
                                    const struct bme280_calib_data *calib_data)
{
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t var4;
    int32_t var5;
    uint32_t humidity;
    uint32_t humidity_max = 102400;

    var1 = calib_data->t_fine - ((int32_t)76800);
    var2 = (int32_t)(uncomp_data->humidity * 16384);
    var3 = (int32_t)(((int32_t)calib_data->dig_H4) * 1048576);
    var4 = ((int32_t)calib_data->dig_H5) * var1;
    var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
    var2 = (var1 * ((int32_t)calib_data->dig_H6)) / 1024;
    var3 = (var1 * ((int32_t)calib_data->dig_H3)) / 2048;
    var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
    var2 = ((var4 * ((int32_t)calib_data->dig_H2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((int32_t)calib_data->dig_H1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    humidity = (uint32_t)(var5 / 4096);
    if (humidity > humidity_max)
    {
        humidity = humidity_max;
    }

    return humidity;
}

/*!
 * @brief This API is used to compensate the pressure and/or
 * temperature and/or humidity data according to the component selected
 * by the user.
 */
int8_t bme280_compensate_data(uint8_t sensor_comp,
                              const struct bme280_uncomp_data *uncomp_data,
                              struct bme280_data *comp_data,
                              struct bme280_calib_data *calib_data)
{
    int8_t rslt = BME280_OK;

    if ((uncomp_data != NULL) && (comp_data != NULL) && (calib_data != NULL))
    {
        /* Initialize to zero */
        comp_data->temperature = 0;
        comp_data->pressure = 0;
        comp_data->humidity = 0;

        /* If pressure or temperature component is selected */
        if (sensor_comp & (BME280_PRESS | BME280_TEMP | BME280_HUM))
        {
            /* Compensate the temperature data */
            comp_data->temperature = compensate_temperature(uncomp_data, calib_data);
        }
        if (sensor_comp & BME280_PRESS)
        {
            /* Compensate the pressure data */
            comp_data->pressure = compensate_pressure(uncomp_data, calib_data);
        }
        if (sensor_comp & BME280_HUM)
        {
            /* Compensate the humidity data */
            comp_data->humidity = compensate_humidity(uncomp_data, calib_data);
        }
    }
    else
    {
        rslt = BME280_E_NULL_PTR;
    }

    return rslt;
}


/*!
 *  @brief This internal API is used to parse the temperature and
 *  pressure calibration data and store it in device structure.
 */
void parse_temp_press_calib_data(const uint8_t *reg_data, struct bme280_calib_data *calib_data)
{
    calib_data->dig_T1 = BME280_CONCAT_BYTES(reg_data[1], reg_data[0]);
    calib_data->dig_T2 = (int16_t)BME280_CONCAT_BYTES(reg_data[3], reg_data[2]);
    calib_data->dig_T3 = (int16_t)BME280_CONCAT_BYTES(reg_data[5], reg_data[4]);
    calib_data->dig_P1 = BME280_CONCAT_BYTES(reg_data[7], reg_data[6]);
    calib_data->dig_P2 = (int16_t)BME280_CONCAT_BYTES(reg_data[9], reg_data[8]);
    calib_data->dig_P3 = (int16_t)BME280_CONCAT_BYTES(reg_data[11], reg_data[10]);
    calib_data->dig_P4 = (int16_t)BME280_CONCAT_BYTES(reg_data[13], reg_data[12]);
    calib_data->dig_P5 = (int16_t)BME280_CONCAT_BYTES(reg_data[15], reg_data[14]);
    calib_data->dig_P6 = (int16_t)BME280_CONCAT_BYTES(reg_data[17], reg_data[16]);
    calib_data->dig_P7 = (int16_t)BME280_CONCAT_BYTES(reg_data[19], reg_data[18]);
    calib_data->dig_P8 = (int16_t)BME280_CONCAT_BYTES(reg_data[21], reg_data[20]);
    calib_data->dig_P9 = (int16_t)BME280_CONCAT_BYTES(reg_data[23], reg_data[22]);
    calib_data->dig_H1 = reg_data[25];
}


/*!
 *  @brief This internal API is used to parse the humidity calibration data
 *  and store it in device structure.
 */
void parse_humidity_calib_data(const uint8_t *reg_data, struct bme280_calib_data *calib_data)
{
    int16_t dig_H4_lsb;
    int16_t dig_H4_msb;
    int16_t dig_H5_lsb;
    int16_t dig_H5_msb;

    calib_data->dig_H2 = (int16_t)BME280_CONCAT_BYTES(reg_data[1], reg_data[0]);
    calib_data->dig_H3 = reg_data[2];
    dig_H4_msb = (int16_t)(int8_t)reg_data[3] * 16;
    dig_H4_lsb = (int16_t)(reg_data[4] & 0x0F);
    calib_data->dig_H4 = dig_H4_msb | dig_H4_lsb;
    dig_H5_msb = (int16_t)(int8_t)reg_data[5] * 16;
    dig_H5_lsb = (int16_t)(reg_data[4] >> 4);
    calib_data->dig_H5 = dig_H5_msb | dig_H5_lsb;
    calib_data->dig_H6 = (int8_t)reg_data[6];
}
