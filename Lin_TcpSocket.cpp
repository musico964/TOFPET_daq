/* Lin_TcpSocket.cpp
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define PORT 30

#include "Lin_TcpSocket.h"

void Lin_TcpSocket::_GetMyInetAddr(void)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);	// get "eth0" from config.txt

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);
	strcpy(my_inet_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
//printf("my_inet_addr = '%s'\n", my_inet_addr);
}

int Lin_TcpSocket::TcpSocket_linuxOpen(char *inetaddr)
{
	TcpSocket_linux_status = 1;
#ifdef TCP
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
#else
//	if( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 )
	if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )
#endif
	{
		perror("socket");
		TcpSocket_linux_status = 0;
		return BUS_COMM_ERROR;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr=inet_addr(inetaddr);
	servaddr_len = sizeof(servaddr);
	_GetMyInetAddr();

#ifdef TCP
	if( connect(sockfd, (struct sockaddr *) &servaddr, servaddr_len) == -1 )
	{
		perror("connect");
		TcpSocket_linux_status = 0;
		return BUS_COMM_ERROR;
	}
	else
#endif
//		SendClientInetAddr();
		return BUS_OK;
}


int Lin_TcpSocket::TcpSocket_linuxClose(void)
{
	TcpSocket_linux_status = 0;
	close(sockfd);
	return BUS_OK;
}

int Lin_TcpSocket::TcpSocket_linuxWrite(int Size, void *Buffer, int *Transferred)
{
#ifdef TCP
	if ((*Transferred = send(sockfd, Buffer, Size, 0)) == -1)
#else
	if ((*Transferred = sendto(sockfd, Buffer, Size, 0,
		(struct sockaddr *) &servaddr, servaddr_len)) == -1)
#endif
	{
		perror("send");
		return BUS_COMM_ERROR;
	}
	return BUS_OK;
}

int Lin_TcpSocket::TcpSocket_linuxRead(int Size, void *Buffer, int *Transferred)
{
#ifdef TCP
	if ((*Transferred = recv(sockfd, Buffer, Size, 0)) == -1)
#else
	if ((*Transferred = recvfrom(sockfd, Buffer, Size, 0, NULL, NULL)) == -1)
#endif
	{
		//perror("recv");
		return BUS_COMM_ERROR;
	}
	return BUS_OK;
}

int Lin_TcpSocket::TcpSocket_linuxScan(int slot, unsigned int *rev, unsigned int *timec)
{
	return BUS_GENERIC_ERROR;
}

int Lin_TcpSocket::TcpSocket_linuxCtrlWrite(int Size, void *Buffer, int *Transferred)
{
	return BUS_GENERIC_ERROR;
}

int Lin_TcpSocket::TcpSocket_linuxFastRead(int Size, void *Buffer, int *Transferred)
{
	return BUS_GENERIC_ERROR;
}

int Lin_TcpSocket::TcpSocket_linuxFlush(void)
{
	return BUS_GENERIC_ERROR;
}

int Lin_TcpSocket::WriteConfigFifo(unsigned int *data, int size)
{
	char *ptx;
	int Transferred, len;

	ptx = _command_string;
	ptx += sprintf(ptx,"%c %d", WRITE_CFG_FIFO, size);
	for(int i=0; i<size; i++)
		ptx += sprintf(ptx," 0x%08X", data[i]);
	ptx += sprintf(ptx,"\n");
	len = strlen(_command_string);
//printf("WriteConfigFifo(): _command_string = '%s'\n",_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteConfigFifo");
		return BUS_COMM_ERROR;
	}
/*
	len = 5;
	TcpSocket_linuxRead(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteConfigFifoACK");
		return BUS_COMM_ERROR;
	}
*/
	return BUS_OK;
}
/*
int Lin_TcpSocket::SendClientInetAddr(void)
{
	int Transferred, len;

	sprintf(_command_string,"%c %s\n", CLIENT_NET_ADDR, my_inet_addr);
	len = strlen(_command_string);
//printf("WriteClientAddr(): _command_string = '%s'\n",_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteClientAddr");
		return BUS_COMM_ERROR;
	}
	len = 5;
	TcpSocket_linuxRead(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteClientAddrACK");
		return BUS_COMM_ERROR;
	}
	return BUS_OK;
}
*/
/*
int Lin_TcpSocket::WriteNbitReg(int n1, int n2)
{
	int Transferred, len;

	sprintf(_command_string,"%c 0x00%02X00%02X\n", WRITE_NBIT_REG, n2, n1);
	len = strlen(_command_string);
//printf("WriteNbitReg(): _command_string = '%s'\n",_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteNbitReg");
		return BUS_COMM_ERROR;
	}
	len = 5;
	TcpSocket_linuxRead(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteNbitRegACK");
		return BUS_COMM_ERROR;
	}
	return BUS_OK;
}
*/
int Lin_TcpSocket::WriteNbitReg(int n1, int n2, int delay, int width_low)
{
	int Transferred, len;

	sprintf(_command_string,"%c 0x%02X%02X%02X%02X\n", WRITE_NBIT_REG, width_low, n2, delay, n1);
	len = strlen(_command_string);
//printf("WriteNbitReg(): _command_string = '%s'\n",_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteNbitReg");
		return BUS_COMM_ERROR;
	}
