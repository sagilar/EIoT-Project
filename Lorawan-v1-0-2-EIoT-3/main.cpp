/**
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include "mbed.h"
#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"

// Math Library
#include <math.h>

// Application helpers
#include "eiot.h"
#include "trace_helper.h"
#include "lora_radio_helper.h"

using namespace events;

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[255];
uint8_t rx_buffer[255];

/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
#define TX_TIMER                        20000

/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS            10

/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER     3


/**
 * Context variables
 */
 
AnalogIn analog_value_current(A0);
InterruptIn  fire_detector_in(D7,PullDown);
AnalogOut current_voltage(A2);
DigitalOut led1(LED1);
DigitalOut led3(LED4);
DigitalOut led4(LED3);
//Ticker tick_current;
Timer temp_c;
Timer timer;
Thread thread_c(osPriorityNormal, 1 * 1024);
//EventQueue current_queue(2*EVENTS_EVENT_SIZE);

//Volt Refrigerator : 119.5 V, Volt tests: 108.5V

const float RMS_VOLTAGE = 119.5;    //AC RMS Voltage
const float SENS = 185.0;   //Current Sensor Factor
const float MAX_CURRENT = 5.0;   //Max current output(sensor capacity)
const float FS = 600.0 * 1.1; //10% extra, compensación por procesamiento


double v_sub1 = 0.0;
double v_sub2 = 0.0;
//Peak filter 60Hz @ 3000 Hz Sampling Frequency
//double a0= 0.031474551558414499, a1= 0, a2 = -0.031474551558414499, b0 = 1, b1 = -1.9275262288479234, b2 = 0.937050896883171, gc= 1.0;

//Peak filter 60Hz @ 1000 Hz Sampling Frequency
//double a0=0.11003649853038844, a1= 0, a2 = -0.11003649853038844, b0 = 1, b1 = -1.6549342739304382 , b2 = 0.77992700293922312, gc= 1.0;

//Peak filter 60Hz @ 600 Hz Sampling Frequency
double a0=0.2373755155128614, a1= 0, a2 = -0.2373755155128614, b0 = 1, b1 = -1.2339523365530571 , b2 = 0.5252489689742772, gc= 1.0;

//Peak filter 60Hz @ 400 Hz Sampling Frequency
//double a0= 0.38970091175151578, a1= 0, a2 = -0.38970091175151578, b0 = 1, b1 = -0.71744960712000327, b2 = 0.22059817649696845, gc= 1.0;

//Peak filter 60Hz @ 300 Hz Sampling Frequency
//double a0= 0.46665964438072494 , a1= 0, a2 = -0.46665964438072494 , b0 = 1, b1 = -0.32962246734466799, b2 = 0.066680711238550128, gc= 1.0;

//Bandpass filter 20-100Hz @ 1000 Hz Sampling Frequency
//double a0= 1, a1= 0, a2 = -1, b0 = 1, b1 = -1.5276383342122288, b2 =   0.59139835139947117, gc= 0.20430082430026447;

//Bandpass filter 20-100Hz @ 600 Hz Sampling Frequency
//double a0= 1, a1= 0, a2 = -1, b0 = 1, b1 = -1.2255214153622767, b2 = 0.38386403503541588, gc= 0.30806798248229206;

//Bandpass filter 20-100Hz @ 400 Hz Sampling Frequency
//double a0= 1, a1= 0, a2 = -1, b0 = 1, b1 = -0.84161555967546375, b2 = 0.15838444032453636, gc= 0.42080777983773182;

//Bandpass filter 20-100Hz @ 300 Hz Sampling Frequency
//double a0= 1, a1= 0, a2 = -1, b0 = 1, b1 = -0.4376157616410008, b2 = -0.052407779283041155, gc= 0.52620388964152054;



//Current and related variables calculation
float current_sensor_value = 0.0;
float inst_current = 0.0;
float mean_current = 0.0;
float mean_current_static = 0.0;
float inst_power = 0.0;
float mean_power = 0.0;
float mean_power_static = 0.0;
float max_power = 0.0;
float max_power_static = 0.0;
float iter_sum = 0.0;
int iter_count = 0;



