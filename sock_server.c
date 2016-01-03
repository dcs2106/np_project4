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
void printcontent(char array[]);
int main(int argc,char *argv[])
{
	int port;
	int sockfd;
	int clientfd;
	struct sockaddr_in dest;
	//port=atoi(argv[1]);
	port=6555;
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
	int optval=1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
	bind(sockfd,(struct sockaddr*)&dest,sizeof(dest));
	listen(sockfd,MaxHost);//listen*/
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
			int csock;
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
			
			unsigned char vn_reply; 
			unsigned char cd_reply;
			unsigned char ip_reply[4];
			unsigned char port_reply[2];
			
			
			if(vn == 4){
				int dstport;
				char domain[Maxlinelen];
				clear_array(domain,Maxlinelen);
				
				dstport=(int)(dst_port[0]*256+dst_port[1]);
				
				if(dst_ip[0]==0 && dst_ip[1]==0 && dst_ip[2]==0){//DST IP=0.0.0.x
					read(clientfd,domain,Maxlinelen);
				}
				else{
					sprintf(domain,"%u.%u.%u.%u",dst_ip[0],dst_ip[1],dst_ip[2],dst_ip[3]);			
				}
				printf("--------------------------------------------------------------------------------\n");
				printf("VN: %u, CD: %u, DST IP: %s, DST PORT: %d,USERID: %s\n",vn,cd,domain,dstport,user_id);
				printf("Permit Src = %s(%d), DST = %s(%d)\n",inet_ntoa(client_addr.sin_addr),client_addr.sin_port,domain,dstport);
				printf("--------------------------------------------------------------------------------\n");
				fflush(stdout);
				
				struct hostent *he;
				
				
				he=gethostbyname(domain);
				if(he!=NULL){
					struct sockaddr_in fsin;
					
					bzero(&fsin,sizeof(fsin));
					fsin.sin_family= AF_INET;
					fsin.sin_addr = *((struct in_addr *)he->h_addr);
					fsin.sin_port = htons(dstport);
					
					/*char destip[Maxlinelen];
					inet_ntop(AF_INET, (void *)(&fsin.sin_addr.s_addr), destip, sizeof(destip));
					
					int filefd;
					char data[Maxlinelen];
					char IP[Maxlinelen];
					char ip[4];
					char *str;
					
					int access=0;
					
					filefd=open("socks.conf",O_RDONLY);
					readline(filefd,data,Maxlinelen);//permit c 140.113.*.*
					
					str=strtok(data," ");//permit
					str=strtok(NULL," ");//c
					str=strtok(NULL," ");//140.113.*.*
					strcpy(IP,str);
					
					if(strncmp(IP,destip,8)==0){
						access=1;
						printf("Permit Src = %s(%d), DST = %s(%d)\n",inet_ntoa(client_addr.sin_addr),client_addr.sin_port,domain,dstport);
					}
					else{
						vn_reply=0;
						cd_reply=91;
						memcpy(ip_reply,dst_ip,4);
						memcpy(port_reply,dst_port,2);
						
						//vn cd port ip
						write(clientfd,&vn_reply,1);
						write(clientfd,&cd_reply,1);
						write(clientfd,port_reply,2);
						write(clientfd,ip_reply,4);
						
						printf("Deny access\n");
						fflush(stdout);
						
						return 0;
					}*/
					
					if(cd==1){//connect
						csock=socket(AF_INET,SOCK_STREAM,0);
						if(connect(csock,(struct sockaddr *)&fsin,sizeof(fsin))==-1){//fail
							//unsigned char vn_reply; 
							//unsigned char cd_reply;
							//unsigned char ip_reply[4];
							//unsigned char port_reply[2];
							
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
						//unsigned char vn_reply; 
						//unsigned char cd_reply;
						//unsigned char ip_reply[4];
						//unsigned char port_reply[2];
						
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
						
						int nfds;
						fd_set cfds;
						fd_set afds;
						
						nfds=getdtablesize();
						FD_ZERO(&afds);
						FD_SET(clientfd,&afds);
						FD_SET(csock,&afds);
						
						int conn = 2;
						while(conn > 0){
							memcpy(&cfds,&afds,sizeof(cfds));
							
							if(select(nfds,&cfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL) < 0){
								printf("cd=1,select error");
								fflush(stdout);
								exit(0);
							}
							if(FD_ISSET(clientfd,&cfds)){
								int len;
								char browserbuf[Maxlinelen];
								clear_array(browserbuf,Maxlinelen);
								
								len = read(clientfd,browserbuf,Maxlinelen);
								printcontent(browserbuf);
								if(len > 0){
									write(csock,browserbuf,len);
								}
								else{
									conn--;
									FD_CLR(clientfd,&afds);
								}
							}
							if(FD_ISSET(csock,&cfds)){
								int len;
								char serverbuf[Maxlinelen];
								clear_array(serverbuf,Maxlinelen);
								
								len = read(csock,serverbuf,Maxlinelen);
								printcontent(serverbuf);
								if(len > 0){
									write(clientfd,serverbuf,len);
								}
								else{
									conn--;
									FD_CLR(csock,&afds);
								}
							}
						}
						
					}
					else if (cd==2){
						int psock;
						struct sockaddr_in bind_addr;
						int BIND_PORT;
						BIND_PORT=port+1;
						
						bzero((char *)&bind_addr,sizeof(bind_addr));
						bind_addr.sin_family = AF_INET;
						bind_addr.sin_addr.s_addr = htons(INADDR_ANY);
						bind_addr.sin_port = htons(INADDR_ANY);
						
						psock=socket(AF_INET,SOCK_STREAM,0);
						if(psock < 0){
							printf("psock error\n");
							exit(3);
						}
						
						bind(psock,(struct sockaddr*)&bind_addr,sizeof(bind_addr));
						
						struct sockaddr_in sa;
						int sa_len;
						getsockname(psock,(struct sockaddr *)&sa,&sa_len);
						
						listen(psock,MaxHost);
						
						
						//unsigned char vn_reply; 
						//unsigned char cd_reply;
						//unsigned char ip_reply[4];
						//unsigned char port_reply[2];
						
						vn_reply=0;
						cd_reply=90;
						port_reply[0]=(unsigned char)(ntohs(sa.sin_port)/256);
						port_reply[1]=(unsigned char)(ntohs(sa.sin_port)%256);
						ip_reply[0]=0;
						ip_reply[1]=0;
						ip_reply[2]=0;
						ip_reply[3]=0;
						write(clientfd,&vn_reply,1);
						write(clientfd,&cd_reply,1);
						write(clientfd,port_reply,2);
						write(clientfd,ip_reply,4);
						
						int ftp_fd;
						struct sockaddr_in ftp_addr;
						socklen_t ftp_addrlen=sizeof(ftp_addr);
						ftp_fd=accept(psock,(struct sockaddr *)&ftp_addr,&ftp_addrlen);
						if(ftp_fd<0){
							printf("ftp accept error\n");
							fflush(stdout);
						}
						
						//reply again
						write(clientfd,&vn_reply,1);
						write(clientfd,&cd_reply,1);
						write(clientfd,port_reply,2);
						write(clientfd,ip_reply,4);
						
						printf("SOCKS_BIND GRANTED\n");
						fflush(stdout);
						
						int nfds;
						nfds=getdtablesize();
						fd_set cfds;
						fd_set afds;
						
						FD_ZERO(&afds);
						FD_SET(ftp_fd,&afds);
						FD_SET(clientfd,&afds);
					
						int conn=1;
						while(conn>0){
							FD_ZERO(&cfds);
							memcpy(&cfds,&afds,sizeof(cfds));
							
							if(select(nfds,&cfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL) < 0){
								printf("cd=2,select error");
								fflush(stdout);
								exit(0);
							}
							if(FD_ISSET(clientfd,&cfds)){
								int len;
								char browserbuf[Maxlinelen];
								clear_array(browserbuf,Maxlinelen);
								
								len = read(clientfd,browserbuf,Maxlinelen);
								printcontent(browserbuf);
								if(len > 0){
									write(ftp_fd,browserbuf,len);
								}
								else{
									conn--;
									FD_CLR(clientfd,&afds);
								}
							}
							if(FD_ISSET(ftp_fd,&cfds)){
								int len;
								char serverbuf[Maxlinelen];
								clear_array(serverbuf,Maxlinelen);
								
								len = read(ftp_fd,serverbuf,Maxlinelen);
								printcontent(serverbuf);
								if(len > 0){
									write(clientfd,serverbuf,len);
								}
								else{
									conn--;
									FD_CLR(ftp_fd,&afds);
								}
							}
						}
					}
					else{//cd != 1 !=2
						//unsigned char vn_reply; 
						//unsigned char cd_reply;
						//unsigned char ip_reply[4];
						//unsigned char port_reply[2];
						
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
				}
			}
			else{//not a socks packet
				//unsigned char vn_reply; 
				//unsigned char cd_reply;
				//unsigned char ip_reply[4];
				//unsigned char port_reply[2];
				
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
				close(clientfd);
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
void printcontent(char array[])
{
	char temp[20];
	strncpy(temp,array,20);
	printf("Content : %s\n",temp);
}
