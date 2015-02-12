# TSL2561_ESP8266
A basic TSL2561 driver for use on the ESP8266 wifi micro-controller.

This allows for users to read the current light levels from a TSL2561 and prints the current value via UART.

This code requires the ExpressIf SDK and the i2c code written by Rudy Hardeman. Please check out http://www.esp8266.com, they can help you get started.

This package is only a skeleton that allows for reading the full lux value. Please feel free to fork it and add configuration setting such as changing the sample rate or interrupt pins. I don't need them for my project so I need to move onto 