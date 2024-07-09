// Author: Peter C. Bender    06/14

/** \addtogroup Sorting
 *  @{
 */

#ifndef TFRAGMENT_H
#define TFRAGMENT_H

#include "Globals.h"
#include "TDetectorHit.h"
#include "TPPG.h"

#include <iostream>
#include <vector>
#include <ctime>

#include "Rtypes.h"

/////////////////////////////////////////////////////////////////
///
/// \class TFragment
///
/// This Class contains all of the information in an event
/// fragment
///
/////////////////////////////////////////////////////////////////

class TFragment : public TDetectorHit {
public:
   TFragment();
   TFragment(const TFragment&) = default;
   TFragment(TFragment&&)      = default;
   ~TFragment()                = default;

   TFragment& operator=(const TFragment&) = default;   // use default assignment operator (to shut up gcc 9.1)
   TFragment& operator=(TFragment&&)      = default;   // use default move assignment operator (to shut up gcc 9.1)

   //////////////////// basic setter functions ////////////////////

   void SetAcceptedChannelId(UShort_t value) { fAcceptedChannelId = value; }
   void SetCcLong(Int_t value) { fCcLong = value; }
   void SetCcShort(Int_t value) { fCcShort = value; }
   void SetChannelId(UInt_t value) { fChannelId = value; }
   void SetModuleType(UShort_t value) { fModuleType = value; }
   void SetDeadTime(UShort_t value) { fDeadTime = value; }
   void SetDetectorType(UShort_t value) { fDetectorType = value; }
   void SetEntryNumber() { fEntryNumber = fNumberOfFragments++; }
   void SetDaqId(Int_t value) { fDaqId = value; }
   void SetFragmentId(Int_t value) { fFragmentId = value; }
   void SetDaqTimeStamp(time_t value) { fDaqTimeStamp = value; }
   void SetNetworkPacketNumber(Int_t value) { fNetworkPacketNumber = value; }
   void SetNumberOfFilters(UShort_t)
   {
      std::cerr << "Error, " << __PRETTY_FUNCTION__ << " called, TFragment shouldn't have a number of filters." << std::endl;
   }
   void SetNumberOfPileups(Short_t value) { fNumberOfPileups = value; }
   void SetNumberOfWords(UShort_t value) { fNumberOfWords = value; }
   void SetTriggerBitPattern(Int_t value) { fTriggerBitPattern = value; }
   void SetTriggerId(Long_t value) { fTriggerId.push_back(value); }
   void SetZc(Int_t value) { fZc = value; }

   //////////////////// basic getter functions ////////////////////

   UShort_t GetAcceptedChannelId() const { return fAcceptedChannelId; }
   Int_t    GetCcLong() const { return fCcLong; }
   Int_t    GetCcShort() const { return fCcShort; }
   UInt_t   GetChannelId() const { return fChannelId; }
   Long64_t GetEntryNumber() const { return fEntryNumber; }
   UShort_t GetModuleType() const { return fModuleType; }
   UShort_t GetDeadTime() const { return fDeadTime; }
   UShort_t GetDetectorType() const { return fDetectorType; }
   Int_t    GetDaqId() const { return fDaqId; }
   Int_t    GetFragmentId() const { return fFragmentId; }
   time_t   GetDaqTimeStamp() const { return fDaqTimeStamp; }
   Int_t    GetNetworkPacketNumber() const { return fNetworkPacketNumber; }
   UShort_t GetNumberOfFilters() const { return fNumberOfWords - 9; }
   Short_t  GetNumberOfPileups() const { return fNumberOfPileups; }
   UShort_t GetNumberOfWords() const { return fNumberOfWords; }
   Int_t    GetTriggerBitPattern() const { return fTriggerBitPattern; }
   Long_t   GetTriggerId(size_t iter = 0) const
   {
      if(iter < fTriggerId.size()) {
         return fTriggerId[iter];
      }
      return 0;
   }
   Int_t GetZc() const { return fZc; }

   //////////////////// advanced getter functions ////////////////////

   TPPG*     GetPPG();
   double    GetTZero() const;
   ULong64_t GetTimeInCycle();
   ULong64_t GetCycleNumber();
   Int_t     Get4GCfd() const;

   //////////////////// misc. functions ////////////////////
   bool IsDetector(const char* prefix, Option_t* opt = "CA") const;

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   void Print(std::ostream& out) const override;

   TObject* Clone(const char* name = "") const override;

   bool operator<(const TFragment& rhs) const { return (GetTimeStamp() < rhs.GetTimeStamp()); }
   bool operator>(const TFragment& rhs) const { return (GetTimeStamp() > rhs.GetTimeStamp()); }

   Int_t GetSharcMesyBoard() const;

private:
   //////////////////// data members, sorted by size (as far as possible) to reduce padding ////////////////////
   time_t fDaqTimeStamp;          ///< Timestamp of the Daq event
   Int_t  fDaqId;                 ///< Daq ID
   Int_t  fFragmentId;            ///< Channel Trigger ID ??? not needed anymore ???
   Int_t  fTriggerBitPattern;     ///< PrimaryFilterPattern in Griffin DAQ
   Int_t  fNetworkPacketNumber;   ///< Network packet number
   UInt_t fChannelId;             ///< Threshold crossing counter for a channel
   UInt_t fAcceptedChannelId;     ///< Accepted threshold crossing counter for a channel

   /// Added to combine Grif Fragment  ////

   UShort_t fDeadTime;          ///< Deadtime from trigger
   UShort_t fModuleType;        ///< Data Type (GRIF-16, 4G, etc.)
   UShort_t fDetectorType;      ///< Detector Type (PACES,HPGe, etc.)
   Short_t  fNumberOfPileups;   ///< Number of piled up hits 1-3

   std::vector<Long_t> fTriggerId;   ///<  PrimaryFilterID in Griffin DAQ

   //////////////////// transient members ////////////////////
   TPPG* fPPG;   //!<! Programmable pattern generator value

   Long64_t fEntryNumber;     //!<! Entry number in fragment tree
   Int_t    fZc;              //!<! Zero-crossing value from 4G (saved in separate branch)
   Int_t    fCcShort;         //!<! Short integration over waveform peak from 4G (saved in separate branch)
   Int_t    fCcLong;          //!<! Long integration over waveform tail from 4G (saved in separate branch)
   UShort_t fNumberOfWords;   //!<! Number of non-waveform words in fragment, only used for check while parsing the fragment

   static Long64_t fNumberOfFragments;

   // int NumberOfHits;  //!<! transient member to count the number of pile-up hits in the original fragment
   // int HitIndex;    //!<! transient member indicating which pile-up hit this is in the original fragment

   /// \cond CLASSIMP
   ClassDefOverride(TFragment, 8);   // Event Fragments
   /// \endcond
};
/*! @} */
#endif   // TFRAGMENT_H
