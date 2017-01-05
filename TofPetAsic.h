#define G_CONFIG_V_SIZE 16
#define T_CONFIG_V_SIZE 3
#define T_COMMAND_V_SIZE 5
#define CH_T_CONFIG_V_SIZE 3
#define CH_CONFIG_V_SIZE 9
#define FIFO_DATA_SIZE 20

#define Sleep(x) usleep(1000*x)


class TofPetAsic
{
public:

	TofPetAsic(void);
	~TofPetAsic(void) { };

// Get data
	void DataFifoReset(void);
	bool FifoEmpty(int chip);
	bool FifoFull(int chip);
	int FifoNdataAvail(int chip);
	void GetFifoData(int chip, int n, unsigned int *data, int *transferred);
	int ReadChannelDarkCounter(int chip, int channel);
	void GetFifoDataNoHsk(int n, unsigned int *data, int *transferred);

// Control Signals Generation
	void GenerateResetSignal(void);
	void GenerateWideResetSignal(void);
	void GenerateTestSignal(void);
	void EnableTestSignal(void);


// Configuration commands
	void ReadoutEnable(int chip);
	void ReadoutDisable(int chip);
	void SclkEnable(void);
	void SclkDisable(void);
	void WriteChannelConfiguration(int chip, int channel);
	void ReadChannelConfiguration(int chip, int channel);
	void WriteChannelTestConfig(int chip, int channel);
	void ReadChannelTestConfig(int chip, int channel);
	void WriteGlobalConfiguration(int chip);
	bool ReadGlobalConfiguration(int chip);
	void WriteGlobalTestConfig(int chip);
	void WriteTestCommand(int chip);
	void ReadGlobalTestConfig(int chip);
	void GenerateTestPulse(int chip);
	void ReadChannelCounter(int chip, int channel);
	bool GotAck(void);
	bool CfgRunning(void);
	void GetFpgaStatus(void);
	unsigned int ReadFpgaStatusReg(void);

// Access to private variables
	int Get_TDCcomp_sw(int ch) {return(_TDCcomp_sw[ch]);}
	int Get_compvcas_sw(int ch) {return(_compvcas_sw[ch]);}
	int Get_TDCiref_sw(int ch) {return(_TDCiref_sw[ch]);}
	int Get_TDCtac_sw(int ch) {return(_TDCtac_sw[ch]);}
	int Get_lvlshft_sw(int ch) {return(_lvlshft_sw[ch]);}
	int Get_vib2_sw(int ch) {return(_vib2_sw[ch]);}
	int Get_vib1_sw(int ch) {return(_vib1_sw[ch]);}
	int Get_sipm_iDAC_range_sw(int ch) {return(_sipm_iDAC_range_sw[ch]);}
	int Get_sipm_iDAC_dcstart_sw(int ch) {return(_sipm_iDAC_dcstart_sw[ch]);}
	int Get_disc_iDAC_range_sw(int ch) {return(_disc_iDAC_range_sw[ch]);}
	int Get_disc_iDAC_dcstart_sw(int ch) {return(_disc_iDAC_dcstart_sw[ch]);}
	int Get_postamp_sw(int ch) {return(_postamp_sw[ch]);}
	int Get_FE_discibias_sw(int ch) {return(_FE_discibias_sw[ch]);}
	int Get_FE_discvcas_sw(int ch) {return(_FE_discvcas_sw[ch]);}
	int Get_clk_o_enable(int ch) {return(_clk_o_enable[ch]);}
	int Get_test_pattern(int ch) {return(_test_pattern[ch]);}
	int Get_external_veto(int ch) {return(_external_veto[ch]);}
	int Get_event_data_mode(int ch) {return(_event_data_mode[ch]);}
	int Get_counter_interval(int ch) {return(_counter_interval[ch]);}
	int Get_count_trig_error(int ch) {return(_count_trig_error[ch]);}
	int Get_fine_counter_kf(int ch) {return(_fine_counter_kf[ch]);}
	int Get_fine_counter_saturate(int ch) {return(_fine_counter_saturate[ch]);}
	int Get_TAC_refresh(int ch) {return(_TAC_refresh[ch]);}
	int Get_external_pulse_enable(int ch) {return(_external_pulse_enable[ch]);}
	int Get_DDR_mode(int ch) {return(_DDR_mode[ch]);}
	int Get_TX_mode(int ch) {return(_TX_mode[ch]);}
	int Get_calib_sw(int ch) {return(_calib_sw[ch]);}
	int Get_global_calib_enable(int ch) {return(_global_calib_enable[ch]);}
	int Get_interval_between_pulses(int ch) {return(_interval_between_pulses[ch]);}
	int Get_pulse_length(int ch) {return(_pulse_length[ch]);}
	int Get_number_of_pulses(int ch) {return(_number_of_pulses[ch]);}

