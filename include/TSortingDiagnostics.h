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
#include <map>

#include "TObject.h"
#include "TH1F.h"

#include "TSortingDiagnostics.h"
#include "TPPG.h"
#include "TFragment.h"

class TSortingDiagnostics : public TSingleton<TSortingDiagnostics> {
public:
	friend class TSingleton<TSortingDiagnostics>;

   TSortingDiagnostics();
   TSortingDiagnostics(const TSortingDiagnostics&);
   ~TSortingDiagnostics() override;

private:
   // analysis tree diagnostics 
   std::map<long, std::pair<long, long>> fFragmentsOutOfOrder;
   std::map<double, std::pair<double, double>> fFragmentsOutOfTimeOrder;
   std::vector<Long_t> fPreviousTimeStamps; ///< timestamps of previous fragments, saved every 'BuildWindow' entries
   std::vector<double> fPreviousTimes;      ///< times of previous fragments, saved every 'BuildWindow' entries
   long                fMaxEntryDiff{0};
	std::map<TClass*, long> fMissingDetectorClasses; ///< counts of missing detector classes

	std::map<TClass*, std::pair<long, long> > fHitsRemoved; ///< removed hits and total hits per detector class

public:
   //"setter" functions
   void OutOfTimeOrder(double newFragTime, double oldFragTime, long newEntry);
   void OutOfOrder(long newFragTS, long oldFragTS, long newEntry);
   void AddTime(double val)      { fPreviousTimes.push_back(val); }
   void AddTimeStamp(Long_t val) { fPreviousTimeStamps.push_back(val); }
	void AddDetectorClass(TChannel*);
	void RemovedHits(TClass* detClass, long removed, long total);

   // getter functions
   size_t NumberOfFragmentsOutOfOrder() const { return fFragmentsOutOfOrder.size(); }
   std::map<long, std::pair<long, long>> FragmentsOutOfOrder() { return fFragmentsOutOfOrder; }
   size_t NumberOfFragmentsOutOfTimeOrder() const { return fFragmentsOutOfTimeOrder.size(); }
   std::map<double, std::pair<double, double>> FragmentsOutOfTimeOrder() { return fFragmentsOutOfTimeOrder; }
   long MaxEntryDiff() const { return fMaxEntryDiff; }

   // other functions
   void WriteToFile(const char*) const;

   void Copy(TObject&) const override;
   void Clear(Option_t* opt = "all") override;
   void Print(Option_t* opt = "") const override;
   void Draw(Option_t* opt = "") override;

   /// \cond CLASSIMP
   ClassDefOverride(TSortingDiagnostics, 2);
   /// \endcond
};
/*! @} */
#endif
