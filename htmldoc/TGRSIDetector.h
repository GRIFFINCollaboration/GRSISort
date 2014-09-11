#ifndef TGRSIDETECTOR_H
#define TGRSIDETECTOR_H

#include "Globals.h"

#include <cstdio>

#ifndef __CINT__
#endif

#include "TObject.h"

#include <TGRSIDetectorData.h>
#include <TFragment.h>
#include <TChannel.h>



class TGRSIDetector : public TObject	{
	public:
		TGRSIDetector();
		virtual ~TGRSIDetector();

	public: 
		virtual void BuildHits(TGRSIDetectorData *data=0,Option_t * = "") = 0;			   //!
		virtual void FillData(TFragment*,TChannel*,MNEMONIC*) = 0; //!

      virtual void Clear(Option_t *opt = "") = 0;		//!
		virtual void Print(Option_t *opt = "") = 0;		//!

      TGRSIDetectorData *GetData() { return data;}

	private: 

      TGRSIDetectorData *data;    //!

   ClassDef(TGRSIDetector,0)  
};


#endif
