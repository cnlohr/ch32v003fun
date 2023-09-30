//
// BME280 temperature/pressure/humidity sensor
// code to initialize and read+calculate the calibrated values
// from the I2C bus
//
// Written by Larry Bank - 2/1/2017
// Copyright (c) 2017 BitBank Software, Inc.
// bitbank@pobox.com
// updated 2/13/2018 for Arduino
//
//
// Initialized the I2C device
// reads the calibration data and sets the device
// into auto sensing mode
// If device not present or problems, returns -1
// otherwise returns 0
//
int bme280Init(void);
//
// Read the sensor register values
// and translate them into calibrated readings
// using the previously loaded calibration data
// The values are integers instead of floats, but scaled to include more precision
// Temperature is expressed in Celsius degrees as T * 100 (for 2 decimal places)
// Pressure is 256 times the integer value
// Humidity is express as H * 1024 (10 bit fraction)
//
int bme280ReadValues(uint32_t *T, uint32_t *P, uint32_t *H);

