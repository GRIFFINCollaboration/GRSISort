// Author: Peter C. Bender    06/14

/** \addtogroup Sorting
 *  @{
 */

#ifndef TFRAGMENT_H
#define TFRAGMENT_H

#include "Globals.h"
#include "TPPG.h"

#include <vector>
#include <iostream>
#include <time.h>

#include "Rtypes.h"
#include "TObject.h"
#include "TVirtualFragment.h"

/////////////////////////////////////////////////////////////////
///
/// \class TFragment
///
/// This Class contains all of the information in an event
/// fragment for new GRIFFIN data and TIGRESS data
///
/////////////////////////////////////////////////////////////////

class TFragment : public TVirtualFragment	{
public:
   TFragment(); 
	TFragment(const TFragment&); ///< copy constructor
   virtual ~TFragment(); 

private:
	//////////////////// data members, sorted by size (as far as possible) to reduce padding ////////////////////
   time_t   fMidasTimeStamp;          ///< Timestamp of the MIDAS event
   Int_t    fMidasId;                 ///<  MIDAS ID
   Int_t    fTriggerBitPattern;	     ///<  MasterFilterPattern in Griffin DAQ
			   							     
   Int_t    fNetworkPacketNumber;     ///<  Network packet number
   Int_t    fTimeStampLow;            ///<  Timestamp low bits
   Int_t    fTimeStampHigh;           ///<  Timestamp high bits
   UInt_t   fChannelAddress;          ///<  Address of the channel
   UInt_t   fChannelId;               ///< Threshold crossing counter for a channel
   UInt_t   fAcceptedChannelId;       ///< Accepted threshold crossing counter for a channel
											     
   Int_t    fCfd;                     ///< CFD
   Int_t    fPulseHeight;             ///< The Integrated Charge
   UShort_t fIntLength;               ///< Integration Length
											     	    
   UShort_t fDeadTime;	              ///< Deadtime from trigger
   UShort_t fDataType;                ///< 
   UShort_t fDetectorType;            ///< Detector Type (PACES,HPGe, etc)

   std::vector<Long_t> fTriggerId;    ///< MasterFilterID in Griffin DAQ  
   std::vector<Short_t>  fWavebuffer; ///< waveform words

	//////////////////// transient members (except for fNumberOfPileups!) ////////////////////

   TPPG*    fPPG;                     //!<! pointer to TPPG class which holds all cycle information
			   
   Int_t    fZc;                      //!<! ZC of each pileup hit
   Int_t    fCcShort;                 //!<! Integration over the waveform rise (Descant only)
   Int_t    fCcLong;                  //!<! Integration over the wavefrom fall (Descant only)
											     
   Short_t  fNumberOfPileups;         ///< Number of piled up hits 1-3
   Short_t  fNumberOfWords;           //!<! Number of non-waveform words in fragment

public:  

	//////////////////// basic setter functions ////////////////////

	void SetAcceptedChannelId(UShort_t value) { fAcceptedChannelId = value; }
	void SetCcLong(Int_t value) { fCcLong = value; }
	void SetCcShort(Int_t value) { fCcShort = value; }
	void SetCfd(Int_t value, Int_t iter = 0) { fCfd = value; }
	void SetChannelAddress(UInt_t value) { fChannelAddress = value; }
	void SetChannelId(UInt_t value) { fChannelId = value; }
	void SetDataType(UShort_t value) { fDataType = value; }
	void SetDeadTime(UShort_t value) { fDeadTime = value; }
	void SetDetectorType(UShort_t value) { fDetectorType = value; }
	void SetIntLength(UShort_t value, Int_t iter = 0) { fIntLength = value; }
	void SetMidasId(time_t value) { fMidasId = value; }
	void SetMidasTimeStamp(time_t value) { fMidasTimeStamp = value; }
	void SetNetworkPacketNumber(Int_t value) { fNetworkPacketNumber = value; }
	void SetNumberOfFilters(UShort_t value) { std::cerr<<"Error, "<<__PRETTY_FUNCTION__<<" called, TFragment shouldn't have a number of filters."<<std::endl; }
	void SetNumberOfPileups(UShort_t value) { fNumberOfPileups = value; }
	void SetNumberOfWords(UShort_t value) { fNumberOfWords = value; }
	void SetPPGWord(UShort_t value) {}
	void SetPulseHeight(Int_t value, Int_t iter = 0) { fPulseHeight = value; }
	void SetTimeStampHigh(Int_t value) { fTimeStampHigh = value; }
	void SetTimeStampLow(Int_t value) { fTimeStampLow = value; }
	void SetTriggerBitPattern(Int_t value) { fTriggerBitPattern = value; }
	void SetTriggerId(Long_t value) { fTriggerId.push_back(value); }
	void SetWavebuffer(Short_t value) { fWavebuffer.push_back(value); }
	void SetZc(Int_t value) { fZc = value; }

