# Basic Example
tbd

# Multifunction Shield
tbd

# ESP32 as Wifi Coprocessor with AT-Command-Firmware
tbd

# Multi Sensor Reading and Processing 
Got myself some CO2 sensors and like to compare them.  
- [CCS811](https://www.az-delivery.de/products/co2-gassensor-ccs811?variant=43961284067595)  
- [ENS160](https://www.ebay.de/itm/315089224848?itmmeta=01JH545T2H1F4BY6BSW9YKCXKX&hash=item495cc80490:g:pIkAAOSwLzNln8qE)  
- [SCD4x](https://www.amazon.de/Jevina-Gassensormodul-Erkennt-Kohlendioxidtemperatur-Feuchtigkeit/dp/B0CPHX4DSG/ref=pd_day0_d_sccl_3_6/257-2786381-1588211?pd_rd_w=1naQq&content-id=amzn1.sym.0f8cb1f5-d0ae-4ed8-b7c8-e1284e2ce458&pf_rd_p=0f8cb1f5-d0ae-4ed8-b7c8-e1284e2ce458&pf_rd_r=AZTDAPJDVQBBQ1C8FNYP&pd_rd_wg=t6t50&pd_rd_r=5fd559e8-1ddb-4d8e-b365-814866b32866&pd_rd_i=B0CPHX4DSG&psc=1)  

Another one worth tying could be this  
- [SGP40](https://www.amazon.de/Hailege-Dioxide-Formaldehyde-Monitoring-GY-SGP40/dp/B0CJY3SM8S/ref=pd_day0_d_sccl_3_1/257-2786381-1588211?pd_rd_w=1naQq&content-id=amzn1.sym.0f8cb1f5-d0ae-4ed8-b7c8-e1284e2ce458&pf_rd_p=0f8cb1f5-d0ae-4ed8-b7c8-e1284e2ce458&pf_rd_r=AZTDAPJDVQBBQ1C8FNYP&pd_rd_wg=t6t50&pd_rd_r=5fd559e8-1ddb-4d8e-b365-814866b32866&pd_rd_i=B0CJY3SM8S&psc=1)  

## Sensirion CO2 Sensor
Power Supply  
3,3 ... 5,0 V -> Vdd-Pin

I2C Addresses
- 0x62

## CCS811 Sensor Module
Power Supply  
1,8 ... 3,6 V -> Vcc-Pin

I2C Addresses
- 0x5A -> ADD-Pin Low
- 0x5B -> ADD-Pin High

## ENS160+AHT21 CO2 Room Temperature Module
Power Supply  
5 V -> VIN-Pin
3,3 V -> 3V3-Pin

I2C Addresses  
- ENS160
  - 0x52 -> ADD Pin LOW
  - 0x53 -> ADD Pin HIGH 
- AHT21
  - 0x38 