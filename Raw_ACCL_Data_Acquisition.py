import time
import serial


#List to store the timestamp received from the microcontroller
rcv = []

#Confirmation code to notify the microcontroller that handshake is successfull
ConfCode = [255 ,0 ,255 ,0]

#List to capture the raw accelerometer data
seq = []

#Character type variable to store the character read from InputCommandFile.txt file 
Char=''

#Function to convert integer into bytes
def int_to_bytes(value, length):
    result = []

    for i in range(0, length):
        result.append(value >> (i * 8) & 0xff)

    result.reverse()

    return result

#Function to convert bytes to integer
def bytes_to_int(bytes):
    result = 0

    for b in bytes:
        result = result * 256 + int(b)

    return result

#open the serial port at baudrate 9600 in linux
port = serial.Serial("/dev/ttyUSB0", baudrate=9600, timeout=1)
    

#get the current time in UTC format in milliseconds
timestamp = (int(time.time() * 1000))-1560000000000
print(timestamp)

#convert time stamp into bytes 
timestamp_bytes = int_to_bytes(timestamp, 4)

#send timestamp to serial port in bytes
port.write(timestamp_bytes)
print(timestamp_bytes)

#   Infinite loop to handshake between the script and microcontroller
while True:
    #Get the number of received bytes 
    RcvdNoOfBytes = port.inWaiting()
    
    #check for received number of bytes equal to or greater than 4 
    if RcvdNoOfBytes >= 4:
        
        #store the number of bytes in rcv list
        rcv = port.read(RcvdNoOfBytes)
        print(rcv)
        
        #check for what we received is equal to the bytes of timestamp we sent earlier
        if rcv[0] == timestamp_bytes[0] and rcv[1] == timestamp_bytes[1] and rcv[2] == timestamp_bytes[2] and rcv[3] == timestamp_bytes[3]:
          
            #send back the confirmation code to the microcontroller to  indicate handshake successfull and break the infinite loop
            port.write(ConfCode)
            print(ConfCode)
            print("handshake successfull")
            print("Data Capturing is in Progress")
            break
    

#Infinite loop to capture the data
while True:
    
    #Read the data coming from serial port and append in the seq list
    for i in port.read():
        seq.append(i)
     
    #open the text file in  read mode  
    f = open("Command_Resposne_File.txt" , "r+")
    
    #Read one character from the file 
    Char = f.read(1)
     
    #If Char is equal to 2 interrupt the data capturing       
    if Char == '2':
        print("DataCapturing Interrupted")
        break       
            

#Convert the captured data into byte array      
s = bytearray(seq) 

#Open the binary file to save the captured data
file = open("Raw_ACCL_Data" ,"a+b")
print("Data saved to file")

#save the data to the file
file.write(s)

#close the file
file.close()
    
            
        
        
    
    
