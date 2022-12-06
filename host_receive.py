import serial
import time

print("Before connect")
ser = serial.Serial('COM4', 9600, timeout=100)
print("After connect")

try:
    while True:
        print("in loop")
        s = ser.read(64) #Read NMEA message
        # with open("nmea_output.txt", "wb") as binary_file:
        #     # Write bytes to file
        #     binary_file.write(s)
        print(s.decode('utf-8'))
except KeyboardInterrupt:
    ser.close()
    quit()

