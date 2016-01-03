#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#define F_CONNECTING 0
#define F_READING 1
#define F_WRITING 2
#define F_DONE 3

#define MaxClientNum 5
#define MaxCommandLen 1024

int linelen(int fd,char *ptr,int maxlen);
void clean_array(char array[],int size);
int main(int argc, char* argv[], char *envp[])
{
	char ip[MaxClientNum][50];
	char port[MaxClientNum][20];
	char file[MaxClientNum][20];
	char sock_ip[MaxClientNum][50];
	char sock_port[MaxClientNum][20];
	int conn=0;
	for(int i=0;i<MaxClientNum;i++){
		clean_array(ip[i],50);
		clean_array(port[i],20);
		clean_array(file[i],20);
		clean_array(sock_ip[i],50);
		clean_array(sock_port[i],20);
	}
	
	char *queryString=getenv("QUERY_STRING");
	setenv("REQUEST_METHOD","GET",1);
	setenv("SCRIPT_NAME","/net/gcs/104/0456115/public_html/hello.cgi",1);
	setenv("REMOTE_HOST","nplinux0.cs.nctu.edu.tw",1);
	setenv("REMOTE_ADDR","140.113.216.139",1);
	setenv("CONTENT_LENGTH","4096",1);
	setenv("AUTH_TYPE","http",1);
	setenv("REMOTE_USER","Jian_De",1);
	setenv("REMOTE_IDENT","Jian_De",1);
	
	
	char *temp;
	temp=strtok(queryString,"&");
	
	while(temp!=NULL){
		char head;
		char head2;
		char *parameter;
		int num;
		head=temp[0];
		head2=temp[1];
		
		if(head=='h'){
			num=atoi(temp+1)-1;
			parameter=strchr(temp,'=');
			strcpy(ip[num],parameter+1);
		}
		else if(head=='p'){
			num=atoi(temp+1)-1;
			parameter=strchr(temp,'=');
			strcpy(port[num],parameter+1);
		}
		else if(head=='f'){
			num=atoi(temp+1)-1;
			parameter=strchr(temp,'=');
			strcpy(file[num],parameter+1);
		}
		else if(head=='s'){//sh1 or sp1
			if(head2=='h'){
				num=atoi(temp+2)-1;
				parameter=strchr(temp,'=');
				strcpy(sock_ip[num],parameter+1);
			}
			else{
				num=atoi(temp+2)-1;
				parameter=strchr(temp,'=');
				strcpy(sock_port[num],parameter+1);
			}
		}
		else{
			fprintf(stderr,"parameter error");
			fflush(stderr);
		}
		temp=strtok(NULL,"&");
	}
	//************************************************ parser parameter end *****************************************
	//************************************************ initial client ***********************************************
	int csock[MaxClientNum];
	int file_fd[MaxClientNum];
	struct sockaddr_in dest[MaxClientNum];
	for(int i=0 ; i < MaxClientNum ; i++){
		csock[i]=-1;
		
		if(strcmp(ip[i],"")!=0 && strcmp(port[i],"")!=0 && strcmp(file[i],"")!=0 ){
			char filepath[MaxCommandLen];
			clean_array(filepath,MaxCommandLen);
			strcpy(filepath,file[i]);
			
			file_fd[i]=open(filepath,O_RDONLY);
			
			if(file_fd[i]!=-1){
				struct hostent *host;
				host = gethostbyname(ip[i]);
				
				if(strcmp(sock_ip[i],"")!=0){
					host = gethostbyname(sock_ip[i]);
				}
				else{
					host = gethostbyname(ip[i]);
				}
				
				bzero(&dest[i],sizeof(dest[i]));
				dest[i].sin_family = AF_INET;
				dest[i].sin_addr= *((struct in_addr *)host->h_addr);
				
				if(strcmp(sock_port[i],"")!=0){
					dest[i].sin_port=htons(atoi(sock_port[i]));
				}
				else{
					dest[i].sin_port=htons(atoi(port[i]));
				}
				
				csock[i] = socket(AF_INET,SOCK_STREAM,0);
				int flag = fcntl(csock[i],F_GETFL,0);
				fcntl(csock[i],F_SETFL, flag | O_NONBLOCK);
				
				if(connect(csock[i],(struct sockaddr *)&dest[i],sizeof(dest[i]))==-1){
					if(errno != EINPROGRESS){
						printf("connect error");
						close(csock[i]);
						csock[i]=-1;
					}
				}
				
				//send msg to sock_server
				if(strcmp(sock_ip[i],"")!=0){
					struct hostent *host_sock_server;
					struct sockaddr_in sock_server;
					int temp_port;
					char str[MaxCommandLen];
					host_sock_server = gethostbyname(sock_ip[i]);
					
					bzero(&sock_server,sizeof(sock_server));
					sock_server.sin_family = AF_INET;
					sock_server.sin_addr= *((struct in_addr *)host_sock_server->h_addr);
					sock_server.sin_port=htons(atoi(sock_port[i]));
					
					temp_port=atoi(sock_port[i]);
					inet_ntop(AF_INET, (void *)(&sock_server.sin_addr.s_addr), str, sizeof(str));
					
					unsigned char end=0;
                    unsigned char vn=4;
                    unsigned char cd=1;
                    unsigned char msg_port[2];
                    unsigned char msg_ip[4];
					
					msg_port[0]= (unsigned char)(temp_port/256);
					msg_port[1]= (unsigned char)(temp_port%256);
					
					sscanf(str,"%u.%u.%u.%u",&msg_ip[0],&msg_ip[1],&msg_ip[2],&msg_ip[3]);
					
					write(csock[i],&vn,1);
					write(csock[i],&cd,1);
					write(csock[i],msg_port,2);
					write(csock[i],msg_ip,4);
					write(csock[i],&end,1);
				}
			}
			else{
				fprintf(stderr, "File '%s' open failed\n", file[i]);
                fflush(stderr);
			}
		}
	}
	
	//****************************************start I/O*************************************************************
	
	char* s = "Test CGI";
	
	printf("Content-type: text/html\n\n");
	printf("<html>\n");
	
		printf("<head>\n");
			printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n");
			printf("<title>Network Programming Homework 3</title>\n");
		printf("</head>\n");
	
		printf("<body bgcolor=#1E90FF>\n");
		printf("<font face=\"Courier New\" size=2 color=#FFFF99>\n");
		printf("<table width=\"800\" border=\"1\">\n");
	
		printf("<tr>\n");
			for(int i=0;i<MaxClientNum;i++){
				printf("<td>%s</td>\n",ip[i]);
			}
		printf("</tr>\n");
		
		printf("<tr>\n");
			for(int i=0;i<MaxClientNum;i++){
				printf("<td valign=\"top\" id=\"m%d\"></td>", i);
			}
		printf("</tr>\n");
		printf("</table>\n");
	int nfds;
	int status[MaxClientNum];
	fd_set rfds; /* readable file descriptors*/
	fd_set wfds; /* writable file descriptors*/
	fd_set rs;   /* active file descriptors*/
	fd_set ws;   /* active file descriptors*/
	
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&rs);
	FD_ZERO(&ws);
	
	nfds=getdtablesize();
	
	for(int i=0;i<MaxClientNum;i++){
		if(csock[i]!=-1){
			status[i]=F_CONNECTING;
			FD_SET(csock[i],&rs);
			FD_SET(csock[i],&ws);
			conn++;
		}
		else{
			status[i]= F_DONE;
		}
	}
	unsigned char vn_reply[MaxClientNum]; 
	unsigned char cd_reply[MaxClientNum];
	unsigned char ip_reply[MaxClientNum][4];
	unsigned char port_reply[MaxClientNum][2];
	
	while(conn > 0){
		memcpy(&rfds,&rs,sizeof(rfds));
		memcpy(&wfds,&ws,sizeof(wfds));
		
		select(nfds,&rfds,&wfds,(fd_set *)NULL,(struct timeval *)NULL);
		
		
		for(int i=0;i<MaxClientNum;i++){
			if(status[i]==F_CONNECTING && ( FD_ISSET(csock[i], &rfds) || FD_ISSET(csock[i], &wfds) )){
				int n=sizeof(errno);
				if(getsockopt(csock[i],SOL_SOCKET,SO_ERROR,&errno,&n)<0 || errno!=0){
					return -1;
				}
				
				if(strcmp(sock_ip[i],"")!=0){
					read(csock[i],&vn_reply[i],1);
					read(csock[i],&cd_reply[i],1);
					read(csock[i],&port_reply[i],2);
					read(csock[i],&ip_reply[i],4);
					
					if(cd_reply[i] == 90){//granted
						status[i]=F_READING;
						FD_CLR(csock[i],&ws);
					}
					else if(cd_reply[i]==91){//fail
						status[i]=F_DONE;
						FD_CLR(csock[i],&rs);
						FD_CLR(csock[i],&ws);
						conn--;
						close(csock[i]);
						csock[i]=-1;
						continue;
					}
				}
				else{
					status[i]=F_READING;
					FD_CLR(csock[i],&ws);
				}
			}
			else if(status[i]==F_WRITING && FD_ISSET(csock[i],&wfds)){
				char command[MaxCommandLen];
				clean_array(command,MaxCommandLen);
				if(linelen(file_fd[i],command,MaxCommandLen) > 0){
					write(csock[i],command,strlen(command));
					
					strtok(command,"\r\n");
					
					printf("<script>document.all['m%d'].innerHTML += \"%% <b>%s</b><br>\";</script>\n", i, command);
					fflush(stdout);
					
					if(strcmp(command,"exit")==0){
						status[i]=F_DONE;
						FD_CLR(csock[i],&ws);
						conn--;
					}
					else{
						status[i]=F_READING;
						FD_CLR(csock[i],&ws);
						FD_SET(csock[i],&rs);
					}
				}
				else{
					fprintf(stderr,"Command error\n");
					fflush(stderr);
				}
			}
			else if(status[i]==F_READING && FD_ISSET(csock[i],&rfds)){
				char ResultFromServer[MaxCommandLen];
				clean_array(ResultFromServer,MaxCommandLen);
				
				if(linelen(csock[i],ResultFromServer,MaxCommandLen) > 0 || ( linelen(csock[i],ResultFromServer,MaxCommandLen)==-1 &&errno==EWOULDBLOCK )){
					
					strtok(ResultFromServer,"\r\n");
					
					if(strchr(ResultFromServer,'%')!=NULL){
						status[i]=F_WRITING;
						FD_CLR(csock[i],&rs);
						FD_SET(csock[i],&ws);
					}
					else{
						printf("<script>document.all['m%d'].innerHTML += \"%s<br>\";</script>\n", i, ResultFromServer);
						fflush(stdout);
					}
				}
				else{
					fprintf(stderr,"Read error\n");
					fflush(stderr);
				}
			}
		}
	}
		
		
	printf("</font>\n");
	printf("</body>\n");
	printf("</html>\n");
	fflush(stdout);
	return 0;
}
int linelen(int fd,char *ptr,int maxlen)
{
	int n,rc;
	char c;
	for(n=1;n<maxlen;n++){
		if((rc=read(fd,&c,1)) == 1){
			*ptr++ = c;	
			if(c=='\n')  break;
		}
		else if(rc==0){
			if(n==1)     return(0);
			else         break;
		}
		else return(-1);
	}
	*ptr=0;
	return(n);
}
void clean_array(char array[],int size)
{
	for(int i=0 ; i<size;i++){
		array[i]='\0';
	}
}