	//////////////////// basic getter functions ////////////////////

	UShort_t GetAcceptedChannelId() const      { return fAcceptedChannelId; }
	Int_t GetCcLong() const                    { return fCcLong; }
	Int_t GetCcShort() const                   { return fCcShort; }
	Int_t GetCfd(Int_t iter = 0) const         { return fCfd; }
   UInt_t GetChannelAddress() const           { return fChannelAddress; }
	UInt_t GetChannelId() const                { return fChannelId; }
   UShort_t GetDataType() const               { return fDataType; }
	UShort_t GetDeadTime() const               { return fDeadTime; }
   UShort_t GetDetectorType() const           { return fDetectorType; }
   UShort_t GetIntLength(Int_t iter = 0) const{ return fIntLength; }
	Int_t  GetMidasId() const                  { return fMidasId; }
	time_t GetMidasTimeStamp() const           { return fMidasTimeStamp; }
   Int_t  GetNetworkPacketNumber() const      { return fNetworkPacketNumber; }
   Short_t GetNumberOfFilters() const         { return fNumberOfWords-9; }
	Int_t GetNumberOfHits() const              { return 1; }
   Short_t GetNumberOfPileups() const         { return fNumberOfPileups; }
   Short_t GetNumberOfWords() const           { return fNumberOfWords; }
	UShort_t GetPPGWord() const                { return 0; }
	Int_t GetPulseHeight(Int_t iter = 0) const { return fPulseHeight; }
	Int_t  GetTimeStampHigh() const            { return fTimeStampHigh; }
   Int_t  GetTimeStampLow() const             { return fTimeStampLow; }
	Int_t  GetTriggerBitPattern() const        { return fTriggerBitPattern; }
	Long_t GetTriggerId(size_t iter=0) const   { if(iter < fTriggerId.size()) return fTriggerId[iter]; return 0; }
	std::vector<Short_t> GetWavebuffer() const { return fWavebuffer; }
	Short_t GetWavebuffer(size_t iter) const   { if(iter < fWavebuffer.size()) return fWavebuffer[iter]; return 0; }
	size_t GetWavebufferSize() const           { return fWavebuffer.size(); }
   Int_t GetZc() const                        { return fZc; }
	
	//////////////////// advanced getter functions ////////////////////

	Short_t GetChannelNumber() const;
	TPPG*  GetPPG();
	double GetTime() const;
	long   GetTimeStamp() const;
	double GetTZero() const;
	const char* GetName() const;
	double GetEnergy(Int_t iter=0) const;
   Float_t GetCharge(Int_t iter=0) const;
	long GetTimeStamp_ns() const;
	ULong64_t GetTimeInCycle();
	ULong64_t GetCycleNumber();
   size_t GetNumberOfCharges() { return 1; }
	Int_t Get4GCfd(Int_t i=0) const;

	//////////////////// misc. functions ////////////////////
	bool IsDetector(const char *prefix, Option_t *opt = "CA") const;
	int  GetColor(Option_t *opt = "") const;
   bool HasWave() const { return (fWavebuffer.size()>0) ?  true : false; }

   void Clear(Option_t *opt = "");
   void Print(Option_t *opt = "") const;

	TFragment* Clone(const char* name = "");
   
   bool operator<(const TFragment& rhs) const { return (GetTimeStamp() < rhs.GetTimeStamp()); }
   bool operator>(const TFragment& rhs) const { return (GetTimeStamp() > rhs.GetTimeStamp()); }

	TFragment& operator=(const TFragment&);

/// \cond CLASSIMP
   ClassDef(TFragment,1);  // Event Fragments
/// \endcond
};
/*! @} */
#endif // TFRAGMENT_H
