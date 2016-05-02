// Author: Peter C. Bender    06/14

/** \addtogroup Sorting
 *  @{
 */

#ifndef TVIRTUALFRAGMENT_H
#define TVIRTUALFRAGMENT_H

#include "Globals.h"
#include "TPPG.h"

#include <vector>
#include <time.h>

#include "Rtypes.h"
#include "TObject.h"

/////////////////////////////////////////////////////////////////
///
/// \class TVirtualFragment
///
/// This class is the basis for TFragment and TOldFragment.
/// All member functions are virtual and redefined in the derived
/// classes.
///
/////////////////////////////////////////////////////////////////

class TVirtualFragment : public TObject	{
public:
   TVirtualFragment() {};
	TVirtualFragment(const TVirtualFragment&) : TObject() {} ///<copy constructor
   virtual ~TVirtualFragment() {}

	//////////////////// basic setter functions ////////////////////

	virtual void SetAcceptedChannelId(UShort_t value)         = 0;
	virtual void SetCcLong(Int_t value)                       = 0;
	virtual void SetCcShort(Int_t value)                      = 0;
	virtual void SetCfd(Int_t value, Int_t iter = 0)          = 0;
	virtual void SetChannelAddress(UInt_t value)              = 0;
	virtual void SetChannelId(UInt_t value)                   = 0;
	virtual void SetDataType(UShort_t value)                  = 0;
	virtual void SetDeadTime(UShort_t value)                  = 0;
	virtual void SetDetectorType(UShort_t value)              = 0;
	virtual void SetIntLength(UShort_t value, Int_t iter = 0) = 0;
	virtual void SetMidasId(time_t value)                     = 0;
	virtual void SetMidasTimeStamp(time_t value)              = 0;
	virtual void SetNetworkPacketNumber(Int_t value)          = 0;
	virtual void SetNumberOfFilters(UShort_t value)           = 0;
	virtual void SetNumberOfPileups(UShort_t value)           = 0;
	virtual void SetNumberOfWords(UShort_t value)             = 0;
	virtual void SetPPGWord(UShort_t value)                   = 0;
	virtual void SetPulseHeight(Int_t value, Int_t iter = 0)  = 0;
	virtual void SetTimeStampHigh(Int_t value)                = 0;
	virtual void SetTimeStampLow(Int_t value)                 = 0;
	virtual void SetTriggerBitPattern(Int_t value)            = 0;
	virtual void SetTriggerId(Long_t value)                   = 0;
	virtual void SetWavebuffer(Short_t value)                 = 0;
	virtual void SetZc(Int_t value)                           = 0;

	//////////////////// basic getter functions ////////////////////

	virtual UShort_t GetAcceptedChannelId() const       = 0;
	virtual Int_t GetCcLong() const                     = 0;
	virtual Int_t GetCcShort() const                    = 0;
	virtual Int_t GetCfd(Int_t iter = 0) const          = 0;
   virtual UInt_t GetChannelAddress() const            = 0;
	virtual UInt_t GetChannelId() const                 = 0;
   virtual UShort_t GetDataType() const                = 0;
	virtual UShort_t GetDeadTime() const                = 0;
   virtual UShort_t GetDetectorType() const            = 0;
   virtual UShort_t GetIntLength(Int_t iter = 0) const = 0;
	virtual Int_t  GetMidasId() const                   = 0;
	virtual time_t GetMidasTimeStamp() const            = 0;
   virtual Int_t  GetNetworkPacketNumber() const       = 0;
   virtual Short_t GetNumberOfFilters() const          = 0;
	virtual Int_t GetNumberOfHits() const               = 0;
   virtual Short_t GetNumberOfPileups() const          = 0;
   virtual Short_t GetNumberOfWords() const            = 0;
	virtual UShort_t GetPPGWord() const                 = 0;
	virtual Int_t GetPulseHeight(Int_t iter = 0) const  = 0;
	virtual Int_t  GetTimeStampHigh() const             = 0;
   virtual Int_t  GetTimeStampLow() const              = 0;
	virtual Int_t  GetTriggerBitPattern() const         = 0;
	virtual Long_t GetTriggerId(size_t iter = 0) const  = 0;
	virtual std::vector<Short_t> GetWavebuffer() const  = 0;
	virtual Short_t GetWavebuffer(size_t iter) const    = 0;
	virtual size_t GetWavebufferSize() const            = 0;
   virtual Int_t GetZc() const                         = 0;

	//////////////////// advanced getter functions ////////////////////

	virtual Short_t GetChannelNumber() const            = 0;
	virtual TPPG*  GetPPG()                             = 0;
	virtual double GetTime() const                      = 0;
	virtual long   GetTimeStamp() const                 = 0;
	virtual double GetTZero() const                     = 0;
	virtual const char* GetName() const                 = 0;
	virtual double GetEnergy(Int_t iter = 0) const      = 0;
   virtual Float_t GetCharge(Int_t iter = 0) const     = 0;
	virtual long GetTimeStamp_ns() const                = 0;
	virtual ULong64_t GetTimeInCycle()                  = 0;
	virtual ULong64_t GetCycleNumber()                  = 0;
   virtual size_t GetNumberOfCharges()                 = 0;
	virtual Int_t Get4GCfd(Int_t i = 0) const           = 0;

	//////////////////// misc. functions ////////////////////
	virtual bool IsDetector(const char *prefix, Option_t *opt = "CA") const { return false; }
	virtual int  GetColor(Option_t *opt = "") const     { return -1; }
   virtual bool HasWave() const { return false; }

   virtual void Clear(Option_t *opt = "") {}
   virtual void Print(Option_t *opt = "") const {}
   
	virtual bool operator<(const TVirtualFragment& rhs) const  { return false; }
	virtual bool operator>(const TVirtualFragment& rhs) const  { return false; }

/// \cond CLASSIMP
   ClassDef(TVirtualFragment,6);  // Event Fragments
/// \endcond
};

template<class T> struct PointerLess {
	bool operator()(T* lhs, T* rhs) { return *lhs < *rhs; }
};

/*! @} */
#endif // TVIRTUALFRAGMENT_H
