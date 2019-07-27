#ifndef TPARSINGDIAGNOSTICS_H
#define TPARSINGDIAGNOSTICS_H

/** \addtogroup Sorting
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TParsingDiagnostics
///
/// This class gathers various diagnostics calculated during the sorting from
/// a raw file to a fragment tree and analysis tree and provides convenient
/// methods of printing and/or visualizing them.
///
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>

#ifndef __CINT__
#include <memory>
#endif

#include "TObject.h"
#include "TH1F.h"

#include "TSingleton.h"
#include "TPPG.h"
#include "TFragment.h"

class TParsingDiagnostics : public TSingleton<TParsingDiagnostics> {
public:
	friend class TSingleton<TParsingDiagnostics>;

   TParsingDiagnostics();
   TParsingDiagnostics(const TParsingDiagnostics&);
   ~TParsingDiagnostics() override;

private:
   // fragment tree diagnostics (should these all be static?)
	// detector type unordered_maps
   std::unordered_map<Short_t, Long_t> fNumberOfGoodFragments; ///< unordered_map of number of good fragments per detector type
   std::unordered_map<Short_t, Long_t> fNumberOfBadFragments;  ///< unordered_map of number of bad fragments per detector type

	// channel address unordered_maps
   std::unordered_map<UInt_t, UInt_t> fMinChannelId; ///< unordered_map of minimum channel id per channel address
   std::unordered_map<UInt_t, UInt_t> fMaxChannelId; ///< unordered_map of maximum channel id per channel address

   std::unordered_map<UInt_t, Long_t> fNumberOfHits; ///< unordered_map of number of hits per channel address

   std::unordered_map<UInt_t, long> fDeadTime;     ///< unordered_map of deadtime per channel address
   std::unordered_map<UInt_t, long> fMinTimeStamp; ///< unordered_map of minimum timestamp per channel address
   std::unordered_map<UInt_t, long> fMaxTimeStamp; ///< unordered_map of maximum timestamp per channel address

   time_t fMinDaqTimeStamp; ///< minimum daq timestamp
   time_t fMaxDaqTimeStamp; ///< maximum daq timestamp

   Int_t fMinNetworkPacketNumber; ///< minimum network packet id
   Int_t fMaxNetworkPacketNumber; ///< maximum network packet id

   Long_t fNumberOfNetworkPackets;

   // ppg diagnostics
   ULong64_t fPPGCycleLength;

   //
   TH1F* fIdHist; ///< histogram of event survival

public:
//"setter" functions
#ifndef __CINT__
   void GoodFragment(const std::shared_ptr<const TFragment>&);
#endif
   void GoodFragment(Short_t detType) { fNumberOfGoodFragments[detType]++; }
   void BadFragment(Short_t detType) { fNumberOfBadFragments[detType]++; }

   void ReadPPG(TPPG*);

   // getter functions
   Long_t NumberOfGoodFragments(Short_t detType)
   {
      if(fNumberOfGoodFragments.find(detType) != fNumberOfGoodFragments.end()) {
         return fNumberOfGoodFragments[detType];
      }
      return 0;
   }
   Long_t NumberOfBadFragments(Short_t detType)
   {
      if(fNumberOfBadFragments.find(detType) != fNumberOfBadFragments.end()) {
         return fNumberOfBadFragments[detType];
      }
      return 0;
   }

   ULong64_t PPGCycleLength() { return fPPGCycleLength; }

   // other functions
   void WriteToFile(const char*) const;

   void Copy(TObject&) const override;
   void Clear(Option_t* opt = "all") override;
   void Print(Option_t* opt = "") const override;
   void Draw(Option_t* opt = "") override;

   /// \cond CLASSIMP
   ClassDefOverride(TParsingDiagnostics, 2);
   /// \endcond
};
/*! @} */
#endif
