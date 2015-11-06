#ifndef TRF_PHASE_H
#define TRF_PHASE_H

#include <vector>
#include <iostream>
#include <stdio.h>

#include "TFragment.h"
#include "TDetector.h"
#ifndef __CINT__
#include "TRFFitter.h"
#else
class TRFFitter;
#endif

class TRF : public TDetector {
	public:
		TRF();
		~TRF();

		Double_t Phase()     { return fPhase; }
		Double_t Time()      { return fTime; }
		Long_t   TimeStamp() { return fTimeStamp; }	
		time_t   MidasTime() { return fMidasTime; }

		void AddFragment(TFragment*, MNEMONIC*);	//!<!
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment

		void Clear(Option_t *opt = ""); 	      //!<!
		void Print(Option_t *opt = "") const; 	//!<!

	private:
		TRFFitter* fFitter;		            //!<!

		time_t fMidasTime;
		Long_t fTimeStamp;
		double fPhase;
		double fTime;

/// \cond CLASSIMP
		ClassDef(TRF,3)
/// \endcond
};
#endif
