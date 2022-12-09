import serial
import time

print("Before connect")
ser = serial.Serial('COM4', 9600)
print("After connect")

t = int(time.time())
print(t)
try:
    print("reading at the start")
    #start = ser.read_until(b'$GPRMC')
    while True:
        print("in loop")
        #start = bytearray(ser.read_until(b'$GPRMC'))
        #msg = bytearray(ser.read_until(b'$'))
        #msg = start[-6:0] + msg #Adds $GPRMC to front of msg
        s = ser.read(80) #Read NMEA message
        # Write bytes to file
        with open("nmea_output"+str(t)+".txt", "ab") as binary_file:
            binary_file.write(s)
            binary_file.write(b'\n')
        print(str(s))
except KeyboardInterrupt:
    ser.close()
    quit()
