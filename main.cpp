#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <fstream>

using namespace std;

#include "Lin_TcpSocket.h"
#include "Configure.h"
#include "DaqControl.h"
#include "TofPetAsic.h"

#define BLOCK_DATA_SIZE		10000
#define	FIFO_READ_BLOCK_SIZE	256
#define CFG_MAX_COUNT		1000

void ctrl_c(int);

Lin_TcpSocket *IO;
DaqConfig DaqConf;
TofPetAsic *Asic;
DaqControl DaqCtrl;

unsigned int *FifoData;

static bool sig_int = false;

void print_bin(unsigned int x, int nbit)
{
	for(int i=0; i<nbit; i++)
	{
		if( (i%4) == 0 && i > 0 )
			printf("_");
		if( x & (1<<(31-i)) )
			printf("1");
		else
			printf("0");
	}
	printf("\n");
}

void ctrl_c(int)
{
	sig_int = true;
	cout << endl << "CTRL-C pressed" << endl;
}//ctrl_c

int report_ack(const char *s, int j, int i)
{
	int cfg_timeout = 0;
	int retval = 0;

	while( Asic->CfgRunning() && cfg_timeout < CFG_MAX_COUNT )
	{
		cfg_timeout++;
	}
	if( cfg_timeout == CFG_MAX_COUNT )
	{
		if( i == -1 )
			cout << "WARNING timeout during " << s << "(" << j << ") FAILED!" << endl;
		else
			cout << "WARNING timeout during " << s << "(" << j << "," << i << ") FAILED!" << endl;
	}

	if( i == -1 )
	{
		if( !Asic->GotAck() )
			cout << "WARNING " << s << "(" << j << ") FAILED!" << endl;
		else
		{
			retval = 1;
//			cout << s << "(" << j << ") succeded!" << endl;
		}
	}
	else
	{
		if( !Asic->GotAck() )
			cout << "WARNING " << s << "(" << j << "," << i << ") FAILED!" << endl;
		else
		{
			cout << ".";
			retval = 1;
//			cout << s << "(" << j << "," << i << ") succeded!" << endl;
		}
	}
	return retval;
}

unsigned int Swap32(unsigned int x)
{
	return   (x << 24) |
		((x <<  8) & 0x00ff0000) |
		((x >>  8) & 0x0000ff00) |
		((x >> 24) & 0x000000ff);
}

void Swap32Array(unsigned int *x, int n)
{
	for(int i=0; i< n; i++)
		x[i] = Swap32(x[i]);
}

