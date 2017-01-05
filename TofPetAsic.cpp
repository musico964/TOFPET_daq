
#include <string>

using namespace std;

#include "TofPetAsic.h"
#include "DaqControl.h"
#include "Lin_TcpSocket.h"

void print_bin(unsigned int x, int nbit);	// defined in main.cpp

TofPetAsic::TofPetAsic()
{
	_command = 0;
	for(int i=0; i<6; i++)
	{
		_chip_is_present[i] = false;
		for(int j=0; j<64; j++)
			_channel_enabled[i][j] = false;
	}
}
/*
assign STATUS = {GotAck, RWrunning, TestRunning, 5'b0,
				2'b0, synced1,
				2'b0, synced0,
				4'b0, CtrlFifo_OUT_full, CtrlFifo_OUT_empty, CtrlFifo_IN_full, CtrlFifo_IN_empty};
*/
unsigned int TofPetAsic::ReadFpgaStatusReg(void)
{
	GetFpgaStatus();
	return _FpgaStatusRegister;
}

bool TofPetAsic::GotAck(void)
{
	unsigned int status[5];

	IO->ReadFpgaStatus(status);
//printf("GotAck(): Status = 0x%08X\n", status[0]);
	if( status[0] & STATUS_CMD_ACK )
		return true;
	else
		return false;
}

bool TofPetAsic::CfgRunning(void)
{
	unsigned int status[5];

	IO->ReadFpgaStatus(status);
//printf("CfgRunning(): Status = 0x%08X\n", status[0]);
	if( status[0] & STATUS_CFG_RUN )
		return true;
	else
		return false;
}

void TofPetAsic::DataFifoReset(void)
{
	_command |= COMMAND_DATAFIFO_RST;	// DATA_FIFO_RESET
	IO->WriteCommandReg(_command);
	Sleep(15);
	_command &= ~COMMAND_DATAFIFO_RST;
	IO->WriteCommandReg(_command);
	Sleep(15);
}

void TofPetAsic::GetFpgaStatus(void)
{
	unsigned int status[5];

	IO->ReadFpgaStatus(status);
	_FpgaStatusRegister = status[0];
	_FifoStatusRegister = status[1];
	_FifoWordCount10 = status[2];
	_FifoWordCount32 = status[3];
	_FifoWordCount54 = status[4];
}

bool TofPetAsic::FifoEmpty(int chip)
{
	if( _FifoStatusRegister & (1 << chip) )
		return true;
	else
		return false;
}

bool TofPetAsic::FifoFull(int chip)
{
	if( _FifoStatusRegister & (0x100 << chip) )
		return true;
	else
		return false;
}

int TofPetAsic::FifoNdataAvail(int chip)
{
	switch( chip )
	{
		case 0: return _FifoWordCount10 & 0xFFFF; break;
		case 1: return _FifoWordCount10 >> 16; break;
		case 2: return _FifoWordCount32 & 0xFFFF; break;
		case 3: return _FifoWordCount32 >> 16; break;
		case 4: return _FifoWordCount54 & 0xFFFF; break;
		case 5: return _FifoWordCount54 >> 16; break;
	}
	return 0;
}

void TofPetAsic::GetFifoData(int chip, int n, unsigned int *data, int *transferred)
{
	IO->ReadDataFifo(chip, n, data, transferred);
}

void TofPetAsic::GetFifoDataNoHsk(int n, unsigned int *data, int *transferred)
{
	IO->ReadDataFifoNoHsk(n, data, transferred);
}

void TofPetAsic::ReadoutEnable(int chip)
{
	_command |= (0x1000000 << chip);
	IO->WriteCommandReg(_command);
}

void TofPetAsic::ReadoutDisable(int chip)
{
	_command &= ~(0x1000000 << chip);
	IO->WriteCommandReg(_command);
}

void TofPetAsic::SclkEnable(void)
{
	_command |= SCLK_ENABLE;
	IO->WriteCommandReg(_command);
}

