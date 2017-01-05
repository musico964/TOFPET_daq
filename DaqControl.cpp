/* DaqControl.cpp
*/
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

using namespace std;

#include "TofPetAsic.h"
#include "DaqControl.h"

#define DAQ_RUN_FILE "last.run"


DaqControl::DaqControl()
{
/*
	_t_thr_start = _t_thr_stop = _t_thr_step = _t_thr_loop = 0;
	_e_thr_start = _e_thr_stop = _e_thr_step = _e_thr_loop = 0;
*/
}

void DaqControl::Init(void)
{
/*
	if( !single_scan )
	{
		_t_thr_loop = _t_thr_start;
		_e_thr_loop = _e_thr_start;
	}
	else
	{
		_t_thr_loop = _t_thr_start;
		_e_thr_loop = _t_thr_start;
	}
*/
}

void DaqControl::Start(void)
{
	event_loop = 0;

	time_real = 0;
	progress_old = 0;

	time(&time_acqstart);
}

void DaqControl::printSummary()
{
	char logstring[256];

	sprintf(logstring, "===================================");
	cout << logstring << endl;
	sprintf(logstring, "Acq File Prefix: %s", file_prefix.data());
	cout << logstring << endl;
	sprintf(logstring, "Presets: time = %d (s), events = %d", time_preset, event_preset);
	cout << logstring << endl;
/*
    sprintf(logstring, "Time Thr (Start/Stop/Step) = %d/%d/%d", _t_thr_start, _t_thr_stop, _t_thr_step);
	cout << logstring << endl;
    sprintf(logstring, "Energy Thr (Start/Stop/Step) = %d/%d/%d", _e_thr_start, _e_thr_stop, _e_thr_step);
	cout << logstring << endl;
*/
}

void DaqControl::printCurrent()
{
/*
	char logstring[256];

	sprintf(logstring, "Start RUN with t_thr = %d, e_thr = %d", _t_thr_loop, _e_thr_loop);
	cout << logstring << endl;
*/
}

void DaqControl::printEventSummary(void)
{
	char logstring[256];

	sprintf(logstring,"# Events = %d\n# Time_Real_sec = %d\n# Events/Real_sec = %f\n",
		event_loop, (int)time_real, ((float) event_loop)/time_real);
	cout << logstring << endl;
}

void DaqControl::progressBar(void)
{

	int progress;
	int progress_time, progress_evt;

	progress_time = (((float) time_real)/time_preset)*progress_step;
	progress_evt = (((float) event_loop)/event_preset)*progress_step;
	progress = progress_time > progress_evt ? progress_time : progress_evt;

	progress = progress < progress_step ? progress : progress_step;

	for (int jprog = 0; jprog<(progress - progress_old); jprog++)
	{
		if (progress_time > progress_evt)
		{
			printf("T");
		}
		else
		{
			printf("E");
		}
		fflush(stdout);
	}

	if (progress == progress_step)
		printf("\n");

	progress_old = progress;
}

void DaqControl::incEvt(long int evt)
{

	event_loop += evt;

}

void DaqControl::incEvt()
{
	incEvt(1);
}

bool DaqControl::acqRunning()
{

	time_t temp;

	time(&temp);
	time_real = temp - time_acqstart;

	progressBar();
	if (event_loop >= event_preset)
		return false;

	if (time_real >= time_preset)
		return false;

	return true;
}

// handle run numbers

int DaqControl::readRunNum()
{
	ifstream ff;
	int rr;

	ff.open(DAQ_RUN_FILE);
	if( ff.good() )
	{
		ff >> rr;	// fscanf(ff,"%d",&rr);
		ff.close();
	}
	else
	{
		rr = -1;
	}
	return rr;
}

void DaqControl::writeRunNum(int rr)
{
	ofstream ff;
	ff.open(DAQ_RUN_FILE);
	if( ff.good() )
	{
		ff << rr;	// fprintf(ff,"%d",rr);
		ff.close();
	}
	else
	{
		cerr << "WARNING: cannot update run file " << DAQ_RUN_FILE << endl;
	}
}

bool DaqControl::UpdateRunParameters()
{
/*
	if( !single_scan )
	{
		_t_thr_loop += _t_thr_step;
		if (_t_thr_loop > _t_thr_stop)
		{
			_t_thr_loop = _t_thr_start;
			_e_thr_loop += _e_thr_step;
			if (_e_thr_loop > _e_thr_stop)
				return false;
		}
		return true;
	}
	else
	{
		_t_thr_loop += _t_thr_step;
		_e_thr_loop = _t_thr_loop;
		if (_t_thr_loop > _t_thr_stop)
				return false;
		return true;
	}
*/
	return false;
}

bool DaqControl::openOutFile(void)
{
	char mode[3] = "wb";

	if( daq_format == DAQ_BIN )
	{
		sprintf(output_file_name, "%s_%04d.bin", file_prefix.data(), run_num);
	}
	else
	{
		sprintf(output_file_name, "%s_%04d.txt", file_prefix.data(), run_num);
		mode[1] = 0;
	}

	if ( (fout=fopen(output_file_name, mode)) == NULL )
	{
		cout << "ERROR: Cannot open output file " << output_file_name << endl;
		return false;
	}
	cout << "Writing data on: '" << output_file_name << "'" << endl;
	return true;
}

void DaqControl::closeOutFile(void)
{
    fclose(fout);
}

