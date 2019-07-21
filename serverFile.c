#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <openssl/md5.h>
#include <signal.h>
#include <sys/types.h>
#include <strings.h>
#include <libxml/xmlreader.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

// DEFINITIONS
#define MAX_SIZE 50
#define STORAGE_SIZE 40
#define BUFFER_LENGTH 1024
#define FILES_PER_HASH 20
#define CHAR_LENGTH 256


// DATA STRUCTURE FOR FILE STORAGE
struct fileStructure{
	unsigned char *hashName; // Hashname for file
	char *knownAs[FILES_PER_HASH]; // Char array containing names of files for same hash
	int fileCount; // Current number of files related to hash
};

// GLOBAL VARIABLES USED

char client_message[BUFFER_LENGTH];
char server_message[BUFFER_LENGTH];
char buffer[CHAR_LENGTH * 4];
char fileLocation[CHAR_LENGTH];
char dedupLocation[CHAR_LENGTH];
struct fileStructure dedupFile[STORAGE_SIZE];
int structCount = -1; // Global index for dedupFile
int flagXML = 0; // For XML reading
pthread_mutex_t lock;

// HELPER FUNCTIONS

int cfileexists(const char * filename){
	/* try to open file to read */
	FILE *file;
	if (file = fopen(filename, "r")){
		fclose(file);
		return 1;
	}
	return 0;
}

// FILESTRUCTURE FUNCTIONS

// Initialize Empty Structure
void initFileStruct(){
	for(int i = 0; i < STORAGE_SIZE; i++){
		dedupFile[i].fileCount = 0;	
		dedupFile[i].hashName = malloc(33);
		for(int j = 0; j < FILES_PER_HASH; j++){
			dedupFile[i].knownAs[j] = malloc(MAX_SIZE);	
		}
	}
}

// Free memory Structure used
void freeStruct(){
	for(int i = 0; i < STORAGE_SIZE; i++){
		free(dedupFile[i].hashName);
		for(int j = 0; j < FILES_PER_HASH; j++){
			free(dedupFile[i].knownAs[j]);	
		}
	}
}
// Used for printing how the strucure looks like for a given fileStructure
void printFileInfo(struct fileStructure *file){
//	printf("Printing Structure info\n");
//	printf("Hash Name\n");
//	printf("%s\n", file->hashName);
//	printf("\nFile Names\n");
//	for(int i = 0; i < file->fileCount; i++) printf("%s\n", file->knownAs[i]);
//	printf("\nNumber of files: %d\n", file->fileCount);
}


// Compare given hash with hashes in data stucture
// Return index of file if hash exists, return -1 otherwise
int searchHash(const char *hash){
	for(int i = 0; i < structCount+1; i++){
		if(strcmp(hash, dedupFile[i].hashName) == 0){
			return i;
		}
	}
	return -1;
}


// Compare given fileName with knownAs entries in data stucture
// Return index of file if file exists, return -1 otherwise
int searchFile(const char *filename){
	for(int i = 0; i < structCount+1; i++){
		for(int j = 0; j < dedupFile[i].fileCount+1; j++){
			if(strcmp(filename, dedupFile[i].knownAs[j]) == 0){
				return i;
			}
		}	
	}
	return -1;
}

// Searches through struct and finds the known as index
int searchFileforKnownAs(const char *filename){
	for(int i = 0; i < structCount+1; i++){
		for(int j = 0; j < dedupFile[i].fileCount+1; j++){
			if(strcmp(filename, dedupFile[i].knownAs[j]) == 0){
				return j;
			}
		}	
	}
	return -1;
}
// Adds a new hash Entry
void addHashEntry(unsigned char *hashName, const char *filename){
	structCount ++;

	strcpy(dedupFile[structCount].hashName, hashName);
	strcpy(dedupFile[structCount].knownAs[0], filename);
	dedupFile[structCount].fileCount++;
	//printFileInfo(&dedupFile[structCount]);

}


void addKnownAsEntry(int index, const char *filename){

	strcpy(dedupFile[index].knownAs[dedupFile[index].fileCount], filename);
	dedupFile[index].fileCount++;
	//printFileInfo(&dedupFile[index]);


}

int getTotalFilenames(){
	int sum = 0;
	for(int i = 0; i < structCount + 1; i++){
		sum += dedupFile[i].fileCount;
	}

	return sum;
}

