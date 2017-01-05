//#define DEBUG
#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif

// Computes x^8 + x^2 + x + 1 CCITT 8-bit CRC
unsigned char crc8(unsigned char *data, unsigned int nbits)
{
	unsigned char init_val = 0x8A;
//	unsigned char init_val = 0x00;
	unsigned char crc;
	int i, j, k, x, y[8];

	crc = init_val;
	j = 7;

	while( nbits )
	{
		for(k=0; k<8; k++)
			y[k] = (crc >> k) & 0x01	;
		x = (*data & (1 << j)) >> j;
		i = y[7] ^ x;
		y[7] = y[6];
		y[6] = y[5];
		y[5] = y[4];
		y[4] = y[3];
		y[3] = y[2];
		y[2] = y[1] ^ i;
		y[1] = y[0] ^ i;
		y[0] = i;
		crc = 128*y[7]+64*y[6]+32*y[5]+16*y[4]+8*y[3]+4*y[2]+2*y[1]+y[0];
		j--;
		nbits--;
		if( j == -1 )
		{
			j = 7;
			data++;
		}
	}
	return( crc );
}

// Computes x^16 + x^12 + x^5 + 1 CCITT 16-bit CRC
unsigned short crc16(unsigned char *data, unsigned int nbits)
{
	unsigned short init_val = 0x0F4A;
//	unsigned short init_val = 0x00;
	unsigned short crc;
	int i, j, k, x, y[16];

	crc = init_val;
	j = 7;

	while( nbits )
	{
		for(k=0; k<16; k++)
			y[k] = (crc >> k) & 0x01	;
		x = (*data & (1 << j)) >> j;
		i = y[15] ^ x;
		y[15] = y[14];
		y[14] = y[13];
		y[13] = y[12];
		y[12] = y[11] ^ i;
		y[11] = y[10];
		y[10] = y[9];
		y[9] = y[8];
		y[8] = y[7];
		y[7] = y[6];
		y[6] = y[5];
		y[5] = y[4] ^ i;
		y[4] = y[3];
		y[3] = y[2];
		y[2] = y[1];
		y[1] = y[0];
		y[0] = i;
		crc = 32768*y[15]+16384*y[14]+8192*y[13]+4096*y[12]+
			2048*y[11]+1024*y[10]+512*y[9]+256*y[8]+
			128*y[7]+64*y[6]+32*y[5]+16*y[4]+8*y[3]+4*y[2]+2*y[1]+y[0];
		j--;
		nbits--;
		if( j == -1 )
		{
			j = 7;
			data++;
		}
	}
	return( crc );
}

#ifdef DEBUG
main(int argc, char **argv)
{
	unsigned char c1;
	unsigned short c2, x;
	unsigned char d[2];

	if( argc != 2 )
	{
		printf("Usage: crc val\n");
		exit(1);
	}
	x = (unsigned short) strtoul(argv[1], NULL, 0);
	d[0] = x >> 8;
	d[1] = x & 0xFF;
	c1 = crc8(d, 8);
	printf("Data = 0x%02X, crc8 = 0x%02X\n", x, c1);
	c2 = crc16(d, 16);
	printf("Data = 0x%04X, crc16 = 0x%04X\n", x, c2);
}
#endif
