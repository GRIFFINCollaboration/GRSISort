#ifndef TDETECTOR_H
#define TDETECTOR_H

#include "Globals.h"

#include <cstdio>
#include <vector>
#include "TVector3.h"

#ifndef __CINT__
#endif

#include "TObject.h"
#ifndef __CINT__
#include "TDetectorData.h"
#else
class TDetectorData;
#endif

#include "TFragment.h"
#include "TChannel.h"

////////////////////////////////////////////////////////////////
//                                                            //
// TDetector                                                  //
//                                                            //
// This is an abstract class that contains the basic info     //
// about a detector. This is where the hits are built and
// the data is filled.
//                                                            //
////////////////////////////////////////////////////////////////


class TDetector : public TObject	{
	public:
		TDetector();
		TDetector(const TDetector&);
		virtual ~TDetector();

	public: 
      //virtual TDetectorHit* GetHit(const Int_t idx = 0) { AbstractMethod("GetHit()"); return 0;}
      //Might remove TGRSIDetectorData and make a TDetectorData? or at least fix the inheritance.
		virtual void BuildHits(TDetectorData *data=0,Option_t * = "") { AbstractMethod("BuildHits()"); } //! = 0; //!
		virtual void FillData(TFragment*,TChannel*,MNEMONIC*)             { AbstractMethod("FillData()");  } //! = 0; //!

      virtual void Copy(TObject&) const;              //!
      virtual void Clear(Option_t *opt = "");         //!
		virtual void Print(Option_t *opt = "") const;   //!

   ClassDef(TDetector,1) //Abstract class for detector systems 
};


#endif