void TofPetAsic::SclkDisable(void)
{
	_command &= ~SCLK_ENABLE;
	IO->WriteCommandReg(_command);
}

void TofPetAsic::GenerateResetSignal(void)
{
	_command |= COMMAND_RST;	// COMMAND_RESET
	IO->WriteCommandReg(_command);
	_command &= ~COMMAND_RST;
	IO->WriteCommandReg(_command);
}

void TofPetAsic::GenerateWideResetSignal(void)
{
	_command |= COMMAND_WIDERST;	// COMMAND_WIDE_RESET
	IO->WriteCommandReg(_command);
	_command &= ~COMMAND_WIDERST;
	IO->WriteCommandReg(_command);
}

void TofPetAsic::EnableTestSignal(void)
{
//	IO->WriteNbitReg(_n1, _n2, _test_pulse_coarse_delay, _test_pulse_zero_width);
	IO->WriteNbitReg(_n1, (_n2&0xFC)|((_test_pulse_coarse_delay>>8)&3), (_test_pulse_coarse_delay&0xFF),
		(_test_pulse_zero_width&0x7F)|((_test_pulse_polarity&1) << 7));
// Set test_pulse_tap_select = 0
	_command &= ~0x80;
	_command &= ~0x4000;
	_command &= ~0x8000;
	_command &= ~0x40000000;
	_command &= ~0x80000000;
	IO->WriteCommandReg(_command);

	_command |= COMMAND_TEST;	// COMMAND_TEST
//assign test_pulse_tap_select = {COMMAND[31:30],COMMAND[15:14],COMMAND[7]};
	if( _test_pulse_fine_delay & 0x01 ) _command |= 0x80; else _command &= ~0x80;
	if( _test_pulse_fine_delay & 0x02 ) _command |= 0x4000; else _command &= ~0x4000;
	if( _test_pulse_fine_delay & 0x04 ) _command |= 0x8000; else _command &= ~0x8000;
	if( _test_pulse_fine_delay & 0x08 ) _command |= 0x40000000; else _command &= ~0x40000000;
	if( _test_pulse_fine_delay & 0x10 ) _command |= 0x80000000; else _command &= ~0x80000000;
	IO->WriteCommandReg(_command);
	Sleep(100);
	_command &= ~COMMAND_TEST;
	IO->WriteCommandReg(_command);
}

void TofPetAsic::GenerateTestSignal(void)
{
//	IO->WriteNbitReg(_n1, _n2, _test_pulse_coarse_delay, _test_pulse_zero_width);
	IO->WriteNbitReg(_n1, (_n2&0xFC)|((_test_pulse_coarse_delay>>8)&3), (_test_pulse_coarse_delay&0xFF),
		(_test_pulse_zero_width&0x7F)|((_test_pulse_polarity&1) << 7));
	_command |= COMMAND_TEST;	// COMMAND_TEST

//assign test_pulse_tap_select = {COMMAND[31:30],COMMAND[15:14],COMMAND[7]};
	if( _test_pulse_fine_delay & 0x01 ) _command |= 0x80; else _command &= ~0x80;
	if( _test_pulse_fine_delay & 0x02 ) _command |= 0x4000; else _command &= ~0x4000;
	if( _test_pulse_fine_delay & 0x04 ) _command |= 0x8000; else _command &= ~0x8000;
	if( _test_pulse_fine_delay & 0x08 ) _command |= 0x40000000; else _command &= ~0x40000000;
	if( _test_pulse_fine_delay & 0x10 ) _command |= 0x80000000; else _command &= ~0x80000000;

	IO->WriteCommandReg(_command);
	if( _test_pulse_duration != 0 && _test_pulse_zero_width != 0 )
	{
		Sleep(_test_pulse_duration);
		IO->WriteNbitReg(_n1, _n2, _test_pulse_coarse_delay, 0);
	}
	_command &= ~COMMAND_TEST;
	IO->WriteCommandReg(_command);
}

void TofPetAsic::WriteChannelConfiguration(int chip, int channel)
{
	_PrepareChannelConfigVector(chip, 0, channel);
	_DoConfig(chip);
}

