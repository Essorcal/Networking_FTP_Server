/************************************
Caleb Scott
CS 372 - Project #2
Last Modified 3/8/2019 11:07 PM EST
Server side C program to demonstrate Socket programming with client/server. FTP server to transfer text files
************************************/
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <dirent.h>
#include <ctype.h>

//Socket connection struct
struct SockCon
{
	int behindTheScenes;
	int socket;
};

//Function Declarations
struct SockCon makeSockConnection(int PORT);
void handleClientCommand(char buffer[], int control_socket, char *serverName, int PORT);
void listFiles(int control_socket, char *serverName);
int getDataPortNum(int control_socket);
void sendFile(int control_socket, char *serverName, int PORT);
int isNumeric (const char * s);
char *readFileIn(char *fileName);
void getFileName(int control_socket, char *fileNameBuffer);
int sendall(int s, char *buf, int *len);



int main(int argc, char const *argv[]) 
{ 
	//Input argument error handling
	if (argc != 2)
	{
	    printf("usage: %s portNum\n", argv[0]);
		return 1;
	}
	if (isNumeric(argv[1]) == 0)
	{
		printf("Error: Port must be a number\n");
		return 1;
	}
	printf("Server open on %s\n",argv[1]);
	int PORT = atoi(argv[1]);
	
	//Main while look to allow additional connections after the first has been completed
	while (1)
	{
		//Setting up connection variables
		struct SockCon control_socket_sc = makeSockConnection(PORT);
		int control_socket = control_socket_sc.socket;
		int control_behind = control_socket_sc.behindTheScenes;
		char buffer[1024] = {0};
		char serverName[1024] = {0};
		char *commandAck = "Ack"; 
		//Getting Server name from client then clearing buffer
		read( control_socket , buffer, 1024); 		
		sprintf(serverName, "%s",buffer);
		printf("Connection from %s\n",serverName);
		memset(buffer, 0, 1024);
		//Sending acknowledgement in order to separate packets
		send(control_socket, commandAck, strlen(commandAck), 0);
		//Receive -l or -g command
		read( control_socket , buffer, 1024);
		
		//process command from client
		handleClientCommand(buffer, control_socket, serverName, PORT);

		//close sockets
		close(control_socket);
		close(control_behind);
		printf("connection with client closed ready for next connection\n");
		//sleep(1);
	}
	return 0; 
} 

//Determines if an input is numeric or not
//Pre: Requires command line input argument
//Post: Returns whether or not an input is a number
int isNumeric (const char * s)
{
    if (s == NULL || *s == '\0' || isspace(*s))
      return 0;
    char * p;
    strtol (s, &p, 10);
    return *p == '\0';
}

//Processes the -l or -g command from the client
//Pre: Requires buffer, control socket, server name and control port
//Post: Calls the function chosen by the client
void handleClientCommand(char buffer[], int control_socket, char *serverName, int PORT)
{
	char *invalidCommandError = "ERROR- Invalid Command, use '-l' or 'g'"; 
	if (strcmp(buffer,"-l") == 0)
	{
		listFiles(control_socket, serverName);				
	}
	else if (strcmp(buffer,"-g") == 0)
	{
		sendFile(control_socket, serverName, PORT);
	}
	else
	{
		send(control_socket, invalidCommandError, strlen(invalidCommandError), 0);
	}
	//printf("%d\n",close(control_socket));
}

