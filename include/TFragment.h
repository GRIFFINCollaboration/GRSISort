// Author: Peter C. Bender    06/14

/** \addtogroup Sorting
 *  @{
 */

#ifndef TFRAGMENT_H
#define TFRAGMENT_H

#include "Globals.h"
#include "TPPG.h"

#include <vector>
#include <time.h>

#include "Rtypes.h"
#include "TObject.h"

/////////////////////////////////////////////////////////////////
///
/// \class TFragment
///
/// This class is the basis for TNewFragment and TOldFragment.
/// All member functions are virtual and redefined in the derived
/// classes.
///
/////////////////////////////////////////////////////////////////

class TFragment : public TObject	{
public:
   TFragment() {};
	TFragment(const TFragment&, int hit = -1) : TObject() {}; ///<copy constructor that only copies the requested hit (if hit is in range 0 - Cfd.size())
   virtual ~TFragment() {};

	//////////////////// basic setter functions ////////////////////

	virtual void SetAcceptedChannelId(UShort_t value)         {}
	virtual void SetCcLong(Int_t value)                       {}
	virtual void SetCcShort(Int_t value)                      {}
	virtual void SetCfd(Int_t value, Int_t iter = 0)          {}
	virtual void SetChannelAddress(UInt_t value)              {}
	virtual void SetChannelId(UInt_t value)                   {}
	virtual void SetDataType(UShort_t value)                  {}
	virtual void SetDeadTime(UShort_t value)                  {}
	virtual void SetDetectorType(UShort_t value)              {}
	virtual void SetIntLength(UShort_t value, Int_t iter = 0) {}
	virtual void SetMidasId(time_t value)                     {}
	virtual void SetMidasTimeStamp(time_t value)              {}
	virtual void SetNetworkPacketNumber(Int_t value)          {}
	virtual void SetNumberOfFilters(UShort_t value)           {}
	virtual void SetNumberOfPileups(UShort_t value)           {}
	virtual void SetPPGWord(UShort_t value)                   {}
	virtual void SetPulseHeight(Int_t value, Int_t iter = 0)  {}
	virtual void SetTimeStampHigh(Int_t value)                {}
	virtual void SetTimeStampLow(Int_t value)                 {}
	virtual void SetTriggerBitPattern(Int_t value)            {}
	virtual void SetTriggerId(Long_t value)                   {}
	virtual void SetWavebuffer(Short_t value)                 {}
	virtual void SetZc(Int_t value)                           {}

	//////////////////// basic getter functions ////////////////////

	virtual UShort_t GetAcceptedChannelId() const       { return 0; }
	virtual Int_t GetCcLong() const                     { return 0; }
	virtual Int_t GetCcShort() const                    { return 0; }
	virtual Int_t GetCfd(Int_t iter = 0) const          { return 0; }
   virtual UInt_t GetChannelAddress() const            { return 0; }
	virtual UInt_t GetChannelId() const                 { return 0; }
   virtual UShort_t GetDataType() const                { return 0; }
	virtual UShort_t GetDeadTime() const                { return 0; }
   virtual UShort_t GetDetectorType() const            { return 0; }
	virtual Short_t GetHitIndex() const                 { return 0; }
   virtual UShort_t GetIntLength(Int_t iter = 0) const { return 0; }
	virtual Int_t  GetMidasId() const                   { return 0; }
	virtual time_t GetMidasTimeStamp() const            { return 0; }
   virtual Int_t  GetNetworkPacketNumber() const       { return 0; }
   virtual size_t GetNumberOfFilters() const           { return 0; }
   virtual size_t GetNumberOfPileups() const           { return 0; }
	virtual UShort_t GetPPGWord() const                 { return 0; }
	virtual Int_t GetPulseHeight(Int_t iter = 0) const  { return 0; }
	virtual Int_t  GetTimeStampHigh() const             { return 0; }
   virtual Int_t  GetTimeStampLow() const              { return 0; }
	virtual Int_t  GetTriggerBitPattern() const         { return 0; }
	virtual Long_t GetTriggerId(size_t iter = 0) const  { return 0; }
	virtual std::vector<Short_t> GetWavebuffer() const  { return std::vector<Short_t>(); }
	virtual Short_t GetWavebuffer(size_t iter) const    { return 0; }
	virtual size_t GetWavebufferSize() const            { return 0; }
   virtual Int_t GetZc() const                         { return 0; }

	//////////////////// advanced getter functions ////////////////////

	virtual Short_t GetChannelNumber() const            { return 0; }
	virtual TPPG*  GetPPG() const                       { return NULL; }
	virtual double GetTime() const                      { return 0.; }
	virtual long   GetTimeStamp() const                 { return 0; }
	virtual double GetTZero() const                     { return 0.; }
	virtual const char* GetName() const                 { return ""; }
	virtual double GetEnergy(Int_t iter = 0) const      { return 0.; }
   virtual Float_t GetCharge(Int_t iter = 0) const     { return 0.; }
	virtual long GetTimeStamp_ns() const                { return 0; }
	virtual ULong64_t GetTimeInCycle()                  { return 0; }
	virtual ULong64_t GetCycleNumber()                  { return 0; }
   virtual size_t GetNumberOfCharges()                 { return 0; }
	virtual Int_t Get4GCfd(Int_t i = 0) const           { return 0; }

	//////////////////// misc. functions ////////////////////
	virtual bool IsDetector(const char *prefix, Option_t *opt = "CA") const { return false; }
	virtual int  GetColor(Option_t *opt = "") const     { return -1; }
   virtual bool HasWave() const { return false; }

   virtual void Clear(Option_t *opt = "") {}
   virtual void Print(Option_t *opt = "") const {}
   
	virtual bool operator<(const TFragment& rhs) const  { return false; }
	virtual bool operator>(const TFragment& rhs) const  { return false; }

/// \cond CLASSIMP
   ClassDef(TFragment,6);  // Event Fragments
/// \endcond
};
/*! @} */
#endif // TFRAGMENT_H
