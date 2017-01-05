/* Configure.cpp
 * Uses libconfig
*/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <string>

#include <stdint.h>
#include <getopt.h>
#include <string.h>

#include <libconfig.h++>

using namespace std;
using namespace libconfig;

#include "Configure.h"
#include "DaqControl.h"
#include "TofPetAsic.h"

Config cfg;

DaqConfig::DaqConfig()
{
}

void DaqConfig::Save(char *outfile)
{
  cfg.writeFile(outfile);
}

int DaqConfig::Read(char *configfile)
{

	try
	{
		cfg.readFile(configfile);
	}
	catch(const FileIOException &fioex)
	{
		std::cerr << "I/O error while reading file." << std::endl;
		return(EXIT_FAILURE);
	}
	catch(const ParseException &pex)
	{
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
			<< " - " << pex.getError() << std::endl;
		return(EXIT_FAILURE);
	}

	GetDefaultParameters();
	GetIndividualParameters();
	GetRunParameters();

	return 0;

}

void DaqConfig::GetDefaultParameters(void)
{
	int TDCcomp_sw, compvcas_sw, TDCiref_sw, TDCtac_sw, lvlshft_sw, vib2_sw, vib1_sw, sipm_iDAC_range_sw,
	sipm_iDAC_dcstart_sw, disc_iDAC_range_sw, disc_iDAC_dcstart_sw, postamp_sw, FE_discibias_sw, FE_discvcas_sw;
	bool clk_o_enable, test_pattern, external_veto, event_data_mode, external_pulse_enable,
	DDR_mode, global_calib_enable;
	int counter_interval, count_trig_error, fine_counter_kf, fine_counter_saturate, TAC_refresh, TX_mode,
	calib_sw, interval_between_pulses, pulse_length, number_of_pulses;
	int de, test_cfg;
	bool test_T_EN, test_E_EN, sync, gtest, deadtime_EN, test_mode, praedictio, ch_enable, ch_calib_enable;
	int deadtime, meta_cfg, latch_cfg,
	d, cgate_cfg, g0_2_bar, g0_4_bar, sh_5n_bar, sh_2n5_bar, sw_E, sw, vbl_sw, inp_pol, g0_bar, sh_bar,
	tmjitter;
	unsigned int ch_31_0_enable_mask, ch_63_32_enable_mask;

	const Setting& root = cfg.getRoot();

	try
	{
		const Setting &TofPetDefault = root["default"]["TofPet"][0];
		if(!(TofPetDefault.lookupValue("ch_31_0_enable_mask", ch_31_0_enable_mask))) ch_31_0_enable_mask = 0;
		if(!(TofPetDefault.lookupValue("ch_63_32_enable_mask", ch_63_32_enable_mask))) ch_63_32_enable_mask = 0;
	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Default Channel Enable Mask catch reached\n");
		// Ignore.
	}
	try
	{
		const Setting &TofPetGlobalDefault = root["default"]["TofPet"][0]["global"];
		if(!(TofPetGlobalDefault.lookupValue("TDCcomp", TDCcomp_sw))) TDCcomp_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("compvcas", compvcas_sw))) compvcas_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("TDCiref", TDCiref_sw))) TDCiref_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("TDCtac", TDCtac_sw))) TDCtac_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("lvlshft", lvlshft_sw))) lvlshft_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("vib2", vib2_sw))) vib2_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("vib1", vib1_sw))) vib1_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("sipm_iDAC_range", sipm_iDAC_range_sw))) sipm_iDAC_range_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("sipm_iDAC_dcstart", sipm_iDAC_dcstart_sw))) sipm_iDAC_dcstart_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("disc_iDAC_range", disc_iDAC_range_sw))) disc_iDAC_range_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("disc_iDAC_dcstart", disc_iDAC_dcstart_sw))) disc_iDAC_dcstart_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("postamp", postamp_sw))) postamp_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("FE_discibias", FE_discibias_sw))) FE_discibias_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("FE_discvcas", FE_discvcas_sw))) FE_discvcas_sw = 0;
		if(!(TofPetGlobalDefault.lookupValue("clk_o_enable", clk_o_enable))) clk_o_enable = false;
		if(!(TofPetGlobalDefault.lookupValue("test_pattern", test_pattern))) test_pattern = false;
		if(!(TofPetGlobalDefault.lookupValue("external_veto", external_veto))) external_veto = false;
		if(!(TofPetGlobalDefault.lookupValue("event_data_mode", event_data_mode))) event_data_mode = false;
		if(!(TofPetGlobalDefault.lookupValue("counter_interval", counter_interval))) counter_interval = 0;
		if(!(TofPetGlobalDefault.lookupValue("count_trig_error", count_trig_error))) count_trig_error = 0;
		if(!(TofPetGlobalDefault.lookupValue("fine_counter_kf", fine_counter_kf))) fine_counter_kf = 0;
		if(!(TofPetGlobalDefault.lookupValue("fine_counter_saturate", fine_counter_saturate))) fine_counter_saturate = 0;
		if(!(TofPetGlobalDefault.lookupValue("TAC_refresh", TAC_refresh))) TAC_refresh = 0;
		if(!(TofPetGlobalDefault.lookupValue("external_pulse_enable", external_pulse_enable))) external_pulse_enable = false;
		if(!(TofPetGlobalDefault.lookupValue("DDR_mode", DDR_mode))) DDR_mode = false;
		if(!(TofPetGlobalDefault.lookupValue("TX_mode", TX_mode))) TX_mode = 0;
	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Default Global Parameters catch reached\n");
		// Ignore.
	}

	try
	{
		const Setting &t_param = root["default"]["TofPet"][0]["global_test"];
		if(!(t_param.lookupValue("calib_sw", calib_sw))) calib_sw = 0;
		if(!(t_param.lookupValue("global_calib_enable", global_calib_enable))) global_calib_enable = false;
	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Default Calib Enable catch reached\n");
		// Ignore.
	}

	try
	{
		const Setting &tp_param = root["default"]["TofPet"][0]["test_pulse"];
		if(!(tp_param.lookupValue("interval_between_pulses", interval_between_pulses))) interval_between_pulses = 0;
		if(!(tp_param.lookupValue("pulse_length", pulse_length))) pulse_length = 0;
		if(!(tp_param.lookupValue("number_of_pulses", number_of_pulses))) number_of_pulses = 0;

	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Default Test Pulse catch reached\n");
		// Ignore.
	}

	try
	{
		const Setting &d_param = root["default"]["TofPet"][0]["channel"][0];
		if(!(d_param.lookupValue("d_E", de))) de = 0;
		if(!(d_param.lookupValue("test_cfg", test_cfg))) test_cfg = 0;
		if(!(d_param.lookupValue("test_T_EN", test_T_EN))) test_T_EN = false;
		if(!(d_param.lookupValue("test_E_EN", test_E_EN))) test_E_EN = false;
		if(!(d_param.lookupValue("sync", sync))) sync = false;
		if(!(d_param.lookupValue("gtest", gtest))) gtest = false;
		if(!(d_param.lookupValue("deadtime_EN", deadtime_EN))) deadtime_EN = false;
		if(!(d_param.lookupValue("deadtime", deadtime))) deadtime = 0;
		if(!(d_param.lookupValue("meta_cfg", meta_cfg))) meta_cfg = 0;
		if(!(d_param.lookupValue("latch_cfg", latch_cfg))) latch_cfg = 0;
		if(!(d_param.lookupValue("d_T", d))) d = 0;
		if(!(d_param.lookupValue("cgate_cfg", cgate_cfg))) cgate_cfg = 0;
		if(!(d_param.lookupValue("g0_bar", g0_bar))) g0_bar = 0;
			g0_4_bar = g0_bar & 0x01; g0_2_bar = (g0_bar & 0x02) >> 1;
		if(!(d_param.lookupValue("sh_bar", sh_bar))) sh_bar = 0;
			sh_2n5_bar = sh_bar & 0x01; sh_5n_bar = (sh_bar & 0x02) >> 1;
		if(!(d_param.lookupValue("sw_E", sw_E))) sw_E = 0;
		if(!(d_param.lookupValue("sw_T", sw))) sw = 0;
		if(!(d_param.lookupValue("vbl", vbl_sw))) vbl_sw = 0;
		if(!(d_param.lookupValue("inp_pol", inp_pol))) inp_pol = 0;
		if(!(d_param.lookupValue("test_mode", test_mode))) test_mode = false;
		if(!(d_param.lookupValue("tmjitter", tmjitter))) tmjitter = 0;
		if(!(d_param.lookupValue("praedictio", praedictio))) praedictio = false;
		if(!(d_param.lookupValue("ch_enable", ch_enable))) ch_enable = false;
		if(!(d_param.lookupValue("ch_calib_enable", ch_calib_enable))) ch_calib_enable = false;
	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Default Channel Parameters catch reached\n");
		// Ignore.
	}

	for(int j=0; j<6; j++)
	{
		Asic->Set_TDCcomp_sw(j, TDCcomp_sw);
		Asic->Set_compvcas_sw(j, compvcas_sw);
		Asic->Set_TDCiref_sw(j, TDCiref_sw);
		Asic->Set_TDCtac_sw(j, TDCtac_sw);
		Asic->Set_lvlshft_sw(j, lvlshft_sw);
		Asic->Set_vib2_sw(j, vib2_sw);
		Asic->Set_vib1_sw(j, vib1_sw);
		Asic->Set_sipm_iDAC_range_sw(j, sipm_iDAC_range_sw);
		Asic->Set_sipm_iDAC_dcstart_sw(j, sipm_iDAC_dcstart_sw);
		Asic->Set_disc_iDAC_range_sw(j, disc_iDAC_range_sw);
		Asic->Set_disc_iDAC_dcstart_sw(j, disc_iDAC_dcstart_sw);
		Asic->Set_postamp_sw(j, postamp_sw);
		Asic->Set_FE_discibias_sw(j, FE_discibias_sw);
		Asic->Set_FE_discvcas_sw(j, FE_discvcas_sw);
		Asic->Set_clk_o_enable(j, clk_o_enable ? 1 : 0);
		Asic->Set_test_pattern(j, test_pattern ? 1 : 0);
		Asic->Set_external_veto(j, external_veto ? 1 : 0);
		Asic->Set_event_data_mode(j, event_data_mode ? 1 : 0);
		Asic->Set_counter_interval(j, counter_interval);
		Asic->Set_count_trig_error(j, count_trig_error);
		Asic->Set_fine_counter_kf(j, fine_counter_kf);
		Asic->Set_fine_counter_saturate(j, fine_counter_saturate);
		Asic->Set_TAC_refresh(j, TAC_refresh);
		Asic->Set_external_pulse_enable(j, external_pulse_enable ? 1 : 0);
		Asic->Set_DDR_mode(j, DDR_mode ? 1 : 0);
		Asic->Set_TX_mode(j, TX_mode);
		Asic->Set_calib_sw(j, calib_sw);
		Asic->Set_global_calib_enable(j, global_calib_enable ? 1 : 0);
		Asic->Set_interval_between_pulses(j, interval_between_pulses);
		Asic->Set_pulse_length(j, pulse_length);
		Asic->Set_number_of_pulses(j, number_of_pulses);

		for(int i=0; i<64; i++)
		{
			Asic->Set_de(j, i, de);
			Asic->Set_test_cfg(j, i, test_cfg);
			Asic->Set_test_T_EN(j, i, test_T_EN ? 1 : 0);
			Asic->Set_test_E_EN(j, i, test_E_EN ? 1 : 0);
			Asic->Set_sync(j, i, sync ? 1 : 0);
			Asic->Set_gtest(j, i, gtest ? 1 : 0);
			Asic->Set_deadtime_EN(j, i, deadtime_EN ? 1 : 0);
			Asic->Set_deadtime(j, i, deadtime);
			Asic->Set_meta_cfg(j, i, meta_cfg);
			Asic->Set_latch_cfg(j, i, latch_cfg);
			Asic->Set_d(j, i, d);
			Asic->Set_cgate_cfg(j, i, cgate_cfg);
			Asic->Set_g0_2_bar(j, i, g0_2_bar);
			Asic->Set_g0_4_bar(j, i, g0_4_bar);
			Asic->Set_sh_5n_bar(j, i, sh_5n_bar);
			Asic->Set_sh_2n5_bar(j, i, sh_2n5_bar);
			Asic->Set_sw_E(j, i, sw_E);
			Asic->Set_sw(j, i, sw);
			Asic->Set_vbl_sw(j, i, vbl_sw);
			Asic->Set_inp_pol(j, i, inp_pol);
			Asic->Set_test_mode(j, i, test_mode ? 1 : 0);
			Asic->Set_tmjitter(j, i, tmjitter);
			Asic->Set_praedictio(j, i, praedictio ? 1 : 0);
//			Asic->Set_ch_enable(j, i, ch_enable ? 1 : 0);
			Asic->Set_ch_enable(j, i, 1);
			Asic->Set_ch_calib_enable(j, i, ch_calib_enable ? 1 : 0);
			if( i < 32 )
			{
				if( ch_31_0_enable_mask & (1<<i) )
					Asic->Set_ChannelEnabled(j, i);
				else
					Asic->Set_ChannelDisabled(j, i);
			}
			else
			{
				if( ch_63_32_enable_mask & (1<<(i-32)) )
					Asic->Set_ChannelEnabled(j, i);
				else
					Asic->Set_ChannelDisabled(j, i);
			}
		}
	}
}

