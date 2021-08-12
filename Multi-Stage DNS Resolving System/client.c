#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>
#include<string.h>
#define MAX_LIMIT 50
#define PORT 8080
// function that trims "www" infront of a url
void InputMessage(char str[],char *dst, int flag){
    int len = strlen(str);
    dst[0] = '0'+flag;
    if(len>4 && str[0]=='w' && str[1]=='w' && str[2]=='w' && str[3]=='.') {
        strcat(dst, str+4);
    }
    else {
        strcat(dst, str);
    }
    return;
}
// function to convert string to integer
int StrToInt(char *str){
    int len=strlen(str), z=0;
    for(int i=0;i<len;i++){
        z= z * 10;
        z = z+ str[i]-'0';
    }
    return z;
}

int main(int argc, char** argv){
    // socket file descriptor
    int serverPort,SockFileDes = 0;
    char *serverIpAddr;
    serverIpAddr = argv[1];
    serverPort = StrToInt(argv[2]);
    printf("%s\t", serverIpAddr);
    printf("%d\n", serverPort);

    // Setup server socket address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_aton(serverIpAddr,(struct in_addr* )&serverAddr.sin_addr.s_addr);

    // create socket
    SockFileDes = socket(AF_INET,SOCK_STREAM,0);
    if(SockFileDes<0) {
        printf("Error in creating socket\n");
        return 0;
    } else {
        printf("Socket Creation Successful\n");
    }

    // Establishing connection
    if (connect(SockFileDes,(struct sockaddr*) &serverAddr,sizeof(serverAddr)) < 0) {
        printf("Error in establishing the connection with the server\n");
        return 0;
    }
    else {
        printf("Establishing connection with the server Successful\n");
    }
    char str[256],InputMsg[256];
    int flag;
    char type[256];

    // Input DNS request message
    printf("Requesting input: request and type of request\n");
    scanf("%s %s", type, str);
    if(type[4]=='1') flag = 0;
    if(type[4]=='2') flag = 1;

    InputMessage(str,InputMsg,flag);
    printf("Request Message to server: %s\n", InputMsg);

    send(SockFileDes,InputMsg,strlen(InputMsg)*sizeof(char),0);
    char Response[256];
    recv(SockFileDes,Response,sizeof(Response),0);
    printf("Response from server: %s\n", Response);
    printf("The requested data: %s\n", Response+1);

    // free the socket
    close(SockFileDes);

    return 0;
}
