#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>


#define MAX_SIZE 50
#define BUFFER_LENGTH 1024
#define NUM_CLIENT 1
void *connection_handler(void *socket_desc);

int cfileexists(const char * filename){
    /* try to open file to read */
    FILE *file;
    if (file = fopen(filename, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    // INITIALIZATION
    int sock_desc;
	int write_fd;
	int read_fd;
    struct sockaddr_in serv_addr;
    struct stat stat_buf;
    char sbuff[MAX_SIZE],rbuff[MAX_SIZE], filebuff[MAX_SIZE];
    char *address = argv[1];
	int portInput = atoi(argv[2]);

    // SOCKET CREATION
    if((sock_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("Failed creating socket\n");

    // NETWORK CONFIGURATION
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(address);
    serv_addr.sin_port = htons(portInput);

    // CONNECTION
    if (connect(sock_desc, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        printf("Failed to connect to server\n");
    }

    // MAIN LOOP
    while(1)
    {
        // GET USER INPUT
        fgets(sbuff, MAX_SIZE , stdin);
        char * givenCommand = strtok(sbuff, " ");
      
        // USER ENTERED QUIT COMMAND
        if(strcmp(givenCommand, "q\n") == 0){
            sbuff[0] = 0x08; //
            send(sock_desc, sbuff, 1, 0);
            if(recv(sock_desc,rbuff,MAX_SIZE,0)==0){
                printf("Error");
            }
            if(rbuff[0] == 0x09){
                printf("OK\n");    
            }else{
                printf("No Response\n");
            }
            break;
        }
    
        // USER ENTERED UPLOAD COMMAND
        if(strcmp(givenCommand, "u") == 0){
            givenCommand = strtok(NULL, " \n");
            if(!cfileexists(givenCommand)){
                    printf("CERROR file not found in local directory\n");
            }else{
                // Tell server it is trying to update command
                sbuff[0] = 0x02;
                send(sock_desc, sbuff, 1, 0);

                // Recieve that the server is available
                if(recv(sock_desc,rbuff,MAX_SIZE,0)==0){
                    printf("Error");
                }
                else{
                    // Used as a split
                    rbuff[strcspn(rbuff, "\n")] = 0;
                    if (strcmp(rbuff, "1") == 0){
                        // Sends file name
                        send(sock_desc, givenCommand, strlen(givenCommand), 0);
                        
                        if(recv(sock_desc, rbuff,MAX_SIZE-1,0) == 0){
                            printf("Error\n");
                        }
                        else {
                            if(rbuff[0] == -1){
                                printf("SERROR file already exists\n");
                            }else{
                                read_fd = open (givenCommand, O_RDONLY);
                                fstat(read_fd, &stat_buf);

                                // Sends the size to the socket
                                int sendingbytes = stat_buf.st_size;
                                send(sock_desc, &sendingbytes, 4, 0);

                                sleep(1);

                                sendfile(sock_desc, read_fd, 0, stat_buf.st_size);
    
                                close(read_fd);
                                if(recv(sock_desc,rbuff,MAX_SIZE-1,0) == 0){
                                    printf("Error\n");
                                }
                                else {
                                    if(rbuff[0] == 0x03){
                                        printf("OK\n");
                                    }
                                    else{
                                        printf("SERROR\n");
                                    }
                                }
                            }
                        }
                    }
                }
            }
            sleep(2);
			bzero(rbuff, strlen(rbuff));

        }
            
        // USER ENTERED LIST COMMAND
       else if (strcmp(givenCommand, "l\n") == 0){
            sbuff[0] = 0x00; //
            send(sock_desc, sbuff, 1, 0);

            if(recv(sock_desc, rbuff, MAX_SIZE, 0) == 0){
                printf("Error\n");
            }else{
                if(rbuff[0] == 0x01){
                    if(recv(sock_desc, rbuff, MAX_SIZE, 0) == 0){
                        printf("Error\n");
                    }else{
                        int listFiles = rbuff[0];
                        bzero(rbuff, strlen(rbuff));
                        for(int i = 0; i < listFiles; i++){
                            rbuff[strcspn(rbuff, "\n")] = 0;
                            if(recv(sock_desc, rbuff, MAX_SIZE - 1, 0) == 0){
                                printf("Error\n");
                            }else{
                                //printf("OK+ %s  %lu\n", rbuff, strlen(rbuff));
                                printf("OK+ %s\n", rbuff);
                                bzero(rbuff, strlen(rbuff));
                            }
                        }
                    }
                }
                printf("OK\n");
            }
        }

        // USER ENTERED DELETE COMMAND
      else  if (strcmp(givenCommand, "r") == 0){
            sbuff[0] = 0x04;
            send(sock_desc, sbuff, 1, 0);
            givenCommand = strtok(NULL, " \n");
            send(sock_desc, givenCommand, MAX_SIZE, 0);
            
            if(recv(sock_desc, rbuff, MAX_SIZE, 0) == 0){
                printf("Error\n");
            }else{
                if(rbuff[0] == 0x05){
                    printf("OK\n");
                }else if(rbuff[0] == 0xFF){
                    printf("SERROR File not found.\n");
                }
            }
        }

        // USER ENTERED DOWNLOAD COMMAND 
       else if (strcmp(givenCommand, "d") == 0){
            int b;
            char client_message[BUFFER_LENGTH];
            sbuff[0] = 0x06;
            send(sock_desc, sbuff, 1, 0);
            givenCommand = strtok(NULL, "\n");
            if(recv(sock_desc, rbuff, 1, 0) == 0){
                printf("Error\n");
            }else{
                // Sending file name
                send(sock_desc, givenCommand, strlen(givenCommand), 0);
                sleep(1);
                // Recieve the size
                if(b = recv(sock_desc, rbuff, sizeof(int), 0) == 0){
                    printf("Error\n");
                }else{
                    if(rbuff[0] == -1){
                                printf("SERROR file not found\n");
                            }else{
                                // sets ints
                                int bytenumber = rbuff[0];
                                int bytes = 0;
                                write_fd = open(givenCommand, O_WRONLY | O_CREAT, 0666);
                                
                                // Loop to get large files
                                do {
                                        bzero(client_message, sizeof(client_message));
                                        b = recv(sock_desc, client_message, BUFFER_LENGTH, 0);
                                        bytes += b;
                                        write(write_fd, client_message, strlen(client_message));

                                } while(bytes < bytenumber);

                                close(write_fd);
                                send(sock_desc, "1", 1, 0);
                                if(recv(sock_desc, rbuff, MAX_SIZE, 0) == 0){
                                    printf("Error\n");
                                }else{
                                    if(rbuff[0] == 0x07){
                                        printf("OK\n");
                                    }else{
                                        printf("Download Error\n");
                                    }
                                }
                            }
                }
            }
        }
    }
        
    close(sock_desc);
    return 0;
}
