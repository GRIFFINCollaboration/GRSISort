// Author: Peter C. Bender    06/14

/** \addtogroup Sorting
 *  @{
 */

#ifndef TOLDFRAGMENT_H
#define TOLDFRAGMENT_H

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
/// \class TOldFragment
///
/// This Class contains all of the information in an event
/// fragment for old GRIFFIN data (multiple pile-up hits in one
/// fragment)
///
/////////////////////////////////////////////////////////////////

class TOldFragment : public TVirtualFragment {
public:
   TOldFragment(); 
	TOldFragment(const TOldFragment&); ///<copy constructor
   virtual ~TOldFragment(); 

private:
	//////////////////// data members, sorted by size (as far as possible) to reduce padding ////////////////////
	// moving fNumberOfFilters up, just after fPPGWord would reduce the padding between members to zero
	// (right now fPPGWord is padded by two bytes), but wouldn't reduce the total size as the padding 
	// would just be shifted to the end of the class (size of class is a multiple of eight bytes)
	//sizeof(TOldFragment) is 152 bytes, sum of all members (including 16 bytes from TObject) is 148 bytes
   time_t    fMidasTimeStamp;         ///< Timestamp of the MIDAS event
   Long_t    fTriggerId;              ///< MasterFilterID in Griffin DAQ
			    								  
   Int_t     fMidasId;                ///< MIDAS ID
   Int_t     fTriggerBitPattern;	     ///< MasterFilterPattern in Griffin DAQ
   Int_t     fTimeStampLow;           ///< Timestamp low bits
   Int_t     fTimeStampHigh;          ///< Timestamp high bits
   UInt_t    fChannelAddress;         ///< Address of the channel
   UInt_t    fChannelId;              ///< Threshold crossing counter for a channel
   UInt_t    fAcceptedChannelId;      ///< Accepted threshold crossing counter for a channel
											   
   Int_t     fNumberOfHits;           ///< Number of piled up hits stored in this fragment (has to be Int_t for ROOT)
	/// CFD of each pileup hit
   Int_t*    fCfd;                    //[fNumberOfHits]
   /// The Integrated Charge 
   Int_t*    fPulseHeight;	           //[fNumberOfHits]
	/// integration length for each pileup hit
   UShort_t* fIntLength;              //[fNumberOfHits]
											     
   Int_t     fNetworkPacketNumber;    ///< Network packet number
   UShort_t  fDeadTime;	              ///< Deadtime from trigger
   UShort_t  fDataType;               ///< 
   UShort_t  fDetectorType;           ///< Detector Type (PACES,HPGe, etc)
   Short_t   fNumberOfPileups;        ///< Number of piled up hits 1-3
	UShort_t  fPPGWord;                ///< status of PPG

   std::vector<Short_t> fWavebuffer;  ///< waveform words

	//////////////////// transient members ////////////////////

   TPPG*     fPPG;                    //!<! pointer to TPPG class which holds all cycle information

   Int_t     fZc;                     //!<! ZC of each pileup hit
   Int_t     fCcShort;                //!<! Integration over the waveform rise (Descant only)
   Int_t     fCcLong;                 //!<! Integration over the wavefrom fall (Descant only)

   Short_t   fNumberOfFilters;        //!<! Number of filter patterns passed

public:  

	//////////////////// basic setter functions ////////////////////

