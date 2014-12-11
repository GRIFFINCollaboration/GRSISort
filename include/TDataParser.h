#ifndef TDATAPARSER_H
#define TDATAPARSER_H

#include "Globals.h"
#include <stdint.h>
#include <ctime>
#include <vector>
#include <map>

#include <TChannel.h>
#include <TFragment.h>
//#include <TObject.h>

class TDataParser { //: public TObject { 

  private:
    static TDataParser *fDataParser;  //A pointer to the global DataParser Class
    static bool no_waveforms;         //The flag to turn wave_forms on or off
    static bool record_stats;         //The flag to turn on stats recording
	 static TChannel *gChannel;
    TDataParser();
	
  public:
    ~TDataParser();
    static TDataParser *instance();    //returns the global TDataParser Object
    static void SetNoWaveForms(bool temp = true) {no_waveforms = temp;}
    static void SetRecordStats(bool temp = true) {record_stats = temp;}      

  private:
    static const unsigned long fgMaxTriggerId; //The last trigger ID Called
    static unsigned long fgLastMidasId;        //The last MIDAS ID in the midas file
    static unsigned long fgLastTriggerId;      //The last Trigged ID in the MIDAS File
    static unsigned long fgLastNetworkPacket;  //The last network packet recieved.

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
    static bool SetGRIFNetworkPacket(uint32_t,TFragment*);

    static bool SetGRIFWaveForm(uint32_t,TFragment*);
    static bool SetGRIFDeadTime(uint32_t,TFragment*);

    static void FillStats(TFragment*);

    ClassDef(TDataParser,0); //Parses the MIDAS files into DAQ-dependent TFragments. 

};

#endif
