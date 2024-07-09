#ifndef TSORTINGDIAGNOSTICS_H
#define TSORTINGDIAGNOSTICS_H

/** \addtogroup Sorting
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TSortingDiagnostics
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

#include "TObject.h"
#include "TH1F.h"

#include "TSortingDiagnostics.h"
#include "TPPG.h"
#include "TFragment.h"

class TSortingDiagnostics : public TSingleton<TSortingDiagnostics> {
public:
   friend class TSingleton<TSortingDiagnostics>;

   TSortingDiagnostics();
   //TSortingDiagnostics(const TSortingDiagnostics&);
   //TSortingDiagnostics(TSortingDiagnostics&&);
	//TSortingDiagnostics& operator=(const TSortingDiagnostics&);
	//TSortingDiagnostics& operator=(TSortingDiagnostics&&);
   //~TSortingDiagnostics() override;

private:
   // analysis tree diagnostics
   std::unordered_map<int64_t, std::pair<int64_t, int64_t>>       fFragmentsOutOfOrder;
   std::unordered_map<double, std::pair<double, double>>          fFragmentsOutOfTimeOrder;
   std::vector<Long_t>                                            fPreviousTimeStamps;   ///< timestamps of previous fragments, saved every 'BuildWindow' entries
   std::vector<double>                                            fPreviousTimes;        ///< times of previous fragments, saved every 'BuildWindow' entries
   int64_t                                                        fMaxEntryDiff{0};
   std::unordered_map<UInt_t, int64_t>                            fMissingChannels;          ///< counts of missing channels
   std::unordered_map<TClass*, int64_t>                           fMissingDetectorClasses;   ///< counts of missing detector classes

   std::unordered_map<TClass*, std::pair<int64_t, int64_t>> fHitsRemoved;   ///< removed hits and total hits per detector class

public:
   //"setter" functions
   void OutOfTimeOrder(double newFragTime, double oldFragTime, int64_t newEntry);
   void OutOfOrder(int64_t newFragTS, int64_t oldFragTS, int64_t newEntry);
   void AddTime(double val) { fPreviousTimes.push_back(val); }
   void AddTimeStamp(Long_t val) { fPreviousTimeStamps.push_back(val); }
   void MissingChannel(const UInt_t& address);
   void AddDetectorClass(TChannel*);
   void RemovedHits(TClass* detClass, int64_t removed, int64_t total);

   // getter functions
   size_t                                                   NumberOfFragmentsOutOfOrder() const { return fFragmentsOutOfOrder.size(); }
   std::unordered_map<int64_t, std::pair<int64_t, int64_t>> FragmentsOutOfOrder() { return fFragmentsOutOfOrder; }
   size_t                                                   NumberOfFragmentsOutOfTimeOrder() const { return fFragmentsOutOfTimeOrder.size(); }
   std::unordered_map<double, std::pair<double, double>>    FragmentsOutOfTimeOrder() { return fFragmentsOutOfTimeOrder; }
   int64_t                                                  MaxEntryDiff() const { return fMaxEntryDiff; }

   // other functions
   void WriteToFile(const char*) const;

   void Copy(TObject&) const override;
   void Clear(Option_t* opt = "all") override;
   void Print(Option_t* opt = "") const override;
   void Draw(Option_t* opt = "") override;

   /// \cond CLASSIMP
   ClassDefOverride(TSortingDiagnostics, 4);
   /// \endcond
};
/*! @} */
#endif