void removeHashEntry(int index){
	// Replace data of index
	strcpy(dedupFile[index].hashName, dedupFile[structCount].hashName);
	for(int i = 0; i < dedupFile[structCount].fileCount; i++){
		strcpy(dedupFile[index].knownAs[i], dedupFile[structCount].knownAs[i]);
		//dedupFile[structCount].fileCount = 0;
	}
	dedupFile[index].fileCount = dedupFile[structCount].fileCount;
	dedupFile[structCount].fileCount = 0;
	// Decrement total files
	structCount--;
	//printFileInfo(&dedupFile[structCount]);
}

// Takes index of hash and name of file requested to remove
void deleteKnownAsEntry(int index, const char *filename){
	// Loop to find index filename is located
	for(int j = 0; j < dedupFile[index].fileCount+1; j++){
		if(strcmp(filename, dedupFile[index].knownAs[j]) == 0){
			// Copy last filename over filename at index
			strcpy(dedupFile[index].knownAs[j], dedupFile[index].knownAs[dedupFile[index].fileCount-1]);
		}
	}
	// Decrement filecount
	dedupFile[index].fileCount--;

	//printFileInfo(&dedupFile[index]);
	// If there are no files in hash
	if(dedupFile[index].fileCount == 0){
		removeHashEntry(index);
	}
	// Decrement filecount
	//dedupFile[index].fileCount--;

}

