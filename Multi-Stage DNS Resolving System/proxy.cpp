
#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>

// Function for converting a string to integer
int StrToInt(char *str){
    int len=strlen(str), z=0;
    for(int i=0;i<len;i++){
        z= z * 10;
        z = z+ str[i]-'0';
    }
    return z;
}

void sockSuccess(){
    printf("Client socket for proxy is being created!!!\n");
}

void connection(){
    printf("Connection established woth the DNS server\n");
}
//Function for connecting with the DNS Server and updating the Proxy Cache
int goproxy(char buff[], char *p2d_buff)
{
    int sockfncd, servport;
    char serverip[256];
    struct sockaddr_in servaddr, client;
    printf("Please input the DNS server IP and Port:\n");
    scanf("%s", serverip);
    scanf("%d", &servport);
	printf("Serverip: %s\tserver port: %d\n", serverip, servport);

    //Creating socket
    if((sockfncd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        printf("Error in creating client socket for proxy\n");
        return 0;
    }
    else
    {
        sockSuccess();
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=inet_addr(serverip);
    servaddr.sin_port=htons(servport);

    // Connecting to the server
    if(connect(sockfncd, (struct sockaddr*) &servaddr, sizeof(servaddr))!=0)
    {
        printf("Connection with DNS server failed\n");
        return 0;
    }
    else
    {
        connection();
    }
    
    // Forward the cache miss request to the DNS server
    send(sockfncd, buff, 256, 0);

    char recv_buff[256];
    memset(recv_buff, 0, 256);
    int recvlength = recv(sockfncd, recv_buff, 256, 0);

    printf("Response from DNS server: %s\n", recv_buff);
    if(recv_buff == NULL){
        printf("DNS server did not respond\n");
        return 0;
    }
    // Entry found in DNS server, update cache
    if(recv_buff[0]=='0')
    {
        FILE *fp;
        fp=fopen("proxyCache.txt", "r");
        int c=0;
        char lbuffer[256];
        memset(lbuffer, 0, 256);
        
        // Counting the number of entries in cache
        while(fgets(lbuffer, 256, fp))
            c++;
        fclose(fp);

        if(c<3)
        {
            // Add the entry to the cache file
            fp=fopen("proxyCache.txt", "a");
            char str;
            if(buff[0]=='0')
            {
                for(int i=1; i<256 && buff[i]!='\0'; i++)
                    fputc(buff[i], fp);
                fputc(' ', fp);
                for(int i=1; i<256 && recv_buff[i]!='\0'; i++)
                    fputc(recv_buff[i], fp);
                fputc('\n', fp);
            }
            else
            {
                for (int i = 1; i < 256 && recv_buff[i] != '\0'; i++)
                    fputc(recv_buff[i], fp);
                fputc(' ', fp);
                for (int i = 1; i < 256 && buff[i] != '\0'; i++)
                    fputc(buff[i], fp);
                fputc('\n', fp);
            }
        }
        // else use FIFO 
        else
        {
            FILE *fp1, *fpt;
            fp1=fopen("proxyCache.txt", "r");
            fpt=fopen("temp.txt", "w");
            char copy_buff[256];
            memset(copy_buff, 0, 256);
            fgets(copy_buff, 256, fp1);
            while(c-- > 1)
            {
                fgets(copy_buff, 256, fp1);
                fprintf(fpt, "%s", copy_buff);
            }
            fclose(fp1);
            if (buff[0] == '0')
            {
                for (int i = 1; i < 256 && buff[i] != '\0'; i++)
                    fputc(buff[i], fpt);
                fputc(' ', fpt);
                for (int i = 1; i < 256 && recv_buff[i] != '\0'; i++)
                    fputc(recv_buff[i], fpt);
                fputc('\n', fpt);
            }
            else
            {
                for (int i = 1; i < 256 && recv_buff[i] != '\0'; i++)
                    fputc(recv_buff[i], fpt);
                fputc(' ', fpt);
                for (int i = 1; i < 256 && buff[i] != '\0'; i++)
                    fputc(buff[i], fpt);
                fputc('\n', fpt);
            }
            fclose(fpt);
            remove("proxyCache.txt");
            rename("temp.txt", "proxyCache.txt");
        }
    }
    printf("Response received from the DNS server: %s\n", recv_buff);
    // terminate connection with the DNS server
    close(sockfncd);
    strcpy(p2d_buff, recv_buff);
    return 0;
}

// function that establishConnections with the client
int establishConnection(int sockfncd)
{
    char buff[256];
    int n;
    int check = 1;
    memset(buff, 0, 256);
    int messageLength;
    // recieve message from the client
    messageLength = recv(sockfncd, buff, sizeof(buff), 0);

	printf("Request msg from client: %s\n", buff);
    FILE *fp;
    if(fp=fopen("proxyCache.txt", "r"))
    {
        printf("cache file open successful\n");
    }
    else
    {
        printf("cache file open failed\n");
        return 0;
    }
    char lbuffer[256];
    memset(lbuffer, 0, 256);

    // flag indication cache miss/hit
    int cache_flag=0;
    int check_flag = 0;
    // iterate over the lines in the cache file
    while(fgets(lbuffer, 256, fp))
    {
        // if request of type 0 ie requesting the ip_addr
        if(buff[0]=='0')
        {
            int i;
            for(i=0; i<sizeof(lbuffer)/sizeof(char); i++)
                if(lbuffer[i]==' ')
                    break;
            
            int j=i+1, k;
            for(k=j; k<sizeof(lbuffer)/sizeof(char); k++)
                if(lbuffer[k]=='\n')
                    break;
            if(strncmp(buff+1, lbuffer, messageLength-1) != 0)
            {
                continue;
            }
            cache_flag=1;

            char send_buf[k-j+2];

			send_buf[0]='0';
            for(int itr=j+1; itr<=k; itr++)
                send_buf[itr-j]=lbuffer[itr-1];
            send_buf[k-j+1] = '\0';
            printf("Response to client: %s\n", send_buf);
            // send the requested data to the client using sockfncd
            check = check_flag;
            send(sockfncd, send_buf, sizeof(send_buf), 0);
            break;
        }
        else if(buff[0]=='1')
        {
            int i;
            for(i=0; i<sizeof(lbuffer)/sizeof(char); i++)
                if(lbuffer[i]==' ')
                    break;
            int j=i+1;

            if(strncmp(buff+1, lbuffer+j, messageLength-1) != 0)
                continue;
            
            cache_flag=1;
            char send_buf[i+2];
            send_buf[0]='0';
            for(int itr=1; itr<=i; itr++)
                send_buf[itr]=lbuffer[itr-1];
            send_buf[i+1]='\0';
            check_flag = 1;
            send(sockfncd, send_buf, sizeof(send_buf), 0);
            break;
        }
    }
    fclose(fp);
    if(cache_flag)
        return 0;
    if(check_flag == 1){
    	check_flag = 0;
    }
    printf("Proxy cache miss\n");
    char p2d_buff[30];
    // contact the DNS server
    goproxy(buff, p2d_buff);

    // forward the response from DNS server
	send(sockfncd, p2d_buff, 256, 0);
    close(sockfncd);
    return 0;
}

int main(int argc, char *argv[])
{
    int port, sockfncd, new_socket;
    if ((sockfncd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error in creating server socket for proxy\n");
        return 0;
    }
    else
    {
        printf("Server socket for proxy created\n");
    }
    

    port=StrToInt(argv[1]);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if ((bind(sockfncd, (struct sockaddr *)&address, sizeof(address))) < 0)
    {
        printf("Socket bind failed\n");
        return 0;
    }
    else
    {
        printf("Socket bind successful for proxy\n");
    }
    

    if ((listen(sockfncd, 5)) < 0)
    {
        printf("Listening failed\n");
        return 0;
    }
    else
    {
        printf("Proxy server listening\n");
    }
    
    int Addresslength = sizeof(address);
    int count = 0; 
    listen:
    if ((new_socket = accept(sockfncd, (struct sockaddr *)&address, &Addresslength)) >= 0)
    {
        count++;
        printf("Connection #%d accpeted from client\n", count);
        
    }
    // function thread that takes care of further establishConnection
    int pid = fork();
    if (pid == 0)
        establishConnection(new_socket);
    else
    {
        goto listen;
    }
    
    
    // free the socket
    close(sockfncd);
    
    return 0;
}
