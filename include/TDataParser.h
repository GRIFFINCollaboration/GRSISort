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
/// converts it into a generic TFragment that the rest of
/// GRSISort can deal with. This is where event word masks
/// are applied, and any changes to the event format must
/// be implemented.
///
/////////////////////////////////////////////////////////////////


#include "Globals.h"
#include <ctime>
#include <sstream>
#include <vector>
#include <map>
#include <limits>

#ifndef __CINT__
#include <memory>
#endif

//#include <enum_string.h>

#include "TChannel.h"
#include "TFragment.h"
#include "TPPG.h"
#include "TScaler.h"
#include "TFragmentMap.h"
#include "ThreadsafeQueue.h"
#include "TEpicsFrag.h"

class TDataParser {
public:
  TDataParser();
  ~TDataParser();
  void SetNoWaveForms(bool temp = true) { fNoWaveforms = temp; }
  void SetRecordDiag(bool temp = true) { fRecordDiag = temp; }

  //ENUM(EBank, char, kWFDN,kGRF1,kGRF2,kGRF3,kFME0,kFME1,kFME2,kFME3);
  enum EBank { kWFDN=0,kGRF1=1,kGRF2=2,kGRF3=3,kGRF4=4,kFME0=5,kFME1=6,kFME2=7,kFME3=8 };

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > >&
  AddGoodOutputQueue(size_t maxSize = 50000) { 
	  std::stringstream name; name<<"good_frag_queue_"<<fGoodOutputQueues.size();
     fGoodOutputQueues.push_back(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment> > >(name.str(), maxSize));
     return fGoodOutputQueues.back(); 
  }

  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > >& BadOutputQueue() { return fBadOutputQueue; }

  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TEpicsFrag> > >& ScalerOutputQueue() { return fScalerOutputQueue; }
#endif
  void ClearQueue();
  size_t ItemsPushed() { if(fGoodOutputQueues.size() > 0) return fGoodOutputQueues.back()->ItemsPushed(); return std::numeric_limits<std::size_t>::max(); }
  void SetFinished();
  std::string OutputQueueStatus();

	void SetStatusVariables(std::atomic_size_t* itemsPopped, std::atomic_long* inputSize) {
		fItemsPopped = itemsPopped;
		fInputSize = inputSize;
	}
private:
#ifndef __CINT__
  std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > > > fGoodOutputQueues;
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > > fBadOutputQueue;
  std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TEpicsFrag> > > fScalerOutputQueue;
#endif

  bool fNoWaveforms;         ///< The flag to turn wave_forms on or off
  bool fRecordDiag;         ///< The flag to turn on diagnostics recording
  TChannel* gChannel;

  const unsigned long fMaxTriggerId; ///< The last trigger ID Called
  unsigned long fLastMidasId;        ///< The last MIDAS ID in the midas file
  unsigned long fLastTriggerId;      ///< The last Trigged ID in the MIDAS File
  unsigned long fLastNetworkPacket;  ///< The last network packet recieved.

  std::map<int,int> fFragmentIdMap;
  bool fFragmentHasWaveform;

  TFragmentMap fFragmentMap;

  std::atomic_size_t* fItemsPopped;
  std::atomic_long* fInputSize;

public:
#ifndef __CINT__
  void Push(std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > > >& queue, std::shared_ptr<TFragment> frag);
  void Push(ThreadsafeQueue<std::shared_ptr<const TFragment> >& queue, std::shared_ptr<TFragment> frag);
#endif

  int TigressDataToFragment(uint32_t *data, int size, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
  int GriffinDataToFragment(uint32_t *data, int size, EBank bank, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
  int GriffinDataToPPGEvent(uint32_t *data, int size, unsigned int midasSerialNumber=0, time_t midasTime=0);
  int GriffinDataToScalerEvent(uint32_t *data, int address);

  int EPIXToScalar(float *data, int size, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
  int SCLRToScalar(uint32_t *data, int size, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
  int EightPIDataToFragment(uint32_t stream, uint32_t* data,
                            int size, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
  int FifoToFragment(unsigned short *data, int size, bool zerobuffer=false,
                     unsigned int midasSerialNumber=0, time_t midasTime=0);
  int FippsToFragment(std::vector<char> data);

private:
  //utility
#ifndef __CINT__
  void DeleteAll(std::vector<std::shared_ptr<const TFragment> >*);
  void GRIFNormalizeFrags(std::vector<std::shared_ptr<const TFragment> > *Frags);
#endif

private:
#ifndef __CINT__
  void SetTIGWave(uint32_t, std::shared_ptr<TFragment>);
  void SetTIGAddress(uint32_t, std::shared_ptr<TFragment>);
  void SetTIGCfd(uint32_t, std::shared_ptr<TFragment>);
  void SetTIGCharge(uint32_t, std::shared_ptr<TFragment>);
  void SetTIGLed(uint32_t, std::shared_ptr<TFragment>);

  bool SetTIGTriggerID(uint32_t, std::shared_ptr<TFragment>);
  bool SetTIGTimeStamp(uint32_t*, std::shared_ptr<TFragment>);

  bool SetGRIFHeader(uint32_t, std::shared_ptr<TFragment>, EBank);
  bool SetGRIFMasterFilterPattern(uint32_t, std::shared_ptr<TFragment>, EBank);
  bool SetGRIFMasterFilterId(uint32_t, std::shared_ptr<TFragment>);
  bool SetGRIFChannelTriggerId(uint32_t, std::shared_ptr<TFragment>);
  bool SetGRIFTimeStampLow(uint32_t, std::shared_ptr<TFragment>);
  bool SetGRIFNetworkPacket(uint32_t, std::shared_ptr<TFragment>);

  bool SetGRIFPsd(uint32_t, std::shared_ptr<TFragment>);
  bool SetGRIFCc(uint32_t, std::shared_ptr<TFragment>);

  bool SetGRIFWaveForm(uint32_t,std::shared_ptr<TFragment>);
  bool SetGRIFDeadTime(uint32_t,std::shared_ptr<TFragment>);
#endif

  bool SetNewPPGPattern(uint32_t,TPPGData*);
  bool SetOldPPGPattern(uint32_t,TPPGData*);
  bool SetPPGNetworkPacket(uint32_t,TPPGData*);
  bool SetPPGLowTimeStamp(uint32_t,TPPGData*);
  bool SetPPGHighTimeStamp(uint32_t,TPPGData*);
  bool SetScalerNetworkPacket(uint32_t, TScalerData*);
  bool SetScalerLowTimeStamp(uint32_t, TScalerData*);
  bool SetScalerHighTimeStamp(uint32_t, TScalerData*, int&);
  bool SetScalerValue(int, uint32_t, TScalerData*);
};
/*! @} */
#endif