void DaqConfig::GetIndividualParameters(void)
{
	int i_val, g0_2_bar, g0_4_bar, sh_5n_bar, sh_2n5_bar;
	bool b_val;
	unsigned long long ch_31_0_enable_mask, ch_63_32_enable_mask;
	int TofPetCount = 0;
	int ChCount = 0;
	int i, j;

	const Setting& root = cfg.getRoot();

	try
	{
		const Setting &TofPet = root["TofPet"];
		TofPetCount = TofPet.getLength();
		for(j=0; j<TofPetCount; j++)
		{
			const Setting &tp = root["TofPet"][j];
			ch_31_0_enable_mask = 0; ch_63_32_enable_mask = 0;
			if((tp.lookupValue("ch_31_0_enable_mask", ch_31_0_enable_mask)))
				for(i=0; i<32; i++)
				{
					if( ch_31_0_enable_mask & (1<<i) )
						Asic->Set_ChannelEnabled(j, i);
					else
						Asic->Set_ChannelDisabled(j, i);
				}
			if((tp.lookupValue("ch_63_32_enable_mask", ch_63_32_enable_mask)))
				for(int i=32; i<64; i++)
				{
					if( ch_63_32_enable_mask & (1<<(i-32)) )
						Asic->Set_ChannelEnabled(j, i);
					else
						Asic->Set_ChannelDisabled(j, i);
				}
		}
	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Individual Channel Enable Mask catch reached, j = %d\n", j);
		// Ignore.
	}
	
	try
	{
		for(j=0; j<TofPetCount; j++)
		{
			const Setting &TofPetGlobal = root["TofPet"][j]["global"];
			if( (TofPetGlobal.lookupValue("TDCcomp", i_val))) Asic->Set_TDCcomp_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("compvcas", i_val))) Asic->Set_compvcas_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("TDCiref", i_val))) Asic->Set_TDCiref_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("TDCtac", i_val))) Asic->Set_TDCtac_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("lvlshft", i_val))) Asic->Set_lvlshft_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("vib2", i_val))) Asic->Set_vib2_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("vib1", i_val))) Asic->Set_vib1_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("sipm_iDAC_range", i_val))) Asic->Set_sipm_iDAC_range_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("sipm_iDAC_dcstart", i_val))) Asic->Set_sipm_iDAC_dcstart_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("disc_iDAC_range", i_val))) Asic->Set_disc_iDAC_range_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("disc_iDAC_dcstart", i_val))) Asic->Set_disc_iDAC_dcstart_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("postamp", i_val))) Asic->Set_postamp_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("FE_discibias", i_val))) Asic->Set_FE_discibias_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("FE_discvcas", i_val))) Asic->Set_FE_discvcas_sw(j, i_val);
			if( (TofPetGlobal.lookupValue("clk_o_enable", b_val))) Asic->Set_clk_o_enable(j, b_val ? 1 : 0);
			if( (TofPetGlobal.lookupValue("test_pattern", b_val))) Asic->Set_test_pattern(j, b_val ? 1 : 0);
			if( (TofPetGlobal.lookupValue("external_veto", b_val))) Asic->Set_external_veto(j, b_val ? 1 : 0);
			if( (TofPetGlobal.lookupValue("event_data_mode", b_val))) Asic->Set_event_data_mode(j, b_val ? 1 : 0);
			if( (TofPetGlobal.lookupValue("counter_interval", i_val))) Asic->Set_counter_interval(j, i_val);
			if( (TofPetGlobal.lookupValue("count_trig_error", i_val))) Asic->Set_count_trig_error(j, i_val);
			if( (TofPetGlobal.lookupValue("fine_counter_kf", i_val))) Asic->Set_fine_counter_kf(j, i_val);
			if( (TofPetGlobal.lookupValue("fine_counter_saturate", i_val))) Asic->Set_fine_counter_saturate(j, i_val);
			if( (TofPetGlobal.lookupValue("TAC_refresh", i_val))) Asic->Set_TAC_refresh(j, i_val);
			if( (TofPetGlobal.lookupValue("external_pulse_enable", b_val))) Asic->Set_external_pulse_enable(j, b_val ? 1 : 0);
			if( (TofPetGlobal.lookupValue("DDR_mode", b_val))) Asic->Set_DDR_mode(j, b_val ? 1 : 0);
			if( (TofPetGlobal.lookupValue("TX_mode", i_val))) Asic->Set_TX_mode(j, i_val);
		}
	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Individual Global Parameters catch reached, j = %d\n", j);
		// Ignore.
	}

	try
	{
		for(j=0; j<TofPetCount; j++)
		{
			const Setting &t_param = root["TofPet"][j]["global_test"];
			if((t_param.lookupValue("calib_sw", i_val))) Asic->Set_calib_sw(j, i_val);
			if((t_param.lookupValue("global_calib_enable", b_val))) Asic->Set_global_calib_enable(j, b_val ? 1 : 0);
		}
	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Individual Global Test catch reached, j = %d\n", j);
		// Ignore.
	}

	try
	{
		for(j=0; j<TofPetCount; j++)
		{
			const Setting &tp_param = root["TofPet"][j]["test_pulse"];
			if((tp_param.lookupValue("interval_between_pulses", i_val))) Asic->Set_interval_between_pulses(j, i_val);
			if((tp_param.lookupValue("pulse_length", i_val))) Asic->Set_pulse_length(j, i_val);
			if((tp_param.lookupValue("number_of_pulses", i_val))) Asic->Set_number_of_pulses(j, i_val);
		}

	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Individual Test Pulse catch reached, j = %d\n", j);
		// Ignore.
	}

	try
	{
		for(j=0; j<TofPetCount; j++)
		{
			const Setting &d_param = root["TofPet"][j]["channel"];
			ChCount = d_param.getLength();
			for(i=0; i<ChCount; i++)
			{
				const Setting &ch_param = d_param[i];
				if((ch_param.lookupValue("d_E", i_val))) Asic->Set_de(j, i, i_val);
				if((ch_param.lookupValue("test_cfg", i_val))) Asic->Set_test_cfg(j, i, i_val);
				if((ch_param.lookupValue("test_T_EN", b_val))) Asic->Set_test_T_EN(j, i, b_val ? 1 : 0);
				if((ch_param.lookupValue("test_E_EN", b_val))) Asic->Set_test_E_EN(j, i, b_val ? 1 : 0);
				if((ch_param.lookupValue("sync", b_val))) Asic->Set_sync(j, i, b_val ? 1 : 0);
				if((ch_param.lookupValue("gtest", b_val))) Asic->Set_gtest(j, i, b_val ? 1 : 0);
				if((ch_param.lookupValue("deadtime_EN", b_val))) Asic->Set_deadtime_EN(j, i, b_val ? 1 : 0);
				if((ch_param.lookupValue("deadtime", i_val))) Asic->Set_deadtime(j, i, i_val);
				if((ch_param.lookupValue("meta_cfg", i_val))) Asic->Set_meta_cfg(j, i, i_val);
				if((ch_param.lookupValue("latch_cfg", i_val))) Asic->Set_latch_cfg(j, i, i_val);
				if((ch_param.lookupValue("d_T", i_val))) Asic->Set_d(j, i, i_val);
				if((ch_param.lookupValue("cgate_cfg", i_val))) Asic->Set_cgate_cfg(j, i, i_val);
				if((ch_param.lookupValue("g0_bar", i_val)))
				{
					g0_4_bar = i_val & 0x01; g0_2_bar = (i_val& 0x02) >> 1;
					Asic->Set_g0_2_bar(j, i, g0_2_bar); Asic->Set_g0_4_bar(j, i, g0_4_bar);
				}
				if((ch_param.lookupValue("sh_bar", i_val)))
				{
					sh_2n5_bar = i_val & 0x01; sh_5n_bar = (i_val & 0x02) >> 1;
					Asic->Set_sh_5n_bar(j, i, sh_5n_bar); Asic->Set_sh_2n5_bar(j, i, sh_2n5_bar);
				}
				if((ch_param.lookupValue("sw_E", i_val))) Asic->Set_sw_E(j, i, i_val);
				if((ch_param.lookupValue("sw_T", i_val))) Asic->Set_sw(j, i, i_val);
				if((ch_param.lookupValue("vbl", i_val))) Asic->Set_vbl_sw(j, i, i_val);
				if((ch_param.lookupValue("inp_pol", i_val))) Asic->Set_inp_pol(j, i, i_val);
				if((ch_param.lookupValue("test_mode", b_val))) Asic->Set_test_mode(j, i, b_val ? 1 : 0);
				if((ch_param.lookupValue("tmjitter", i_val))) Asic->Set_tmjitter(j, i, i_val);
				if((ch_param.lookupValue("praedictio", b_val))) Asic->Set_praedictio(j, i, b_val ? 1 : 0);
//				if((ch_param.lookupValue("ch_enable", b_val))) Asic->Set_ch_enable(j, i, b_val ? 1 : 0);
				Asic->Set_ch_enable(j, i, 1);
				if((ch_param.lookupValue("ch_calib_enable", b_val))) Asic->Set_ch_calib_enable(j, i, b_val ? 1 : 0);
			}
		}
	}
	catch(const SettingNotFoundException &nfex)
	{
		printf("Individual Single Channel Parameters catch reached, i = %d, j = %d\n", i, j);
		// Ignore.
	}
}

