Caleb Scott
CS 372 - Project #2

Instructions:
To begin if it matters all of files are saved in my directory (scottca) ~/cs372/proj2

[Setting Up Connection]
1. Otherwise begin by unzipping the ftserver.c and make file to one directory on the flip server and the ftclient.py file and text files to another directory.  I used the directory just below it in the structure ~/cs372/proj2/TestFileLocation.
2. use the command 'make'
	a. This will compile the ftserver.c file into ftserver with gcc ftserver.c -o ftserver.
	b. 'make clean' can be used to deleted the compiled file.
4. type in './ftserver [PORT NUMBER]' on one terminal window.
5. on another terminal window you can type in either 'python3 ftclient.py [flip#].engr.oregonstate.edu [SERVER PORT NUMBER] -l [DATA PORT NUMBER]'
	a. This program will work on any combination of flip servers [1, 2 or 3] so long as the PORT NUMBER is the same and ftclient is started first.
6. Or the other command that will work is 'python3 ftclient.py [flip#].engr.oregonstate.edu [SERVER PORT NUMBER] -g [FILE NAME.txt] [DATA PORT NUMBER]'
	a. This program will work on any combination of flip servers [1, 2 or 3] so long as the PORT NUMBER is the same and ftclient is started first.
	
[Running the program]
1. If the -l command was used on the client side the file directory of the server location will be shown.
2. If the -g command was used on the client, the file will be copied from the server to the client.
	a. if the file was already present the user will be asked to input a new file name to save a copy of the file.
3. Multiple commands can be used while the server is active.

[Notes]
1. Both files will work in the same directory when ran, I just used multiple to test sending files without needing to re-name them
2. test.txt included is the short file I tested.
3. Alice.txt included is the long file I tested.
