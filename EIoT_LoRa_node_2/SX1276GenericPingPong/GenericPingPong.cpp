/*
 * This file contains a copy of the master content sx1276PingPong
 * with adaption for the SX1276Generic environment
 * (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 */

#include "mbed.h"
#include "PinMap.h"
#include "GenericPingPong.h"
#include "sx1276-mbed-hal.h"
#include "main.h"
#include <string>
#include "rtos.h"
#include "ACS712.h"
#include <math.h>

#ifdef FEATURE_LORA

/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   1

/* Set this flag to '1' to use the LoRa modulation or to '0' to use FSK modulation */
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   !USE_MODEM_LORA
#define RF_FREQUENCY            RF_FREQUENCY_915_0  // Hz
#define TX_OUTPUT_POWER         20                  // 20 dBm

#if USE_MODEM_LORA == 1

#define LORA_BANDWIDTH          125000  // LoRa default, details in SX1276::BandwidthMap
#define LORA_SPREADING_FACTOR   LORA_SF7
#define LORA_CODINGRATE         LORA_ERROR_CODING_RATE_4_5

#define LORA_PREAMBLE_LENGTH    8       // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT     5       // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON  false
#define LORA_FHSS_ENABLED       true
#define LORA_NB_SYMB_HOP        4
#define LORA_IQ_INVERSION_ON    false
#define LORA_CRC_ENABLED        true

#elif USE_MODEM_FSK == 1

#define FSK_FDEV                25000     // Hz
#define FSK_DATARATE            19200     // bps
#define FSK_BANDWIDTH           50000     // Hz
#define FSK_AFC_BANDWIDTH       83333     // Hz
#define FSK_PREAMBLE_LENGTH     5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON   false
#define FSK_CRC_ENABLED         true

#else
#error "Please define a modem in the compiler options."
#endif


#define RX_TIMEOUT_VALUE    3500	// in ms

//#define BUFFER_SIZE       32        // Define the payload size here
#define BUFFER_SIZE         64        // Define the payload size here
#define RETRIES         3        // Se define la cantidad de intentos de envio despues de recibir error
/*
 *  Global variables declarations
 */
typedef enum {
    LOWPOWER = 0,
    IDLE,

    RX,
    RX_TIMEOUT,
    RX_ERROR,

    TX,
    TX_TIMEOUT,

    CAD,
    CAD_DONE
} AppStates_t;

volatile AppStates_t State = LOWPOWER;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*
 *  Global variables declarations
 */
SX1276Generic *Radio;


//const uint8_t PingMsg[] = { 0xff, 0xff, 0x00, 0x00, 'P', 'I', 'N', 'G'};// "PING";
//const uint8_t PongMsg[] = { 0xff, 0xff, 0x00, 0x00, 'P', 'O', 'N', 'G'};// "PONG";
//const uint8_t PingMsg[] = { 'M', 'S', 'J', ' ','G','I','L'};// "PING";
//const uint8_t PongMsg[] = {  'R', 'P', 'T', 'A',' ','G','I','L'};// "PONG";
const char EUI[] = "0E20";// 4  bytes que definen el identificador del dispositivo (copiar los bytes en string)
const char nombreDispoitivo[] = "nodo2_EIoT_LoRa";
const char AppEUI[] = "AAAA";// 4  bytes que definen el identificador de la aplicacion (copiar los bytes en string)
const char AppKey[] = "1A1B";// 4  bytes que definen la clave de la aplicacion (copiar los bytes en string) - El protocolo LoRaWAN establece la clave de 16 bytes pero para efectos de prueba se hara de 4
char MsgTX[64] = "";// Mensaje de transmision, se pueden usar los 52 bytes faltantes para completar el payload de 64 bytes. Se puede poner directamente en string.
char MsgRX[64] = "";// Mensaje de recepcion, carga el payload entrante a esta cadena.
char MsgRet[] = "RECIBIDO";// Para verificar el resultado del envio
char DestEUI[] = "0A01"; //Gateway Server
string strRecepcion = "";
uint16_t BufferSize = BUFFER_SIZE;
uint8_t *Buffer;
int reintentos=0;
string msjDeco="";
char *retParse;
char *srcEUI;
char *msjDestEUI;
char *msjContent;