/**
* This event queue is the global event queue for both the
* application and stack. To conserve memory, the stack is designed to run
* in the same thread as the application and the application is responsible for
* providing an event queue to the stack that will be used for ISR deferment as
* well as application information event queuing.
*/
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS * EVENTS_EVENT_SIZE);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Constructing Mbed LoRaWANInterface and passing it down the radio object.
 */
static LoRaWANInterface lorawan(radio);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;


/**
 * Current calculation function - Callback
 */
 
void calculateCurrent()
{

    while(true){
    float estimated_time = 0.0;
    temp_c.reset();
    temp_c.start();
    float vread = analog_value_current.read();
    vread = vread * 3300.0;
    float vfilt = gc*filter2o(vread, v_sub1,v_sub2, a0, a1, a2, b0, b1, b2); //The signal is filtered
    iter_sum = iter_sum + pow(vfilt,2);
    iter_count++;
    temp_c.stop();
    estimated_time = abs((1.0/FS)-temp_c.read());
    //printf("estimated time=%f\n",estimated_time);
    if (iter_count == (int)FS){
        float VrmsSensor = sqrt(iter_sum/iter_count);
        inst_current = VrmsSensor/SENS;
        if(mean_current != 0.0) {
            mean_current = (mean_current + inst_current)/2.0;
        } else {
            mean_current = inst_current;
        }
        
        inst_power = inst_current * RMS_VOLTAGE;

        if(mean_power != 0.0) {
            mean_power = (mean_power + inst_power)/2.0;

        } else {
            mean_power = inst_power;
        }
        
        if(inst_power > max_power) {
            max_power = inst_power;
        }
        iter_count = 0;
        iter_sum = 0.0;
        mean_current_static = mean_current;
        mean_power_static = mean_power;
        max_power_static = max_power;
        /*mean_current = 0.0;
        mean_power = 0.0;
        max_power = 0.0;*/
        inst_current = 0.0;
        currentToVoltage();
        
    }
    //wait((1.0/FS));
    wait(estimated_time);
    }
}

/**
 * Writes the current value to voltage into the analog output
 */
 
void currentToVoltage()
{
     
     //current_voltage.write(readMeanCurrent()/MAX_CURRENT);
     current_voltage = (float)(mean_current_static/MAX_CURRENT);
}

/**
 * Sends a message to the Network Server
 */
static void send_message()
{
    uint16_t packet_len;
    int16_t retcode;
    float m_cur = readMeanCurrent();
    float m_pow = readMeanPower();
    float mx_pow = readMaxPower();
    bool fd_state = readFireDetector();
    printf("\r\n Sending: {\"mean_current\":%.2f,\"mean_power\":%.2f,\"max_power\":%.2f,\"fire_detector\":%d} \r\n", m_cur,m_pow,mx_pow,fd_state);
    
    packet_len = sprintf((char*) tx_buffer, "{\"mean_current\":%.2f,\"mean_power\":%.2f,\"max_power\":%.2f,\"fire_detector\":%d}",
                    m_cur,m_pow,mx_pow,fd_state);

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                           MSG_CONFIRMED_FLAG);

    if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
                : printf("\r\n send() - Error code %d \r\n", retcode);

        if (retcode == LORAWAN_STATUS_WOULD_BLOCK) {
            //retry in 3 seconds
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                ev_queue.call_in(10000, send_message);
            }
        }
        return;
    }

    printf("\r\n %d bytes scheduled for transmission \r\n", retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    int16_t retcode;
    retcode = lorawan.receive(MBED_CONF_LORA_APP_PORT, rx_buffer,
                              sizeof(rx_buffer),
                              MSG_CONFIRMED_FLAG|MSG_UNCONFIRMED_FLAG);

    if (retcode < 0) {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" Data:");

    for (uint8_t i = 0; i < retcode; i++) {
        printf("%x", rx_buffer[i]);
    }

    printf("\r\n Data Length: %d\r\n", retcode);

    memset(rx_buffer, 0, sizeof(rx_buffer));
}


