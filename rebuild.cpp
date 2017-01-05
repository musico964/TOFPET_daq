#include <fstream>
#include <stdio.h>
#include <string.h>

#define CHP  0x434850	// ASCII "CHP"
#define CHP0 0x43485000	// ASCII "CHP" + 0
#define CHP1 0x43485001	// ASCII "CHP" + 1
#define CHP2 0x43485002	// ASCII "CHP" + 2
#define CHP3 0x43485003	// ASCII "CHP" + 3
#define CHP4 0x43485004	// ASCII "CHP" + 4
#define CHP5 0x43485005	// ASCII "CHP" + 5

#define CHP0R 0x00504843	// ASCII "CHP" + 0
#define CHP1R 0x01504843	// ASCII "CHP" + 1
#define CHP2R 0x02504843	// ASCII "CHP" + 2
#define CHP3R 0x03504843	// ASCII "CHP" + 3
#define CHP4R 0x04504843	// ASCII "CHP" + 4
#define CHP5R 0x05504843	// ASCII "CHP" + 5
using namespace std;

void FindChp(FILE *f, unsigned long *bytecount, unsigned int *data)
{
        unsigned int hdr;
        unsigned char n1, n2, n3, n4;

	fread(&n1, 1, 1, f);
	fread(&n2, 1, 1, f);
	fread(&n3, 1, 1, f);
	fread(&n4, 1, 1, f);
	*bytecount += 4;
	hdr = (n1&0xFF)<<24 | (n2&0xFF)<<16 | (n3&0xFF)<<8 | (n4&0xFF);
	while( hdr != CHP0 && hdr != CHP1 && hdr != CHP2 && hdr != CHP3 && hdr != CHP4 && hdr != CHP5 )
	{
		if( fread(&n1,1,1,f) == 0 )
			break;
		*bytecount++;
		hdr <<= 8;
		hdr |= (n1&0xFF);
	}
	*data = hdr;
}

void RebuildFile(char *fname_in)
{
	FILE *ftmp[6], *fout, *fin;
	char *fname_tmp[6];
	char fname_out[256];
	int i, Chip_id;
	unsigned long byte_count[6], chp_cnt[6], in_count;
        unsigned int data;

        if( (fin = fopen(fname_in, "rb")) == NULL )
        {
                printf("Can't open input file '%s'\n", fname_in);
                return;
        }
	strcpy(fname_out, fname_in);
	strcat(fname_out, "_out");
        if( (fout = fopen(fname_out, "wb")) == NULL )
        {
                printf("Can't open output file '%s'\n", fname_out);
		fclose(fin);
                return;
        }
	for(i=0; i<6; i++)
	{
		fname_tmp[i] = tempnam("/tmp", "Topem");
		ftmp[i] = fopen(fname_tmp[i], "wb");
		chp_cnt[i] = 0;
		byte_count[i] = 0;
	}

	in_count = 0;
	FindChp(fin, &in_count, &data);
	Chip_id = data & 0xFF;
	chp_cnt[Chip_id]++;
	printf("Found header for chip %d at byte %d\n", Chip_id, in_count);
	fread(&data, 4, 1, fin);
	while( !feof(fin) )
	{
		in_count += 4;
		if( data == CHP0R || data == CHP1R || data == CHP2R || data == CHP3R || data == CHP4R || data == CHP5R )
		{
			Chip_id = (data & 0xFF000000) >> 24;
			chp_cnt[Chip_id]++;
			printf("Found header for chip %d at byte %d\n", Chip_id, in_count);
		}
		else
		{
			byte_count[Chip_id] += 4;
			fwrite(&data, 4, 1, ftmp[Chip_id]);
		}
		fread(&data, 4, 1, fin);
	}
	fclose(fin);

// Reopen temp files for reading
	for(i=0; i<6; i++)
		fclose(ftmp[i]);
	for(i=0; i<6; i++)
		ftmp[i] = fopen(fname_tmp[i], "rb");
// Sequentially copy temp files to output file
	for(i=0; i<6; i++)
	{
		fread(&data, 4, 1, ftmp[i]);
		while( !feof(ftmp[i]) )
		{
			fwrite(&data, 4, 1, fout);
			fread(&data, 4, 1, ftmp[i]);
		}
	}
	fclose(fout);

// Remove temp files
	for(i=0; i<6; i++)
	{
		fclose(ftmp[i]);
		unlink(fname_tmp[i]);
	}

	printf("Read %d input bytes\n", in_count);
	for(i=0; i<6; i++)
		printf("Chip %d: Written %d bytes, count %d records\n", i, byte_count[i], chp_cnt[i]);
}