//ACS712 cur_sensor(A0,1.0,5);
//ACS712 cur_sensor(A0);
AnalogIn analog_value_current(A0);
AnalogIn analog_value_temp(A2);
//RtosTimer analog_read_timer(readAnalog, osTimerPeriodic);


float meas_r1 = 0.0;
float meas_v1 = 0.0;
float meas_r2 = 0.0;
float meas_v2 = 0.0;
float inst_current = 0.0;
float inst_current_d1 = 0.0;
float mean_current = 0.0;
float max_inst_current = 0.0;
int conteoSenal = 0;
float inst_power = 0.0;
float mean_power = 0.0;
float max_power = 0.0;
float min_power = 0.0;
float temperature = 0.0;
const float FACTOR_CUR = 1.48;
const float RMS_VOLTAGE = 9.4;
const float SENS = 185.0;
const float VDC_STATIC_MV = 1540.0;
Timer tiempo;
//const int SAMP_RATIO = 50;
bool sending_temp = false;
bool sending_electric = false;
Ticker tick_current;
Ticker tick_send_electric;
Ticker tick_temp;
DigitalOut *led3;

//Filtro peak @ 3000 Hz de muestreo
double v_sub1 = 0.0;
double v_sub2 = 0.0;
double a0= 0.031474551558414499, a1= 0, a2 = -0.031474551558414499, b0 = 1, b1 = -1.9275262288479234, b2 = 0.937050896883171, gc= 1.0;
// Filtro Pasa alta @ 3000 Hz de muestreo fc = 5Hz;
double vh_sub1 = 0.0;
double vh_sub2 = 0.0;
double ah0= 1, ah1= -2, ah2 =  1, bh0 = 1, bh1 = -1.9851906578962613, bh2 =  0.98529951312821451, gh= 0.99262254275611894;
//Filtro pasa baja @ 3000 Hz de muestreo, fc = 65Hz;
double vl_sub1 = 0.0;
double vl_sub2 = 0.0;
double al0= 1, al1= 2, al2 =  1, bl0 = 1, bl1 = -0.62020410288672889,bl2 =   0.24040820577345759, gl = 0.1550510257216822;


int SX1276PingPong()
{

    //tick_current.attach(&readCurrent,1.0/(60.0 * SAMP_RATIO));
    tick_current.attach(&readCurrent,5.0);

    tick_send_electric.attach(&sendElectric,60.0);

    tick_temp.attach(&readTemperature,120.0);
    uint8_t i;
    bool isMaster = true;



#if( defined ( TARGET_KL25Z ) || defined ( TARGET_LPC11U6X ) )
    DigitalOut *led = new DigitalOut(LED2);
#elif defined(TARGET_NUCLEO_L073RZ) || defined(TARGET_DISCO_L072CZ_LRWAN1)
    DigitalOut *led = new DigitalOut(LED4);   // RX red
    led3 = new DigitalOut(LED3);  // TX blue
#else
    DigitalOut *led = new DigitalOut(LED1);
    led3 = led;
#endif

    Buffer = new  uint8_t[BUFFER_SIZE];
    *led3 = 1;

#ifdef B_L072Z_LRWAN1_LORA
    Radio = new SX1276Generic(NULL, MURATA_SX1276,
                              LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
                              LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5,
                              LORA_ANT_RX, LORA_ANT_TX, LORA_ANT_BOOST, LORA_TCXO);
#else // RFM95
    Radio = new SX1276Generic(NULL, RFM95_SX1276,
                              LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
                              LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5);

#endif

    dprintf("Aplicacion de comunicacion LoRa punto a punto" );
    dprintf("Frecuencia: %.1f", (double)RF_FREQUENCY/1000000.0);
    dprintf("TXPower: %d dBm",  TX_OUTPUT_POWER);
#if USE_MODEM_LORA == 1
    dprintf("Bandwidth: %d Hz", LORA_BANDWIDTH);
    dprintf("Spreading factor: SF%d", LORA_SPREADING_FACTOR);
#elif USE_MODEM_FSK == 1
    dprintf("Bandwidth: %d kHz",  FSK_BANDWIDTH);
    dprintf("Baudrate: %d", FSK_DATARATE);
#endif
    // Initialize Radio driver
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxError = OnRxError;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    if (Radio->Init( &RadioEvents ) == false) {
        while(1) {
            dprintf("Radio could not be detected!");
            wait( 1 );
        }
    }


    switch(Radio->DetectBoardType()) {
        case SX1276MB1LAS:
            if (DEBUG_MESSAGE)
                dprintf(" > Board Type: SX1276MB1LAS <");
            break;
        case SX1276MB1MAS:
            if (DEBUG_MESSAGE)
                dprintf(" > Board Type: SX1276MB1LAS <");
        case MURATA_SX1276:
            if (DEBUG_MESSAGE)
                dprintf(" > Board Type: MURATA_SX1276_STM32L0 <");
            break;
        case RFM95_SX1276:
            if (DEBUG_MESSAGE)
                dprintf(" > HopeRF RFM95xx <");
            break;
        default:
            dprintf(" > Board Type: unknown <");
    }

    Radio->SetChannel(RF_FREQUENCY );

#if USE_MODEM_LORA == 1

    if (LORA_FHSS_ENABLED)
        dprintf("             > LORA FHSS Mode <");
    if (!LORA_FHSS_ENABLED)
        dprintf("             > LORA Mode <");

    Radio->SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                        LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP,
                        LORA_IQ_INVERSION_ON, 2000 );

    Radio->SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                        LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                        LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0,
                        LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP,
                        LORA_IQ_INVERSION_ON, true );