void TofPetAsic::WriteChannelTestConfig(int chip, int channel)
{
	_PrepareChannelTestConfigVector(0, channel, _ch_calib_enable[chip][channel]);
	_DoConfig(chip);
}

void TofPetAsic::WriteGlobalConfiguration(int chip)
{
	_PrepareGlobalConfigVector(chip, 0);
	_DoConfig(chip);
}

void TofPetAsic::WriteGlobalTestConfig(int chip)
{
	_PrepareTestConfigVector(chip, 0);
	_DoConfig(chip);
}

void TofPetAsic::WriteTestCommand(int chip)
{
	_PrepareTestCommandVector(chip);
	_DoConfig(chip);
}

bool TofPetAsic::ReadGlobalConfiguration(int chip)
{
	unsigned char crc, rd_crc, xx_d[16];

	_PrepareGlobalConfigVector(chip, 1);
	_RdConfig(chip);
	xx_d[0] = (_fifo_data[0] & 0xFF000000) >> 24;
	xx_d[1] = (_fifo_data[0] & 0x00FF0000) >> 16;
	xx_d[2] = (_fifo_data[0] & 0x0000FF00) >> 8;
	xx_d[3] = (_fifo_data[0] & 0x000000FF) >> 0;
	xx_d[4] = (_fifo_data[1] & 0xFF000000) >> 24;
	xx_d[5] = (_fifo_data[1] & 0x00FF0000) >> 16;
	xx_d[6] = (_fifo_data[1] & 0x0000FF00) >> 8;
	xx_d[7] = (_fifo_data[1] & 0x000000FF) >> 0;
	xx_d[8] = (_fifo_data[2] & 0xFF000000) >> 24;
	xx_d[9] = (_fifo_data[2] & 0x00FF0000) >> 16;
	xx_d[10] = (_fifo_data[2] & 0x0000FF00) >> 8;
	xx_d[11] = (_fifo_data[2] & 0x000000FF) >> 0;
	xx_d[12] = (_fifo_data[3] & 0xFF000000) >> 24;
	xx_d[13] = (_fifo_data[3] & 0x00FF0000) >> 16;
	xx_d[14] = (_fifo_data[3] & 0x0000FF00) >> 8;
	xx_d[15] = (_fifo_data[3] & 0x000000FF) >> 0;
	crc = _crc8(xx_d, 110);
	rd_crc = (_fifo_data[3] & 0x3FC00) >> 10;
//	printf("ReadGlobalConfiguration() _fifo_data[3] = 0x%08X, crc 0x%02X, rd_crc = 0x%02X\n", _fifo_data[3], crc, rd_crc);
	if( rd_crc == crc )
		return true;
	else
		return false;
}

void TofPetAsic::ReadGlobalTestConfig(int chip)
{
	_PrepareTestConfigVector(chip, 1);
	_RdConfig(chip);
}

void TofPetAsic::ReadChannelConfiguration(int chip, int channel)
{
	_PrepareChannelConfigVector(chip, 1, channel);
	_RdConfig(chip);
}

int TofPetAsic::ReadChannelDarkCounter(int chip, int channel)
{
	unsigned char crc, xx[4];

	_n1 = 19;
	_n2 = 18;
	xx[0] = 0x40 |	// Command bits
		(channel & 0x78) >> 3;
	xx[1] = (channel & 0x07) << 5;
	xx[2] = xx[3] = 0;;
	crc = _crc8(xx, 11);
	xx[1] &= 0xE0;
	xx[1] |= (crc & 0xF8) >> 3;
	xx[2] = (crc & 0x07) << 5;

	_fifo_data[0] = xx[0] << 24 | xx[1] << 16 | xx[2] << 8 | xx[3];
	_fifo_data[1] = _fifo_data[2] = _fifo_data[3] = 0;
//	printf("ReadChannelDarkCounter() _fifo_data[0] = 0x%08X\n", _fifo_data[0]);

	_RdConfig(chip);

	return( _fifo_data[0] );
}

