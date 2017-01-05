/* Configure.h
* Uses libconfig
*/

class DaqConfig
{
	public:
		DaqConfig();
		~DaqConfig() { };

		int Read(char *configfile);
		void Save(char *outfile);
		int ParseInlineParam(int, char**);

	private:
		void GetRunParameters(void);
		void GetDefaultParameters(void);
		void GetIndividualParameters(void);
		void GetRunNumber(void);
};
