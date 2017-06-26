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
/// a midas file to a fragment tree and analysis tree and provides convenient
/// methods of printing and/or visualizing them.
///
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>

#ifndef __CINT__
#include <memory>
#endif

#include "TObject.h"
#include "TH1F.h"

#include "TPPG.h"
#include "TFragment.h"

class TParsingDiagnostics : public TObject {
public:
   TParsingDiagnostics();
   TParsingDiagnostics(const TParsingDiagnostics&);
   ~TParsingDiagnostics() override;
   static TParsingDiagnostics* Get()
   {
      if(fParsingDiagnostics == nullptr) {
         fParsingDiagnostics = new TParsingDiagnostics;
      }
      return fParsingDiagnostics;
   }

private:
   // fragment tree diagnostics (should these all be static?)
   std::map<Short_t, Long_t> fNumberOfGoodFragments; ///< map of number of good fragments per detector type
   std::map<Short_t, Long_t> fNumberOfBadFragments;  ///< map of number of bad fragments per detector type

   std::map<Short_t, UInt_t> fMinChannelId; ///< map of minimum channel id per channel number
   std::map<Short_t, UInt_t> fMaxChannelId; ///< map of maximum channel id per channel number

   std::map<Short_t, Long_t> fNumberOfHits; ///< map of number of hits per channel number

   std::map<Short_t, long> fDeadTime;     ///< map of deadtime per channel number
   std::map<Short_t, long> fMinTimeStamp; ///< map of minimum timestamp per channel number
   std::map<Short_t, long> fMaxTimeStamp; ///< map of maximum timestamp per channel number

   time_t fMinMidasTimeStamp{}; ///< minimum midas timestamp
   time_t fMaxMidasTimeStamp{}; ///< maximum midas timestamp

   Int_t fMinNetworkPacketNumber{}; ///< minimum network packet id
   Int_t fMaxNetworkPacketNumber{}; ///< maximum network packet id

   Long_t fNumberOfNetworkPackets{};

   // ppg diagnostics
   ULong64_t fPPGCycleLength{};

   //
   TH1F* fIdHist; ///< histogram of event survival

   static TParsingDiagnostics* fParsingDiagnostics;

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
   ClassDefOverride(TParsingDiagnostics, 1);
   /// \endcond
};
/*! @} */
#endif