void TofPetAsic::_PrepareTestCommandVector(int chip)
{
	unsigned char crc;

	_t_command_vector[0] = 0xA0 |	// Command bits
		(_interval_between_pulses[chip]&0xF0) >> 4;
	_t_command_vector[1] = (_interval_between_pulses[chip]&0x0F) << 4 |
		(_pulse_length[chip]&0xF0)>>4;
	_t_command_vector[2] = (_pulse_length[chip]&0x0F) << 4 |
		(_number_of_pulses[chip]&0x3C0)>>6;
	_t_command_vector[3] =
		(_number_of_pulses[chip]&0x3F)<<2;
	crc = _crc8(_t_command_vector, 30);
	_t_command_vector[3] |= (crc&0xC0) >> 6;
	_t_command_vector[4] = (crc&0x3F) << 2;
	_n1 = 38;
	_n2 = 1;
	_fifo_data[0] = _t_command_vector[0] << 24 |
		_t_command_vector[1] << 16 |
		_t_command_vector[2] << 8 |
		_t_command_vector[3];
	_fifo_data[1] = _t_command_vector[4] << 24;
	_fifo_data[2] = 0;
	_fifo_data[3] = 0;
}

void TofPetAsic::_PrepareGlobalConfigVector(int chip, int rw)
{
	unsigned char crc;

	_g_config_vector[0] = rw ? 0x90 : 0x80 |	// Command bits
		(_TDCcomp_sw[chip] & 0x3C) >> 2;
	_g_config_vector[1] = (_TDCcomp_sw[chip] & 0x03) << 6 |
		(_compvcas_sw[chip] & 0x3F);
	_g_config_vector[2] = (_TDCiref_sw[chip] & 0x3F) << 2 |
		(_TDCtac_sw[chip] & 0x30) >> 4;
	_g_config_vector[3] = (_TDCtac_sw[chip] & 0x0F) << 4 |
// MSB first
/*
		(_lvlshft_sw[chip] & 0x3C) >> 2;
	_g_config_vector[4] = (_lvlshft_sw[chip] & 0x03) << 6 |
		(_vib2_sw[chip] & 0x3F);
	_g_config_vector[5] = (_vib1_sw[chip] & 0x3F) << 2 |
		(_sipm_iDAC_range_sw[chip] & 0x30) >> 4;
	_g_config_vector[6] = (_sipm_iDAC_range_sw[chip] & 0x0F) << 4 |
		(_sipm_iDAC_dcstart_sw[chip] & 0x3C) >> 2;
	_g_config_vector[7] = (_sipm_iDAC_dcstart_sw[chip] & 0x03) << 6 |
*/
// LSB First
		(_lvlshft_sw[chip] & 0x01) << 3 | (_lvlshft_sw[chip] & 0x02) << 1 |
		(_lvlshft_sw[chip] & 0x04) >> 1 | (_lvlshft_sw[chip] & 0x08) >> 3;
	_g_config_vector[4] = (_lvlshft_sw[chip] & 0x10) << 3 | (_lvlshft_sw[chip] & 0x20) << 1 |
		(_vib2_sw[chip]&0x01) << 5 | (_vib2_sw[chip]&0x02) << 3 | (_vib2_sw[chip]&0x04) << 1 | 
		(_vib2_sw[chip]&0x08) >> 1 | (_vib2_sw[chip]&0x10) >> 3 | (_vib2_sw[chip]&0x20) >> 5;
	_g_config_vector[5] = (_vib1_sw[chip]&0x01) << 7 | (_vib1_sw[chip]&0x02) << 5 | (_vib1_sw[chip]&0x04) << 3 | 
		(_vib1_sw[chip]&0x08) << 1 | (_vib1_sw[chip]&0x10) >> 1 | (_vib1_sw[chip]&0x20) >> 3 |
		(_sipm_iDAC_range_sw[chip] & 0x01) << 1 | (_sipm_iDAC_range_sw[chip] & 0x02) >> 1;
	_g_config_vector[6] = (_sipm_iDAC_range_sw[chip] & 0x04) << 5 | (_sipm_iDAC_range_sw[chip] & 0x08) << 3 |
		(_sipm_iDAC_range_sw[chip] & 0x10) << 1 | (_sipm_iDAC_range_sw[chip] & 0x20) >> 1 |
		(_sipm_iDAC_dcstart_sw[chip] & 0x01) << 3 | (_sipm_iDAC_dcstart_sw[chip] & 0x02) << 1 |
		(_sipm_iDAC_dcstart_sw[chip] & 0x04) >> 1 | (_sipm_iDAC_dcstart_sw[chip] & 0x08) >> 3;
	_g_config_vector[7] = (_sipm_iDAC_dcstart_sw[chip] & 0x10) << 3 | (_sipm_iDAC_dcstart_sw[chip] & 0x20) << 1 |
// end LSB-MSB
		(_disc_iDAC_range_sw[chip] & 0x3F);
	_g_config_vector[8] = (_disc_iDAC_dcstart_sw[chip] & 0x3F) << 2 |
		(_postamp_sw[chip] & 0x30) >> 4;
	_g_config_vector[9] = (_postamp_sw[chip] & 0x0F) << 4 |
		(_FE_discibias_sw[chip] & 0x3C) >> 2;
	_g_config_vector[10] = (_FE_discibias_sw[chip] & 0x03) << 6 |
		(_FE_discvcas_sw[chip] & 0x3F);

	_g_config_vector[11] = (_clk_o_enable[chip] & 0x01) << 7 |
		(_test_pattern[chip] & 0x01) << 6 |
		(_external_veto[chip] & 0x01) << 5 |
		(_event_data_mode[chip] & 0x01) << 4;
		(_counter_interval[chip] & 0x0F);

	_g_config_vector[12] = (_count_trig_error[chip] & 0x01) << 7 |
		(_fine_counter_kf[chip] & 0xFE) >> 1;
	_g_config_vector[13] = (_fine_counter_kf[chip] & 0x01) << 7 |
		(_fine_counter_saturate[chip] & 0x01) << 6 |
		(_TAC_refresh[chip] & 0x0F) << 2 |
		(_external_pulse_enable[chip] & 0x01) << 1 |
		(_DDR_mode[chip] & 0x01) ;
	_g_config_vector[14] = (_TX_mode[chip] & 0x03) << 6;

	crc = _crc8(_g_config_vector, rw ? 4 : 114);

	if( rw )	// Read
	{
		_g_config_vector[0] = 0x90 | (crc & 0xF0) >> 4;
		_g_config_vector[1] = (crc & 0x0F) << 4;
		_g_config_vector[2] =_g_config_vector[3] =_g_config_vector[4] =_g_config_vector[5] = 0;
		_g_config_vector[6] =_g_config_vector[7] =_g_config_vector[8] =_g_config_vector[9] = 0;
		_g_config_vector[10] =_g_config_vector[11] =_g_config_vector[12] =_g_config_vector[13] = 0;
		_g_config_vector[14] =_g_config_vector[15] = 0;
		_n1 = 12;
		_n2 = 118;
	}
	else
	{
		_g_config_vector[14] |= (crc & 0xFC) >> 2;
		_g_config_vector[15] = (crc & 0x03) << 6;
		_n1 = 122;
		_n2 = 1;
	}
	_fifo_data[0] = _g_config_vector[0] << 24 |
		_g_config_vector[1] << 16 |
		_g_config_vector[2] << 8 |
		_g_config_vector[3];
	_fifo_data[1] = _g_config_vector[4] << 24 |
		_g_config_vector[5] << 16 |
		_g_config_vector[6] << 8 |
		_g_config_vector[7];
	_fifo_data[2] = _g_config_vector[8] << 24 |
		_g_config_vector[9] << 16 |
		_g_config_vector[10] << 8 |
		_g_config_vector[11];
	_fifo_data[3] = _g_config_vector[12] << 24 |
		_g_config_vector[13] << 16 |
		_g_config_vector[14] << 8 |
		_g_config_vector[15];
//for(int i=0;i<4;i++) printf("    0x%08X\n",_fifo_data[i]);
}

