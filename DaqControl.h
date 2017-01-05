/* DaqControl.h
*/

#define DAQ_HEX	1
#define DAQ_DEC	2
#define DAQ_BIN	3

#define MODE_HW_FETP	0x01
#define MODE_SW_FETP	0x81
#define MODE_HW_TDCA	0x02
#define MODE_SW_TDCA	0x82
#define MODE_EVENT	0x04
#define MODE_DARK_COUNT	0x05

class DaqControl
{
	public:
		DaqControl();
		~DaqControl() { };

		void SetConfigFileName(string f) {config_file_name = f; };
		void SetInetAddr(string f) {inet_addr = f; };
		void SetDaqMode(int m) { daq_mode = m; }
		void SetDaqFormat(int f) { daq_format = f; }
		void SetProgressStep(int s) { progress_step = s; }
		void SetTimePreset(int t) { time_preset = t; }
		void SetEventPreset(int e) { event_preset = e; }
		void SetFilePrefix(string f) { file_prefix = f; }
		void SetHwConfigOnly(bool n) { hw_config_only = n; }
		void SetSkipEmptyEvents(bool n) { skip_empty_events = n; }

		char *GetConfigFileName(void) { return (char*)config_file_name.data(); };
		char *GetInetAddr(void) { return (char*)inet_addr.data(); };
		int GetDaqMode(void) { return daq_mode; }
		int GetDaqFormat(void) { return daq_format; }
		int GetProgressStep(void) { return progress_step; }
		int GetTimePreset(void) { return time_preset; }
		int GetEventPreset(void) { return event_preset; }
		int GetEventNumber(void) { return event_loop; }
		string GetFilePrefix(void) { return file_prefix; }
		bool GetHwConfigOnly(void) { return hw_config_only; }
		bool GetSkipEmptyEvents(void) { return skip_empty_events; }
		void Start(void);
		void Init(void);
		void incEvt(long int evt);
		void incEvt();
		bool acqRunning(void);
		void progressBar(void);
		void printSummary(void);
		void printEventSummary(void);
		void printCurrent(void);

		void SetRunNumber(int l) { run_num = l; writeRunNum(run_num+1); }
		void SetRunNumber() { run_num = readRunNum(); writeRunNum(run_num+1); }
		int GetRunNumber(void) { return run_num; }
		int readRunNum();
/*
		void SetTimeThresholdStart(int l) { _t_thr_start = l; }
		void SetTimeThresholdStop(int l) { _t_thr_stop = l; }
		void SetTimeThresholdStep(int l) { _t_thr_step = l; }
		void SetTimeThresholdLoop(int l) { _t_thr_loop = l; }
		int GetTimeThresholdStart(void) { return _t_thr_start; }
		int GetTimeThresholdStop(void) { return _t_thr_stop; }
		int GetTimeThresholdStep(void) { return _t_thr_step; }
		int GetTimeThresholdLoop(void) { return _t_thr_loop; }
		void SetEnergyThresholdStart(int l) { _e_thr_start = l; }
		void SetEnergyThresholdStop(int l) { _e_thr_stop = l; }
		void SetEnergyThresholdStep(int l) { _e_thr_step = l; }
		void SetEnergyThresholdLoop(int l) { _e_thr_loop = l; }
		int GetEnergyThresholdStart(void) { return _e_thr_start; }
		int GetEnergyThresholdStop(void) { return _e_thr_stop; }
		int GetEnergyThresholdStep(void) { return _e_thr_step; }
		int GetEnergyThresholdLoop(void) { return _e_thr_loop; }
*/
		char *GetOutputFileName(void) { return output_file_name; }

        bool UpdateRunParameters(void);

        bool openOutFile(void);
        void closeOutFile(void);
        FILE *OutFile(void) {return fout; }

private:

		void writeRunNum(int rr);

		string config_file_name;
		string inet_addr;
		char output_file_name[512];
		int daq_mode;
		int daq_format;
		int progress_step;
		int time_preset;
		int event_preset;
		string file_prefix;

		time_t time0, time1;  // timeout times
		time_t time_acqstart, time_real;
		int event_loop;
		int progress_old;

		int run_num;
/*
		int _t_thr_start;
		int _t_thr_stop;
		int _t_thr_step;
		int _t_thr_loop;
		int _e_thr_start;
		int _e_thr_stop;
		int _e_thr_step;
		int _e_thr_loop;
*/
		bool hw_config_only;
		bool skip_empty_events;

		FILE *fout;
};


extern DaqControl DaqCtrl;
