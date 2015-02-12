/*
 * @file	tsl2561.c
 * @version	1.0
 * @author	DinsFire64 (dinsfire.com)
 *
 * calculateLux taken from the Adafruit library. The license for that method is below.
 *
 * @section	LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 DinsFire64
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**************************************************************************/
/*!
 @file     tsl2561.c
 @author   K. Townsend (microBuilder.eu / adafruit.com)

 @section LICENSE

 Software License Agreement (BSD License)

 Copyright (c) 2010, microBuilder SARL, Adafruit Industries
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. Neither the name of the copyright holders nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**************************************************************************/

#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "c_types.h"
#include "user_interface.h"
#include "driver/tsl2561.h"
#include "driver/i2c.h"

/*
 * Function:  ScanForDevices
 * --------------------
 * Searches for connected i2c devices.
 * Debugging purposes only, simply outputs the address over UART
 */
void ICACHE_FLASH_ATTR ScanForDevices() {
	i2c_init();

	uint8 i = 0;
	uint8 result;

	while (i < 0xff) {
		i2c_start();
		i2c_writeByte(i);
		i2c_stop();

		result = i2c_check_ack();

		if (result == 1)
			ets_uart_printf("Sent Address %02x, received %d\r\n", i, result);

		i++;
	}

}

/*
 * Function:  tsl1561Init
 * --------------------
 * Send the wakeup command to the TSL2561
 * See page 13 of the datasheet for instructions on how this works.
 *
 * returns: 0 if error, 1 if success
 */
int ICACHE_FLASH_ATTR tsl1561Init() {

	i2c_init();
	i2c_start();

	//Send address with write bit
	uint8 toWrite = (TSL2561_ADDR << 1) + 0x00;
	i2c_writeByte(toWrite);
	if (!i2c_check_ack()) {
		i2c_stop();
		return 0;
	}

	//We want to write to the control register
	i2c_writeByte(0x80);
	if (!i2c_check_ack()) {
		i2c_stop();
		return 0;
	}

	//Command bit to wake up the device
	i2c_writeByte(0x03);
	if (!i2c_check_ack()) {
		i2c_stop();
		return 0;
	}

	i2c_stop();

	return 1;

}

/*
 * Function:  readLight
 * --------------------
 * This is where the magic happens.
 * Gets the current lux value from the TSL25561.
 *
 * It starts by reading from channel0 and channel1,
 * then calling calculateLux to turn the lumosity into lux.
 *
 * Adapted from the datasheet, page 19.
 *
 *
 *
 * returns: current lux value
 */
uint32 ICACHE_FLASH_ATTR readLight() {
	//Read word from channel0
	uint16 channel0 = readWord(0xAC);

	//Read word from channel1
	uint16 channel1 = readWord(0xAE);

	//Check for error
	if(channel0 == -1 | channel1 == -1)
		return -1;

	//Get lux value.
	uint32 lux = calculateLux(channel0, channel1);

	//ets_uart_printf("C0: %d C1: %d Lux: %d\r\n", channel0, channel1, lux);

	return lux;

}

/*
 * Function:  readWord
 * --------------------
 * This allows for the ESP8266 to read from a register in the TSL2561.
 *
 * Sent a 8bit command code, it will return the 16bit value in that register.
 *
 * See page 14 for the command register commands.
 */

uint16 ICACHE_FLASH_ATTR readWord(uint8 commandCode) {
	//Turn on light conversion
	if (tsl1561Init()) {

		/*
		 * Got to wait by default the 400ms for it to take light reading
		 * This is according ot the manual, I have had success
		 * ignoring this, but at your discretion.
		 *
		 * If you have trouble, uncomment this line.
		 */

		//os_delay_us(402000);

		//Send start bit
		i2c_start();

		//Send address with write bit for some reason
		i2c_writeByte((TSL2561_ADDR << 1) + 0x00);
		if (!i2c_check_ack()) {
			i2c_stop();
			return -1;
		}

		//Send the command code to read from register
		i2c_writeByte(commandCode);
		if (!i2c_check_ack()) {
			i2c_stop();
			return -1;
		}

		//Send repeated start
		i2c_start();

		//Send address with read bit
		i2c_writeByte((TSL2561_ADDR << 1) + 0x01);
		if (!i2c_check_ack()) {
			i2c_stop();
			return -1;
		}

		//Now data is going to be returned to us

		//Get low 8
		uint8 low8 = i2c_readByte();
		i2c_send_ack(1);

		//Get high 8
		uint8 high8 = i2c_readByte();
		i2c_send_ack(0); //Send NACK because we are done

		uint8 result = i2c_check_ack();

		i2c_stop();

		uint16 value = (high8 << 8) | low8;

		//ets_uart_printf("Raw: %02x %02x Full: %d P: %d\r\n", high8, low8, value, result);

		return value;
	} else {
		return -1;
	}
}

/*
 * Function:  calculateLux
 * --------------------
 * This method, adapted from the Adafruit arduino libraries,
 * takes the current channel0 and channel1 values and turns them into lux measurements.
 *
 * This method was originally in the datasheet, but MR. Townsend did a great job here.
 */

uint32 ICACHE_FLASH_ATTR calculateLux(uint16_t ch0, uint16_t ch1) {
	unsigned long chScale;
	unsigned long channel1;
	unsigned long channel0;

	chScale = (1 << TSL2561_LUX_CHSCALE);

	// Scale for gain (1x or 16x)
	//if (!_gain) chScale = chScale << 4;
	chScale = chScale << 4;

	// scale the channel values
	channel0 = (ch0 * chScale) >> TSL2561_LUX_CHSCALE;
	channel1 = (ch1 * chScale) >> TSL2561_LUX_CHSCALE;

	// find the ratio of the channel values (Channel1/Channel0)
	unsigned long ratio1 = 0;
	if (channel0 != 0)
		ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE + 1)) / channel0;

	// round the ratio value
	unsigned long ratio = (ratio1 + 1) >> 1;

	unsigned int b, m;

	if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1T)) {
		b = TSL2561_LUX_B1T;
		m = TSL2561_LUX_M1T;
	} else if (ratio <= TSL2561_LUX_K2T) {
		b = TSL2561_LUX_B2T;
		m = TSL2561_LUX_M2T;
	} else if (ratio <= TSL2561_LUX_K3T) {
		b = TSL2561_LUX_B3T;
		m = TSL2561_LUX_M3T;
	} else if (ratio <= TSL2561_LUX_K4T) {
		b = TSL2561_LUX_B4T;
		m = TSL2561_LUX_M4T;
	} else if (ratio <= TSL2561_LUX_K5T) {
		b = TSL2561_LUX_B5T;
		m = TSL2561_LUX_M5T;
	} else if (ratio <= TSL2561_LUX_K6T) {
		b = TSL2561_LUX_B6T;
		m = TSL2561_LUX_M6T;
	} else if (ratio <= TSL2561_LUX_K7T) {
		b = TSL2561_LUX_B7T;
		m = TSL2561_LUX_M7T;
	} else if (ratio > TSL2561_LUX_K8T) {
		b = TSL2561_LUX_B8T;
		m = TSL2561_LUX_M8T;
	}

	unsigned long temp;
	temp = ((channel0 * b) - (channel1 * m));

	// do not allow negative lux value
	if (temp < 0)
		temp = 0;

	// round lsb (2^(LUX_SCALE-1))
	temp += (1 << (TSL2561_LUX_LUXSCALE - 1));

	// strip off fractional portion
	uint32_t lux = temp >> TSL2561_LUX_LUXSCALE;

	// Signal I2C had no errors
	return lux;
}