void TofPetAsic::_PrepareTestConfigVector(int chip, int rw)
{
	unsigned char crc;

	_t_config_vector[0] = rw ? 0xD0 : 0xC0 |	// Command bits
		(_calib_sw[chip] & 0x3C) >> 2;
	_t_config_vector[1] = (_calib_sw[chip] & 0x03) << 6 |
		(_global_calib_enable[chip] & 0x01) << 5;
	crc = _crc8(_t_config_vector, rw ? 4 : 11);
	if( rw )	// Read
	{
		_t_config_vector[0] = 0xD0 | (crc & 0xF0) >> 4;
		_t_config_vector[1] = (crc & 0x0F) << 4;
		_t_config_vector[2] = 0;
		_n1 = 12;
		_n2 = 15;
	}
	else
	{
		_t_config_vector[1] |= (crc & 0xF8) >> 3;
		_t_config_vector[2] = (crc & 0x07) << 5;
		_n1 = 19;
		_n2 = 1;
	}
	_fifo_data[0] = _t_config_vector[0] << 24 |
		_t_config_vector[1] << 16 |
		_t_config_vector[2] << 8;
	_fifo_data[1] = 0;
	_fifo_data[2] = 0;
	_fifo_data[3] = 0;
//printf("G_TConfig: "); print_bin(_fifo_data[0], rw ? 12 : 19);
}

