/*
 * @file	tsl2561.c
 * @version	1.0
 * @author	DinsFire64 (dinsfire.com)
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

#include "ets_sys.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"

#define SAMPLE_RATE_IN_MS 500

static volatile os_timer_t tsl2561Timer;

void printCurrentLuxValue(void *arg)
{
    ets_uart_printf("Lux: %d\r\n", readLight());
}

void user_init(void)
{
    //Start the UART
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(1000);

    ets_uart_printf("Program Start...\r\n");

    //Check to see if device is connected
    if (tsl1561Init()) {
    	ets_uart_printf("TSL2561 init complete.\r\n");

    	//Disarm timer
        os_timer_disarm(&tsl2561Timer);

        //Set the timer function
        os_timer_setfn(&tsl2561Timer, (os_timer_func_t *)printCurrentLuxValue, NULL);

        //Arm timer
        os_timer_arm(&tsl2561Timer, SAMPLE_RATE_IN_MS, 1);
    }
    else
    	ets_uart_printf("TSL2561 init error.\r\n");
}

