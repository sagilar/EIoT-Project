#include <mbed.h>
/** A Hall-Effect sensor for measuring current levels in a given path
 *
 * Can be used as a current detector
 *
 * Example:
 * @code
 * // Periodically read current levels in a circuit and
 * // send output to PC terminal
 *
 * #include "mbed.h"
 * #include "ACS712.h"
 *
 * // Connect the sensor analog output pin to mbed's AnalogIn pin
 * ACS712 dev(p18);
 * // Connect mbed to pc's USB port
 * Serial pc(USBTX, USBRX);
 *
 * int main() {
 *      pc.printf("Sensor Log: \n\n\r");
 *      while (1) {
 *          // Read current from sensor and output to pc terminal
 *          pc.printf("Sensor Value: %2.2f A\n\r", dev);
 *          wait(0.200);
 *      }
 * }
 * @endcode
 */
 
class ACS712 { 
    
    public:
        /** Create a hall-effect sensor of the specified type
         *
         * @param _pin mbed AnalogIn pin where the analog output of sensor is connected
         * @param voltDivRatio resistor voltage division ratio at output of the sensor
         * @param type type of ACS712 sensor used
         *
         * @note Supported types of sensors:
         */
        ACS712(PinName _pin, float voltDivRatio = 1, short type = 5);
        
        /** Read the value of the measured current in amps
         *
         * @return current value in amps
         */
        float read();
        ACS712& operator=(const ACS712&);
        
        /** Read the value of the measured current in amps
         *  Allows the ACS712 object to be used in a float context
         * 
         * @return current value in amps
         */
        operator float() { return read(); }
        
    private:
        AnalogIn sensor;
        float translate(float);
        float ratio;
        short type;

};

ACS712::ACS712(PinName _pin, float voltDivRatio, short sensorType) : sensor(_pin){
    ratio = voltDivRatio;
    type = sensorType;    
}

float ACS712::translate(float val){
    switch(type){
        case 5: 
            return (val*ratio - 2.46*ratio)/(.185*ratio);
        case 20:
            return (val*ratio - 2.46*ratio)/(.1*ratio);
        case 30:
            return (val*ratio - 2.46*ratio)/(.066*ratio);
        default:
            return 999;
    }
}


float ACS712::read(){
    return ACS712::translate((sensor) * 3.3);   
}

ACS712& ACS712::operator=(const ACS712& rhs){
    sensor = rhs.sensor;
    ratio = rhs.ratio;
    type = rhs.type;
    return *this;
}
