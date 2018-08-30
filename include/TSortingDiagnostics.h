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
/// a midas file to a fragment tree and analysis tree and provides convenient
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
   std::vector<Long_t> fPreviousTimeStamps; ///< timestamps of previous fragments, saved every 'BuildWindow' entries
   long                fMaxEntryDiff{0};

public:
   //"setter" functions
   void OutOfOrder(long newFragTS, long oldFragTS, long newEntry);
   void AddTimeStamp(Long_t val) { fPreviousTimeStamps.push_back(val); }

   // getter functions
   size_t NumberOfFragmentsOutOfOrder() const { return fFragmentsOutOfOrder.size(); }
   std::map<long, std::pair<long, long>> FragmentsOutOfOrder() { return fFragmentsOutOfOrder; }
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
