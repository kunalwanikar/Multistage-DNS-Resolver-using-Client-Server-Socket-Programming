#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#define PORT 8080 
#define SA struct sockaddr

// Function for converting a string to integer
int StrToInt(char *str){
    int len=strlen(str), z=0;
    for(int i=0;i<len;i++){
        z= z * 10;
        z = z+ str[i]-'0';
    }
    return z;
}	

int Dnsmain(int serverSocket){ //It is called once connection of proxy_server is accepted by DNS server
	char buffer[256];
	int n;
	
	char IP[20]="0";
	char dom[30]="0";
	char delimiter='\0'; 
	int check_flag = 0;
	memset(buffer,0,sizeof(buffer));
	
	n=recv(serverSocket,buffer,sizeof(buffer),0);
	
	if (n < 0) 
	{
        perror("ERROR reading from socket");
        exit(1);
    }
       
	
	printf("proxy server sent %s to server\n",buffer);
	
	
	char file_data[256];
	int flag = 0;
	char error[256]="1entry not found in the database\0";


	FILE *fptr;
	fptr = fopen("dnsCache.txt","r"); //dnsCache.txt is our directory of address mapings 
	

if(buffer[0]=='0' || buffer[0]=='1'){

	while( fscanf(fptr,"%[^\n]\n",file_data)!=-1)
	{
		int i,j;
		for (i = 0; i < 18; i++)
		{
			if(file_data[i]==' ')
				break;
		}
		strncat(IP,file_data,i);
		strncat(IP,&delimiter,1);

		for (j = i+1; j < 256; j++)
		{
			if (file_data[j]=='\n')
				break;
		}
		strncat(dom,file_data+i+1,j-i-1);
		strncat(dom,&delimiter,1);  			
		
		//ip address is of length i and is f[0->i-1]
		//dom name is of size j-i-1 and is f[i+1 -> j-1] 

		// proxy_server requesting IP addresss
		if (buffer[0]=='0')
		{
			if(strncmp(file_data+i+1,buffer+1,j-i-1)==0)
			{
				printf("Boom Hit! Ip requested is:  %s\n", IP);
				send(serverSocket, IP,strlen(IP)*sizeof(char),0);
				flag = 1;
				break;
			}
			
		}
		//proxy_server requesting dom_name
		else if (buffer[0]=='1')
		{
			if(strncmp(file_data,buffer+1,i)==0)
			{
				printf("Boom Hit! Dom name requested is:  %s\n", dom);
				send(serverSocket, dom,strlen(dom)*sizeof(char),0);
				flag = 1;
				break;
			}
		}
	}
	if (flag==0)
	{
		send(serverSocket,error,strlen(error)*sizeof(char),0);
		printf("%s\n", error);
	}
	}
	
	if(check_flag == 1){
    		check_flag = 0;
    	}

	fclose(fptr);
	close(serverSocket);
	return 0;
}

int main(int argc, char* argv[]){

	int serverSocket, connectSocket;
    // socket create and verification 
	//sockfd= socket(AF_INET,SOCK_STREAM,0);
	//creating a IPv4 socket for TCP returns Socet file descriptor
    if ((serverSocket = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
    	printf("socket creation failed!! \n");
    	return 0;
    } 
    
    printf("socket creation successful!\n");
    
	
    int port = StrToInt(argv[1]);
         
   // Configurinng the socket's address.
    struct sockaddr_in address;
   
    bzero(&address,sizeof(address)); 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port=htons(port);

    if(bind(serverSocket, (struct sockaddr *) &address, sizeof(address)) < 0) 
      {
          perror("ERROR on binding");
      	  exit(1);
      } 
    
    
    if ((listen(serverSocket,5)) < 0)
    {
    	printf("Socket is not in listening state!\n");
    	return 0;
    }
    
	
    printf("Now socket is in listening state!\n");
	
	int client_len = sizeof(address);
	while (1)
	{
    	if ((connectSocket = accept(serverSocket,(struct sockaddr *)&address,&client_len))<0)
    	{
    		perror("Socket Accept Failure : ");
			exit(EXIT_FAILURE);
    	}
    	
    	printf("YAY! server accepted client!\n");

    	Dnsmain(connectSocket);
	}

    close(serverSocket);
	return 0;
}
