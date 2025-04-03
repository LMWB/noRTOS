#ifndef INC_APP_MAIN_H_
#define INC_APP_MAIN_H_

#include <stdint.h>
#include "Drivers/Communication/ESP32AT/esp32at.h"

// Declare a global instance of mqtt_client
extern mqtt_client_t esp32_mqtt_client;
void app_main(void);

#endif /* INC_APP_MAIN_H_ */
