#ifndef TDATAPARSER_H
#define TDATAPARSER_H

#include "Globals.h"
#include <stdint.h>
#include <ctime>
#include <vector>
#include <map>

#include "TFragment.h"
//#include <TObject.h>

class TDataParser { //: public TObject { 

	private:
		static TDataParser *fDataParser;
		static bool no_waveforms;
                static bool record_stats;
		TDataParser();
	
	public:
		~TDataParser();
		static TDataParser *instance();
	        static bool SetNoWaveForms(bool temp = true) {no_waveforms = temp;}
                static bool SetRecordStats(bool temp = true) {record_stats = temp;}      

	private:
		static const unsigned long fgMaxTriggerId;
		static unsigned long fgLastMidasId;
		static unsigned long fgLastTriggerId;
		static std::map<int,int> fragment_id_map;


	public:
		//static std::vector<TFragment*> TigressDataToFragment(uint32_t *data, int size,unsigned int midasserialnumber = 0, time_t midastime = 0);
		static int TigressDataToFragment(uint32_t *data, int size,unsigned int midasserialnumber = 0, time_t midastime = 0);
		static int GriffinDataToFragment(uint32_t *data, int size,unsigned int midasserialnumber = 0, time_t midastime = 0);

   private:
      static void SetTIGWave(uint32_t, TFragment*);
      static void SetTIGAddress(uint32_t, TFragment*);
      static void SetTIGCfd(uint32_t, TFragment*);
      static void SetTIGCharge(uint32_t, TFragment*);
      static void SetTIGLed(uint32_t, TFragment*);

      static bool SetTIGTriggerID(uint32_t, TFragment*);
      static bool SetTIGTimeStamp(uint32_t*, TFragment*);

		


  		 static bool SetGRIFHeader(uint32_t,TFragment*);
   	 static bool SetGRIFPPG(uint32_t,TFragment*);
   	 static bool SetGRIFMasterFilterId(uint32_t,TFragment*);
   	 static bool SetGRIFMasterFilterPattern(uint32_t,TFragment*);
   	 static bool SetGRIFChannelTriggerId(uint32_t,TFragment*);  
   	 static bool SetGRIFTimeStampLow(uint32_t,TFragment*);

       static bool SetGRIFWaveForm(uint32_t,TFragment*);
       static bool SetGRIFDeadTime(uint32_t,TFragment*);


	static void FillStats(TFragment*);





	//ClassDef(TParser,0);

};

#endif