	void SetAcceptedChannelId(UShort_t value) { fAcceptedChannelId = value; }
	void SetCcLong(Int_t value) { fCcLong = value; }
	void SetCcShort(Int_t value) { fCcShort = value; }
	void SetCfd(Int_t value, Int_t iter = 0) { if(iter < fNumberOfHits) fCfd[iter] = value; }
	void SetChannelAddress(UInt_t value) { fChannelAddress = value; }
	void SetChannelId(UInt_t value) { fChannelId = value; }
	void SetDataType(UShort_t value) { fDataType = value; }
	void SetDeadTime(UShort_t value) { fDeadTime = value; }
	void SetDetectorType(UShort_t value) { fDetectorType = value; }
	void SetIntLength(UShort_t value, Int_t iter = 0) { if(iter < fNumberOfHits) fIntLength[iter] = value; }
	void SetMidasId(time_t value) { fMidasId = value; }
	void SetMidasTimeStamp(time_t value) { fMidasTimeStamp = value; }
	void SetNetworkPacketNumber(Int_t value) { fNetworkPacketNumber = value; }
	void SetNumberOfFilters(UShort_t value) { fNumberOfFilters = value; }
	void SetNumberOfPileups(UShort_t value) { 
		fNumberOfPileups = value;
		if(fNumberOfHits != 0) printf("Warning, replacing %d number of hits with %d\n", fNumberOfHits, fNumberOfPileups+1);
		fNumberOfHits = fNumberOfPileups+1;//number of pile-ups doesn't count the first hit
		if(fCfd != NULL) delete[] fCfd;
		fCfd = new Int_t[fNumberOfHits]();
		if(fPulseHeight != NULL) delete[] fPulseHeight;
		fPulseHeight = new Int_t[fNumberOfHits]();
		if(fIntLength != NULL) delete[] fIntLength;
		fIntLength = new UShort_t[fNumberOfHits]();
		//printf("Changed fragment %ld to %d hits, %p, %p, %p\n", fragId, fNumberOfHits, static_cast<void*>(fCfd), static_cast<void*>(fIntLength), static_cast<void*>(fPulseHeight));
	}
	void SetNumberOfWords(UShort_t value) { std::cerr<<"Error, "<<__PRETTY_FUNCTION__<<" called, TOldFragment shouldn't have a number of words."<<std::endl; }
	void SetPPGWord(UShort_t value) { fPPGWord = value; }
	void SetPulseHeight(Int_t value, Int_t iter = 0) { if(iter < fNumberOfHits) fPulseHeight[iter] = value; }
	void SetTimeStampHigh(Int_t value) { fTimeStampHigh = value; }
	void SetTimeStampLow(Int_t value) { fTimeStampLow = value; }
	void SetTriggerBitPattern(Int_t value) { fTriggerBitPattern = value; }
	void SetTriggerId(Long_t value) { fTriggerId = value; }
	void SetWavebuffer(Short_t value) { fWavebuffer.push_back(value); }
	void SetZc(Int_t value) { fZc = value; }

	//////////////////// basic getter functions ////////////////////

	UShort_t GetAcceptedChannelId() const      { return fAcceptedChannelId; }
	Int_t GetCcLong() const                    { return fCcLong; }
	Int_t GetCcShort() const                   { return fCcShort; }
	Int_t GetCfd(Int_t iter) const             { if(iter < fNumberOfPileups) return fCfd[iter]; return 0; }
   UInt_t GetChannelAddress() const           { return fChannelAddress; }
	UInt_t GetChannelId() const                { return fChannelId; }
   UShort_t GetDataType() const               { return fDataType; }
	UShort_t GetDeadTime() const               { return fDeadTime; }
   UShort_t GetDetectorType() const           { return fDetectorType; }
   UShort_t GetIntLength(Int_t iter) const    { if(iter < fNumberOfPileups) return fIntLength[iter]; return 0; }
	Int_t  GetMidasId() const                  { return fMidasId; }
	time_t GetMidasTimeStamp() const           { return fMidasTimeStamp; }
   Int_t  GetNetworkPacketNumber() const      { return fNetworkPacketNumber; }
   Short_t GetNumberOfFilters() const         { return fNumberOfFilters; }
   Int_t GetNumberOfHits() const              { return fNumberOfHits; }
   Short_t GetNumberOfPileups() const         { return fNumberOfPileups; }
   Short_t GetNumberOfWords() const           { return -1; } // we could change this to calculate the number of words
	UShort_t GetPPGWord() const                { return fPPGWord; }
	Int_t GetPulseHeight(Int_t iter) const     { if(iter < fNumberOfPileups) return fPulseHeight[iter]; return 0; }
	Int_t  GetTimeStampHigh() const            { return fTimeStampHigh; }
   Int_t  GetTimeStampLow() const             { return fTimeStampLow; }
	Int_t  GetTriggerBitPattern() const        { return fTriggerBitPattern; }
	Long_t GetTriggerId(size_t iter = 0) const { return fTriggerId; }
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
   size_t GetNumberOfCharges() { return static_cast<size_t>(fNumberOfHits); }
	Int_t Get4GCfd(Int_t i=0) const;

	//////////////////// misc. functions ////////////////////
	bool IsDetector(const char *prefix, Option_t *opt = "CA") const;
	int  GetColor(Option_t *opt = "") const;
   bool HasWave() const { return (fWavebuffer.size()>0) ?  true : false; }

   void Clear(Option_t *opt = "");
   void Print(Option_t *opt = "") const;

	TOldFragment* Clone(const char* name = "");
   
   bool operator<(const TOldFragment& rhs) const { return (GetTimeStamp() < rhs.GetTimeStamp()); }
   bool operator>(const TOldFragment& rhs) const { return (GetTimeStamp() > rhs.GetTimeStamp()); }

	TOldFragment& operator=(const TOldFragment&);

/// \cond CLASSIMP
   ClassDef(TOldFragment,1);  // Event Fragments
/// \endcond
};
/*! @} */
#endif // TOLDFRAGMENT_H
