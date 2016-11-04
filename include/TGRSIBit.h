#ifndef TGBIT_H
#define TGBIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <stdexcept>
#include <cstdio>
#include <vector>
#ifndef __CINT__
#include <memory>
#endif

#include "TVector3.h"
#include "TObject.h"

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"


////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIBit                                              //
//                                                            //
// This is an abstract class for mutable bit setting
//                                                            //
////////////////////////////////////////////////////////////////


class TGRSIBit {
	public:
			
	enum EGRSIBitFlag {
		kGRSIBit0 = BIT(0),
		kGRSIBit1 = BIT(1),
		kGRSIBit2 = BIT(2),
		kGRSIBit3 = BIT(3),
		kGRSIBit4 = BIT(4),
		kGRSIBit5 = BIT(5),
		kGRSIBit6 = BIT(6),
		kGRSIBit7 = BIT(7),
	};
		
	TGRSIBit(){Clear();}
	virtual ~TGRSIBit(){};
	
	void Clear(){fGRSIBit=0;}
	
	void SetGBit(UInt_t flag, Bool_t set) const {
	if(set)
		fGRSIBit |= flag;
	else
		fGRSIBit &= (~flag);
	}
	
	bool TestGBit(UInt_t flag) const { return fGRSIBit & flag; }
	
	private: 
	mutable UInt_t fGRSIBit;

/// \cond CLASSIMP
		ClassDef(TGRSIBit,1) //Abstract class for detector systems 
/// \endcond
};
/*! @} */
#endif