void DaqConfig::GetRunParameters(void)
{
	int time_p, event_p, prog_step;
	int test_pulse_coarse_delay, test_pulse_fine_delay, test_pulse_zero_width, test_pulse_duration, test_pulse_polarity;
	bool hw_config_only;
	bool skip_empty;
	string fmt, mode, net, fname;
	unsigned int chip_present_mask;

	const Setting& root = cfg.getRoot();
	try
	{
		const Setting &r_param = root["Run"];
		if(!(r_param.lookupValue("chip_present_mask", chip_present_mask))) chip_present_mask = 0;
		if(!(r_param.lookupValue("hw_config_only", hw_config_only))) hw_config_only = false;
		if(!(r_param.lookupValue("time_preset", time_p))) time_p = 0;
		if(!(r_param.lookupValue("event_preset", event_p))) event_p = 0;
		if(!(r_param.lookupValue("progress_step", prog_step))) prog_step = 0;
		if(!(r_param.lookupValue("skip_empty_events", skip_empty))) skip_empty = false;
		if(!(r_param.lookupValue("daq_format", fmt))) fmt = "hex";
		if(!(r_param.lookupValue("daq_mode", mode))) mode = "event";
		if(!(r_param.lookupValue("file_prefix", fname))) fname = "NoName";
		if(!(r_param.lookupValue("inet_addr", net))) net = "127.0.0.1";
		if(!(r_param.lookupValue("test_pulse_coarse_delay", test_pulse_coarse_delay))) test_pulse_coarse_delay = 0;
		if(!(r_param.lookupValue("test_pulse_fine_delay", test_pulse_fine_delay))) test_pulse_fine_delay = 0;
		if(!(r_param.lookupValue("test_pulse_zero_width", test_pulse_zero_width))) test_pulse_zero_width = 0;
		if(!(r_param.lookupValue("test_pulse_duration", test_pulse_duration))) test_pulse_duration = 0;
		if(!(r_param.lookupValue("test_pulse_polarity", test_pulse_polarity))) test_pulse_polarity = 0;
	}
	catch(const SettingNotFoundException &nfex)
	{
		// Ignore.
	}

	Asic->Set_TestPulseCoarseDelay(test_pulse_coarse_delay);
	Asic->Set_TestPulseFineDelay(test_pulse_fine_delay);
	Asic->Set_TestPulseZeroWidth(test_pulse_zero_width);
	Asic->Set_TestPulseDuration(test_pulse_duration);
	Asic->Set_TestPulsePolarity(test_pulse_polarity);
	for(int i=0; i<6; i++)
		if( chip_present_mask & (1<<i) )
			Asic->Set_ChipPresent(i);
	if( fmt == "hex" )
		DaqCtrl.SetDaqFormat(DAQ_HEX);
	if( fmt == "dec" )
		DaqCtrl.SetDaqFormat(DAQ_DEC);
	if( fmt == "bin" )
		DaqCtrl.SetDaqFormat(DAQ_BIN);
	if( mode == "event" )
		DaqCtrl.SetDaqMode(MODE_EVENT);
	if( mode == "hw_fetp" )
		DaqCtrl.SetDaqMode(MODE_HW_FETP);
	if( mode == "sw_fetp" )
		DaqCtrl.SetDaqMode(MODE_SW_FETP);
	if( mode == "hw_tdca" )
		DaqCtrl.SetDaqMode(MODE_HW_TDCA);
	if( mode == "sw_tdca" )
		DaqCtrl.SetDaqMode(MODE_SW_TDCA);
	if( mode == "dark_count" )
		DaqCtrl.SetDaqMode(MODE_DARK_COUNT);

	DaqCtrl.SetHwConfigOnly(hw_config_only);
	DaqCtrl.SetTimePreset(time_p);
	DaqCtrl.SetEventPreset(event_p);
	DaqCtrl.SetProgressStep(prog_step);
	DaqCtrl.SetSkipEmptyEvents(skip_empty);
	DaqCtrl.SetFilePrefix(fname);
	DaqCtrl.SetInetAddr(net);
}