void TofPetAsic::_PrepareChannelConfigVector(int chip, int rw, int ch)
{
	unsigned char crc;

	_ch_config_vector[0] = rw ? 0x10 : 0x00 |	// Command bits
		(ch & 0x78) >> 3;
	_ch_config_vector[1] = (ch & 0x07) << 5 |
// LSB First
		(_de[chip][ch] & 0x01) << 4 |		// d0e
		(_de[chip][ch] & 0x02) << 2 |		// d1e
		(_de[chip][ch] & 0x04) << 0 |		// d2e
		(_de[chip][ch] & 0x08) >> 2 |		// d3e
		(_de[chip][ch] & 0x10) >> 4;		// d4e
// MSB first
/*
		(_de[chip][ch] & 0x1F);		// de
*/
// end LSB-MSB
	_ch_config_vector[2] = (_test_cfg[chip][ch] & 0x03) << 6 |
		(_test_T_EN[chip][ch] & 0x01) << 5 |
		(_test_E_EN[chip][ch] & 0x01) << 4 |
		(_sync[chip][ch] & 0x01) << 3 |
		(_gtest[chip][ch] & 0x01) << 2 |
		(_deadtime_EN[chip][ch] & 0x01) << 1 |
		(_deadtime[chip][ch] & 0x01);
	_ch_config_vector[3] = (_meta_cfg[chip][ch] & 0x03) << 6 |	// meta_cfg[1:0]
		(_latch_cfg[chip][ch] & 0x06) << 3 |	// latch_cfg[2:1]
		(_d[chip][ch] & 0x1E) >> 1;		// d[4:1]
	_ch_config_vector[4] = (_d[chip][ch] & 0x01) << 7|// d[0]
		(_latch_cfg[chip][ch] & 0x01) << 6 |	// latch_cfg[0]
		(_cgate_cfg[chip][ch] & 0x07) << 3 |	// cgate_cfg[2:0]
		(_g0_2_bar[chip][ch] & 0x01) << 2 |
		(_g0_4_bar[chip][ch] & 0x01) << 1 |
		(_sh_5n_bar[chip][ch] & 0x01);
	_ch_config_vector[5] = (_sh_2n5_bar[chip][ch] & 0x01) << 7 |
		(_sw_E[chip][ch] & 0x01) << 6 |	// sw1_E
		(_sw_E[chip][ch] & 0x02) << 4 |	// sw2_E
		(_sw_E[chip][ch] & 0x04) << 2 |	// sw4_E
		(_sw_E[chip][ch] & 0x08) << 0 |	// sw8_E
		(_sw_E[chip][ch] & 0x10) >> 2 |	// sw16_E
		(_sw_E[chip][ch] & 0x20) >> 4 |	// sw32_E
		(_sw[chip][ch] & 0x01);		// sw1
	_ch_config_vector[6] = (_sw[chip][ch] & 0x02) << 6 |	// sw2
		(_sw[chip][ch] & 0x04) << 4 |	// sw4
		(_sw[chip][ch] & 0x08) << 2 |	// sw8
		(_sw[chip][ch] & 0x10) << 0 |	// sw16
		(_sw[chip][ch] & 0x20) >> 2 |	// sw32
		(_vbl_sw[chip][ch] & 0x01) << 2 |	// vbl_sw1
		(_vbl_sw[chip][ch] & 0x02) << 0 |	// vbl_sw2
		(_vbl_sw[chip][ch] & 0x04) >> 2;	// vbl_sw4
	_ch_config_vector[7] = (_vbl_sw[chip][ch] & 0x08) << 4 |	// vbl_sw8
		(_inp_pol[chip][ch] & 0x01) << 6 |
		(_test_mode[chip][ch] & 0x01) << 5 |
		(_tmjitter[chip][ch] & 0x01) << 4 |
		(_vbl_sw[chip][ch] & 0x20) >> 2 |	// vbl_sw32
		(_vbl_sw[chip][ch] & 0x10) >> 2 |	// vbl_sw16
		(_praedictio[chip][ch] & 0x01) << 1 |
//		(_ch_enable[chip][ch] & 0x01);
		(1 & 0x01);	// MUST be always 1

	crc = _crc8(_ch_config_vector, rw ? 11 : 64);
	if( rw )	// Read
	{
		_ch_config_vector[1] &= 0xE0;
		_ch_config_vector[1] |= (crc & 0xF8) >> 3;
		_ch_config_vector[2] = (crc & 0x07) << 5;
		_ch_config_vector[3] = _ch_config_vector[4] = _ch_config_vector[5] = 0;
		_ch_config_vector[6] = _ch_config_vector[7] = _ch_config_vector[8] = 0;
		_n1 = 19;
		_n2 = 61;
	}
	else
	{
		_ch_config_vector[8] = crc;
		_n1 = 72;
		_n2 = 53;
	}
	_fifo_data[0] = _ch_config_vector[0] << 24 |
		_ch_config_vector[1] << 16 |
		_ch_config_vector[2] << 8 |
		_ch_config_vector[3];
	_fifo_data[1] = _ch_config_vector[4] << 24 |
		_ch_config_vector[5] << 16 |
		_ch_config_vector[6] << 8 |
		_ch_config_vector[7];
	_fifo_data[2] = _ch_config_vector[8] << 24;
	_fifo_data[3] = 0;
//for(int i=0;i<4;i++) printf("    0x%08X\n",_fifo_data[i]);
//printf("sw[%d][%d] = %d, sw_E[%d][%d] = %d, vbl[%d][%d] = %d\n", chip, ch, _sw[chip][ch], chip, ch, _sw_E[chip][ch], chip, ch, _vbl_sw[chip][ch]);
}