#elif USE_MODEM_FSK == 1

    dprintf("              > FSK Mode <");
    Radio->SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                        FSK_DATARATE, 0,
                        FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                        FSK_CRC_ENABLED, 0, 0, 0, 2000 );

    Radio->SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                        0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                        0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, FSK_CRC_ENABLED,
                        0, 0, false, true );

#else

#error "Please define a modem in the compiler options."

#endif

    if (DEBUG_MESSAGE)
        dprintf("Inicializando nodo");
    dprintf("EUI (ID): %s",EUI);

    Radio->Rx( RX_TIMEOUT_VALUE );

    while( 1 ) {
#ifdef TARGET_STM32L4
        WatchDogUpdate();
#endif
        /*
        const char EUI[] = "0A10";// 4  bytes que definen el identificador del dispositivo (copiar los bytes en string)
        const char AppEUI[] = "AAAA";// 4  bytes que definen el identificador de la aplicacion (copiar los bytes en string)
        const char AppKey[] = "1A1B";// 4  bytes que definen la clave de la aplicacion (copiar los bytes en string) - El protocolo LoRaWAN establece la clave de 16 bytes pero para efectos de prueba se hara de 4
        char MsgTX[64] = "";// Mensaje de transmision, se pueden usar los 52 bytes faltantes para completar el payload de 64 bytes. Se puede poner directamente en string.
        char MsgRX[64] = "";// Mensaje de recepcion, carga el payload entrante a esta cadena.
        char MsgRet[] = "RECIBIDO";// Para verificar el resultado del envio
        char DestEUI[] = "0A01";
        string strRecepcion = "";
        uint16_t BufferSize = BUFFER_SIZE;
        uint8_t *Buffer;
        int reintentos=0;
        string msjDeco="";
        char *retParse;
        char *srcEUI;
        char *msjDestEUI;
        char *msjContent;*/

        switch( State ) {
            case RX:
                reintentos=0;
                *led = !*led;
                //dprintf("Mensaje para depurar: %s",MsgRX);
                msjDeco = MsgRX;
                splitOnPosition(MsgRX, 0);

                dprintf("Source EUI: %s, Destination EUI: %s, Content: %s",srcEUI,msjDestEUI,msjContent);
                strRecepcion = msjContent;
                if(strcmp(EUI,msjDestEUI) == 0) {
                    dprintf("Mismo EUI, Soy el destinatario");

                } else {
                    dprintf("Diferente EUI, ignorar mensaje");
                    wait_ms( 500 );
                    *led = !*led;
                    State = LOWPOWER;
                    break;
                }


                if( BufferSize > 0 ) {

                    if (strstr(msjContent, "RECIBIDO") != NULL) {
                        dprintf( "Mensaje recibido por el servidor correctamente" );
                    }
                    if (strstr(msjContent, "ERROR") != NULL) {
                        dprintf( "Mensaje no fue recibido por el servidor correctamente" );
                    }
                    if (strstr(msjContent, "DENIED") != NULL) {
                        dprintf( "Mensaje rechazado por el servidor correctamente" );
                    }
                    /*if( RecFound == true ) {

                        dprintf( "Mensaje recibido por el servidor correctamente" );
                    } else if(ErrorFound == true) { // Error en la recepcion
                        dprintf( "Mensaje no fue recibido por el servidor correctamente" );

                    } else if(DenFound == true) { // Negacion en la recepcion
                        dprintf( "Mensaje rechazado por el servidor correctamente" );

                    }*/
                }
                wait_ms( 500 );
                *led = !*led;
                State = LOWPOWER;
                break;
            case TX:

                //dprintf("Mensaje a enviar: %s",MsgTX);
                *led3 = !*led3;

                if(reintentos<RETRIES) {
                    Radio->Rx( RX_TIMEOUT_VALUE );
                    reintentos++;
                }
                wait_ms( 500 );
                *led = !*led;
                State = LOWPOWER;
                break;
            case RX_TIMEOUT:
                if(sending_electric == true) {
                    //sendElectric();
                } else if(sending_temp == true) {
                    //readTemperature();
                }

                State = LOWPOWER;
                break;
            case RX_ERROR:
                // We have received a Packet with a CRC error, send reply as if packet was correct
                if( isMaster == true ) {
                    // Send the next PING frame
                    memcpy(Buffer, MsgTX, sizeof(MsgTX));
                    for( i = 4; i < BufferSize; i++ ) {
                        Buffer[i] = i - 4;
                    }
                    wait_ms( 1000 );
                    Radio->Send( Buffer, BufferSize );
                } else {
                    // Send the next PONG frame
                    memcpy(Buffer, MsgTX, sizeof(MsgTX));
                    for( i = sizeof(MsgTX); i < BufferSize; i++ ) {
                        Buffer[i] = i - sizeof(MsgTX);
                    }
                    wait_ms( 1000 );
                    Radio->Send( Buffer, BufferSize );
                }
                State = LOWPOWER;
                break;
            case TX_TIMEOUT:
                Radio->Rx( RX_TIMEOUT_VALUE );
                State = LOWPOWER;
                break;
            case LOWPOWER:
                sleep();
                break;
            default:
                State = LOWPOWER;
                break;
        }
    }
}

