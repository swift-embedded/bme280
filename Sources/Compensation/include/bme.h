/**
 * Copyright (C) 2018 - 2019 Bosch Sensortec GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the copyright holder nor the names of the
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 * The information provided is believed to be accurate and reliable.
 * The copyright holder assumes no responsibility
 * for the consequences of use
 * of such information nor for any infringement of patents or
 * other rights of third parties which may result from its use.
 * No license is granted by implication or otherwise under any patent or
 * patent rights of the copyright holder.
 *
 * @file    bme280.h
 * @date    08 Mar 2019
 * @version 3.3.6
 * @brief
 *
 */

/*! @file bme280.h
 * @brief Sensor driver for BME280 sensor
 */

/*!
 * @defgroup BME280 SENSOR API
 */
#ifndef BME280_H_
#define BME280_H_

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

/* Header includes */
#include "bme_defs.h"

/*!
 *  @brief This API is used to parse the pressure, temperature and
 *  humidity data and store it in the bme280_uncomp_data structure instance.
 *
 *  @param[in] reg_data     : Contains register data which needs to be parsed
 *  @param[out] uncomp_data : Contains the uncompensated pressure, temperature
 *  and humidity data.
 */
void bme280_parse_sensor_data(const uint8_t *reg_data, struct bme280_uncomp_data *uncomp_data);

/*!
 * @brief This API is used to compensate the pressure and/or
 * temperature and/or humidity data according to the component selected by the
 * user.
 *
 * @param[in] sensor_comp : Used to select pressure and/or temperature and/or
 * humidity.
 * @param[in] uncomp_data : Contains the uncompensated pressure, temperature and
 * humidity data.
 * @param[out] comp_data : Contains the compensated pressure and/or temperature
 * and/or humidity data.
 * @param[in] calib_data : Pointer to the calibration data structure.
 *
 * @return Result of API execution status.
 * @retval zero -> Success / -ve value -> Error
 */
int8_t bme280_compensate_data(uint8_t sensor_comp,
                              const struct bme280_uncomp_data *uncomp_data,
                              struct bme280_data *comp_data,
                              struct bme280_calib_data *calib_data);

void parse_humidity_calib_data(const uint8_t *reg_data, struct bme280_calib_data *calib_data);

void parse_temp_press_calib_data(const uint8_t *reg_data, struct bme280_calib_data *calib_data);
#ifdef __cplusplus
}
#endif /* End of CPP guard */
#endif /* BME280_H_ */
/** @}*/
