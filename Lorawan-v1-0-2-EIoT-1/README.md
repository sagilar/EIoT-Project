# EIoT LoRaWAN 1.0.2 first development.
It was extended from https://github.com/ARMmbed/mbed-os-example-lorawan.  

## Setup
This app is running in the region US 915 over a STM DISCO-L072CZ-LRWAN1 board, some configurations are required.
In mbed_app.json:
- "lora.fsb-mask": "{0xFF00, 0x0000, 0x0000, 0x0000, 0x0002}" ---Gateway channel configuration
- "lora.phy": "US915" ---Region
- "main_stack_size":      2048 ---Board RAM size (very little)

In LoRaMAC.cpp in the LoRaWAN stack:
- #define MBED_CONF_LORA_MAX_SYS_RX_ERROR				50   ---Needed to solve a chrystal calibration problem in the board. It must be avoided. See https://github.com/ARMmbed/mbed-os-example-lorawan/issues/21#issuecomment-366937981

eiot.h:  
- The customized functions definition.

main.cpp:  
- Customized application program.