	int Get_de(int ch, int c) {return(_de[ch][c]);}
	int Get_test_cfg(int ch, int c) {return(_test_cfg[ch][c]);}
	int Get_test_T_EN(int ch, int c) {return(_test_T_EN[ch][c]);}
	int Get_test_E_EN(int ch, int c) {return(_test_E_EN[ch][c]);}
	int Get_sync(int ch, int c) {return(_sync[ch][c]);}
	int Get_gtest(int ch, int c) {return(_gtest[ch][c]);}
	int Get_deadtime_EN(int ch, int c) {return(_deadtime_EN[ch][c]);}
	int Get_deadtime(int ch, int c) {return(_deadtime[ch][c]);}
	int Get_meta_cfg(int ch, int c) {return(_meta_cfg[ch][c]);}
	int Get_latch_cfg(int ch, int c) {return(_latch_cfg[ch][c]);}
	int Get_d(int ch, int c) {return(_d[ch][c]);}
	int Get_cgate_cfg(int ch, int c) {return(_cgate_cfg[ch][c]);}
	int Get_g0_2_bar(int ch, int c) {return(_g0_2_bar[ch][c]);}
	int Get_g0_4_bar(int ch, int c) {return(_g0_4_bar[ch][c]);}
	int Get_sh_5n_bar(int ch, int c) {return(_sh_5n_bar[ch][c]);}
	int Get_sh_2n5_bar(int ch, int c) {return(_sh_2n5_bar[ch][c]);}
	int Get_sw_E(int ch, int c) {return(_sw_E[ch][c]);}
	int Get_sw(int ch, int c) {return(_sw[ch][c]);}
	int Get_vbl_sw(int ch, int c) {return(_vbl_sw[ch][c]);}
	int Get_inp_pol(int ch, int c) {return(_inp_pol[ch][c]);}
	int Get_test_mode(int ch, int c) {return(_test_mode[ch][c]);}
	int Get_tmjitter(int ch, int c) {return(_tmjitter[ch][c]);}
	int Get_praedictio(int ch, int c) {return(_praedictio[ch][c]);}
	int Get_ch_enable(int ch, int c) {return(_ch_enable[ch][c]);}
	int Get_ch_calib_enable(int ch, int c) {return(_ch_calib_enable[ch][c]);}

	bool Get_ChipPresent(int x) {return(_chip_is_present[x]);}
	int Get_ChannelEnabled(int c, int x) {return((int)_channel_enabled[c][x]);}

	int Get_TestPulseCoarseDelay(void) {return (_test_pulse_coarse_delay);}
	int Get_TestPulseFineDelay(void) {return (_test_pulse_fine_delay);}
	int Get_TestPulseZeroWidth(void) {return (_test_pulse_zero_width);}
	int Get_TestPulseDuration(void) {return (_test_pulse_duration);}
	int Get_TestPulsePolarity(void) {return (_test_pulse_polarity);}