int DaqConfig::ParseInlineParam(int argc, char *argv[]) {

	int i, run_num;
	char fn_cnf[256], add[256];

	sprintf(fn_cnf,"config.txt");
	sprintf(add,"193.206.144.243");

	run_num = DaqCtrl.readRunNum();;

	for (i=1;i<argc; i++) {

	if (strcmp("-cfg", argv[i]) == 0)
	{
		i++;
		sprintf(fn_cnf,"%s",argv[i]);
	}

	if (strcmp("-inet", argv[i]) == 0)
	{
		i++;
		sprintf(add,"%s",argv[i]);
	}

	if (strcmp("-run", argv[i]) == 0)
	{
		i++;
		sscanf(argv[i],"%d",&run_num);
	}

	if (strcmp("-h", argv[i]) == 0)
	{
		i++;
		printf(" Command line options :\n");
		printf("  -cfg prefix    : config parameter file name [%s]\n", fn_cnf);
		printf("  -inet addr     : internet address v4 [%s]\n", add);
		printf("  -run number    : run number [%d]\n", run_num);
		printf("  -h             : this help\n");
		return -1;
	}
  }

  DaqCtrl.SetConfigFileName(string(fn_cnf));
  DaqCtrl.SetInetAddr(string(add));
  DaqCtrl.SetRunNumber(run_num);

//  printf("config_file = %s, inet_addr = %s, run-num = %d\n", fn_cnf, add, run_num);

  return 0;

}
