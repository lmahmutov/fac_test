#include "product_test.h"
#include "command.h"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include "Wifi.h"



int udp_fd = -1;

pthread_t udp_cmd_id, udp_broadcast_id;


bool exit_broadcast = false;
bool udp_send_status = false;

struct sockaddr_in sock_from = {0};

char cli_ip[50] = {0};


int send_message(char *buf);

/*
int get_local_ip_mac(char *buf_ip, char *buf_mac)
{
	struct sockaddr_in *local_ip;
	struct ifreq ifr_ip;
	int sockfd = -1;
	char ipaddr[50];

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("get_local_ip socket");
		return -1;
	}

	memset(&ifr_ip, 0, sizeof(struct ifreq));
	strncpy(ifr_ip.ifr_name, network_interface, sizeof(ifr_ip.ifr_name) - 1);
	
	if(buf_ip){
		if(ioctl(sockfd, SIOCGIFADDR, &ifr_ip) < 0){
			perror("ioctl socket");
			return -1;
		}

		local_ip = (struct sockaddr_in *)&ifr_ip.ifr_addr;
		strcpy(ipaddr, inet_ntoa(local_ip->sin_addr));
		//printf("local ip --> %s\n", ipaddr);

		strcpy(buf_ip, ipaddr);
		//printf("local ip --> %s\n", buf_ip);
	}

	if(buf_mac){
		if(ioctl(sockfd, SIOCGIFHWADDR, &ifr_ip) < 0){
			perror("ioctl socket");
			return -1;
		}

		sprintf(buf_mac, "%02x%02x%02x%02x%02x%02x", \
				ifr_ip.ifr_hwaddr.sa_data[0], \
				ifr_ip.ifr_hwaddr.sa_data[1], \
				ifr_ip.ifr_hwaddr.sa_data[2], \
				ifr_ip.ifr_hwaddr.sa_data[3], \
				ifr_ip.ifr_hwaddr.sa_data[4], \
				ifr_ip.ifr_hwaddr.sa_data[5]);
		//printf("mac --> %s\n", buf_mac);
	}
	close(sockfd);
}
*/

void *udp_broadcast(void *arg)
{
	struct sockaddr_in sockaddr = {0};
	int sockfd = -1;
	int ret = -1;
	char buf[100];
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1){
		perror("udp_broadcast socket");
		pthread_exit(NULL);
	}
	printf("udp_broadcast socketfd:%d\n", sockfd);
	
	const int opt = 1;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if(ret == -1){
		perror("udp_broadcast setsockopt");
		pthread_exit(NULL);
	}

	//bzero(&sockaddr, sizeof(struct sockaddr_in)); 
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	sockaddr.sin_port = htons(BROADCAST_PORT); //要使用htons

	sprintf(buf, "%s", (char *)arg);
	
	while(1){
		sleep(1);
		if(exit_broadcast == false){
			ret = sendto(sockfd, buf, strlen(buf), 0, \
						(const struct sockaddr *)&sockaddr, sizeof(sockaddr));
			if(ret < 0)
				printf("broadcast fail!\n");
			else
				printf("broadcast --> %s\n", buf);
		}
		else
			break;
	}
	pthread_exit(NULL);
}

void *get_cmd_from_udp(void *arg)
{
	int ret = -1;
	struct sockaddr_in sockaddr = {0};
	char buf[MAXINPUTCHAR];
	size_t rev_len = 0;
	socklen_t len;
	//char cli_ip[50] = {0};
	
	if((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("udp_fd socket");
		pthread_exit(NULL);
	}

	const int opt = 1;
	ret = setsockopt(udp_fd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if(ret == -1){
		perror("udp_fd setsockopt");
		pthread_exit(NULL);
	}

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr.sin_port = htons(UDP_CMD_PORT);

	ret = bind(udp_fd, (const struct sockaddr *)&sockaddr, sizeof(sockaddr));
	if(ret < 0){
		perror("udp_fd bind");
		pthread_exit(NULL);
	}

	while(1){
		memset(buf, 0, MAXINPUTCHAR);
		len = sizeof(struct sockaddr_in);
		rev_len = recvfrom(udp_fd, buf, MAXINPUTCHAR, 0, \
						  (struct sockaddr*)&sock_from, (socklen_t*)&len);

		inet_ntop(AF_INET, &sock_from.sin_addr.s_addr, cli_ip, len);
		//printf("cli_ip --> %s:%d\n", cli_ip, sock_from.sin_port);

		if(rev_len <= 0){
			perror("udp recv");
		}
		else{
				exit_broadcast = true;
				strcpy((char *)arg, buf);
			//printf("udp input buf:%s\n", buf);
		}
	}
}

int send_message(char *buf)
{
	//system("echo 2 >> /tmp/output_buf");
	while(udp_fd == -1);
	while(cli_ip[1] == 0);
	if(!buf){
		return -1;
	}
	char buf_s[MAXBUF] = {0};
	char *send_b = buf_s;
	int i = 0;
	
	memcpy(buf_s, buf, MAXBUF);

	for(;i < MAXBUF; i++){
		if(buf_s[i] == '\n'){
			buf_s[i] = '\0';
			if(udp_fd > 0){
					sendto(udp_fd, send_b, strlen(send_b), 0, \
							(const struct sockaddr *)&sock_from, sizeof(sock_from));
			}
			if((i + 1) < MAXBUF && buf_s[i + 1] != '\0'){
				send_b = &buf_s[i + 1];
			}
		}
		else if(buf_s[i] == '\0'){
			break;
		}
	}
/*	
	if(udp_fd > 0){
		sendto(udp_fd, buf_s, strlen(buf_s), 0, \
				(const struct sockaddr *)&sock_from, sizeof(sock_from));
*/
		return 0;
//	}
}

#if 0
int main()
{
	char buf[MAXINPUTCHAR];
	get_local_ip_mac(local_ip, local_mac);
	pthread_create(&udp_broadcast_id, NULL, udp_broadcast, NULL);
	pthread_detach(udp_broadcast_id);
	pthread_create(&udp_cmd_id, NULL, get_cmd_from_udp, (void *)buf);
	pthread_detach(udp_cmd_id);
	while(1);
	return 0;
}
#endif
