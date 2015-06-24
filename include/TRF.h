#ifndef TRF_PHASE_H
#define TRF_PHASE_H


#include <vector>
#include <iostream>
#include <stdio.h>

#include "TGRSIDetector.h"

#include "TFragment.h"
#ifndef __CINT__
#include "TRFFitter.h"
#else
class TRFFitter;
#endif

using namespace std;

class TRF :  public TGRSIDetector {
	
	public:
		TRF();
		~TRF();

		//std::vector<Short_t> GetWave() { return rf_wave;	};
		
      Double_t Phase()     { return phase; }
		Double_t Time()      { return time; }
		Long_t   TimeStamp() { return timestamp; }	
      time_t   MidasTime() { return midastime; }

		//bool HasWave() { return !rf_wave.empty(); };

		void BuildHits(TGRSIDetectorData *data=0, Option_t * = "");	//!
		void FillData(TFragment*,TChannel*,MNEMONIC*);	//!

		void Clear(Option_t *opt = ""); 	      //!
		void Print(Option_t *opt = "") const; 	//!

		
	
	private:
		TRFFitter *data;		            //!

		//std::vector<Short_t> rf_wave;
      time_t midastime;
      Long_t timestamp;
		double phase;
		double time;
		
	ClassDef(TRF,2)

};



#endif
