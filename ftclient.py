#######################################
#Caleb Scott
#CS 372 - Project #2
#Last Modified 3/8/2019 11:07 PM EST
#Client side python program to demonstrate socket programming with client/server. FTP server to transfer text files
######################################
from socket import *
import sys
import time
import os

#Function to send data on the control socket
#Pre: Requires variable to be sent on the socket
#Post: Encodes the data as a string and sends it to the server
def easySend(x):
    controlSocket.send(str.encode(x))   

#Creates the socket connection
#Pre: Requires server name and data port number
#Post: returns the data socket
def attemptConnection(serverName, data_port):
    dataSocket = socket(AF_INET, SOCK_STREAM)
    while True:
        try:
            dataSocket.connect((serverName,data_port))
            return dataSocket
        except ConnectionRefusedError as e:
            pass

#Checks for duplicate file names and gets input to change the file name
#Pre: Requires file name
#Post: Returns new file name
def fixFileName(fileName):
    while os.path.exists(fileName):
        fileName = input("Error duplicate filename already exists in this location, Input new file name: ")
    return fileName

#Receives the data from the server and reads it into a variable 
#Pre: Doesn't Require anything as variables are created and assigned and passed to other functions
#Post: writes the data to the file contents
def receiveData():
    fileLength = int(serverResponse)
    dataSocket.send(str.encode("ok"))   
    fileContents = ""
    while len(fileContents) < fileLength:
        fileContents += dataSocket.recv(4096).decode('utf-8')

    newFileName = fixFileName(sendFileName)

    with open(newFileName,"w") as f:
        f.write(fileContents)


#Command Line Args
serverName = sys.argv[1]
control_port = int(sys.argv[2])
command = sys.argv[3]

#Saves command line arguments to separate variables
if command == '-l':
    data_port = int(sys.argv[4])
elif command == '-g':
    sendFileName = sys.argv[4]
    data_port = int(sys.argv[5]) 

#sets up control socket
controlSocket = socket(AF_INET, SOCK_STREAM)
controlSocket.connect((serverName,control_port))

#Sends the server name to the server
easySend(serverName)
serverResponse = controlSocket.recv(1024).decode('utf-8')
#Sends the -l/-g command to the server
easySend(command)
serverResponse = controlSocket.recv(1024).decode('utf-8')

#Checks the server response to ensure the ports match
if serverResponse != "port?":
    print ("Error: Server responded with " + serverResponse)
    sys.exit()

easySend(str(data_port))

#Sets up the data socket
dataSocket = attemptConnection(serverName, data_port)

#Recieves the directory from the server
if command == "-l":
    print("Receiving directory structure from " + serverName + ":" + str(data_port))
    listSize = int(dataSocket.recv(20).decode('utf-8'))
    dataSocket.send(str.encode("ok"))
    fileNames = ""
    while len(fileNames) < listSize:
        fileNames += dataSocket.recv(4096).decode('utf-8')
    print(fileNames)
#Receives the file asked for by the client
elif command == "-g":
    serverResponse = controlSocket.recv(1024).decode('utf-8')
    if serverResponse != "filename?":
        print("Error: Server Error")
        sys.exit()
    easySend(sendFileName)
    serverResponse = controlSocket.recv(1024).decode('utf-8')
    
    if serverResponse == "ERROR- File Not Found.\n":
        print(serverName + " says Error: File Not Found.")
        sys.exit()

    receiveData()
    print("Receiving " + '"' + sendFileName + '"' + " from " + serverName + ":" + str(data_port))
    print("File Transfer Complete")
    
controlSocket.close()