// Computes the hash for a given filename
unsigned char* getFileHash(const char * filename){
	unsigned char c[MD5_DIGEST_LENGTH];

	int i;
	FILE *inFile = fopen(filename, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (inFile == NULL) 
	{
		printf ("%s can't be opened.\n", filename);
		return 0;
	}

	MD5_Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
		MD5_Update (&mdContext, data, bytes);
	MD5_Final (c,&mdContext);

	unsigned char *tempHash = malloc(33);

	for(int i = 0; i < MD5_DIGEST_LENGTH; i++){
		snprintf(&(tempHash[i * 2]), 16*2, "%02x", (unsigned int)c[i]);			
	}

	fclose (inFile);
	return tempHash;
}

// SOCKET THREAD FUNCTION
void * socketThread(void *arg)
{
	struct stat stat_buf;
	int read_fd;
	int newSocket = *((int *)arg);
	int write_fd;
	int byteAmount = 0;

	while(1)
	{
		char str[80];
		char uploadLocation[CHAR_LENGTH];
			// Client sends an action
			if(recv(newSocket,client_message,BUFFER_LENGTH,0)==0){
				printf("Error\n");
			}
			else
			{
				pthread_mutex_lock(&lock);

				// USER ENTERS QUIT [0x08]
				if(client_message[0] == 0x08) {
					buffer[0] = 0x09;
					send(newSocket, buffer, 1, 0);
					pthread_mutex_unlock(&lock);
					close(newSocket);
					pthread_exit(NULL);

				}

				// USER UPLOADS FILE [0x02]
				else if(client_message[0] == 0x02) {		
					char * message = "1\n";

					int b;
					// Sends that server is ready
					send(newSocket, message, strlen(message), 0);

					// Gets the file name
					if(b = recv(newSocket, client_message, BUFFER_LENGTH, 0) == 0){
						printf("Error\n");
					}
					else{
						client_message[strcspn(client_message, "\n")] = 0;

						char * fileName = malloc(sizeof(client_message)+1);
						strcpy(fileName, client_message);
						fileName[strlen(client_message)] = '\0';

						strcpy(str, "/");			 
						strcat(str, fileName);

						int knownAsIndex = searchFile(fileName);

						if(knownAsIndex >= 0){
							strcpy(buffer, "");
							bzero(buffer, strlen(buffer));
							strcpy(uploadLocation, "");
							strcpy(str, "");

							buffer[0] = 0xFF;
							send(newSocket, buffer, 1, 0);
							free(fileName);					
							bzero(client_message, strlen(client_message));
						}
						else{
							send(newSocket,"2", 1, 0);
							strcpy(uploadLocation, fileLocation);
							strcat(uploadLocation, str);

							bzero(client_message, strlen(client_message));
							strcpy(client_message, "");

							// Gets the number of bytes to go through
							// client_message should be returning the file size
							int bytenumber = 0;
							if(b = recv(newSocket, &bytenumber, 4, 0)==0) {
								printf("Error\n");
							} else {
								write_fd = open(uploadLocation, O_WRONLY | O_CREAT, 0666);
								//byteAmount = client_message[0];
								int bytes = 0;
								byteAmount = bytenumber; // This will allow bigtest.txt to work


								// recv loop to get all file contents. Ends when we have
								// as many bytes as expected
								do {
									bzero(client_message, sizeof(client_message));
									b = recv(newSocket,client_message,BUFFER_LENGTH+50,0);
									bytes = bytes + b;
									write(write_fd, client_message, strlen(client_message));
									//sendfile(newSocket, write_fd, 0, MAX_SIZE);
									
								} while (bytes < byteAmount);
								bzero(client_message, sizeof(client_message));
								close(write_fd);

								unsigned char* tempHash = malloc(33);

								strcpy(tempHash, getFileHash(uploadLocation));
								int hashIndex = searchHash(tempHash);

								if(hashIndex < 0){
									addHashEntry(tempHash, fileName);
								}
								else{
									addKnownAsEntry(hashIndex, fileName);
									if(remove(uploadLocation) != 0){
										printf("Remove Error\n");
									}
								}
								// Free's and resets to prepare for next work
								free(fileName);
								free(tempHash);						
								bzero(client_message, strlen(client_message));		
								strcpy(buffer, "");
								strcpy(uploadLocation, "");
								strcpy(str, "");
								buffer[0] = 0x03;
								send(newSocket, buffer, 1, 0);
							}
							}
						}					
					}


					// USER REMOVES FILE
					else if(client_message[0] == 0x04){
						// Recieves file name
						if(recv(newSocket, client_message, BUFFER_LENGTH, 0) == 0){
							printf("Error\n");
						}else{
							char * fileName = malloc(sizeof(client_message));
							strcpy(fileName, client_message);
							int knownAsIndex = searchFile(fileName); 
							int hashIndex = searchFileforKnownAs(fileName);
							if(knownAsIndex < 0){
								buffer[0] = 0xFF;
								send(newSocket, buffer, 1, 0);
							}else{
								strcpy(str, "/");			 
								strcat(str, fileName);
								strcpy(uploadLocation, fileLocation);
								strcat(uploadLocation, str);

								// Deleting the file name
								deleteKnownAsEntry(knownAsIndex, fileName);
								if(hashIndex == 0){
									if(dedupFile[knownAsIndex].fileCount > 1){
										// moves the last file name to the file we replaced
										// also lowers the counter and frees the last posistion
										char *newName = malloc(sizeof("./testDir") + sizeof(dedupFile[knownAsIndex].knownAs[dedupFile[knownAsIndex].fileCount]));
										strcpy(newName, dedupFile[knownAsIndex].knownAs[dedupFile[knownAsIndex].fileCount]);
										rename(uploadLocation, newName);
										free(newName);
									}
									else{
										// Removes the file completely if its the last filename
										remove(uploadLocation);
									}
								}
								//Free and reset for next time
								buffer[0] = 0x05;
								send(newSocket, buffer, 1, 0);
								free(fileName);
								bzero(client_message, strlen(client_message));		
								strcpy(buffer, "");
								strcpy(uploadLocation, "");
								strcpy(str, "");
							}
						}
					}

					// USER DOWNLOADS FILE
					else if(client_message[0] == 0x06){
						send(newSocket, "1", 1, 0);
						// Recieves the filename
						if(recv(newSocket, client_message, BUFFER_LENGTH, 0) == 0){
							printf("Error\n");
						}else{
							char * fileName = malloc(MAX_SIZE);
							strcpy(fileName, client_message);
							int knownAsIndex = searchFile(fileName);
							if (knownAsIndex < 0){
								// Sends error if the file is not found
								buffer[0] = 0xFF;
								send(newSocket, buffer, 1, 0);
							}else{
								// gets to the correct directory
								strcpy(str, "/");			 
								strcat(str, dedupFile[knownAsIndex].knownAs[0]);
								strcpy(uploadLocation, fileLocation);
								strcat(uploadLocation, str);

								// opens and sends the bytes, then file
								read_fd = open(uploadLocation, O_RDONLY);
								fstat(read_fd, &stat_buf);
								int sendingbytes = stat_buf.st_size;
								send(newSocket, &sendingbytes, sizeof(int), 0);

								sendfile(newSocket, read_fd, 0, sendingbytes);

								close(read_fd);

								if(recv(newSocket, client_message, BUFFER_LENGTH, 0) == 0){
									printf("Error\n");
								}else{
									// Sends that its ended. Resets for next time
									buffer[0] = 0x07;
									send(newSocket, buffer, 1, 0);
									bzero(client_message, strlen(client_message));		
									strcpy(buffer, "");
									strcpy(uploadLocation, "");
									strcpy(str, "");
									free(fileName);
								}
							}
						}
					}

					// USER LISTS FILES
					// Without the sleeps it only worked on one of our machines
					else if(client_message[0] == 0x00){
						int totalFiles = getTotalFilenames();
						buffer[0] = 0x01;
						send(newSocket, buffer, 1, 0);
						sleep(1);
						buffer[0] = totalFiles;
						send(newSocket, buffer, 1, 0);
						for(int i = 0; i < structCount + 1; i++){
							//printFileInfo(&dedupFile[i]);
							for(int j = 0; j < dedupFile[i].fileCount; j++){
								sleep(1);
								send(newSocket, dedupFile[i].knownAs[j], MAX_SIZE - 1, 0);
							}
						}

					}
					// Unlocks the lock since the server is done and ready for new request
					pthread_mutex_unlock(&lock);
				}
			}

		send(newSocket,buffer,13,0);
		close(newSocket);
		pthread_exit(NULL);

	}

	// Process each node in the XML
	static void processNode(xmlTextReaderPtr reader) {
		const xmlChar *name;
		char* namething;
		char *value;
		name = xmlTextReaderConstName(reader);
		if (name == NULL)
			name = BAD_CAST "--";
		value = xmlTextReaderValue(reader);
		// Waits till the name is equal to hash and the node is of type 1. 
		// If the flag is used, structCount can be incremented
		// This allows for structCount to be increased only when there is a new hash
		if ((strcmp(name,"hashname")==0) && (xmlTextReaderNodeType(reader) == 1)) {
			if (flagXML == 1) {
				structCount++;
			}
			flagXML = 0;
		}
		// Finds the depth of 3
		if (xmlTextReaderDepth(reader) == 3) {
			if (flagXML == 0){
				dedupFile[structCount].hashName = value;
				flagXML = 1;
			} else if (flagXML == 1) {
				dedupFile[structCount].knownAs[dedupFile[structCount].fileCount] = value;
				dedupFile[structCount].fileCount++;		
			}
		}
	}

	// Read through the XML file passed
	static void streamFile(const char *filename) {
		xmlTextReaderPtr reader;
		int ret;
		/*
		 * Pass some special parsing options to activate DTD attribute defaulting,
		 * entities substitution and DTD validation
		 */
		reader = xmlReaderForFile(filename, NULL,
				XML_PARSE_DTDATTR); // |  /* default DTD attributes */
		//	XML_PARSE_NOENT);// |    /* substitute entities */
		//XML_PARSE_DTDVALID); /* validate with the DTD */
		if (reader != NULL) {
			ret = xmlTextReaderRead(reader);
			while (ret == 1) {
				processNode(reader);
				ret = xmlTextReaderRead(reader);
			}
			
			xmlFreeTextReader(reader);
			if (ret != 0) {
				fprintf(stderr, "%s : failed to parse\n", filename);
			}
		} else {
			fprintf(stderr, "Unable to open %s\n", filename);
		}
	}

	// Reads the XML dedup file, passed as char array
	int readXMLdedup(char *fileDedup) {

		/*
		 * this initialize the library and check potential ABI mismatches
		 * between the version it was compiled for and the actual shared
		 * library used.
		 */
		//

		// Set structCount to start at 0 instead of -1
		structCount = 0;

		streamFile(fileDedup);

		/*
		 * Cleanup function for the XML library.
		 */
		xmlCleanupParser();
		/*
		 * this is to debug memory for regression tests
		 */
		xmlMemoryDump();
		int i = 0;
		return 0;
	}

	// Writes the struct to the dedup file
	// uri is the directory/.dedup
	void testXmlwriterFilename(const char *uri)
	{
		int rc;
		xmlTextWriterPtr writer;
		xmlChar *tmp;
		/* Create a new XmlWriter for uri, with no compression. */
		writer = xmlNewTextWriterFilename(uri, 0);
		if (writer == NULL) {
			printf("testXmlwriterFilename: Error creating the xml writer\n");
			return;
		}
		rc = xmlTextWriterStartDocument(writer, NULL, NULL, NULL);
		if (rc < 0) {
			printf("testXmlwriterFilename: Error at xmlTextWriterStartDocument\n");
			return;
		}
		rc = xmlTextWriterStartElement(writer, BAD_CAST "repository");
		if (rc < 0) {
			printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
			return;
		}
		// Loops through all hashes
		for (int i = 0; i < structCount + 1; i++) {
			rc = xmlTextWriterStartElement(writer, BAD_CAST "file");
			if (rc < 0) {
				printf("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
				return;
			}

			rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "hashname", "%s",
					dedupFile[i].hashName);
			if (rc < 0) {
				printf ("testXmlwriterFilename: Error at xmlTextWriterWriteFormatElement\n");
				return;
			}
			// Loops through each file name associated with each hash
			for (int j = 0; j < dedupFile[i].fileCount; j++) {
				//names
				rc = xmlTextWriterWriteElement(writer, BAD_CAST "knownas", dedupFile[i].knownAs[j]);
				if (rc < 0) {
					printf("testXmlwriterFilename: Error at xmlTextWriterWriteElement\n");
					return;
				}

			}
			rc = xmlTextWriterEndElement(writer);
			if (rc < 0) {
				printf ("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
				return;
			}
		}
		rc = xmlTextWriterEndDocument(writer);
		if (rc < 0) {
			printf
				("testXmlwriterFilename: Error at xmlTextWriterEndDocument\n");
			return;
		}

		xmlFreeTextWriter(writer);
	}

	// Calls the xml writer
	void writerXML(char *repodepo) {
		/* first, the file version */
		testXmlwriterFilename(repodepo); 
		/*
		 * Cleanup function for the XML library.
		 */
		xmlCleanupParser();
		/*
		 * this is to debug memory for regression tests
		 */
		xmlMemoryDump();
	}

	// SIGTERM HANDLER FOR CLOSING SERVER
	static void SigHandler(int signo) {
		if (signo == SIGTERM) {
			// Writes the XML
			writerXML(dedupLocation);

			// Frees the struct
			freeStruct();
			
			// Clean up mutex
			pthread_mutex_destroy(&lock);

			exit(0);
	}
}

int main(int argc, char *argv[])
{
	int serverSocket, newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	// DEFINE FILE LOCATION AND PORT NUMBER
	strcpy(fileLocation, argv[1]);
	int portInput = atoi(argv[2]);

	printf("PID: %d\n", getpid()); // Needed to test! Not sure how it works in real thing

	pid_t process_id = 0;
	pid_t sid = 0;
	// Create child process
	process_id = fork();
	// Indication of fork() failure
	if (process_id < 0)
	{
		printf("fork failed!\n");
		// Return failure in exit status
		exit(1);
	}
	// PARENT PROCESS. Need to kill it.
	if (process_id > 0)
	{
		printf("process_id of child process %d \n", process_id);
		// return success in exit status
		exit(0);
	}
	//unmask the file mode
	umask(0);
	//set new session
	sid = setsid();
	if(sid < 0)
	{
		// Return failure
		exit(1);
	}
	// Change the current working directory to root.
	//chdir("/");
	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);


	// CALL FILE STRUCT INITIALIZER
	initFileStruct();

	// READ IF .DEDUP EXISTS
	memcpy(dedupLocation, fileLocation, sizeof(fileLocation));
	strcat(dedupLocation, "/.dedup");
	if (cfileexists(dedupLocation)) {
		readXMLdedup(dedupLocation);
	}
//	dumpFileHere = malloc(sizeof(dedupLocation));
//	strcpy(dumpFileHere, dedupLocation);
//	memcpy(&(dumpFileHere), dedupLocation, sizeof(dedupLocation));


	// INITIALIZE MUTEX
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return 1;
	}

	//Create the socket. 
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	// Configure settings of the server address struct
	// Address family = Internet 
	serverAddr.sin_family = AF_INET;

//	printf("directory is:%s\n port is %d\n", fileLocation, portInput );
	//Set port number, using htons function to use proper byte order 
	serverAddr.sin_port = htons(portInput);

	//Set IP address to localhost 
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//Set all bits of the padding field to 0 
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	//Bind the address struct to the socket 
	bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	// Starts SIGHANDLER SETUP
//	printf("PID: %d\n", getpid()); // Needed to test!
	struct sigaction action;
	action.sa_handler = SigHandler;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGTERM);
	action.sa_flags = 0;
	action.sa_flags |= SA_RESTART;
	sigaction(SIGTERM, &action, NULL);
	// Ends SIGHANDLER SETUP

	//Listen on the socket, with 40 max connection requests queued 
	if(listen(serverSocket,50)==0){
		//printf("Listening\n");
	}else{
		printf("Error\n");
	}
	pthread_t tid[60];
	int i = 0;
	while(1)
	{
		//Accept call creates a new socket for the incoming connection
		addr_size = sizeof serverStorage;
		newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

		//for each client request creates a thread and assign the client request to it to process
		//so the main thread can entertain next request
		if( newSocket >= 0)
		{
		//	printf("%d", i);
		//	printf(" I am making sockets\n");
			if( pthread_create(&tid[i], NULL, socketThread, &newSocket) != 0 )
				printf("Failed to create thread\n");
			i++;
		}
		else if( newSocket < 0)
			printf("Failed to connect");

		if( i >= 50)
		{
			i = 0;
			while(i < 50)
			{
				pthread_join(tid[i++],NULL);
			}
			i = 0;
		}
		// Implement and call some function that does core work for this daemon.

	}
	return 0;
}