char *splitOnPosition(char *msj, int pos)
{
    int i=0;
    char *substring = strtok (msj,"|");
    char *strOutput="";
    while (substring != NULL) {
        //dprintf ("substring: %s, index: %d",substring,i);

        if(i == 0) {
            srcEUI = substring;
        } else if(i == 3) {
            msjDestEUI = substring;
        } else if(i == 4) {
            msjContent = substring;
        } else if(i > 4) {
            strcat(msjContent," ");
            strcat(msjContent,substring);
        }

        if(i == pos) {
            strOutput = substring;
        }
        substring = strtok (NULL, "|");
        i++;
    }
    return strOutput;
}

void readCurrent()
{
    float vread;
    float vfilt = 0.0;
    float vh_ac = 0.0;
    int cont = 0;
    tiempo.start();
    float sum = 0.0;
    while(tiempo < 1.0)

    {

        vread = analog_value_current.read();
        vread = vread * 3300.0;

        //vh_ac = gh*filter2o(vread, vh_sub1,vh_sub2, ah0, ah1, ah2, bh0, bh1, bh2);
        //vfilt = filter2o(vh_ac, v_sub1,v_sub2, a0, a1, a2, b0, b1, b2);
        //vfilt = gh*filter2o(vread, vh_sub1,vh_sub2, ah0, ah1, ah2, bh0, bh1, bh2);

        //vfilt = gl*filter2o(vh_ac, vl_sub1,vl_sub2, al0, al1, al2, bl0, bl1, bl2);
        vfilt = gc*filter2o(vread, v_sub1,v_sub2, a0, a1, a2, b0, b1, b2);
        //dprintf("%.3f",vfilt);
        sum = sum + pow(vfilt,2);
        cont++;

    }
    float VrmsSensor = sqrt(sum/cont);
    inst_current = VrmsSensor/SENS;
    dprintf("%.3f",inst_current);
    //dprintf("voltajerms: %.3f, corrientersm: %.3f, vread: %.3f, conteo: %d",VrmsSensor,inst_current,vread, cont);
    tiempo.stop();
    tiempo.reset();

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
    if(min_power != 0) {
        min_power = inst_power;
    } else {
        if(inst_power < min_power) {
            min_power = inst_power;
        }
    }
}

