
num =[]
a = [0 ,0]
NoofBytestToRead = 11
seq= []
NoofSample=0
timestampBytes = [0 ,0 ,0 ,0]
x = -0
y = -0
z = -0
xg = 0.0
yg = 0.0
zg = 0.0
Index = 0
seq = []
seq1 = []
seq2 = []

#Convert input bytes to integer 
def bytes_to_int(bytes):
    result = 0

    for b in bytes:
        result = result * 256 + int(b)

    return result



#open the binary file RawAcceleroDataFile in read mode
f = open("ACCL_Data_binary" , "r+b")

#read the data from file and append into seq list
for i in f.read():
    seq.append(i)

#Print the length of the list
print("Length")
LengthOfList = len(seq)

#Discard last 11 bytes 
LengthOfList =  LengthOfList -11
print(LengthOfList)

while Index < LengthOfList:
    num  = seq
    
    #Calculate the checksum of 10 bytes
    a[0] = num[NoofBytestToRead - 11] + num [NoofBytestToRead - 10] + num [NoofBytestToRead - 9] + num [NoofBytestToRead - 8] + num [NoofBytestToRead - 7] + num[NoofBytestToRead - 6] + num [NoofBytestToRead - 5] + num [NoofBytestToRead - 4] + num [NoofBytestToRead - 3] + num [NoofBytestToRead - 2]
    #Mask LSB    
    a[0] = a[0] & 0x00ff
   # print(a[0])
   # print(num[NoofBytestToRead-1])
    
    #Check for the calucated checksum and received checksum equality
    if num[NoofBytestToRead-1] ==  a[0]:
        #print(NoofSample)
       # print("Data accepted")
        
        #Extract the timestamp bytes 
        timestampBytes[0] = num[NoofBytestToRead - 11]
        timestampBytes[1] = num [NoofBytestToRead - 10]
        timestampBytes[2] = num [NoofBytestToRead - 9]
        timestampBytes[3] = num [NoofBytestToRead - 8]

        
        #Convert timestamp bytes to integer
        Timestamp = bytes_to_int(timestampBytes)
        
        #Convert timestamp bytes to UTC format
        Timestamp = 1560000000000 + Timestamp
        
        #Convert the received hexadecimal data of x ,y ,z to the numerical form
        x =  ((num [NoofBytestToRead - 6] << 8) | num [NoofBytestToRead - 7]) & 0x0000ffff
        y =  ((num [NoofBytestToRead - 4] << 8) | num [NoofBytestToRead - 5]) & 0x0000ffff
        z =  ((num [NoofBytestToRead - 2] << 8) | num [NoofBytestToRead - 3]) & 0x0000ffff

        if x > 32767:
            x = x-65535
        if y > 32767:
            y = y-65535
        if z > 32767:    
            z = z-65535
        
        #Get the x ,y ,z value in g 
        xg = (x * .039)
        yg = (y * .039)
        zg = (z * .039)
        
        
        #append the pattern id 
        seq1.append('1')       # Pattern id
        seq1.append(',')
        
        #Append pattern name
        seq1.append("Normal")  # Pattern name
        seq1.append(',')
        seq1.append(Timestamp)
        seq1.append(',')
        seq1.append(xg)
        seq1.append(',')
        seq1.append(yg)
        seq1.append(',')
        seq1.append(zg)
        seq1.append('\r')
        seq1.append('\n')
        joined_seq = ''.join( str(v)for v in seq1)
    else:
        #If calculated checksum is not equal to the received checksum then discard the data
        print(NoofSample)
        print("data discarded")
    NoofBytestToRead = NoofBytestToRead + 11
    NoofSample=NoofSample+1
    Index = Index + 11


file = open("XYZ_ACC_Labeled_Data.txt.txt" ,"a")
print("Data saved to file")
print(joined_seq)
file.write(joined_seq)
file.close()
    
