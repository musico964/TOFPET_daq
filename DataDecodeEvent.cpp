#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "Riostream.h"
#include "TMath.h"
#include "TF1.h"
#include "TH1I.h"
#include "TH2I.h"
#include "TCanvas.h"
#include "TStopwatch.h"
#include "TStyle.h"
#include "TLine.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraphErrors.h"
#include "TMarker.h"
#include "TText.h"

#define CHP0 0x43485000	// ASCII "CHP" + 0
#define CHP1 0x43485001	// ASCII "CHP" + 1
#define CHP2 0x43485002	// ASCII "CHP" + 2
#define CHP3 0x43485003	// ASCII "CHP" + 3
#define CHP4 0x43485004	// ASCII "CHP" + 4
#define CHP5 0x43485005	// ASCII "CHP" + 5

#define CHP0R 0x00504843	// ASCII "CHP" + 0 reversed
#define CHP1R 0x01504843	// ASCII "CHP" + 1 reversed
#define CHP2R 0x02504843	// ASCII "CHP" + 2 reversed
#define CHP3R 0x03504843	// ASCII "CHP" + 3 reversed
#define CHP4R 0x04504843	// ASCII "CHP" + 4 reversed
#define CHP5R 0x05504843	// ASCII "CHP" + 5 reversed

#define HDR0 0x48445230	// ASCII "HDR0"
#define HDR1 0x48445231	// ASCII "HDR1"
#define HDR2 0x48445232	// ASCII "HDR2"
#define HDR3 0x48445233	// ASCII "HDR3"
#define HDR4 0x48445234	// ASCII "HDR4"
#define HDR5 0x48445235	// ASCII "HDR5"

using namespace std;

TFile *ofile;
TTree *Tdata;

TH1F *T_diff;
TCanvas *D_canvas;

TH1I *hLead_Coarse, *hTrail_Coarse, *TOT_coarse, *TOT_fine, *Tfine1, *Tfine2, *hSoC, *hT_EoC, *hE_EoC, *T_Tac, *E_Tac, *hNEvents;
TH1F *T0;
TCanvas *CH_canvas;
TCanvas *Diff_canvas;
TH1I *TOT_coarse1, *TOT_coarse2, *TOT_fine1, *TOT_fine2, *TOT_Diff_coarse, *TOT_Diff_fine;

// LCOMB_CELL Delay Line unit delay (ns)
Float_t tap_delay[32] = {0.8,0.8,0.8,0.8, 0.8,0.8,0.8,0.8,
			 0.6,0.6,0.6,0.6, 0.9,0.9,0.9,0.9,
			 0.7,0.7,0.7,0.7, 0.8,0.8,0.8,0.8,
			 0.5,0.5,0.5,0.5, 0.6,0.6,0.6,0.6};


unsigned char crc_array[1000];

int	Chip_id, Nevents, Frame_id;
int	Lead_coarse,  T_SoC, T_EoC, T_Channel_id, T_Tac_id,
	Trail_coarse, E_SoC, E_EoC, E_Channel_id, E_Tac_id;

// TREE Utilities
void PrintTree(char *rootfname, Int_t n=-1, char *tid="Tdata")
{
        TFile *f = new TFile(rootfname);
        TTree *a = (TTree *)f->Get(tid);
        a->Print();
        printf("\nTree has %d entries\n", a->GetEntries());
        if( n != -1)
                a->Show(n);
}

void CreateTree(char *fname)
{
        ofile = new TFile(fname, "RECREATE");
        Tdata = new TTree("Tdata", "Event Tree");

	Tdata->Branch("Chip_id",      &Chip_id,      "Chip_id/I");
	Tdata->Branch("Frame_id",     &Frame_id,     "Frame_id/I");
	Tdata->Branch("Lead_coarse",  &Lead_coarse,  "Lead_coarse/I");
	Tdata->Branch("T_SoC",        &T_SoC,        "T_SoC/I");
	Tdata->Branch("T_EoC",        &T_EoC,        "T_EoC/I");
	Tdata->Branch("T_Channel_id", &T_Channel_id, "T_Channel_id/I");
	Tdata->Branch("T_Tac_id",     &T_Tac_id,     "T_Tac_id/I");
	Tdata->Branch("Trail_coarse", &Trail_coarse, "Trail_coarse/I");
	Tdata->Branch("E_SoC",        &E_SoC,        "E_SoC/I");
	Tdata->Branch("E_EoC",        &E_EoC,        "E_EoC/I");
	Tdata->Branch("E_Channel_id", &E_Channel_id, "E_Channel_id/I");
	Tdata->Branch("E_Tac_id",     &E_Tac_id,     "E_Tac_id/I");
}

