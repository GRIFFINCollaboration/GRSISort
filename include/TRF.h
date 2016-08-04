#ifndef TRF_PHASE_H
#define TRF_PHASE_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <iostream>
#include <stdio.h>

#include "TMath.h"

#include "TFragment.h"
#include "TPulseAnalyzer.h"
#include "TDetector.h"

class TRF : public TDetector {
	public:
		TRF();
		TRF(const TRF&);
		~TRF();

		Double_t Phase() const     { return (fTime/fPeriod)*TMath::TwoPi(); }
		Double_t Time() const      { return fTime; }
		Long_t   TimeStamp() const { return fTimeStamp; }	
		time_t   MidasTime() const { return fMidasTime; }

		void AddFragment(TFragment*, TChannel*);	//!<!
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment

		void Copy(TObject&) const;
		void Clear(Option_t *opt = ""); 	      //!<!
		void Print(Option_t *opt = "") const; 	//!<!

	private:
		time_t fMidasTime;
		Long_t fTimeStamp;
		double fTime;

		static Double_t fPeriod;

/// \cond CLASSIMP
		ClassDef(TRF,4)
/// \endcond
};
/*! @} */
#endif