void TofPetAsic::_PrepareChannelTestConfigVector(int rw, int ch, int enable_calib)
{
	unsigned char crc;

	_ch_t_config_vector[0] = rw ? 0x30 : 0x20 |	// Command bits
		(ch & 0x78) >> 3;
	_ch_t_config_vector[1] = (ch & 0x07) << 5 |
//		(_default_ch_calib_enable & 0x01) << 4;
		(enable_calib & 0x01) << 4;
	crc = _crc8(_ch_t_config_vector, rw ? 11 : 12);
	if( rw )	// Read
	{
		_ch_t_config_vector[1] |= (crc & 0xF8) >> 3;
		_ch_t_config_vector[2] = (crc & 0x07) << 5;
		_n1 = 19;
		_n2 = 9;
	}
	else
	{
		_ch_t_config_vector[1] |= (crc & 0xF0) >> 4;
		_ch_t_config_vector[2] = (crc & 0x0F) << 4;
		_n1 = 20;
		_n2 = 1;
	}
	_fifo_data[0] = _ch_t_config_vector[0] << 24 |
		_ch_t_config_vector[1] << 16 |
		_ch_t_config_vector[2] << 8;
	_fifo_data[1] = 0;
	_fifo_data[2] = 0;
	_fifo_data[3] = 0;
//printf("Ch_TConfig[%d]: ", ch); print_bin(_fifo_data[0], rw ? 19 : 20);
}