	void Set_TDCcomp_sw(int ch, int x) {_TDCcomp_sw[ch]=x;}
	void Set_compvcas_sw(int ch, int x) {_compvcas_sw[ch]=x;}
	void Set_TDCiref_sw(int ch, int x) {_TDCiref_sw[ch]=x;}
	void Set_TDCtac_sw(int ch, int x) {_TDCtac_sw[ch]=x;}
	void Set_lvlshft_sw(int ch, int x) {_lvlshft_sw[ch]=x;}
	void Set_vib2_sw(int ch, int x) {_vib2_sw[ch]=x;}
	void Set_vib1_sw(int ch, int x) {_vib1_sw[ch]=x;}
	void Set_sipm_iDAC_range_sw(int ch, int x) {_sipm_iDAC_range_sw[ch]=x;}
	void Set_sipm_iDAC_dcstart_sw(int ch, int x) {_sipm_iDAC_dcstart_sw[ch]=x;}
	void Set_disc_iDAC_range_sw(int ch, int x) {_disc_iDAC_range_sw[ch]=x;}
	void Set_disc_iDAC_dcstart_sw(int ch, int x) {_disc_iDAC_dcstart_sw[ch]=x;}
	void Set_postamp_sw(int ch, int x) {_postamp_sw[ch]=x;}
	void Set_FE_discibias_sw(int ch, int x) {_FE_discibias_sw[ch]=x;}
	void Set_FE_discvcas_sw(int ch, int x) {_FE_discvcas_sw[ch]=x;}
	void Set_clk_o_enable(int ch, int x) {_clk_o_enable[ch]=x;}
	void Set_test_pattern(int ch, int x) {_test_pattern[ch]=x;}
	void Set_external_veto(int ch, int x) {_external_veto[ch]=x;}
	void Set_event_data_mode(int ch, int x) {_event_data_mode[ch]=x;}
	void Set_counter_interval(int ch, int x) {_counter_interval[ch]=x;}
	void Set_count_trig_error(int ch, int x) {_count_trig_error[ch]=x;}
	void Set_fine_counter_kf(int ch, int x) {_fine_counter_kf[ch]=x;}
	void Set_fine_counter_saturate(int ch, int x) {_fine_counter_saturate[ch]=x;}
	void Set_TAC_refresh(int ch, int x) {_TAC_refresh[ch]=x;}
	void Set_external_pulse_enable(int ch, int x) {_external_pulse_enable[ch]=x;}
	void Set_DDR_mode(int ch, int x) {_DDR_mode[ch]=x;}
	void Set_TX_mode(int ch, int x) {_TX_mode[ch]=x;}
	void Set_calib_sw(int ch, int x) {_calib_sw[ch]=x;}
	void Set_global_calib_enable(int ch, int x) {_global_calib_enable[ch]=x;}
	void Set_interval_between_pulses(int ch, int x) {_interval_between_pulses[ch]=x;}
	void Set_pulse_length(int ch, int x) {_pulse_length[ch]=x;}
	void Set_number_of_pulses(int ch, int x) {_number_of_pulses[ch]=x;}

	void Set_de(int ch, int c, int x) {_de[ch][c]=x;}
	void Set_test_cfg(int ch, int c, int x) {_test_cfg[ch][c]=x;}
	void Set_test_T_EN(int ch, int c, int x) {_test_T_EN[ch][c]=x;}
	void Set_test_E_EN(int ch, int c, int x) {_test_E_EN[ch][c]=x;}
	void Set_sync(int ch, int c, int x) {_sync[ch][c]=x;}
	void Set_gtest(int ch, int c, int x) {_gtest[ch][c]=x;}
	void Set_deadtime_EN(int ch, int c, int x) {_deadtime_EN[ch][c]=x;}
	void Set_deadtime(int ch, int c, int x) {_deadtime[ch][c]=x;}
	void Set_meta_cfg(int ch, int c, int x) {_meta_cfg[ch][c]=x;}
	void Set_latch_cfg(int ch, int c, int x) {_latch_cfg[ch][c]=x;}
	void Set_d(int ch, int c, int x) {_d[ch][c]=x;}
	void Set_cgate_cfg(int ch, int c, int x) {_cgate_cfg[ch][c]=x;}
	void Set_g0_2_bar(int ch, int c, int x) {_g0_2_bar[ch][c]=x;}
	void Set_g0_4_bar(int ch, int c, int x) {_g0_4_bar[ch][c]=x;}
	void Set_sh_5n_bar(int ch, int c, int x) {_sh_5n_bar[ch][c]=x;}
	void Set_sh_2n5_bar(int ch, int c, int x) {_sh_2n5_bar[ch][c]=x;}
	void Set_sw_E(int ch, int c, int x) {_sw_E[ch][c]=x;}
	void Set_sw(int ch, int c, int x) {_sw[ch][c]=x;}
	void Set_vbl_sw(int ch, int c, int x) {_vbl_sw[ch][c]=x;}
	void Set_inp_pol(int ch, int c, int x) {_inp_pol[ch][c]=x;}
	void Set_test_mode(int ch, int c, int x) {_test_mode[ch][c]=x;}
	void Set_tmjitter(int ch, int c, int x) {_tmjitter[ch][c]=x;}
	void Set_praedictio(int ch, int c, int x) {_praedictio[ch][c]=x;}
	void Set_ch_enable(int ch, int c, int x) {_ch_enable[ch][c]=x;}
	void Set_ch_calib_enable(int ch, int c, int x) {_ch_calib_enable[ch][c]=x;}

