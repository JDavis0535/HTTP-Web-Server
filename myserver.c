#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>


void *HTTPparse(void *new_socket ){

	char buffer[1024];
	char header[200];
	char *Filepath, *Method;
	int sizeofmessage;
	size_t bytesread;
	char *buff2 = NULL;

	//get the socket descriptor
	int sock = *(int*)new_socket;

	//Get input from client
	sizeofmessage = recv(sock, buffer, 1024, 0);
	buffer[sizeofmessage] = '\0';

	printf("%s\n", buffer);

	//Parse the HTTP GET request
	Method = strtok(buffer, " \n\r");

	//Path of file
	Filepath = strtok(NULL, " \n\r");

	//Open the file that we wish to send
        FILE * fp = fopen(Filepath,"rb");

        //check if it opened correctly
	//If it doesn't find file send 404
        if(fp== NULL)
        {
        sprintf(header, "HTTP/1.1 404 Not Found\r\n\r\n<html><title>NOT FOUND</title><body>The file was not found</body></html>");
	send (sock, header, sizeof(header), 0);
	return;
        }

	//Getting the file size
        fseek(fp, 0, SEEK_END);
        long lSize=ftell(fp);// size
        fseek(fp, 0, SEEK_SET);


	//Checking file extensions
	if (strstr(Filepath, ".html")){
	sprintf(header,  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %lu\r\n\r\n", lSize);
	}

	if (strstr(Filepath, ".gif")){
        sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Length: %lu\r\n\r\n", lSize);
	}

	else if (strstr(Filepath, ".jpg")){
	sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %lu\r\n\r\n", lSize);
	}

	else if(strstr(Filepath, ".txt")){
	sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %lu\r\n\r\n", lSize);
	}

	//Allocating buffer
	buff2 = header;
	buff2 = malloc (lSize+ sizeof(header));

        // error check malloc
        if (buff2 == NULL){
        fprintf(stderr, "Error allocating memory for file");
        fclose(fp);
        return;
        }

	// Read file mand send the bytes read
	while((	bytesread = fread(buff2, 1, lSize, fp)) > 0){

	//Make sure all bytes are read from file
	if(bytesread !=lSize){
	fputs ("Reading error",stderr);
	}

	//Send the file to the client
        send(sock, buff2, bytesread, 0);

	}

	//close the file
	fclose(fp);
	//close socket
	close(sock);

}

int main(int argc, char *argv[]){

	int socket_desc;
        struct sockaddr_in server, client;

	//Make sure input is formatted correctly
	if(argc != 3)
        {
	puts("Input format incorrect");
        exit(1);
        }

        //Find port
	int port = atoi(argv[1]);

        //Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM, 0);

	if (socket_desc == -1){

		puts("Could not create socket");
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind socket to port
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
        puts("Bind failed");
        close(socket_desc);
        exit(1);
	}

	//Listen for incoming connection on the socket
	listen(socket_desc, 3);

	//Accept an incoming connection
	printf("Waiting for incoming connections on port %i \n", port);
    	socklen_t size = sizeof(struct sockaddr_in);

        pthread_t thread;

        while(1){
    	int new_socket = accept(socket_desc, (struct sockaddr *)&client, &size);

		puts("Connection accepted");

		int *sock_two = malloc(sizeof(int));
		*sock_two = new_socket;

		if(pthread_create( &thread, NULL,  HTTPparse, sock_two) < 0) {
			perror("could not create thread");
			return 1;
		}

	if (new_socket < 0) {
		perror("accept failed");
		return 1;
	}
	}
        return 0;

}
