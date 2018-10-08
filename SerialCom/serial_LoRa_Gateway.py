###########

#Script para capturar lectura por puerto serial y publicarla en el servidor#

###########

import serial
import requests

connected = False
port = '/dev/ttyAMA0'
baud = 115200

serial_port = serial.Serial(port, baud, timeout=3.0)

def handle_data(data):
    print(data)
    
    try:
        msg = data.split(':')
        content = msg[1]
        #print(content)
        contSplit = content.split(' ')[1:]
        valor = contSplit[0]
        variable = contSplit[1]
        equipo = contSplit[2]
        print ('valor: ' + valor + ', variable: ' + variable + ', equipo: ' + equipo)
        r = requests.post('http://192.168.137.100:5000/eiot', data = {'valor':valor,'id_disp':equipo,'var':variable})
    except Exception as e:
        pass
        #print("type error: " + str(e))
        #print(traceback.format_exc())    

while(serial_port.isOpen()):
    reading = serial_port.readline().decode()
    handle_data(reading)    