int main(int argc, char *argv[])
{
	char fout_cnf[256];
	int wr_cnt, rd_cnt, ff_cnt, zero_cnt;
	int ndata, transferred;
	int chip_count;
	unsigned int fpga_status;
	bool handshake_readout = false;


	Asic = new TofPetAsic;

	cout << "Function: " << __FUNCTION__ << "() last compiled: " << __DATE__ << " " << __TIME__ << endl;

	signal(SIGINT, ctrl_c);

	if (DaqConf.ParseInlineParam(argc, argv) < 0) // Get config file name and run number
	{
		exit(1);
	}
	if( DaqConf.Read(DaqCtrl.GetConfigFileName()) == EXIT_FAILURE )
	{
		cout << endl << endl << " Error reading configuration file" << endl;
		cout  << " Exiting..." << endl << endl;
		exit(1);
	}

	IO = new Lin_TcpSocket(DaqCtrl.GetInetAddr());
	Sleep(100);

// Blind scan
	Asic->SclkEnable();
	Sleep(20);
	Asic->GenerateWideResetSignal();
	Sleep(20);
	chip_count = 0;
	cout << "TofPET Asic index: ";
	for(int j=0; j<6; j++)
	{
		if( Asic->ReadGlobalConfiguration(j) )
		{
			cout << j << " ";
			chip_count++;
		}
	}
	cout << endl << "N. " << chip_count << " TofPET Asic present in the system" << endl;

// Perform initial default initializations
	for(int j=0; j<6; j++)
	{
		if( DaqCtrl.GetDaqMode() == MODE_SW_FETP || DaqCtrl.GetDaqMode() == MODE_SW_TDCA )
			Asic->Set_external_pulse_enable(j, 0);
		else
			Asic->Set_external_pulse_enable(j, 1);
		if( DaqCtrl.GetDaqMode() == MODE_HW_FETP || DaqCtrl.GetDaqMode() == MODE_SW_FETP )
			Asic->Set_global_calib_enable(j, 1);
		else
			Asic->Set_global_calib_enable(j, 0);
//Asic->Set_global_calib_enable(j, 1);	// remove
	}
//	Asic->SclkEnable();
//	Sleep(20);
//	Asic->GenerateWideResetSignal();
//	Sleep(20);
	for(int j=0; j<6; j++)
	{
		if( Asic->Get_ChipPresent(j) )
		{
			cout << "Initial Global Configuration on TofPetAsic(" << j << ")" << endl;
			Asic->WriteGlobalConfiguration(j);
			report_ack("WriteGlobalConfiguration", j, -1);
			Sleep(20);
			Asic->WriteGlobalTestConfig(j);
			report_ack("WriteGlobalTestConfig", j, -1);
			Sleep(20);
/*
			for(int i=0; i<64; i++)
			{
				if( Asic->Get_ChannelEnabled(j, i) )
				{
					cout << "Configuring channel " << i << endl;
					Asic->WriteChannelConfiguration(j, i);
					report_ack("WriteChannelConfiguration", j, i);
					Sleep(20);
					Asic->WriteChannelTestConfig(j, i);
					report_ack("WriteChannelTestConfig", j, i);
					Sleep(20);
				}
			}
*/
		}
	}

	// save config
	if( !DaqCtrl.GetHwConfigOnly() )
	{
		sprintf(fout_cnf, "%s_%04d_config.txt", (DaqCtrl.GetFilePrefix()).data(), DaqCtrl.GetRunNumber());
		DaqConf.Save(fout_cnf);
	}

	FifoData = new unsigned int [BLOCK_DATA_SIZE];

	sig_int = false;

// Perform some other initializations
	for(int j=0; j<6; j++)
	{
		if( Asic->Get_ChipPresent(j) )
		{
// Set proper configuration for each enabled channel
			for(int i=0; i<64; i++)
			{
				if( Asic->Get_ChannelEnabled(j, i) )
				{
					if( DaqCtrl.GetDaqMode() == MODE_HW_TDCA || DaqCtrl.GetDaqMode() == MODE_SW_TDCA )
					{
						Asic->Set_gtest(j, i, 1);
						Asic->Set_test_T_EN(j, i, 1);
						Asic->Set_test_E_EN(j, i, 1);
					}
					else
					{
						Asic->Set_gtest(j, i, 0);
						Asic->Set_test_T_EN(j, i, 0);
						Asic->Set_test_E_EN(j, i, 0);
					}
					if( DaqCtrl.GetDaqMode() == MODE_HW_FETP || DaqCtrl.GetDaqMode() == MODE_SW_FETP )
					{
						Asic->Set_ch_calib_enable(j, i, 1);
					}
					else
					{
						Asic->Set_ch_calib_enable(j, i, 0);
					}
//Asic->Set_ch_calib_enable(j, i, 1);	// remove

					Asic->Set_sync(j, i, 1);
					if( DaqCtrl.GetDaqMode() == MODE_DARK_COUNT )
						Asic->Set_praedictio(j, i, 0);
					else
						Asic->Set_praedictio(j, i, 1);

					cout << "Configuring channel " << i << endl;
					Asic->WriteChannelConfiguration(j, i);
					report_ack("WriteChannelConfiguration", j, i);
					Sleep(10);
					Asic->WriteChannelTestConfig(j, i);
					report_ack("WriteChannelTestConfig", j, i);
					Sleep(10);
/*
Asic->ReadGlobalConfiguration(j);
report_ack("ReadGlobalConfig", j, i);
Asic->ReadChannelConfiguration(j, i);
report_ack("ReadChannelConfig", j, i);
exit(1);
*/
				}
				else
				{	// Do nothing on disabled channels
				}
			}
			cout << endl;
		}
	}
	if( DaqCtrl.GetHwConfigOnly() )
	{
		Sleep(20);
		Asic->EnableTestSignal();
		fpga_status = Asic->ReadFpgaStatusReg();
		cout << "fpga_status = 0x" << hex << fpga_status << dec << endl;
		cout << "GotAck = " << ((fpga_status & 0x80000000) >> 31) << endl;
		cout << "RWrunning = " << ((fpga_status & 0x40000000) >> 30) << endl;
		cout << "TestRunning = " << ((fpga_status & 0x20000000) >> 29) << endl;
		cout << "synced1 = 0x" << hex << ((fpga_status & 0x00FF0000) >> 16) << dec << endl;
		cout << "synced0 = 0x" << hex << ((fpga_status & 0x0000FF00) >> 8) << dec << endl;
		cout << "CtrlFifo_OUT_full = " << ((fpga_status & 0x00000008) >> 3) << endl;
		cout << "CtrlFifo_OUT_empty = " << ((fpga_status & 0x00000004) >> 2) << endl;
		cout << "CtrlFifo_IN_full = " << ((fpga_status & 0x00000002) >> 1) << endl;
		cout << "CtrlFifo_IN_empty = " << ((fpga_status & 0x00000001) >> 0) << endl;
		Asic->SclkDisable();
		cout << endl << " HW config only: exiting!" << endl;
		IO->QuitServer();
		delete IO;
		delete Asic;
		exit(0);
	}

	Asic->SclkDisable();
	for(int j=0; j<6; j++)
		if( Asic->Get_ChipPresent(j) )
			Asic->ReadoutEnable(j);

	DaqCtrl.openOutFile();
	DaqCtrl.printSummary();
	DaqCtrl.Start();
	Asic->DataFifoReset();
	wr_cnt = 0;
	zero_cnt = 0;
	rd_cnt = 0;
	ff_cnt = 0;
	if( DaqCtrl.GetDaqMode() != MODE_DARK_COUNT )
	{
		if( DaqCtrl.GetDaqMode() == MODE_HW_FETP || DaqCtrl.GetDaqMode() == MODE_HW_TDCA )
			Asic->EnableTestSignal();
		do // loop on event and time
		{
// Get data and write to disk

			for(int j=0; j<6; j++)
			{
				bool ff;
				if( Asic->Get_ChipPresent(j) )
				{
					if( DaqCtrl.GetDaqMode() == MODE_SW_FETP || DaqCtrl.GetDaqMode() == MODE_SW_TDCA )
						Asic->WriteTestCommand(j);
					if( handshake_readout )
					{
						Asic->GetFpgaStatus();
						if( Asic->FifoFull(j) )
						{
							ff = true;
							ff_cnt++;
							if( !(ff_cnt % 10) )
								cout << "ERROR: FIFO Full on TofPet ASIC " << j << endl;
						}
						else
							ff = false;
						if( !Asic->FifoEmpty(j) )
						{
							DaqCtrl.incEvt();
							ndata = Asic->FifoNdataAvail(j);
							ndata  = (ndata > FIFO_READ_BLOCK_SIZE) ? FIFO_READ_BLOCK_SIZE : ndata;
							if( ndata == 0 && ff )
								ndata = FIFO_READ_BLOCK_SIZE;
//printf("Getting data %d\n", ndata);
							Asic->GetFifoData(j, ndata, FifoData, &transferred);
							wr_cnt++;
							switch( DaqCtrl.GetDaqFormat() )
							{
								case DAQ_HEX:
										for(int i=0; i<ndata; i++)
										{
											fprintf(DaqCtrl.OutFile(), "%02X %02X %02X %02X ",
											(FifoData[i]&0xFF000000) >> 24,(FifoData[i]&0xFF0000) >> 16,
											(FifoData[i]&0xFF00) >> 8,(FifoData[i]&0xFF));
											if( (i % 8) == 0 && i > 0 )
												fprintf(DaqCtrl.OutFile(), "\n");
										}
										break;
								case DAQ_DEC:
										for(int i=0; i<ndata; i++)
										{
											fprintf(DaqCtrl.OutFile(), "%d ", FifoData[i]);
											if( (i % 8) == 0 && i > 0 )
												fprintf(DaqCtrl.OutFile(), "\n");
										}
										break;
								case DAQ_BIN:
										Swap32Array(FifoData, ndata);	// Change endianess
										fwrite(FifoData, 4, ndata, DaqCtrl.OutFile());
										break;
							}
						}
						else	// Empty FIFO
						{
							Sleep(10);
						}
					}
					else	// Not handshake_readout
					{
						ndata = 256;
						Asic->GetFifoDataNoHsk(ndata, FifoData, &transferred);
//if( transferred != ndata ) printf("%d Warning: Got %d data instead of %d\n", rd_cnt, transferred, ndata);
						rd_cnt++;
						if( transferred == 0 )
						{
							usleep(100);
							zero_cnt++;
						}
						else
						{
							DaqCtrl.incEvt();
							wr_cnt++;
						}
						switch( DaqCtrl.GetDaqFormat() )
						{
							case DAQ_HEX:
									for(int i=0; i<transferred; i++)
									{
										fprintf(DaqCtrl.OutFile(), "%02X %02X %02X %02X ",
										(FifoData[i]&0xFF000000) >> 24,(FifoData[i]&0xFF0000) >> 16,
										(FifoData[i]&0xFF00) >> 8,(FifoData[i]&0xFF));
										if( (i % 8) == 0 && i > 0 )
											fprintf(DaqCtrl.OutFile(), "\n");
									}
									break;
							case DAQ_DEC:
									for(int i=0; i<transferred; i++)
									{
										fprintf(DaqCtrl.OutFile(), "%d ", FifoData[i]);
										if( (i % 8) == 0 && i > 0 )
											fprintf(DaqCtrl.OutFile(), "\n");
									}
									break;
							case DAQ_BIN:
									Swap32Array(FifoData, transferred);	// Change endianess
									fwrite(FifoData, 4, transferred, DaqCtrl.OutFile());
									break;
						}
					}

				}
			}
		} while( DaqCtrl.acqRunning() && !sig_int );
	}
	else	// DARK COUNT mode
	{
		do
		{
			int drkcnt;
			for(int j=0; j<6; j++)
			{
				if( Asic->Get_ChipPresent(j) )
				{
					for(int i=0; i<64; i++)
					{
						if( Asic->Get_ChannelEnabled(j, i) )
						{
							Sleep(10);
							drkcnt = Asic->ReadChannelDarkCounter(j, i);
							fprintf(DaqCtrl.OutFile(), "%01d %02d %d 0x%02X\n",
								j, i,
								(drkcnt & 0xFFC00000)>>22,	// count
								(drkcnt & 0x003FC000)>>14);	// crc
						}
					}
				}
			}
		} while( DaqCtrl.acqRunning() && !sig_int );
	}

	for(int j=0; j<6; j++)
		if( Asic->Get_ChipPresent(j) )
			Asic->ReadoutDisable(j);

	DaqCtrl.printEventSummary();
	DaqCtrl.closeOutFile();
	cout << rd_cnt << " total read calls from detector" << endl;
	cout << zero_cnt << " read calls returned no data" << endl;
	cout << wr_cnt << " data blocks written to '" << DaqCtrl.GetOutputFileName() << "'" << endl;
	if( ff_cnt )
		cout << ff_cnt << " FIFO full events" << endl;

	IO->QuitServer();
	delete FifoData;
	delete IO;
	delete Asic;
}

