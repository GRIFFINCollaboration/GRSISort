#ifndef TDATAPARSER_H
#define TDATAPARSER_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TDataParser
///
/// The TDataParser is the DAQ dependent part of GRSISort.
/// It takes a "DAQ-dependent"-flavoured MIDAS file and
/// converts it into a generic TVirtualFragment that the rest of
/// GRSISort can deal with. This is where event word masks
/// are applied, and any changes to the event format must
/// be implemented.
///
/////////////////////////////////////////////////////////////////

#include "Globals.h"
#include <ctime>
#include <vector>
#include <map>

#include "TChannel.h"
#include "TVirtualFragment.h"
#include "TPPG.h"
#include "TScaler.h"

class TDataParser {
  private:
    static TDataParser* fDataParser;  ///< A pointer to the global DataParser Class
    static bool fNoWaveforms;         ///< The flag to turn wave_forms on or off
    static bool fRecordDiag;         ///< The flag to turn on diagnostics recording
	 static TChannel* gChannel;
    TDataParser();
	
  public:
    ~TDataParser();
    static TDataParser* instance();    //returns the global TDataParser Object
    static void SetNoWaveForms(bool temp = true) { fNoWaveforms = temp; }
    static void SetRecordDiag(bool temp = true) { fRecordDiag = temp; }

  private:
    static const unsigned long fMaxTriggerId; ///< The last trigger ID Called
    static unsigned long fLastMidasId;        ///< The last MIDAS ID in the midas file
    static unsigned long fLastTriggerId;      ///< The last Trigged ID in the MIDAS File
    static unsigned long fLastNetworkPacket;  ///< The last network packet recieved.

    static std::map<int,int> fFragmentIdMap;

	 static int fCfdsRead;
	 static int fLedsRead;
	 static int fChargesRead;
	 static int fKValuesRead;

	 static bool fFragmentHasWaveform;

  public:
    static int TigressDataToFragment(uint32_t *data, int size,unsigned int midasSerialNumber = 0, time_t midasTime = 0);
    static int GriffinDataToFragment(uint32_t *data, int size, int bank, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
    static int GriffinDataToPPGEvent(uint32_t *data, int size, int bank, unsigned int midasSerialNumber=0, time_t midasTime=0); 
	 static int GriffinDataToScalerEvent(uint32_t *data, int address);
   
    static int EPIXToScalar(float *data,int size,unsigned int midasSerialNumber = 0,time_t midasTime = 0);
    static int SCLRToScalar(uint32_t *data,int size,unsigned int midasSerialNumber = 0,time_t midasTime = 0);
	 static int EightPIDataToFragment(uint32_t stream,uint32_t* data,
                                     int size,unsigned int midasSerialNumber = 0, time_t midasTime = 0);
    static int FifoToFragment(unsigned short *data,int size,bool zerobuffer=false,
                              unsigned int midasSerialNumber=0, time_t midasTime=0); 


  private:
    static void SetTIGWave(uint32_t, TVirtualFragment*);
    static void SetTIGAddress(uint32_t, TVirtualFragment*);
    static void SetTIGCfd(uint32_t, TVirtualFragment*);
    static void SetTIGCharge(uint32_t, TVirtualFragment*);
    static void SetTIGLed(uint32_t, TVirtualFragment*);

    static bool SetTIGTriggerID(uint32_t, TVirtualFragment*);
    static bool SetTIGTimeStamp(uint32_t*, TVirtualFragment*);

    static bool SetGRIFHeader(uint32_t,TVirtualFragment*&,int);
    static bool SetGRIFMasterFilterPattern(uint32_t,TVirtualFragment*,int);
    static bool SetGRIFMasterFilterId(uint32_t,TVirtualFragment*);
    static bool SetGRIFChannelTriggerId(uint32_t,TVirtualFragment*);  
    static bool SetGRIFTimeStampLow(uint32_t,TVirtualFragment*);
    static bool SetGRIFNetworkPacket(uint32_t,TVirtualFragment*);
    static bool SetGRIFCc(uint32_t, TVirtualFragment*);
    static bool SetGRIFPsd(uint32_t, TVirtualFragment*);

    static bool SetGRIFWaveForm(uint32_t,TVirtualFragment*);
    static bool SetGRIFDeadTime(uint32_t,TVirtualFragment*);

    static bool SetNewPPGPattern(uint32_t,TPPGData*);
    static bool SetOldPPGPattern(uint32_t,TPPGData*);
    static bool SetPPGNetworkPacket(uint32_t,TPPGData*);
    static bool SetPPGLowTimeStamp(uint32_t,TPPGData*);
    static bool SetPPGHighTimeStamp(uint32_t,TPPGData*);
	 static bool SetScalerNetworkPacket(uint32_t, TScalerData*);
	 static bool SetScalerLowTimeStamp(uint32_t, TScalerData*);
	 static bool SetScalerHighTimeStamp(uint32_t, TScalerData*, int&);
	 static bool SetScalerValue(int, uint32_t, TScalerData*);
};
/*! @} */
#endif