void sendElectric()
{
    tick_current.detach();
    //dprintf("Voltaje Cin: Vcin=%f", meas_v1);
    sending_electric = true;
    reintentos=0;
    char valueStr[]="";
    sprintf(valueStr,"%.2f",mean_current);
    //sprintf(valueStr,"%.2f",(float)cur_sensor);
    char variable[] = "corriente_promedio";
    //char MsgEnvio[64] = "";
    strcpy(MsgTX, EUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppKey);
    strcat(MsgTX, "|");
    strcat(MsgTX, DestEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, valueStr);
    strcat(MsgTX, " ");
    strcat(MsgTX, variable);
    strcat(MsgTX, " ");
    strcat(MsgTX, nombreDispoitivo);
    memcpy(Buffer, MsgTX, sizeof(MsgTX));
    for( int i = sizeof(MsgTX); i < BufferSize; i++ ) {
        Buffer[i] = i - sizeof(MsgTX);
    }
    Radio->Send( Buffer, BufferSize );
    Radio->Rx( RX_TIMEOUT_VALUE );
    //dprintf("Enviando corriente: Cmean=%f", mean_current);
    wait_ms( 4000 );

    sprintf(valueStr,"%.2f",mean_power);
    strcpy(variable,"potencia_promedio");
    //char MsgEnvio[64] = "";
    strcpy(MsgTX, EUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppKey);
    strcat(MsgTX, "|");
    strcat(MsgTX, DestEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, valueStr);
    strcat(MsgTX, " ");
    strcat(MsgTX, variable);
    strcat(MsgTX, " ");
    strcat(MsgTX, nombreDispoitivo);
    memcpy(Buffer, MsgTX, sizeof(MsgTX));
    for( int i = sizeof(MsgTX); i < BufferSize; i++ ) {
        Buffer[i] = i - sizeof(MsgTX);
    }
    Radio->Send( Buffer, BufferSize );
    Radio->Rx( RX_TIMEOUT_VALUE );
    //dprintf("Enviando potencia prom: Pmean=%f", mean_power);
    wait_ms( 4000 );

    sprintf(valueStr,"%.2f",max_power);
    strcpy(variable,"potencia_max");
    //char MsgEnvio[64] = "";
    strcpy(MsgTX, EUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppKey);
    strcat(MsgTX, "|");
    strcat(MsgTX, DestEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, valueStr);
    strcat(MsgTX, " ");
    strcat(MsgTX, variable);
    strcat(MsgTX, " ");
    strcat(MsgTX, nombreDispoitivo);
    memcpy(Buffer, MsgTX, sizeof(MsgTX));
    for( int i = sizeof(MsgTX); i < BufferSize; i++ ) {
        Buffer[i] = i - sizeof(MsgTX);
    }
    Radio->Send( Buffer, BufferSize );
    Radio->Rx( RX_TIMEOUT_VALUE );
    //dprintf("Enviando potencia max: Pmax=%f", max_power);
    wait_ms( 3000 );

    /*sprintf(valueStr,"%.2f",min_power);
    strcpy(variable,"potencia_min");
    //char MsgEnvio[64] = "";
    strcpy(MsgTX, EUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppKey);
    strcat(MsgTX, "|");
    strcat(MsgTX, DestEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, valueStr);
    strcat(MsgTX, " ");
    strcat(MsgTX, variable);
    strcat(MsgTX, " ");
    strcat(MsgTX, nombreDispoitivo);
    memcpy(Buffer, MsgTX, sizeof(MsgTX));
    for( int i = sizeof(MsgTX); i < BufferSize; i++ ) {
        Buffer[i] = i - sizeof(MsgTX);
    }
    Radio->Send( Buffer, BufferSize );
    Radio->Rx( RX_TIMEOUT_VALUE );
    //dprintf("Enviando potencia min: Pmin=%f", min_power);
    wait_ms( 3000 );*/

    max_power = 0.0;
    min_power = 0.0;
    max_inst_current = 0.0;
    mean_current = 0.0;

    sending_electric = false;
    tick_current.attach(&readCurrent,5.0);
}

