#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUS_OK			0
#define	BUS_ERROR		-1
#define	BUS_COMM_ERROR		-2
#define	BUS_GENERIC_ERROR	-3
#define	BUS_INVALID_PARAM	-4
#define	BUS_TIMEOUT		-5

#define READ_DATA_FIFO_0	'0'	// "0 n"
#define READ_DATA_FIFO_1	'1'	// "1 n"
#define READ_DATA_FIFO_2	'2'	// "2 n"
#define READ_DATA_FIFO_3	'3'	// "3 n"
#define READ_DATA_FIFO_4	'4'	// "4 n"
#define READ_DATA_FIFO_5	'5'	// "5 n"
#define READ_DATA_FIFO_STATUS	'6'	// "6 0"
#define READ_CFG_FIFO		'8'	// "8 n"
#define WRITE_CFG_FIFO		'9'	// "9 n x1 x2 ... xn"
#define WRITE_NBIT_REG		'A'	// "A val"
#define WRITE_COMMAND_REG	'B'	// "B val"
#define READ_STATUS_REG		'C'	// "C 0"
#define READ_NBIT_REG		'D'	// "D 0"
#define READ_COMMAND_REG	'E'	// "E 0"
#define READ_DUMMY_REG		'F'	// "F add"
#define COMMAND_QUIT		'Q'	// "Q"
#define CLIENT_NET_ADDR		'Z'	// "Z 193.206.144.51"

// Command register bit fields
#define COMMAND_CFG_READ	0x00000001
#define COMMAND_CFG_WRITE	0x00000002
#define COMMAND_RST		0x00000004
#define COMMAND_WIDERST		0x00000008
#define COMMAND_TEST		0x00000010
#define COMMAND_CTRLFIFO_RST	0x00000020
#define COMMAND_DATAFIFO_RST	0x00000040
#define CHIP_SELECT		0x00003F00
#define FE_SELECT		0x00070000
#define FE_ENABLEb		0x00080000
#define SCLK_ENABLE		0x00100000
#define DDR_MODE		0x00200000	// Same as G_CONFIG DDR Mode bit
#define TX_MODE			0x00C00000	// Same as G_CONFIG TX Mode bits
#define READOUT_ENABLE		0x3F000000
#define SKIP_EMPTY_EVENTS	0x80000000

// Status register bit fields
#define STATUS_CMD_ACK		0x80000000
#define STATUS_CFG_RUN		0x40000000

//class Lin_TcpSocket : public IOChannel
class Lin_TcpSocket
{
	private:
		int TcpSocket_linux_status;
		int sockfd, servaddr_len;
		struct addrinfo hints, *servinfo, *p;
		struct sockaddr_in servaddr;
		char my_inet_addr[100];

		int TcpSocket_linuxOpen(char*);
		int TcpSocket_linuxClose(void);
		int TcpSocket_linuxScan(int slot, unsigned int *rev, unsigned int *timec);
		int TcpSocket_linuxWrite(int Size, void *Buffer, int *Transferred);
		int TcpSocket_linuxRead(int Size, void *Buffer, int *Transferred);
		int TcpSocket_linuxFastRead(int Size, void *Buffer, int *Transferred);
		int TcpSocket_linuxCtrlWrite(int Size, void *Buffer, int *Transferred);
		int TcpSocket_linuxFlush(void);

		void _GetMyInetAddr(void);

		char _command_string[1000];
		unsigned int _databuf[2048];


	public:

		int Open(char *inetaddr) { return TcpSocket_linuxOpen(inetaddr); }
		int Close(void) { return TcpSocket_linuxClose(); }
		int Status(void) { return TcpSocket_linux_status; }
		int Scan(int slot, unsigned int *rev, unsigned int *timec) { return TcpSocket_linuxScan(slot, rev, timec); }
		int Write(int Size, void *Buffer, int *Transferred)
			 { return TcpSocket_linuxWrite(Size, Buffer, Transferred); }
		int Read(int Size, void *Buffer, int *Transferred)
			 { return TcpSocket_linuxRead(Size, Buffer, Transferred); }
		int FastRead(int Size, void *Buffer, int *Transferred)
			 { return TcpSocket_linuxFastRead(Size, Buffer, Transferred); }
		int CtrlWrite(int Size, void *Buffer, int *Transferred)
			 { return TcpSocket_linuxCtrlWrite(Size, Buffer, Transferred); }
		int Flush(void)
			 { return TcpSocket_linuxFlush(); }

		Lin_TcpSocket(char *x) { TcpSocket_linuxOpen(x); }
		~Lin_TcpSocket() { TcpSocket_linuxClose(); }

//		int SendClientInetAddr(void);
		int WriteConfigFifo(unsigned int *data, int size);
		int WriteNbitReg(int n1, int n2, int delay, int width_low);
		int WriteNbitReg(int n1, int n2) {return WriteNbitReg(n1, n2, 0, 0);}
		int WriteCommandReg(unsigned int val);
		int WriteDummyCommand(unsigned int val);
		int ReadDataFifo(int c, int n, unsigned int *val, int *Transferred);
		int ReadDataFifoNoHsk(int n, unsigned int *val, int *Transferred);
		int ReadConfigFifo(unsigned int *val, int n);
		int ReadReg(unsigned int *val, int addr);
		int ReadFpgaStatus(unsigned int *val);
		int ReadFifoStatusReg(unsigned int *val) {return ReadReg(val,6);}
		int ReadNbitReg(unsigned int *val) {return ReadReg(val,10);}
		int ReadCommandReg(unsigned int *val) {return ReadReg(val,11);}
//		int ReadStatusReg(unsigned int *val) {return ReadReg(val,12);}
		int ReadNwords10(unsigned int *val) {return ReadReg(val,13);}
		int ReadNwords32(unsigned int *val) {return ReadReg(val,14);}
		int ReadNwords54(unsigned int *val) {return ReadReg(val,15);}

		int QuitServer(void);

};

extern Lin_TcpSocket *IO;
