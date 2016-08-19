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
#include <vector>
#include <map>

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
  enum EBank { kWFDN,kGRF1,kGRF2,kGRF3,kGRF4,kFME0,kFME1,kFME2,kFME3 };

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> >&
  GoodOutputQueue() { return good_output_queue; }

  std::shared_ptr<ThreadsafeQueue<TFragment*> >&
  BadOutputQueue() { return bad_output_queue; }

  std::shared_ptr<ThreadsafeQueue<TEpicsFrag*> >&
  ScalerOutputQueue() { return scaler_output_queue; }
#endif

private:
#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> > good_output_queue;
  std::shared_ptr<ThreadsafeQueue<TFragment*> > bad_output_queue;
  std::shared_ptr<ThreadsafeQueue<TEpicsFrag*> > scaler_output_queue;
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

public:
  void Push(ThreadsafeQueue<TFragment*>& queue, TFragment* frag);

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

private:
  //utility
  void DeleteAll(std::vector<TFragment*>*);
  void GRIFNormalizeFrags(std::vector<TFragment*> *Frags);

private:
  void SetTIGWave(uint32_t, TFragment*);
  void SetTIGAddress(uint32_t, TFragment*);
  void SetTIGCfd(uint32_t, TFragment*);
  void SetTIGCharge(uint32_t, TFragment*);
  void SetTIGLed(uint32_t, TFragment*);

  bool SetTIGTriggerID(uint32_t, TFragment*);
  bool SetTIGTimeStamp(uint32_t*, TFragment*);

  bool SetGRIFHeader(uint32_t, TFragment*, EBank);
  bool SetGRIFMasterFilterPattern(uint32_t, TFragment*, EBank);
  bool SetGRIFMasterFilterId(uint32_t, TFragment*);
  bool SetGRIFChannelTriggerId(uint32_t, TFragment*);
  bool SetGRIFTimeStampLow(uint32_t, TFragment*);
  bool SetGRIFNetworkPacket(uint32_t, TFragment*);

  bool SetGRIFPsd(uint32_t, TFragment*);
  bool SetGRIFCc(uint32_t, TFragment*);

  bool SetGRIFWaveForm(uint32_t,TFragment*);
  bool SetGRIFDeadTime(uint32_t,TFragment*);

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
