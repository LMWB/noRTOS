# Advanced Example 
Nucleo-F446RE / ARM CortexM4 / 84MHz internal Clock  

This example shows how to use the noRTOS framework with use of multiple peripherals of the STM32F446 microcontroller.
- GPIO as output (LED Blinky, SPI Chip Select)
- GPIO as Input (4 pins with internal pull up resistor as push buttons)
- ADC with 7 channels (1 of them is internal Vref and 1 is internal temperature sensor)
- UART (as printf target)
- UART (with ST3485 RS485 transmitter / receiver for use as e.g. Modbus)
- SPI Master Full Duplex (with TI DRV8908 Full Bridge driver as slave)
- Display with 5Hz Periodic Refresh Rate (on a second SPI Master)
- I2C Master (with AHT21 Temperature Sensor)
- CAN (internal CAN with TCAN3414 CAN FD Transceivers)
- Hardware Timer as PWM Output Channel-1 (with OAMP as RC Filter and Voltage Follower as ADC output)
- Hardware Timer as PWM Output Channel-2 (LED Breathing Heartbeat)
- RTC (internal)
- EEPROM (internal virtual and the end of FLASH)

That pretty much includes a modern embedded system application with several interfaces to the digital and analog world. 