void readTemperature()
{
    tick_current.detach();
    sending_temp = true;
    meas_r2 = analog_value_temp.read(); // Read the analog input value (value from 0.0 to 1.0 = full ADC conversion range)
    meas_v2 = meas_r2 * 3.3;
    temperature = meas_v2 * 100.0;

    char valueStr[]="";
    sprintf(valueStr,"%.2f",temperature);
    char variable[] = "temperatura";
    strcpy(MsgTX, EUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, AppKey);
    strcat(MsgTX, "|");
    strcat(MsgTX, DestEUI);
    strcat(MsgTX, "|");
    strcat(MsgTX, valueStr);
    strcat(MsgTX, " ");
    strcat(MsgTX, variable);
    strcat(MsgTX, " ");
    strcat(MsgTX, nombreDispoitivo);
    memcpy(Buffer, MsgTX, sizeof(MsgTX));
    for( int i = sizeof(MsgTX); i < BufferSize; i++ ) {
        Buffer[i] = i - sizeof(MsgTX);
    }
    Radio->Send( Buffer, BufferSize );
    Radio->Rx( RX_TIMEOUT_VALUE );

    dprintf("Enviando temperatura: T=%f", temperature);
    sending_temp = false;
    tick_current.attach(&readCurrent,5.0);
}


void OnTxDone(void *radio, void *userThisPtr, void *userData)
{
    Radio->Sleep( );
    State = TX;
    if (DEBUG_MESSAGE)
        dprintf("> OnTxDone");
}

void OnRxDone(void *radio, void *userThisPtr, void *userData, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio->Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    State = RX;
    if (DEBUG_MESSAGE)
        dprintf("> OnRxDone: RssiValue=%d dBm, SnrValue=%d", rssi, snr);
    //dump("Data:", payload, size);
    strcpy(MsgRX,(char*)payload);
    //dprintf("Msj: %s", MsgRX);
}

void OnTxTimeout(void *radio, void *userThisPtr, void *userData)
{
    *led3 = 0;
    Radio->Sleep( );
    State = TX_TIMEOUT;
    if(DEBUG_MESSAGE)
        dprintf("> OnTxTimeout");
}

void OnRxTimeout(void *radio, void *userThisPtr, void *userData)
{
    *led3 = 0;
    Radio->Sleep( );
    Buffer[BufferSize-1] = 0;
    State = RX_TIMEOUT;
    if (DEBUG_MESSAGE)
        dprintf("> OnRxTimeout");
}

void OnRxError(void *radio, void *userThisPtr, void *userData)
{
    Radio->Sleep( );
    State = RX_ERROR;
    if (DEBUG_MESSAGE)
        dprintf("> OnRxError");
}

double filter2o(double u, double &v1, double &v2, const double a0, const double a1, const double a2, const double b0, const double b1, const double b2)
{
    //notch 60Hz a 12Hz Frecuencia de muestreo params: [num]/[den] = [ 0.99866961055330528, -1.9963536547718377,  0.99866961055330528]/[ 1,-1.9963536547718377,  0.99733922110661055]
    double v = b0*u-b1*v1-b2*v2;
    double y = a0*v+a1*v1+a2*v2;    //Forma directa II
    v2=v1;
    v1=v;
    return y;

}

#endif
