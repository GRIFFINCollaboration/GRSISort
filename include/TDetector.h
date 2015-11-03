#ifndef TDETECTOR_H
#define TDETECTOR_H

#include <stdexcept>
#include <cstdio>
#include <vector>

#include "TVector3.h"
#include "TObject.h"

#include "Globals.h"
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
		TDetector &operator= (const TDetector& other) {
			if(this != &other) 
				other.Copy(*this);
			return *this;
		}

	public: 
		virtual void BuildHits()                                   { AbstractMethod("BuildHits()"); } //! = 0; //!
		virtual void AddFragment(TFragment*, MNEMONIC*)            { AbstractMethod("AddFragment()"); } //! = 0; //!

		virtual void Copy(TObject&) const;              //!
		virtual void Clear(Option_t* opt = "");         //!
		virtual void Print(Option_t* opt = "") const;   //!

		ClassDef(TDetector,1) //Abstract class for detector systems 
};

#endif