// Computes x^8 + x^2 + x + 1 CCITT 8-bit CRC
unsigned char TofPetAsic::_crc8(unsigned char *data, int nbits)
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

void TofPetAsic::_DoConfig(int chip)
{
	_command = (_TX_mode[chip] & 0x03) << 22 |
		(_DDR_mode[chip] & 0x01) << 21 |
		SCLK_ENABLE |
		FE_ENABLEb |		// ?? or negated ??
		(chip & 0x07) << 16 |	// FE_SELECT
		0x100 << chip;		// CHIP_SELECT

	if( DaqCtrl.GetSkipEmptyEvents() )
		_command |= SKIP_EMPTY_EVENTS;

	_command |= COMMAND_CTRLFIFO_RST;	// CONTROL_FIFO_RESET
	IO->WriteCommandReg(_command);
	Sleep(15);
	_command &= ~COMMAND_CTRLFIFO_RST;
	IO->WriteCommandReg(_command);
	Sleep(15);

	IO->WriteConfigFifo(_fifo_data, 4);
	Sleep(15);

	IO->WriteNbitReg(_n1, _n2);
	Sleep(15);

	_command |= COMMAND_CFG_WRITE;	// COMMAND_CFG_WRITE
	IO->WriteCommandReg(_command);
	Sleep(15);
	_command &= ~COMMAND_CFG_WRITE;
	IO->WriteCommandReg(_command);
	Sleep(15);

//	for(int i=0; i<4; i++) printf("WRITE Config FIFO data[%d] = 0x%08X\n", i, _fifo_data[i]);
}

void TofPetAsic::_RdConfig(int chip)
{
	_command = (_TX_mode[chip] & 0x03) << 22 |
		(_DDR_mode[chip] & 0x01) << 21 |
		SCLK_ENABLE |
		FE_ENABLEb |		// ?? or negated ??
		(chip & 0x07) << 16 |	// FE_SELECT
		0x100 << chip;		// CHIP_SELECT

	if( DaqCtrl.GetSkipEmptyEvents() )
		_command |= SKIP_EMPTY_EVENTS;

	_command |= COMMAND_CTRLFIFO_RST;	// CONTROL_FIFO_RESET
	IO->WriteCommandReg(_command);
	Sleep(15);
	_command &= ~COMMAND_CTRLFIFO_RST;
	IO->WriteCommandReg(_command);
	Sleep(15);

	IO->WriteConfigFifo(_fifo_data, 4);
	Sleep(15);

	IO->WriteNbitReg(_n1, _n2);
	Sleep(15);

	_command |= COMMAND_CFG_READ;	// COMMAND_CFG_READ
	IO->WriteCommandReg(_command);
	Sleep(15);
	_command &= ~COMMAND_CFG_READ;
	IO->WriteCommandReg(_command);
	Sleep(100);
//	if( !GotAck() )
//		printf("WARNING _RdConfig() FAILED!\n");

	IO->ReadConfigFifo(_fifo_data, 4);
	Sleep(15);

	int nwords = _n2 / 32 + 1;
	_fifo_data[nwords-1] <<= 32 - (_n2 % 32);

//for(int i=0; i<nwords; i++) printf("READ Config FIFO data[%d] = 0x%08X\n", i, _fifo_data[i]);
//printf("Last word shifted left by %d positions\n\n", 32-(_n2%32));
}

