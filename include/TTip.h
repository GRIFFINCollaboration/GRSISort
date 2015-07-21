#ifndef TTIP_H
#define TTIP_H

#include "Globals.h"

#include <vector>
#include <cstdio>
#include <iostream>
#include <set>
#include <stdio.h>

#include "TTipHit.h"
#ifndef __CINT__
#include "TTipData.h"
#else
class TTipData;
#endif
#include "TVector3.h" 

#include "TGRSIDetector.h" 
#include "TObject.h"

class TTip : public TGRSIDetector {

	public:

		virtual ~TTip();
		TTip();	
      TTip(const TTip& rhs);

		TTipHit *GetTipHit(const int i) ;//!
      TGRSIDetectorHit *GetHit(const int i);
		Short_t GetMultiplicity() const	       {	return tip_hits.size();}	//!

		void BuildHits(TGRSIDetectorData *data =0,Option_t *opt = "");           //!
		void FillData(TFragment*,TChannel*,MNEMONIC*);                           //!
      void Copy(TTip &rhs) const;

      TTip& operator=(const TTip&);  //!

		void Clear(Option_t *opt = "");
		void Print(Option_t *opt = "");

   protected:
     void PushBackHit(TGRSIDetectorHit* tiphit);

	private:

		TTipData *tipdata;                                               //!  Used to build TIP Hits
		std::vector <TTipHit> tip_hits;                                  //   The set of detector hits

	public:

	ClassDef(TTip,1);

}; 

#endif