/*
	len = 5; Transferred = 0;
	TcpSocket_linuxRead(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
printf("WriteNbitReg(): WriteNbitRegACK\n");
		perror("WriteNbitRegACK");
		return BUS_COMM_ERROR;
	}
*/
	return BUS_OK;
}

int Lin_TcpSocket::WriteCommandReg(unsigned int val)
{
	int Transferred, len;

	sprintf(_command_string,"%c 0x%08X\n", WRITE_COMMAND_REG, val);
	len = strlen(_command_string);
//printf("WriteCommandReg(): _command_string = '%s'\n",_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteCommandReg");
		return BUS_COMM_ERROR;
	}
/*
	len = 5; Transferred = 0;
	TcpSocket_linuxRead(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
printf("WriteCommandReg(): WriteCommandRegACK\n");
		perror("WriteCommandRegACK");
		return BUS_COMM_ERROR;
	}
*/
	return BUS_OK;
}

int Lin_TcpSocket::WriteDummyCommand(unsigned int val)
{
	int Transferred, len;

	sprintf(_command_string,"H 0x%08X\n", val);
	len = strlen(_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteDummyCommand");
		return BUS_COMM_ERROR;
	}
	len = 5;
	TcpSocket_linuxRead(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("WriteDummyCommandACK");
		return BUS_COMM_ERROR;
	}
	return BUS_OK;
}

int Lin_TcpSocket::ReadReg(unsigned int *val, int addr)
{
	int Transferred, len;

	sprintf(_command_string,"%c %d\n", READ_DUMMY_REG, addr);
	len = strlen(_command_string);
//printf("ReadDummyReg(): write _command_string = '%s'\n",_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("ReadReg");
		return BUS_COMM_ERROR;
	}
	len = 4;
	TcpSocket_linuxRead(len, (void *)_databuf, &Transferred);
//printf("ReadDummyReg(): read _command_string = '%s'\n",_command_string);
	*val = ntohl(_databuf[0]);
//printf("ReadDummyReg: addr = %d, val = 0x%08X\n", addr, ntohl(_databuf[0]));
	return BUS_OK;
}

int Lin_TcpSocket::ReadDataFifo(int chip, int n, unsigned int *val, int *Transferred)
{
	int len;

	sprintf(_command_string,"%c %d\n", READ_DATA_FIFO_0+chip, n);
	len = strlen(_command_string);
//printf("ReadDataFifo(): write _command_string = '%s'\n",_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, Transferred);
	if( *Transferred != len )
	{
		perror("ReadDataFifo");
		return BUS_COMM_ERROR;
	}
	len = 4 * n;
	TcpSocket_linuxRead(len, (void *)_databuf, Transferred);
//printf("ReadDataFifo(): read _command_string = '%s'\n",_command_string);
	for(int i=0; i<n; i++)
		*(val+i) = ntohl(_databuf[i]);
	if( *Transferred != len )
	{
		perror("ReadDataFifo");
		return BUS_COMM_ERROR;
	}
	return BUS_OK;
}

int Lin_TcpSocket::ReadDataFifoNoHsk(int n, unsigned int *val, int *Transferred)
{
	int len;


	int flags = fcntl(sockfd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flags);

	len = 4 * n;
	TcpSocket_linuxRead(len, (void *)_databuf, Transferred);
	len = *Transferred / 4;
//printf("ReadDataFifoNoHsk(): Transferred = %d, len = %d\n",*Transferred,len);
	for(int i=0; i<len; i++)
		*(val+i) = ntohl(_databuf[i]);
	*Transferred /= 4;

	flags &= ~O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flags);
	return BUS_OK;
}

int Lin_TcpSocket::ReadConfigFifo(unsigned int *val, int n)
{
	int Transferred, len;

	sprintf(_command_string,"%c %d\n", READ_CFG_FIFO, n);
	len = strlen(_command_string);
//printf("ReadConfigFifo(): write _command_string = '%s'\n",_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("ReadConfigFifo");
		return BUS_COMM_ERROR;
	}
	len = 4 * n;
	TcpSocket_linuxRead(len, (void *)_databuf, &Transferred);
//printf("ReadConfigFifo(): read _command_string = '%s'\n",_command_string);
	for(int i=0; i<n; i++)
		*(val+i) = ntohl(_databuf[i]);
	return BUS_OK;
}

int Lin_TcpSocket::ReadFpgaStatus(unsigned int *val)
{
	int Transferred, len;

	sprintf(_command_string,"%c\n", READ_STATUS_REG);
	len = strlen(_command_string);
//printf("ReadFpgaStatus(): write _command_string = '%s'\n",_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("ReadFpgaStatus");
		return BUS_COMM_ERROR;
	}
	len = 5 * 4;
	TcpSocket_linuxRead(len, (void *)_databuf, &Transferred);
//printf("ReadFpgaStatus(): read _command_string = '%s'\n",_command_string);
	for(int i=0; i<5; i++)
		*(val+i) = ntohl(_databuf[i]);
//printf("ReadFpgaStatus: x[0] = 0x%02X\n", ntohl(_databuf[0]));
	return BUS_OK;
}

int Lin_TcpSocket::QuitServer(void)
{
	int Transferred, len;

	sprintf(_command_string,"%c\n", COMMAND_QUIT);
	len = strlen(_command_string);
	TcpSocket_linuxWrite(len, (void *)_command_string, &Transferred);
	if( Transferred != len )
	{
		perror("QuitServer");
		return BUS_COMM_ERROR;
	}
	return BUS_OK;
}

