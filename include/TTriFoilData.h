#ifndef __TriFoilDATA__
#define __TriFoilDATA__

#include "Globals.h"

#include <cstdlib>
#include <vector>

#include "TFragment.h"
#include "TChannel.h"

#include <TGRSIDetectorData.h>

class TTriFoilData : public TGRSIDetectorData  {
private:
  // TriFoil
  // Energy

	Int_t TriFoil_TimeStampHigh;
	Int_t TriFoil_TimeStampLow;
	Int_t TriFoil_Time;

	std::vector<Short_t> TriFoil_WaveBuffer;

	static bool fIsSet; //!

public:
  TTriFoilData();															//!
  virtual ~TTriFoilData(); //!
  
	static void Set(bool flag=true) { fIsSet=flag; } //!
	static bool IsSet() { return fIsSet; }           //!

  void Clear(Option_t *opt=""); //!
  //void Clear(const Option_t*) {};
  void Print(Option_t *opt=""); //!

  /////////////////////           SETTERS           ////////////////////////

	inline void SetTimeStampHigh(Int_t &time)	{ TriFoil_TimeStampHigh = time; }
	inline void SetTimeStampLow(Int_t &time)	{ TriFoil_TimeStampLow  = time; }
	inline void SetTimeToTrig(Int_t &time)	   { TriFoil_Time          = time; }

	inline void SetWaveBuffer(std::vector<Short_t> wave) { TriFoil_WaveBuffer = wave; }

	inline void SetData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
		SetTimeStampHigh(frag->TimeStampHigh);
		SetTimeStampLow(frag->TimeStampLow);
		SetTimeToTrig(frag->TimeToTrig);
		SetWaveBuffer(frag->wavebuffer);
	}
	
  /////////////////////           GETTERS           ////////////////////////

	inline Int_t GetTimeStampHigh()	{ return TriFoil_TimeStampHigh; }
	inline Int_t GetTimeStampLow()   { return TriFoil_TimeStampLow; }
	inline Int_t GetTimeToTrig()		{ return TriFoil_Time; }

	inline std::vector<Short_t> GetWaveBuffer() { return TriFoil_WaveBuffer; }
 
  ClassDef(TTriFoilData,0)  //!  // TriFoilData structure 
};

#endif
