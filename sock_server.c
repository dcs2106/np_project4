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
#include <ctype.h>
#include <fcntl.h>
#include <arpa/inet.h>
#define  MaxHost 5
#define Maxlinelen 1024

int readline(int fd, char * ptr, int maxlen);
void clear_array(char array[],int len);

int main(int argc,char *argv[])
{
	int port;
	int sockfd;
	int clientfd;
	struct sockaddr_in dest;
	port=atoi(argv[1]);
	clearenv();
	chdir("/net/gcs/104/0456115/np_project4");
	
	bzero((char *)&dest,sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	dest.sin_addr.s_addr = INADDR_ANY;
	
	sockfd = socket(PF_INET,SOCK_STREAM,0);
	if(sockfd < 0){//socket
		fprintf(stderr,"socket error\n");
		exit(1);
	}
	
	bind(sockfd,(struct sockaddr*)&dest,sizeof(dest));
	listen(sockfd,MaxHost);//listen*/
	//sockfd = connectsock(argv[1],"tcp");
	printf("SERVER_PORT: %d\n",port);
	
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(client_addr);
	for(;;){
		clientfd = accept(sockfd,(struct sockaddr *)&client_addr, &addrlen);
		
		char request_ip[Maxlinelen];
		int request_port;
		
		inet_ntop(AF_INET,(void *)(&client_addr.sin_addr.s_addr),request_ip,sizeof(request_ip));
		request_port=(int)(ntohs(client_addr.sin_port));
		
		int childpid;
		childpid=fork();
		if(childpid==-1)printf("fork error\n");
		else if(childpid==0){
			unsigned char buf[Maxlinelen];//vn : 1
										  //cd : 1
										  //dst_port : 2
										  //dst_ip : 4
			char *user_id;
			read(clientfd,buf,Maxlinelen);
			unsigned char vn=buf[0];
			unsigned char cd=buf[1];
			unsigned char dst_port[2];
			dst_port[0]=buf[2];
			dst_port[1]=buf[3];
			unsigned char dst_ip[4];
			dst_ip[0]=buf[4];
			dst_ip[1]=buf[5];
			dst_ip[2]=buf[6];
			dst_ip[3]=buf[7];
			user_id=buf+8;
			
			if(vn == 4){
				int dstport;
				char domain[Maxlinelen];
				clear_array(domain,Maxlinelen);
				
				dstport=(int)(dst_port[0]*256+dst_port[1]);
				
				if(dst_ip[0]==0 && dst_ip[1]==0 && dst_ip[2]==0){//DST IP=0.0.0.x
					read(clientfd,domain,Maxlinelen);
				}
				else{
					sprintf(domain,"%u.%u.%u.%u",dst_ip[0],dst_ip[1],dst_ip[2],dst_ip[3]);			}
				
				printf("VN: %u, CD: %u, DST IP: %s, DST PORT: %d,USERID: %s\n",vn,cd,domain,dstport,user_id);
				fflush(stdout);
				
				struct hostent *he;
				
				
				he=gethostbyname(domain);
				if(he!=NULL){
					struct sockaddr_in fsin;
					
					bzero(&fsin,sizeof(fsin));
					fsin.sin_family= AF_INET;
					fsin.sin_addr = *((struct in_addr *)he->h_addr);
					fsin.sin_port = htons(dstport);
					
					
					if(cd==1){
						int csock;
						csock=socket(AF_INET,SOCK_STREAM,0);
						if(connect(csock,(struct sockaddr *)&fsin,sizeof(fsin))==-1){//fail
							unsigned char vn_reply; 
							unsigned char cd_reply;
							unsigned char ip_reply[4];
							unsigned char port_reply[2];
							
							vn_reply=0;
							cd_reply=91;
							memcpy(ip_reply,dst_ip,4);
							memcpy(port_reply,dst_port,2);
							
							//vn cd port ip
							write(clientfd,&vn_reply,1);
							write(clientfd,&cd_reply,1);
							write(clientfd,port_reply,2);
							write(clientfd,ip_reply,4);
							
							printf("REPLY_FAIL FOR FORMAT ERROR\n");
							fflush(stdout);
							csock=-1;
							
							return 0;
						}
						unsigned char vn_reply; 
						unsigned char cd_reply;
						unsigned char ip_reply[4];
						unsigned char port_reply[2];
						
						vn_reply=0;
						cd_reply=90;
						memcpy(ip_reply,dst_ip,4);
						memcpy(port_reply,dst_port,2);
						
						//vn cd port ip
						write(clientfd,&vn_reply,1);
						write(clientfd,&cd_reply,1);
						write(clientfd,port_reply,2);
						write(clientfd,ip_reply,4);
						
						printf("SOCKS_CONNECT GRANTED\n");
						fflush(stdout);
						
						
					}
				}
			}
			else{//not a socks packet
				unsigned char vn_reply; 
				unsigned char cd_reply;
				unsigned char ip_reply[4];
				unsigned char port_reply[2];
				
				vn_reply=0;
				cd_reply=91;
				memcpy(ip_reply,dst_ip,4);
				memcpy(port_reply,dst_port,2);
				
				//vn cd port ip
				write(clientfd,&vn_reply,1);
				write(clientfd,&cd_reply,1);
				write(clientfd,port_reply,2);
				write(clientfd,ip_reply,4);
				
				printf("REPLY_FAIL FOR FORMAT ERROR\n");
				fflush(stdout);
			}
			return 0;
		}
		else{
			close(clientfd);
			continue;
		}
	}
	return 0;
}
int readline(int fd, char * ptr, int maxlen){ 
    int  n, rc;
    char  c; 
    
    for (n = 1; n < maxlen; n++) { 
        if ( (rc = read(fd, &c, 1)) == 1) { 
            *ptr++ = c; 
            if (c == '\n')      break; 
        }
        else if (rc == 0) { 
            if (n == 1) return 0;  /* EOF, no data read */ 
            else  break;  /* EOF, some data was read */ 
        }
        else return -1;  /* error */ 
    } 
    *ptr = 0;
    
    return(n); 
}
void clear_array(char array[],int len)
{
	for(int i=0;i<len;i++){
		array[i]='\0';
	}
}