/**
 * Fire Detector event on_fire
 */
void fd_fall_callback()
{
    ev_queue.call(send_message);
}

/**
 * Fire Detector event on_no_fire
 */
void fd_rise_callback()
{
    ev_queue.call(send_message);
}


/**
 * Entry point for application
 */
int main (void)
{
    //corrección de la ganancia
    gc = gc * 1.25 * 1.5;
    //Period sampling iniciatilization
    thread_c.start(&calculateCurrent);
    
    //Adding fire detector interrupts features:
    fire_detector_in.rise(&fd_rise_callback);
    fire_detector_in.fall(&fd_fall_callback);

    // setup tracing
    setup_trace();
    
    

    // stores the status of a call to LoRaWAN protocol
    lorawan_status_t retcode;

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK) {
        printf("\r\n LoRa initialization failed! \r\n");
        return -1;
    }

    printf("\r\n Mbed LoRaWANStack initialized \r\n");

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER)
                                          != LORAWAN_STATUS_OK) {
        printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
        return -1;
    }

    printf("\r\n CONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);
    /*
    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\r\n enable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");
    */
    
    // Disable adaptive data rate
    if (lorawan.disable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\r\n disable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Disabled \r\n");
    /*
     *    LoRaWAN US or AU:
     *    
     *    0: SF = 10, BW = 125 kHz, BitRate =   980 bps
     *    1: SF =  9, BW = 125 kHz, BitRate =  1760 bps
     *    2: SF =  8, BW = 125 kHz, BitRate =  3125 bps
     *    3: SF =  7, BW = 125 kHz, BitRate =  5470 bps
     *    See https://github.com/ARMmbed/mbed-os/blob/master/features/lorawan/lorastack/phy/LoRaPHYUS915.cpp
    
    */
    uint8_t data_rate = DR_4;
    lorawan.set_datarate(data_rate);
    
    printf("\r\n Data rate - %d \r\n",data_rate);

    retcode = lorawan.connect();

    if (retcode == LORAWAN_STATUS_OK ||
        retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
    } else {
        printf("\r\n Connection error, code = %d \r\n", retcode);
        return -1;
    }

    printf("\r\n Connection - In Progress ...\r\n");

    
    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();

    return 0;
}





/* 
 * Functions
 */
 


float readMeanCurrent()
{
    
    
    float ret_value = mean_current_static;
    mean_current = 0.0;
    mean_power = 0.0;
    max_power = 0.0;
    return ret_value;
    
}

float readMeanPower()
{
    
    float ret_value = mean_power_static;
    return ret_value;
    
}

float readMaxPower()
{
    float ret_value = max_power_static;
    return ret_value;
    
}

bool readFireDetector()
{
    int fd_state = fire_detector_in.read();
    return fd_state;
}

double filter2o(double u, double &v1, double &v2, const double a0, const double a1, const double a2, const double b0, const double b1, const double b2)
{
    
    double v = b0*u-b1*v1-b2*v2;
    double y = a0*v+a1*v1+a2*v2;    //Direct Form II
    v2=v1;
    v1=v;
    return y;

}

/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        case CONNECTED:
            printf("\r\n Connection - Successful \r\n");
            led1 = 1;
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            } else {
                ev_queue.call_every(TX_TIMER, send_message);
            }

            break;
        case DISCONNECTED:
            led1 = 0;
            ev_queue.break_dispatch();
            printf("\r\n Disconnected Successfully \r\n");
            break;
        case TX_DONE:
            led4 = 1;
            printf("\r\n Message Sent to Network Server \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            wait(0.3);
            led4 = 0;
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("\r\n Transmission Error - EventCode = %d \r\n", event);
            // try again
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            receive_message();
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("\r\n Error in reception - Code = %d \r\n", event);
            break;
        case JOIN_FAILURE:
            led3 = 1;
            printf("\r\n OTAA Failed - Check Keys \r\n");
            break;
        case UPLINK_REQUIRED:
            printf("\r\n Uplink required by NS \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}

// EOF
