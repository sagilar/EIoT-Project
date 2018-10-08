/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ( C )2014 Semtech

Description: Contains the callbacks for the IRQs and any application related details

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

/*
 * This file contains a copy of the master content sx1276PingPong
 * with adaption for the SX1276Generic environment
 * (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 */
#include <string>

#ifndef __SX1276PINGPONG_H__
#define __SX1276PINGPONG_H__

#ifdef FEATURE_LORA
int SX1276PingPong(void);
#else
#define  SX1276PingPong(x)   void()
#endif
/*
 * Callback functions prototypes
 */
char *splitOnPosition(char *msj, int pos);
void readTemperature();
void readCurrent();
void sendElectric();
void readGPS();
double filter2o(double u, double &v1, double &v2, const double a0, const double a1, const double a2, const double b0, const double b1, const double b2);
/*!
 * @brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void *radio, void *userThisPtr, void *userData);

/*!
 * @brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(void *radio, void *userThisPtr, void *userData, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * @brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout(void *radio, void *userThisPtr, void *userData);

/*!
 * @brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout(void *radio, void *userThisPtr, void *userData);

/*!
 * @brief Function executed on Radio Rx Error event
 */
void OnRxError(void *radio, void *userThisPtr, void *userData);

/*!
 * @brief Function executed on Radio Fhss Change Channel event
 */
void OnFhssChangeChannel(void *radio, void *userThisPtr, void *userData, uint8_t channelIndex);

/*!
 * @brief Function executed on CAD Done event
 */
void OnCadDone(void *radio, void *userThisPtr, void *userData);

#endif // __MAIN_H__
