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

class TParsingDiagnosticsData : public TObject {
public:
   TParsingDiagnosticsData();
   explicit TParsingDiagnosticsData(const std::shared_ptr<const TFragment>& frag);
	TParsingDiagnosticsData(const TParsingDiagnosticsData&) = default;
	TParsingDiagnosticsData(TParsingDiagnosticsData&&) noexcept = default;
	TParsingDiagnosticsData& operator=(const TParsingDiagnosticsData&) = default;
	TParsingDiagnosticsData& operator=(TParsingDiagnosticsData&&) noexcept = default;
   ~TParsingDiagnosticsData() = default;

   void Update(const std::shared_ptr<const TFragment>& frag);
   using TObject::Print;
   void Print(UInt_t address) const;

   // getters
   UInt_t MinChannelId() const { return fMinChannelId; }
   UInt_t MaxChannelId() const { return fMaxChannelId; }

   Long_t NumberOfHits() const { return fNumberOfHits; }

   int64_t DeadTime() const { return fDeadTime; }
   int64_t MinTimeStamp() const { return fMinTimeStamp; }
   int64_t MaxTimeStamp() const { return fMaxTimeStamp; }

private:
   UInt_t fMinChannelId{0};   ///< minimum channel id per channel address
   UInt_t fMaxChannelId{0};   ///< maximum channel id per channel address

   Long_t fNumberOfHits{0};   ///< number of hits per channel address

   int64_t fDeadTime{0};       ///< deadtime per channel address
   int64_t fMinTimeStamp{0};   ///< minimum timestamp per channel address
   int64_t fMaxTimeStamp{0};   ///< maximum timestamp per channel address

   /// \cond CLASSIMP
   ClassDefOverride(TParsingDiagnosticsData, 1) // NOLINT
   /// \endcond
};

class TParsingDiagnostics : public TSingleton<TParsingDiagnostics> {
public:
   friend class TSingleton<TParsingDiagnostics>;

   TParsingDiagnostics();
   TParsingDiagnostics(const TParsingDiagnostics&);
	TParsingDiagnostics(TParsingDiagnostics&&) noexcept = default;
	TParsingDiagnostics& operator=(const TParsingDiagnostics&) = default;
	TParsingDiagnostics& operator=(TParsingDiagnostics&&) noexcept = default;
   ~TParsingDiagnostics();

private:
   // fragment tree diagnostics (should these all be static?)
   // detector type unordered_maps
   std::unordered_map<Short_t, Long_t> fNumberOfGoodFragments;   ///< unordered_map of number of good fragments per detector type
   std::unordered_map<Short_t, Long_t> fNumberOfBadFragments;    ///< unordered_map of number of bad fragments per detector type

   // channel address unordered_maps
   std::unordered_map<UInt_t, TParsingDiagnosticsData> fChannelAddressData;   ///< unordered_map of data per channel address

   time_t fMinDaqTimeStamp{0};   ///< minimum daq timestamp
   time_t fMaxDaqTimeStamp{0};   ///< maximum daq timestamp

   Int_t fMinNetworkPacketNumber{0x7fffffff};   ///< minimum network packet id
   Int_t fMaxNetworkPacketNumber{0};            ///< maximum network packet id

   Long_t fNumberOfNetworkPackets{0};

   // ppg diagnostics
   ULong64_t fPPGCycleLength{0};

   TH1F* fIdHist{nullptr};   ///< histogram of event survival

public:
//"setter" functions
#ifndef __CINT__
   void GoodFragment(const std::shared_ptr<const TFragment>&);
#endif
   void GoodFragment(Short_t detType)
   {
      fNumberOfGoodFragments[detType]++;
   }
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

   ULong64_t PPGCycleLength() const { return fPPGCycleLength; }

   // other functions
   void WriteToFile(const char*) const;

   void Copy(TObject&) const override;
   void Clear(Option_t* opt = "all") override;
   void Print(Option_t* opt = "") const override;
   void Draw(Option_t* opt = "") override;

   /// \cond CLASSIMP
   ClassDefOverride(TParsingDiagnostics, 2); // NOLINT
   /// \endcond
};
/*! @} */
#endif
