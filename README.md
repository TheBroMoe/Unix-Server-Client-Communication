# How to run program: 

* Compile Server and Client by entering 'make' in the terminal

## For Running Server:

* run './sf.out directory port' in the terminal where directory is the relative or absolute path of the repository of files and port is the port to which the server will listen to incoming clients

## For Closing Server: 

* Sending a SIGTERM by entering 'kill (pid)' in the terminal will result in the server's orderly termination and write of the .dedup file.

## For Running Client: 

* run './cf.out address port' in the terminal where address is the IP Address (Server is set up in local 127.0.0.1) and port is the port to which the server is set up to

## For Closing Client: 

By sending a 'q' command, the server will close the client's socket and user will be returned to terminal

# Acknowledgements: 

Used Lab 1 server-client files for foundation for using pthreads and sockets

Used XMLexample2.c from lab as basis for the XML reader

# Work Balance between Partners:

## Work done by both individuals:

- Internal Data Structure implementation

- Processing requests on client and server end

- Error handling by client and server

- MD5 Hashing

- Debugging

## Mohammad Kebbi:

- Methods used to modify and use Internal Data Structures

- Sighandler and orderly termination of server and closing of pthreads

- Mutual Exclusion implementation


## Zachary Kist:

 - .dedup reading and writing implementation

 - Downloading and Uploading files based on size input

 - Daemonic process Implementation


This assignment was tested using a Unix virtual machine running ubuntu