void CloseTree(void)
{
	Tdata->Write();
	ofile->Close();
}

int gray_encode(int n)
{
	return n ^ (n >> 1);
}
 
int gray_decode(int n)
{
	int p = n;
	while (n >>= 1)
		p ^= n;
	return p;
}

unsigned short crc16(unsigned char *data, unsigned int nbits)
{
        unsigned short init_val = 0x0F4A;
//      unsigned short init_val = 0;
        unsigned short crc;
        int i, j, k, x, y[16];

        crc = init_val;
        j = 7;

        while( nbits )
        {
                for(k=0; k<16; k++)
                        y[k] = (crc >> k) & 0x01        ;
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

// Read data from input file and split them in 6 temp files
	in_count = 0;
	FindChp(fin, &in_count, &data);
	Chip_id = data & 0xFF;
	chp_cnt[Chip_id]++;
	printf("\nRebuilding input stream...\n");
	printf("Found header for chip %d at byte %d\n", Chip_id, in_count);
	fread(&data, 4, 1, fin);
	while( !feof(fin) )
	{
		in_count += 4;
		if( data == CHP0R || data == CHP1R || data == CHP2R || data == CHP3R || data == CHP4R || data == CHP5R )
		{
			Chip_id = (data & 0xFF000000) >> 24;
			chp_cnt[Chip_id]++;
//			printf("Found header for chip %d at byte %d\n", Chip_id, in_count-4);
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

void FindHeader(FILE *f, unsigned long *bytecount, unsigned int *data)
{
        unsigned int hdr;
        unsigned char n1, n2, n3, n4;

	fread(&n1, 1, 1, f);
	fread(&n2, 1, 1, f);
	fread(&n3, 1, 1, f);
	fread(&n4, 1, 1, f);
	hdr = (n1&0xFF)<<24 | (n2&0xFF)<<16 | (n3&0xFF)<<8 | (n4&0xFF);
	*bytecount += 4;
	while( hdr != HDR0 && hdr != HDR1 && hdr != HDR2 && hdr != HDR3 && hdr != HDR4 && hdr != HDR5 )
	{
		if( fread(&n1,1,1,f) == 0 )
			break;
		*bytecount++;
		hdr <<= 8;
		hdr |= (n1&0xFF);
	}
	*data = hdr;
}

void FillTree(char *fname, int check_crc=0, int cnt_max=-1)
{
        unsigned int hdr, i, j;
        FILE *fin;
        char rootfname[100], fname_x[256], *dotptr;
        int frame_count = 0;
        int good_frame_count = 0;
        int bad_frame_count = 0;
        int zero_ev_frame_count = 0;
        int error_frame_count = 0;
        int written_event_count = 0;
        int crc_byte_count;
	unsigned long byte_count;
	unsigned short data_crc, computed_crc;
	bool bad_crc_flag;
	unsigned char d[16];


	RebuildFile(fname);
	strcpy(fname_x, fname);
	strcat(fname_x, "_out");

        if( (fin = fopen(fname_x, "rb")) == NULL )
        {
                cout << "Can't open input file " << fname << endl;
                return;
        }
        else
        {
                dotptr = strrchr(fname, '.');   // find last occurrence of '.'
                if( dotptr )
                        *(dotptr) = '\0';

                sprintf(rootfname, "%s.root", fname);
                printf("\nCreating '%s' file\n", rootfname);
                CreateTree(rootfname);

		bad_crc_flag = false;
		byte_count = 0;

		FindHeader(fin, &byte_count, &hdr);

		if( hdr >= HDR0 && hdr <= HDR5 )
		{
			cout << "Found header for chip " << Chip_id << " at byte " << byte_count << endl;
		}
                while( !feof(fin) )
                {
			Chip_id = hdr - HDR0;
			Lead_coarse = T_SoC = T_EoC = T_Channel_id = T_Tac_id =
			Trail_coarse = E_SoC = E_EoC = E_Channel_id = E_Tac_id = 0;
			fread(&Nevents, 1, 1, fin);
                        if( feof(fin) )
				break;
			byte_count++;
			crc_byte_count = 0;
			crc_array[crc_byte_count++] = Nevents;
			Nevents/=2;	// ???
//printf("Nevents = %d\n",Nevents);
			if( Nevents > 64 )
			{
				cout << "Nevents = " << Nevents <<
					" at byte " << byte_count << endl;
				error_frame_count++;
			}
			if( byte_count > cnt_max )
				break;
			if( Nevents <= 64 && Nevents > 0 )
			{
				fread(d, 1, 4, fin);
				byte_count += 4;
				for(j=0; j<4; j++)
					crc_array[crc_byte_count++] = d[j];
				Frame_id = (d[0]&0xFF)<<24 | (d[1]&0xFF)<<16 | (d[2]&0xFF)<<8 | (d[3]&0xFF);
				if( bad_crc_flag )
				{
					cout << "After bad CRC, Frame_id = 0x" << hex << Frame_id << dec <<
						" file offset = " << ftell(fin) << endl;
					bad_crc_flag = false;
				}
				for(i=0; i<Nevents; i++)
				{
					fread(d, 1, 10, fin);
					for(j=0; j<10; j++)
						crc_array[crc_byte_count++] = d[j];
					byte_count += 10;

					Lead_coarse  = (d[0]&0xFF)<<2 | (d[1]&0xC0)>>6;
					T_SoC        = (d[1]&0x3F)<<4 | (d[2]&0xF0)>>4;
					T_EoC        = (d[2]&0x0F)<<6 | (d[3]&0xFC)>>2;
					T_Channel_id = (d[4]&0xFC)>>2;
					T_Tac_id     = (d[4]&0x03);
					Trail_coarse = (d[5]&0xFF)<<2 | (d[6]&0xC0)>>6;
					E_SoC        = (d[6]&0x3F)<<4 | (d[7]&0xF0)>>4;
					E_EoC        = (d[7]&0x0F)<<6 | (d[8]&0xFC)>>2;
					E_Channel_id = (d[9]&0xFC)>>2;
					E_Tac_id     = (d[9]&0x03);

					Lead_coarse  = gray_decode(Lead_coarse);
					T_SoC        = gray_decode(T_SoC);
					T_EoC        = gray_decode(T_EoC);
					Trail_coarse = gray_decode(Trail_coarse);
					E_SoC        = gray_decode(E_SoC);
					E_EoC        = gray_decode(E_EoC);
					Tdata->Fill();
					written_event_count++;
				}
				fread(d, 1, 1, fin);	// skip padding 0
				byte_count++;
				crc_array[crc_byte_count++] = 0;
			}
			if( Nevents == 0 )
			{
				zero_ev_frame_count++;
//				cout << "Found Zero Event in frame " << frame_count <<
//					" (Frame_id = 0x" << hex << Frame_id << dec <<
//					") at byte " << byte_count << endl;
			}
			bad_crc_flag = false;
			if( check_crc )
			{
				fread(d, 1, 2, fin);
				byte_count += 2;
				data_crc = (d[0]&0xFF)<<8 | (d[1]&0xFF);
				computed_crc = crc16(crc_array, crc_byte_count*8);
// check crc
				if( computed_crc != data_crc)
				{
					bad_crc_flag = true;
					bad_frame_count++;
					cout << "Bad CRC found for frame " << frame_count <<
						" at byte " << byte_count <<
						" file offset = " << ftell(fin) << 
						" data_crc = " << data_crc << 
						" computed_crc = " << computed_crc << endl;
				}
				else
					bad_crc_flag = false;
			}
			if( !bad_crc_flag )
			{
				good_frame_count++;
// fill tree with non zero good frames
			}
			frame_count++;

//			if( bad_crc_flag )
			if( check_crc )
				fseek(fin, -20, SEEK_CUR);	// rewind a little bit
			FindHeader(fin, &byte_count, &hdr);	// find next header
		}
		cout << endl << endl;
		cout << "Read " << byte_count << " bytes, " << frame_count << " complete frames" << endl;
		cout << "  Good frames: " << good_frame_count << endl;
		cout << "  Bad CRC frames: " << bad_frame_count << endl;
		cout << "  Zero event frames: " << zero_ev_frame_count << endl;
		cout << "  Error frames (Nevents > 64): " << error_frame_count << endl;
		cout << "  Saved events: " << written_event_count << endl;
		fclose(fin);
		unlink(fname_x);
	}
	CloseTree();
}

void DisplayChan(char *rootfname, int ch = 0, int chip = 0, int tac = -1, int nmax = 10000000)
{
	int i, j, k, tfine1, tfine2, coarse_tot, coarse_width;
	int row, col, sizex, sizey;
	char id[100], title[100];
	int Coarse_binsize = 128;
	int Coarse_base = 0;
	int Fine_binsize = 1024;
	int Fine_base = 0;
	double m, offset, tq, tf, fine_tot, t0, t1;
	int ch_count[64];

	TFile *f = new TFile(rootfname);
	TTree *a = (TTree *)f->Get("Tdata");

	a->SetBranchAddress("Chip_id",      &Chip_id);
	a->SetBranchAddress("Frame_id",     &Frame_id);
	a->SetBranchAddress("Lead_coarse",  &Lead_coarse);
	a->SetBranchAddress("T_SoC",        &T_SoC);
	a->SetBranchAddress("T_EoC",        &T_EoC);
	a->SetBranchAddress("T_Channel_id", &T_Channel_id);
	a->SetBranchAddress("T_Tac_id",     &T_Tac_id);
	a->SetBranchAddress("Trail_coarse", &Trail_coarse);
	a->SetBranchAddress("E_SoC",        &E_SoC);
	a->SetBranchAddress("E_EoC",        &E_EoC);
	a->SetBranchAddress("E_Channel_id", &E_Channel_id);
	a->SetBranchAddress("E_Tac_id",     &E_Tac_id);

	if( TOT_coarse ) delete TOT_coarse;
	if( hLead_Coarse ) delete hLead_Coarse;
	if( hTrail_Coarse ) delete hTrail_Coarse;
	if( TOT_fine ) delete TOT_fine;
	if( T0 ) delete T0;
	if( Tfine1 ) delete Tfine1;
	if( Tfine2 ) delete Tfine2;
	if( hSoC ) delete hSoC;
	if( hT_EoC ) delete hT_EoC;
	if( hE_EoC ) delete hE_EoC;
	if( T_Tac ) delete T_Tac;
	if( E_Tac ) delete E_Tac;
	if( hNEvents ) delete hNEvents;
	sprintf(id, "hCoarse_%d", ch);
	sprintf(title, "Coarse TOT (Trail_Coarse-Lead_Coarse) for channel %d, chip %d", ch, chip);
	TOT_coarse = new TH1I(id, title, Coarse_binsize, Coarse_base, Coarse_base+Coarse_binsize-1);
	sprintf(id, "hTCoarse_%d", ch);
	sprintf(title, "Lead Coarse time for channel %d, chip %d", ch, chip);
	hLead_Coarse = new TH1I(id, title, Coarse_binsize, Coarse_base, Coarse_base+Coarse_binsize-1);
	sprintf(id, "hECoarse_%d", ch);
	sprintf(title, "Trail Coarse time for channel %d, chip %d", ch, chip);
	hTrail_Coarse = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hFine_%d", ch);
	sprintf(title, "Fine TOT for channel %d, chip %d", ch, chip);
	TOT_fine = new TH1I(id, title, Coarse_binsize, Coarse_base, Coarse_base+Coarse_binsize-1);
	sprintf(id, "hT0_%d", ch);
	sprintf(title, "Fine Leading Time for channel %d, chip %d", ch, chip);
//	T0 = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	T0 = new TH1F(id, title, Fine_binsize*100, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hTfine1_%d", ch);
	sprintf(title, "Tfine1 (T_EoC - T_SoC) for channel %d, chip %d", ch, chip);
	Tfine1 = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hTfine2_%d", ch);
	sprintf(title, "Tfine2 (E_EoC - E_SoC) for channel %d, chip %d", ch, chip);
	Tfine2 = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hSoC_%d", ch);
	sprintf(title, "SoC (T_SoC = E_SoC) for channel %d, chip %d", ch, chip);
	hSoC = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hTEoC_%d", ch);
	sprintf(title, "T_EoC for channel %d, chip %d", ch, chip);
	hT_EoC = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hEEoC_%d", ch);
	sprintf(title, "E_EoC for channel %d, chip %d", ch, chip);
	hE_EoC = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hTTac_%d", ch);
	sprintf(title, "T_Tac_id for channel %d, chip %d", ch, chip);
	T_Tac = new TH1I(id, title, 5, 0, 4);
	sprintf(id, "hETac_%d", ch);
	sprintf(title, "E_Tac_id for channel %d, chip %d", ch, chip);
	E_Tac = new TH1I(id, title, 5, 0, 4);

	m = 128.0;
	offset = 0.0;
	for(i=0; i<a->GetEntries(); i++)
	{
		if( i > nmax )
			break;
		a->GetEvent(i);
		if( Chip_id == chip && T_Channel_id == ch && E_Channel_id == ch )
		{
			coarse_tot = Trail_coarse - Lead_coarse;	// Trail_coarse - Lead_coarse

			tfine1 = T_EoC - T_SoC;
if( tfine1 < 0 ) tfine1 += 1024;	// ???
			tq = (double)tfine1 / m;
			tf = ((Lead_coarse % 2) == 0 ) ?
				((tq < 2.5) ? (2.0 - tq) : (4.0 - tq) ) :
				(3.0 - tq);
			t0 = (double)Lead_coarse + tf + offset;

if( i < 50 ) cout << "Event = " << i << " T_Coarse = " << Lead_coarse << " tfine1 = " << tfine1 << " tq = " << tq << " tf = " << tf << endl;
			tfine2 = E_EoC - E_SoC;
if( tfine2 < 0 ) tfine2 += 1024;	// ???
			tq = (double)tfine2 / m;
			tf = ((Trail_coarse % 2) == 0 ) ?
				((tq < 2.5) ? (2.0 - tq) : (4.0 - tq) ) :
				(3.0 - tq);
			t1 = (double)Trail_coarse + (int)tf + (int)offset;
			fine_tot = t1 - t0;
if( i < 50 )
{
	cout << "Event = " << i << " E_Coarse = " << Trail_coarse << " tfine2 = " << tfine2 << " tq = " << tq << " tf = " << tf << endl;
//	cout << "Event = " << i << " Coarse TOT = " << coarse_tot << " Fine TOT = " << fine_tot << " T0 = " << t0 << endl;
//	cout << "Event = " << i << " T_EoC = " << T_EoC[j] << " T_SoC = " << T_SoC[j] << endl;
}

			hLead_Coarse->Fill(Lead_coarse);
			hTrail_Coarse->Fill(Trail_coarse);
			TOT_coarse->Fill(coarse_tot);
			TOT_fine->Fill((int)fine_tot);
			T0->Fill(t0);
			Tfine1->Fill((int)tfine1);
			Tfine2->Fill((int)tfine2);
			if( tac == -1 || tac == T_Tac_id )
			{
				hSoC->Fill((int)T_SoC);
				hT_EoC->Fill((int)T_EoC);
			}
			if( tac == -1 || tac == E_Tac_id )
			{
				hE_EoC->Fill((int)E_EoC);
			}
			T_Tac->Fill(T_Tac_id);
			E_Tac->Fill(E_Tac_id);
		}
	}
	row = 4; col = 3, sizey = 1000; sizex = 1000;
	if( CH_canvas ) delete CH_canvas;
	CH_canvas = new TCanvas("CC",rootfname,10,10,sizex,sizey);

	CH_canvas->Divide(col,row);
	CH_canvas->cd(1);
	TOT_coarse->Draw();
	CH_canvas->cd(2);
	TOT_fine->Draw();
//	hLead_Coarse->Draw();
	CH_canvas->cd(3);
	hTrail_Coarse->Draw();
	CH_canvas->cd(4);
	T0->Draw();
	CH_canvas->cd(5);
	Tfine1->Draw();
	CH_canvas->cd(6);
	hLead_Coarse->Draw();
//	Tfine2->Draw();
	CH_canvas->cd(7);
	hSoC->Draw();
	CH_canvas->cd(8);
	hT_EoC->Draw();
	CH_canvas->cd(9);
	hE_EoC->Draw();
	CH_canvas->cd(10);
	T_Tac->Draw();
	CH_canvas->cd(11);
	E_Tac->Draw();
	CH_canvas->cd(12);
//	hNEvents->Draw();
	CH_canvas->cd(0);
}

void CoarseToT(char *rootfname, int ch = 0, int chip = 0, int tac = -1, int nmax = 10000000)
{
	int i, j, coarse_tot;
	int sizex, sizey;
	char id[100], title[100];
	int Coarse_binsize = 128;
	int Coarse_base = 0;

	TFile *f = new TFile(rootfname);
	TTree *a = (TTree *)f->Get("Tdata");

	a->SetBranchAddress("Chip_id",      &Chip_id);
	a->SetBranchAddress("Frame_id",     &Frame_id);
	a->SetBranchAddress("Lead_coarse",  &Lead_coarse);
	a->SetBranchAddress("T_SoC",        &T_SoC);
	a->SetBranchAddress("T_EoC",        &T_EoC);
	a->SetBranchAddress("T_Channel_id", &T_Channel_id);
	a->SetBranchAddress("T_Tac_id",     &T_Tac_id);
	a->SetBranchAddress("Trail_coarse", &Trail_coarse);
	a->SetBranchAddress("E_SoC",        &E_SoC);
	a->SetBranchAddress("E_EoC",        &E_EoC);
	a->SetBranchAddress("E_Channel_id", &E_Channel_id);
	a->SetBranchAddress("E_Tac_id",     &E_Tac_id);

	if( TOT_coarse ) delete TOT_coarse;
	sprintf(id, "hCoarse_%d", ch);
	sprintf(title, "Coarse TOT (E_Coarse-T_Coarse) for channel %d, chip %d", ch, chip);
	TOT_coarse = new TH1I(id, title, Coarse_binsize, Coarse_base, Coarse_base+Coarse_binsize-1);

	for(i=0; i<a->GetEntries(); i++)
	{
		if( i > nmax )
			break;
		a->GetEvent(i);
		if( Chip_id == chip && T_Channel_id == ch && E_Channel_id == ch )
		{
			coarse_tot = Trail_coarse - Lead_coarse;	// Trail_coarse - Lead_coarse

			TOT_coarse->Fill(coarse_tot);
		}
	}
	sizey = 400; sizex = 600;
	if( CH_canvas ) delete CH_canvas;
	CH_canvas = new TCanvas("CC",rootfname,10,10,sizex,sizey);

	CH_canvas->cd(1);
	TOT_coarse->Draw();
	CH_canvas->cd(0);
}


void DisplayRawChan(char *rootfname, int ch = 0, int chip = 0, int tac = -1, int nmax = 10000000)
{
	int i, j, k, tfine1, tfine2, coarse_tot, coarse_width;
	int row, col, sizex, sizey;
	char id[100], title[100];
	int Coarse_binsize = 128;
	int Coarse_base = 0;
	int Fine_binsize = 1024;
	int Fine_base = 0;
	double m, offset, tq, tf, fine_tot, t0, t1;
	int ch_count[64];

	TFile *f = new TFile(rootfname);
	TTree *a = (TTree *)f->Get("Tdata");

	a->SetBranchAddress("Chip_id",      &Chip_id);
	a->SetBranchAddress("Frame_id",     &Frame_id);
	a->SetBranchAddress("Lead_coarse",  &Lead_coarse);
	a->SetBranchAddress("T_SoC",        &T_SoC);
	a->SetBranchAddress("T_EoC",        &T_EoC);
	a->SetBranchAddress("T_Channel_id", &T_Channel_id);
	a->SetBranchAddress("T_Tac_id",     &T_Tac_id);
	a->SetBranchAddress("Trail_coarse", &Trail_coarse);
	a->SetBranchAddress("E_SoC",        &E_SoC);
	a->SetBranchAddress("E_EoC",        &E_EoC);
	a->SetBranchAddress("E_Channel_id", &E_Channel_id);
	a->SetBranchAddress("E_Tac_id",     &E_Tac_id);

	if( TOT_coarse ) delete TOT_coarse;
	if( hLead_Coarse ) delete hLead_Coarse;
	if( hTrail_Coarse ) delete hTrail_Coarse;
	if( TOT_fine ) delete TOT_fine;
	if( T0 ) delete T0;
	if( Tfine1 ) delete Tfine1;
	if( Tfine2 ) delete Tfine2;
	if( hNEvents ) delete hNEvents;
	if( hSoC ) delete hSoC;
	if( hT_EoC ) delete hT_EoC;
	if( hE_EoC ) delete hE_EoC;
	if( T_Tac ) delete T_Tac;
	if( E_Tac ) delete E_Tac;
	sprintf(id, "hCoarse_%d", ch);
	sprintf(title, "Coarse TOT (Trail_Coarse-Lead_Coarse) for channel %d, chip %d", ch, chip);
	TOT_coarse = new TH1I(id, title, Coarse_binsize, Coarse_base, Coarse_base+Coarse_binsize-1);
	sprintf(id, "hTCoarse_%d", ch);
	sprintf(title, "Lead Coarse time for channel %d, chip %d", ch, chip);
	hLead_Coarse = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hECoarse_%d", ch);
	sprintf(title, "Trail Coarse time for channel %d, chip %d", ch, chip);
	hTrail_Coarse = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hFine_%d", ch);
	sprintf(title, "Fine TOT for channel %d, chip %d", ch, chip);
	TOT_fine = new TH1I(id, title, Coarse_binsize, Coarse_base, Coarse_base+Coarse_binsize-1);
	sprintf(id, "hT0_%d", ch);
	sprintf(title, "Fine Leading Time for channel %d, chip %d", ch, chip);
//	T0 = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	T0 = new TH1F(id, title, Fine_binsize*100, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hTfine1_%d", ch);
	sprintf(title, "Tfine1 (T_EoC - T_SoC) for channel %d, chip %d", ch, chip);
	Tfine1 = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hTfine2_%d", ch);
	sprintf(title, "Tfine2 (E_EoC - E_SoC) for channel %d, chip %d", ch, chip);
	Tfine2 = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hSoC_%d", ch);
	sprintf(title, "SoC (T_SoC = E_SoC) for channel %d, chip %d", ch, chip);
	hSoC = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hTEoC_%d", ch);
	sprintf(title, "T_EoC for channel %d, chip %d", ch, chip);
	hT_EoC = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hEEoC_%d", ch);
	sprintf(title, "E_EoC for channel %d, chip %d", ch, chip);
	hE_EoC = new TH1I(id, title, Fine_binsize, Fine_base, Fine_base+Fine_binsize-1);
	sprintf(id, "hTTac_%d", ch);
	sprintf(title, "T_Tac_id for channel %d, chip %d", ch, chip);
	T_Tac = new TH1I(id, title, 5, 0, 4);
	sprintf(id, "hETac_%d", ch);
	sprintf(title, "E_Tac_id for channel %d, chip %d", ch, chip);
	E_Tac = new TH1I(id, title, 5, 0, 4);

	m = 128.0;
	offset = 0.0;
	for(i=0; i<a->GetEntries(); i++)
	{
		if( i > nmax )
			break;
		a->GetEvent(i);
		if( Chip_id == chip && T_Channel_id == ch && E_Channel_id == ch )
		{
			coarse_tot = Trail_coarse - Lead_coarse;	// Trail_coarse - Lead_coarse

			tfine1 = T_EoC - T_SoC;
if( tfine1 < 0 ) tfine1 += 1024;	// ???
			tq = (double)tfine1 / m;
			tf = ((Lead_coarse % 2) == 0 ) ?
				((tq < 2.5) ? (2.0 - tq) : (4.0 - tq) ) :
				(3.0 - tq);
			t0 = (double)Lead_coarse + tf + offset;

			T0->Fill(t0);
			hLead_Coarse->Fill(Lead_coarse);
			hTrail_Coarse->Fill(Trail_coarse);
			if( tac == -1 || tac == T_Tac_id )
			{
				hSoC->Fill((int)T_SoC);
				hT_EoC->Fill((int)T_EoC);
			}
			if( tac == -1 || tac == E_Tac_id )
			{
				hE_EoC->Fill((int)E_EoC);
			}
			T_Tac->Fill(T_Tac_id);
			E_Tac->Fill(E_Tac_id);
		}
	}
	row = 3; col = 3, sizey = 1000; sizex = 1000;
	if( CH_canvas ) delete CH_canvas;
	CH_canvas = new TCanvas("CC",rootfname,10,10,sizex,sizey);

	CH_canvas->Divide(col,row);
	CH_canvas->cd(1);
	hLead_Coarse->Draw();
	CH_canvas->cd(2);
	hTrail_Coarse->Draw();
	CH_canvas->cd(3);
	T0->Draw();
	CH_canvas->cd(4);
	hSoC->Draw();
	CH_canvas->cd(5);
	hT_EoC->Draw();
	CH_canvas->cd(6);
	hE_EoC->Draw();
	CH_canvas->cd(7);
	T_Tac->Draw();
	CH_canvas->cd(8);
	E_Tac->Draw();
	CH_canvas->cd(9);
//	hNEvents->Draw();
	CH_canvas->cd(0);
}
