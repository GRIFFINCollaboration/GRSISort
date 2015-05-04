#ifndef TGRSIDETECTOR_H
#define TGRSIDETECTOR_H

#include "Globals.h"

#include <cstdio>
#include <vector>
#include "TVector3.h"

#ifndef __CINT__
#endif

#include "TObject.h"
#ifndef __CINT__
#include "TGRSIDetectorData.h"
#else
class TGRSIDetectorData;
#endif

#include "TFragment.h"
#include "TChannel.h"

////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIDetector                                              //
//                                                            //
// This is an abstract class that contains the basic info     //
// about a detector. This is where the hits are built and
// the data is filled.
//                                                            //
////////////////////////////////////////////////////////////////


class TGRSIDetector : public TObject	{
	public:
		TGRSIDetector();
		virtual ~TGRSIDetector();

	public: 
		virtual void BuildHits(TGRSIDetectorData *data=0,Option_t * = "") = 0;			   //!
		virtual void FillData(TFragment*,TChannel*,MNEMONIC*) = 0; //!

      virtual void Clear(Option_t *opt = "");		//!
		virtual void Print(Option_t *opt = "") const;		//!

      //virtual TGRSIDetectorData *GetData() //{ //return data;}

      //TGRSIDetectorData *data;    //!

   ClassDef(TGRSIDetector,0) //Abstract class for detector systems 
};


#endif
