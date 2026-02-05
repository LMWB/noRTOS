# Advanced Example 
Nucleo-F446RE / ARM CortexM4 / 84MHz internal Clock  

This example shows how to use the noRTOS framework with use of multiple peripherals of the STM32F446 microcontroller.
- GPIO as output (LED Blinky)
- GPIO as Input (4 pins with internal pull up resistor)
- ADC with 7 channels
- UART (as printf target)
- UART (with ST3485 RS485 transmitter / receiver for use as e.g. Modbus)
- SPI Master Full duplex (with TI DRV8908 Full Bridge driver as slave)
- CAN (internal CAN with TCAN3414 CAN FD Transceivers)
- RTC (internal)
- EEPROM (internal virtual and the end of FLASH)

That pretty much includes a modern embedded system application with several interfaces to the digital and analog world. 