	void Set_ChipPresent(int x) {_chip_is_present[x] = true;}
	void Set_ChannelEnabled(int c, int x) {_channel_enabled[c][x] = true;}
	void Set_ChannelDisabled(int c, int x) {_channel_enabled[c][x] = false;}
 
	void Set_TestPulseCoarseDelay(int x) {_test_pulse_coarse_delay = x;}
	void Set_TestPulseFineDelay(int x) {_test_pulse_fine_delay = x;}
	void Set_TestPulseZeroWidth(int x) {_test_pulse_zero_width = x;}
	void Set_TestPulseDuration(int x) {_test_pulse_duration = x;}
	void Set_TestPulsePolarity(int x) {_test_pulse_polarity = x;}
private:

	int _TDCcomp_sw[6];
	int _compvcas_sw[6];
	int _TDCiref_sw[6];
	int _TDCtac_sw[6];
	int _lvlshft_sw[6];
	int _vib2_sw[6];
	int _vib1_sw[6];
	int _sipm_iDAC_range_sw[6];
	int _sipm_iDAC_dcstart_sw[6];
	int _disc_iDAC_range_sw[6];
	int _disc_iDAC_dcstart_sw[6];
	int _postamp_sw[6];
	int _FE_discibias_sw[6];
	int _FE_discvcas_sw[6];
	int _clk_o_enable[6];
	int _test_pattern[6];
	int _external_veto[6];
	int _event_data_mode[6];
	int _counter_interval[6];
	int _count_trig_error[6];
	int _fine_counter_kf[6];
	int _fine_counter_saturate[6];
	int _TAC_refresh[6];
	int _external_pulse_enable[6];
	int _DDR_mode[6];
	int _TX_mode[6];
	int _calib_sw[6];
	int _global_calib_enable[6];
	int _interval_between_pulses[6];
	int _pulse_length[6];
	int _number_of_pulses[6];

	int _de[6][64];
	int _test_cfg[6][64];
	int _test_T_EN[6][64];
	int _test_E_EN[6][64];
	int _sync[6][64];
	int _gtest[6][64];
	int _deadtime_EN[6][64];
	int _deadtime[6][64];
	int _meta_cfg[6][64];
	int _latch_cfg[6][64];
	int _d[6][64];
	int _cgate_cfg[6][64];
	int _g0_2_bar[6][64];
	int _g0_4_bar[6][64];
	int _sh_5n_bar[6][64];
	int _sh_2n5_bar[6][64];
	int _sw_E[6][64];
	int _sw[6][64];
	int _vbl_sw[6][64];
	int _inp_pol[6][64];
	int _test_mode[6][64];
	int _tmjitter[6][64];
	int _praedictio[6][64];
	int _ch_enable[6][64];
	int _ch_calib_enable[6][64];

	int _n1, _n2;
	int _test_pulse_coarse_delay, _test_pulse_fine_delay, _test_pulse_zero_width, _test_pulse_duration, _test_pulse_polarity;

	bool _chip_is_present[6];
	bool _channel_enabled[6][64];

	unsigned char _g_config_vector[G_CONFIG_V_SIZE];
	unsigned char _t_config_vector[T_CONFIG_V_SIZE];
	unsigned char _t_command_vector[T_COMMAND_V_SIZE];
	unsigned char _ch_t_config_vector[CH_T_CONFIG_V_SIZE];
	unsigned char _ch_config_vector[CH_CONFIG_V_SIZE];
	unsigned int _fifo_data[FIFO_DATA_SIZE];
	unsigned int _command;
	unsigned int _FpgaStatusRegister;
	unsigned int _FifoStatusRegister;
	unsigned int _FifoWordCount10;
	unsigned int _FifoWordCount32;
	unsigned int _FifoWordCount54;

	void _PrepareGlobalConfigVector(int chip, int rw);
	void _PrepareTestConfigVector(int chip, int rw);
	void _PrepareTestCommandVector(int chip);
	void _PrepareChannelConfigVector(int chip, int rw, int ch);
	void _PrepareChannelTestConfigVector(int rw, int ch, int enable);
	unsigned char _crc8(unsigned char *data, int nbits);
	void _DoConfig(int chip);
	void _RdConfig(int chip);

};

extern TofPetAsic *Asic;
