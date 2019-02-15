#ifndef TPPG_H
#define TPPG_H

#include "TBuffer.h"

/** \addtogroup Sorting
 *  @{
 */

/*
 * Author:  R.Dunlop, <rdunlop@uoguelph.ca>
 *
 * Please indicate changes with your initials.
 *
 *
 */

//////////////////////////////////////////////////////////////////////////
///
/// \class TPPG
///
/// The TPPG is designed to hold all of the information about the
/// PPG status.
///
//////////////////////////////////////////////////////////////////////////

#include <map>
#include <utility>
#include <iostream>

#include "TFile.h"
#include "TObject.h"
#include "TCollection.h"

#include "Globals.h"
#include "TSingleton.h"

enum class EPpgPattern {
	kBeamOn     = 0x01,
	kDecay      = 0x04,
	kTapeMove   = 0x08,
	kBackground = 0x02,
	//kSync       = 0xc000,
	kJunk       = 0xFF
};

class TPPGData : public TObject {
public:
   TPPGData();
   TPPGData(const TPPGData&);
   ~TPPGData() override = default;

   void Copy(TObject& rhs) const override;

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

	// -------------------- setter functions
   void SetLowTimeStamp(UInt_t lowTime)
   {
      fLowTimeStamp = lowTime;
      SetTimeStamp();
   }
   void SetHighTimeStamp(UInt_t highTime)
   {
      fHighTimeStamp = highTime;
      SetTimeStamp();
   }
   void SetNewPPG(EPpgPattern newPpg) { fNewPpg = newPpg; }
   void SetNewPPG(UInt_t newPpg)
	{ 
		fNewPpg = static_cast<EPpgPattern>(newPpg&0xff);
		switch(fNewPpg) {
			case EPpgPattern::kBeamOn: case EPpgPattern::kDecay: case EPpgPattern::kTapeMove:
			case EPpgPattern::kBackground: case EPpgPattern::kJunk:
				break;
			default:
				if(newPpg != 0) std::cout<<"Warning, unknown ppg pattern 0x"<<std::hex<<newPpg<<std::dec<<", setting new pattern to kJunk!"<<std::endl;
				fNewPpg = EPpgPattern::kJunk;
		}
	}
   void SetOldPPG(EPpgPattern oldPpg) { fOldPpg = oldPpg; }
   void SetOldPPG(UInt_t oldPpg)
	{
		fOldPpg = static_cast<EPpgPattern>(oldPpg&0xff);
		switch(fOldPpg) {
			case EPpgPattern::kBeamOn: case EPpgPattern::kDecay: case EPpgPattern::kTapeMove:
			case EPpgPattern::kBackground: case EPpgPattern::kJunk:
				break;
			default:
				if(oldPpg != 0) std::cout<<"Warning, unknown ppg pattern 0x"<<std::hex<<oldPpg<<std::dec<<", setting old pattern to kJunk!"<<std::endl;
				fOldPpg = EPpgPattern::kJunk;
		}
	}
   void SetNetworkPacketId(UInt_t packet) { fNetworkPacketId = packet; }

   void SetTimeStamp();

	// -------------------- getter functions
   UInt_t      GetLowTimeStamp() const { return fLowTimeStamp; }
   UInt_t      GetHighTimeStamp() const { return fHighTimeStamp; }
   EPpgPattern GetNewPPG() const { return fNewPpg; }
   EPpgPattern GetOldPPG() const { return fOldPpg; }
   UInt_t      GetNetworkPacketId() const { return fNetworkPacketId; }

   Long64_t GetTimeStamp() const { return fTimeStamp; }

private:
   ULong64_t   fTimeStamp; ///< time stamp in ns
   EPpgPattern fOldPpg;
   EPpgPattern fNewPpg;
   UInt_t      fNetworkPacketId;
   UInt_t      fLowTimeStamp; ///< low bits of time stamp in 10 ns
   UInt_t      fHighTimeStamp; ///< high bits of time stamp in 10 ns

   /// \cond CLASSIMP
   ClassDefOverride(TPPGData, 3) // Contains PPG data information
   /// \endcond
};

class TPPG : public TSingleton<TPPG> {
public:
	friend class TSingleton<TPPG>;

   typedef std::map<ULong_t, TPPGData*> PPGMap_t;

   TPPG();
   TPPG(const TPPG&);
   ~TPPG() override;

   void Copy(TObject& obj) const override;
   Int_t Write(const char* name = nullptr, Int_t option = 0, Int_t bufsize = 0) override
   {
      return static_cast<const TPPG*>(this)->Write(name, option, bufsize);
   }
   Int_t Write(const char* name = nullptr, Int_t option = 0, Int_t bufsize = 0) const override;

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   void  Setup();
   bool Correct(bool verbose = false);

   void AddData(TPPGData* pat);
   EPpgPattern GetStatus(ULong64_t time) const;
   EPpgPattern GetNextStatus(ULong64_t time) const;
   ULong64_t GetLastStatusTime(ULong64_t time, EPpgPattern pat = EPpgPattern::kJunk) const;
   ULong64_t GetNextStatusTime(ULong64_t time, EPpgPattern pat = EPpgPattern::kJunk) const;
   Bool_t      MapIsEmpty() const;
   std::size_t PPGSize() const { return fPPGStatusMap->size() - 1; }
   std::size_t OdbPPGSize() const { return fOdbPPGCodes.size(); }
   short       OdbPPGCode(size_t index) const { return fOdbPPGCodes.at(index); }
   int         OdbDuration(size_t index) const { return fOdbDurations.at(index); }
   Long64_t    OdbCycleLength() const { Long64_t result = 0; for(auto dur : fOdbDurations) { result += dur; } return result; }
   Long64_t Merge(TCollection* list);
   void Add(const TPPG* ppg);
   void operator+=(const TPPG& rhs);

   void SetCycleLength(ULong64_t length) { fCycleLength = length; }

	// -------------------- getter functions
   ULong64_t GetCycleLength();
   ULong64_t GetNumberOfCycles();
   ULong64_t GetTimeInCycle(ULong64_t real_time);
   ULong64_t GetCycleNumber(ULong64_t real_time);

   ULong64_t GetStatusStart(EPpgPattern);

   const TPPGData* Next();
   const TPPGData* Previous();
   const TPPGData* First();
   const TPPGData* Last();

   void SetOdbCycle(std::vector<short> ppgCodes, std::vector<int> durations)
   {
      fOdbPPGCodes  = std::move(ppgCodes);
      fOdbDurations = std::move(durations);
   }

private:
   static TPPG*       fPPG; ///< static pointer to TPPG
   PPGMap_t::iterator MapBegin() const { return ++(fPPGStatusMap->begin()); }
   PPGMap_t::iterator MapEnd() const { return fPPGStatusMap->end(); }
   PPGMap_t::iterator fCurrIterator; //!<!

   PPGMap_t* fPPGStatusMap;
   ULong64_t fCycleLength;
   std::map<ULong64_t, int> fNumberOfCycleLengths;

	//bool               fUseOdb;
	//uint16_t           fCycleOffset;  ///< offset of cycle
   std::vector<short> fOdbPPGCodes;  ///< ppg state codes read from odb
   std::vector<int>   fOdbDurations; ///< duration of ppg state as read from odb

   /// \cond CLASSIMP
   ClassDefOverride(TPPG, 4) // Contains PPG information
   /// \endcond
};
/*! @} */
#endif
