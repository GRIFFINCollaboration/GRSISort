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
   // kSync       = 0xc000,
   kJunk = 0xFF
};

class TPPGData : public TObject {
public:
   TPPGData();
   TPPGData(const TPPGData&);
   TPPGData(TPPGData&&) = default;
   ~TPPGData()          = default;

   TPPGData& operator=(const TPPGData&) = default;
   TPPGData& operator=(TPPGData&&)      = default;

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
      fNewPpg = static_cast<EPpgPattern>(newPpg & 0xff);
      switch(fNewPpg) {
      case EPpgPattern::kBeamOn:
      case EPpgPattern::kDecay:
      case EPpgPattern::kTapeMove:
      case EPpgPattern::kBackground:
      case EPpgPattern::kJunk:
         break;
      default:
         if(newPpg != 0) { std::cout << "Warning, unknown ppg pattern " << hex(newPpg, 8) << ", setting new pattern to kJunk!" << std::endl; }
         fNewPpg = EPpgPattern::kJunk;
      }
   }
   void SetOldPPG(EPpgPattern oldPpg) { fOldPpg = oldPpg; }
   void SetOldPPG(UInt_t oldPpg)
   {
      fOldPpg = static_cast<EPpgPattern>(oldPpg & 0xff);
      switch(fOldPpg) {
      case EPpgPattern::kBeamOn:
      case EPpgPattern::kDecay:
      case EPpgPattern::kTapeMove:
      case EPpgPattern::kBackground:
      case EPpgPattern::kJunk:
         break;
      default:
         if(oldPpg != 0) { std::cout << "Warning, unknown ppg pattern " << hex(oldPpg, 8) << ", setting old pattern to kJunk!" << std::endl; }
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

   ULong64_t GetTimeStamp() const { return fTimeStamp; }

private:
   static int16_t fTimestampUnits;   ///< timestamp units of the PPG (10 ns)
   ULong64_t      fTimeStamp{0};     ///< time stamp in ns
   EPpgPattern    fOldPpg{EPpgPattern::kJunk};
   EPpgPattern    fNewPpg{EPpgPattern::kJunk};
   UInt_t         fNetworkPacketId{0};
   UInt_t         fLowTimeStamp{0};    ///< low bits of time stamp in 10 ns
   UInt_t         fHighTimeStamp{0};   ///< high bits of time stamp in 10 ns

   /// \cond CLASSIMP
   ClassDefOverride(TPPGData, 3) // NOLINT
   /// \endcond
};

class TPPG : public TSingleton<TPPG> {
public:
   friend class TSingleton<TPPG>;

   using PPGMap_t = std::map<ULong_t, TPPGData*>;

   TPPG();
   TPPG(const TPPG&);        // the copy constructor needs to create a deep-copy
   TPPG(TPPG&&) = default;   // the move constructor can be default?
   ~TPPG();

   TPPG& operator=(const TPPG&);        // the copy assignment needs to create a deep-copy
   TPPG& operator=(TPPG&&) = default;   // the move assignment can be default?

   void Copy(TObject& obj) const override;
   // why do we have a non-const version that just calls the const version?
   // the arguments are needed to match the TObject version and thus override it
   Int_t Write(const char* name = nullptr, Int_t option = 0, Int_t bufsize = 0) override
   {
      return static_cast<const TPPG*>(this)->Write(name, option, bufsize);
   }
   Int_t Write(const char* name = nullptr, Int_t option = 0, Int_t bufsize = 0) const override;

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   void Setup();
   bool Correct(bool verbose = false);

   void        AddData(TPPGData* pat);
   EPpgPattern GetStatus(ULong64_t time) const;
   EPpgPattern GetNextStatus(ULong64_t time) const;
   ULong64_t   GetLastStatusTime(ULong64_t time, EPpgPattern pat = EPpgPattern::kJunk) const;
   ULong64_t   GetNextStatusTime(ULong64_t time, EPpgPattern pat = EPpgPattern::kJunk) const;
   Bool_t      MapIsEmpty() const;
   std::size_t PPGSize() const { return fPPGStatusMap->size() - 1; }
   std::size_t OdbPPGSize() const { return fOdbPPGCodes.size(); }
   int16_t     OdbPPGCode(size_t index) const { return fOdbPPGCodes.at(index); }
   int         OdbDuration(size_t index) const { return fOdbDurations.at(index); }
   Long64_t    OdbCycleLength() const
   {
      Long64_t result = 0;
      for(auto dur : fOdbDurations) { result += dur; }
      return result;
   }
   Long64_t Merge(TCollection* list);
   void     Add(const TPPG* ppg);

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

   void SetOdbCycle(std::vector<int16_t> ppgCodes, std::vector<int> durations)
   {
      fOdbPPGCodes  = std::move(ppgCodes);
      fOdbDurations = std::move(durations);
   }

   bool OdbMatchesData(bool verbose = false);
   void SetOdbFromData(bool verbose = false);

private:
   bool CalculateCycleFromData(bool verbose = false);

   PPGMap_t::iterator MapBegin() const { return ++(fPPGStatusMap->begin()); }
   PPGMap_t::iterator MapEnd() const { return fPPGStatusMap->end(); }
   PPGMap_t::iterator fCurrIterator;   //!<!

   PPGMap_t*                fPPGStatusMap{nullptr};
   ULong64_t                fCycleLength{0};
   std::map<ULong64_t, int> fNumberOfCycleLengths;

   std::vector<int16_t> fOdbPPGCodes;    ///< ppg state codes read from odb
   std::vector<int>     fOdbDurations;   ///< duration of ppg state as read from odb

   bool                   fCycleSet{false};                //!<! flag to indicate whether the codes and durations have been calculated from the data
   std::vector<int16_t>   fPPGCodes{0x8, 0x2, 0x1, 0x4};   //!<! ppg state codes (these are always set)
   std::vector<ULong64_t> fDurations{0, 0, 0, 0};          //!<! duration of ppg state calculated from data

   /// \cond CLASSIMP
   ClassDefOverride(TPPG, 5) // NOLINT
   /// \endcond
};
/*! @} */
#endif
