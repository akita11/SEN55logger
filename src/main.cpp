/*
 * Copyright (c) 2021, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>
#include "sen55.h"
#include <M5Unified.h>ı

// The used commands use up to 48 bytes. On some Arduino's the default buffer
// space is not large enough
#define MAXBUF_REQUIREMENT 48

#if (defined(I2C_BUFFER_LENGTH) &&				 \
	(I2C_BUFFER_LENGTH >= MAXBUF_REQUIREMENT)) || \
	(defined(BUFFER_LENGTH) && BUFFER_LENGTH >= MAXBUF_REQUIREMENT)
#define USE_PRODUCT_INFO
#endif

SensirionI2CSen5x sen5x;

void setup() {
	Serial.begin(115200);
	Wire.begin();
	sen5x.begin(Wire);
	M5.begin();

	uint16_t error;
	error = sen5x.deviceReset();
	if (error) printSen55ErrorMessage("Error trying to execute deviceReset():", error);

#ifdef USE_PRODUCT_INFO
	printSen55SerialNumber();
	printSen55ModuleVersions();
#endif

	float tempOffset = 0.0;
	error = sen5x.setTemperatureOffsetSimple(tempOffset);
	if (error) printSen55ErrorMessage("Error trying to execute setTemperatureOffsetSimple():", error);
	else {
		printf("Temperature Offset set to %f[degC]\n", tempOffset);
	}

	// Start Measurement
	error = sen5x.startMeasurement();
	if (error) printSen55ErrorMessage("Error trying to execute startMeasurement():", error);
}

uint16_t px = 0;
#define X 320
uint16_t val[X][8]; // 8 values: PM1.0, PM2.5, PM4.0, PM10.0, RH, T, VOC, NOx
uint16_t color[] = {RED, PURPLE, MAGENTA, ORANGE, CYAN, YELLOW, GREEN, BLUE};

uint16_t conv_value(float value, float min, float max) {
	int16_t v = (value - min) / (max - min) * 65535;
	if (v < min) v = min;
	else if (v > max) v = max;
	return(v);
}	

void loop() {
  uint16_t error;

	delay(1000); // sensor sampling: per 1s

	float massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex, noxIndex;

	error = sen5x.readMeasuredValues(
		massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0,
		ambientHumidity, ambientTemperature, vocIndex, noxIndex);

	// value range:
	// VOX, NOX: 1-500 (10-30sec) [index]
	// massConcentration: 0-1000 [ug/m3]

	if (error) printSen55ErrorMessage("Error trying to execute readMeasuredValues():", error);
	else {
		printf("%f,%f,%f,%f,", massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0);
	  if (isnan(ambientHumidity)) printf("n/a,"); else printf("%f, ", ambientHumidity);
	  if (isnan(ambientTemperature)) printf("n/a,"); else printf("%f, ", ambientTemperature);
	  if (isnan(vocIndex)) printf("n/a,"); else printf("%f, ", vocIndex);
	  if (isnan(noxIndex)) printf("n/a\n"); else printf("%f\n", noxIndex);
		val[px][0] = conv_value(massConcentrationPm1p0, 0, 1000);
		val[px][1] = conv_value(massConcentrationPm2p5, 0, 1000);
		val[px][2] = conv_value(massConcentrationPm4p0, 0, 1000);
		val[px][3] = conv_value(massConcentrationPm10p0, 0, 1000);
		val[px][4] = conv_value(ambientHumidity, 0, 100);
		val[px][5] = conv_value(ambientTemperature, 0, 100);
		val[px][6] = conv_value(vocIndex, 1, 500);
		val[px][6] = conv_value(noxIndex, 1, 500);
		px = (px + 1) % X;
		uint16_t x, p;
		p = px;
		for (x = 0; x < X; x++){
			uint8_t t = 0;
			M5.Display.drawFastVLine(x, 0, 240, BLACK);
			for (t = 0; t < 8; t++) {
				M5.Display.drawPixel(x, 240 - val[p][t], color[t]);
			}
			p = (p + 1) % X;
		}
	}
}