#ifndef TRF_PHASE_H
#define TRF_PHASE_H


#include <vector>
#include <iostream>
#include <stdio.h>

#include "TMath.h"
#include "TDetector.h"
#include "TPulseAnalyzer.h"

#include "TFragment.h"

static const Double_t period_ns=84.409;

class TRF :  public TDetector {
		
	public:
	
	TRF();
	TRF(const TRF&);
	virtual ~TRF();
		
	Double_t Time() const     { return time;}
	Long_t   TimeStamp() const  { return timestamp; }	
	time_t   MidasTime() const  { return midastime; }
	Double_t Phase() const      { return (time/period_ns)*TMath::TwoPi(); }
	
	
	void BuildHits(TDetectorData *data=0, Option_t * = "");
	void FillData(TFragment*,TChannel*,MNEMONIC*);	
	
	void Copy(TObject&) const;
	void Clear(Option_t *opt = ""); 	 
	void Print(Option_t *opt = "") const;

	private:

        time_t midastime;
        Long_t timestamp;
	double time;
		
	ClassDef(TRF,3)

};



#endif
