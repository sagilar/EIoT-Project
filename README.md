# EIoT-Project
The Electrical Internet of Things (EIoT) project was an initiative of Codensa S.A E.S.P, a power distribution company located in Bogotá, Colombia, and The T&T research group from the Universidad Nacional de Colombia, located in Medellín, Colombia.  
This project intends to cover multiple electrical engineering applications in the electrical and power industry and subsequently, cover many automation applications of several disciplines.  


## Update 01/Mar/2019
The LoRaWAN-v1-0-2-EIoT 2 folder contains the development of a LoRaWAN node which generates a random voltage value (directly applicable with a voltage input 0 - 3.3V). It uses the Mbed-OS and the LoRaWAN Mbed stack.  

-----------------------------------------------------------------------------------  

The LoRaWAN-v1-0-2-EIoT 3 folder contains the development of a LoRaWAN node that monitors electrical things including a fire detector sensor and an analogic output to convert the current 0 - 5 A to 0 - 3.3V. It uses the Mbed-OS and the LoRaWAN Mbed stack.  



## Update 04/Jan/2019 (LoRaWAN)
The project was migrated to use the LoRaWAN standard. Currently the LoRaWAN version used is 1.0.2.  
Possible network servers:
- TheThingsNetwork: https://www.thethingsnetwork.org/
- Loriot: https://www.loriot.io/
- The one this project uses: LoRaServer (deployable locally in your machines) - https://www.loraserver.io/

The Lorawan-v1-0-2-EIoT-1 folder contains a new development of the EIoT project complying LoRaWAN 1.0.2 specification using Mbed-OS and all the LoRaWAN Mbed stack.  
More details are described in the readme file of such folder.  

-------

## EIoT Web Server (LoRa - non-LoRaWAN)
This folder contains the source code of the EIoT Web Server.  
The Web Server runs in Python using the framework Flask and the SQLite3 Database engine.  
The GUI was designed over a Start Bootstrap open source template with some modifications.  
To run the server is necessary to run the next commands in the root folder (flaskr/):
- . venv/bin/activate
- export FLASK_APP=flaskr
- export FLASK_DEBUG=true (or false)
- flask run --host=0.0.0.0

## SerialCom
It provides the code to run the interface between the Raspberry PI and the EIoT LoRa Gateway.  
This is a single python file, but needs the physical serial connection between both devices to work.  

## EIoT LoRa Gateway
This folder contains the mbed code of the LoRa device which acts as the gateway in the LoRa network.  
Please refer to the SX1276GenericPingPong/GenericPingPong.cpp file to note the differences to the template (STM32 template).  
The same content can be found in the mbed repository: https://os.mbed.com/users/sagilar/code/EIoT_LoRa_Gateway/  

## EIoT LoRa nodes (1,2, and 3)
The folders contain the mbed code of the LoRa devices which act as nodes in the LoRa network.  
Please refer to the SX1276GenericPingPong/GenericPingPong.cpp files to note the differences to the template (STM32 template).  
The same content can be found in the mbed repositories:
- https://os.mbed.com/users/sagilar/code/EIoT_LoRa_node_1/
- https://os.mbed.com/users/sagilar/code/EIoT_LoRa_node_2/
- https://os.mbed.com/users/sagilar/code/EIoT_LoRa_node_3/

## Remarks:
The LoRa codes are embedded into STM DISCO-L072CZ-LRWAN1 end devices.  
**Most of the code and interfaces are in Spanish. Please contact me for help.**