//Sends thte file asked for by the client
//Pre: Requires control socket, server name and control port
//Post: Sends the designated file to the client
void sendFile(int control_socket, char *serverName, int PORT)
{
	int portNum = getDataPortNum(control_socket);
	
	//int data_socket = makeSockConnection(portNum);
	struct SockCon data_socket_sc = makeSockConnection(portNum);
	int data_socket = data_socket_sc.socket;
	int data_behind = data_socket_sc.behindTheScenes;
	char fileName[1024] = {0};
	char okBuffer[1024] = {0};
	char *fileNotFoundError = "ERROR- File Not Found.\n";
	// char *fileName = "test.txt";
	getFileName(control_socket, fileName);
	printf("File \"%s\" requested on port: %d\n",fileName, portNum);
	printf("Sending \"%s\" to %s:%d\n",fileName,serverName,portNum);	
	if( access( fileName, F_OK ) != -1 ) 
	{
		//printf("File Found\n");
	
		char * buffer = readFileIn(fileName);
		if (buffer)
		{
			int len = strlen(buffer);
			char numBytesToSendStr[1024] = {0}; 
			sprintf(numBytesToSendStr, "%d",len);
			send(control_socket, numBytesToSendStr, strlen(numBytesToSendStr), 0);
			read(data_socket , okBuffer, 1024); 

			sendall(data_socket, buffer, &len);
			free(buffer);
		}
	
	} else 
	{
		printf("File Not Found. Sending error message to %s:%d\n",serverName,PORT);
		send(control_socket, fileNotFoundError, strlen(fileNotFoundError), 0);
	}
	//close sockets
	close(data_socket);
	close(data_behind);
}

//Ask for the data port from the client and save it as a variable
//Pre: Requires control socket
//Post: Returns data port number
int getDataPortNum(int control_socket)
{
	char *dataPortRequest = "port?";
	char bufferPort[1024] = {0};
	send(control_socket, dataPortRequest, strlen(dataPortRequest), 0); 
	read( control_socket , bufferPort, 1024); 
	int portNum = atoi(bufferPort);
	return portNum;
}

//Asks for and determines the file name from the server
//Pre: Requires control socket and file name buffer
//Post: Determines file name
void getFileName(int control_socket, char *fileNameBuffer)
{
	
	char *dataPortRequest = "filename?";
	send(control_socket, dataPortRequest, strlen(dataPortRequest), 0); 
	read( control_socket , fileNameBuffer, 1024); 
}

//Function to get and send the file directory list
//Pre: Requires control socket and server name
//Post: Send the contents of the current directory to the client
void listFiles(int control_socket, char *serverName)
{
	int portNum = getDataPortNum(control_socket);
	char okBuffer[1024] = {0};

	printf("List requested on port: %d\n",portNum);
	printf("Sending directory contents to %s:%d\n",serverName,portNum);
	// int data_socket = makeSockConnection(portNum);
	struct SockCon data_socket_sc = makeSockConnection(portNum);
	int data_socket = data_socket_sc.socket;
	int data_behind = data_socket_sc.behindTheScenes;

	DIR *d;
	int listCounter = 0;
	struct dirent *dir;
	d = opendir(".");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			listCounter += strlen(dir->d_name);
			listCounter ++;  // for newline chars that we'll be sending
		}
		closedir(d);
	}
	char numBytesToSendStr[1024] = {0}; 
	sprintf(numBytesToSendStr, "%d",listCounter);
	send(data_socket, numBytesToSendStr, strlen(numBytesToSendStr), 0);
	read(data_socket , okBuffer, 1024); 
	d = opendir(".");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			int len = strlen(dir->d_name);
			int len2 = strlen("\n");
			sendall(data_socket, dir->d_name, &len);
			sendall(data_socket, "\n", &len2);
		}
		closedir(d);
	}
	//close sockets
	close(data_socket);
	close(data_behind);
}
//Reads in the file and saves it to a buffer variable
//Pre: Requires file name
//Post: Returns the buffer variable filled with the file contents
char *readFileIn(char *fileName)
{
    long length;
    char * buffer = 0;
    FILE * f = fopen (fileName, "r");
    
    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length);
        if (buffer)
        {
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }
    return buffer;
}
//Breaks up the the full file to be transfered and sent in manageable chunks
//Pre: Requires the data socket, the buffer contents and the length of the buffer
//Post: Returns success or failure depending on whether the whole file was sent or not
int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 

//Makes the socket connection
//Pre: Requires control port
//Post: Uses a struct to return 2 variables, the socket file and the new socket
struct SockCon makeSockConnection(int PORT)
{
	int server_fd, new_socket; 
	struct sockaddr_in address; 
	int opt = 1;   // option for setsockopt
	int addrlen = sizeof(address); 
	
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
												&opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, 
								sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen))<0) 
	{ 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	} 
	struct SockCon sc = { server_fd, new_socket };
	return sc;
	// return new_socket;
}