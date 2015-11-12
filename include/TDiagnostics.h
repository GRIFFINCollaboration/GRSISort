#ifndef TDIAGNOSTICS_H
#define TDIAGNOSTICS_H

////////////////////////////////////////////////////////////////////////////////
///
/// \class TDiagnostics
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

#include "TPPG.h"
#include "TFragment.h"

class TDiagnostics : public TObject {
	public:
		TDiagnostics();
		TDiagnostics(const TDiagnostics&);
		~TDiagnostics();

	private:
		//fragment tree diagnostics
		std::map<Short_t, Long_t> fNumberOfGoodFragments;  ///< map of number of good fragments per detector type
		std::map<Short_t, Long_t> fNumberOfBadFragments;   ///< map of number of bad fragments per detector type

		//ppg diagnostics
		ULong64_t fPPGCycleLength;
	
	public:
		//"setter" functions
		void GoodFragment(TFragment*);
		void GoodFragment(Short_t detType) { fNumberOfGoodFragments[detType]++; }
		void BadFragment(Short_t detType)  { fNumberOfBadFragments[detType]++; }

		void Read(TPPG*);

		//getter functions
		Long_t NumberOfGoodFragments(Short_t detType) { if(fNumberOfGoodFragments.find(detType) != fNumberOfGoodFragments.end()) return fNumberOfGoodFragments[detType]; return 0; }
		Long_t NumberOfBadFragments(Short_t detType) { if(fNumberOfBadFragments.find(detType) != fNumberOfBadFragments.end()) return fNumberOfBadFragments[detType]; return 0; }

		ULong64_t PPGCycleLength() { return fPPGCycleLength; }

		//other functions
		void Copy(TObject&) const;
		void Clear(Option_t* opt = "all");
		void Print(Option_t* opt = "") const;

/// \cond CLASSIMP
	ClassDef(TDiagnostics,1);
/// \endcond
};
#endif
