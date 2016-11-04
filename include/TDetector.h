#ifndef TDETECTOR_H
#define TDETECTOR_H

/** \addtogroup Detectors
 *  @{
 */

#include <stdexcept>
#include <cstdio>
#include <vector>

#include "TVector3.h"
#include "TObject.h"

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"

/////////////////////////////////////////////////////////////////
///
/// \class TDetector
///
/// This is an abstract class that contains the basic info
/// about a detector. This is where the hits are built and
/// the data is filled. It's main role is to act as a wrapper
/// for every other type of detector system.
///
/////////////////////////////////////////////////////////////////


class TDetector : public TObject	{
	public:
			
	enum EBitFlag {
		//Reference for the TObject fBits that are reserved for derived classes.
		kDetBit0 = BIT(14),
		kDetBit1 = BIT(15),
		kDetBit2 = BIT(16),
		kDetBit3 = BIT(17),
		kDetBit4 = BIT(18),
		kDetBit5 = BIT(19),
		kDetBit6 = BIT(20),
		kDetBit7 = BIT(21),
		kDetBit8 = BIT(22),
		kDetBit9 = BIT(23),
		kDetBit10= BIT(24),
	};
		
		TDetector();
		TDetector(const TDetector&);
		virtual ~TDetector();
		TDetector &operator= (const TDetector& other) {
			if(this != &other) 
				other.Copy(*this);
			return *this;
		}
		
	

	public: 
		virtual void BuildHits()                                   { AbstractMethod("BuildHits()"); } //!<!
		virtual void AddFragment(TFragment*, TChannel*)            { AbstractMethod("AddFragment()"); } //!<!

		virtual void ClearBits(){ResetBit(0x000ffc00);} // Zero the inheritance reserved bits of TObject
		
		virtual void Copy(TObject&) const;              //!<!
		virtual void Clear(Option_t* opt = "");         //!<!
		virtual void Print(Option_t* opt = "") const;   //!<!

/// \cond CLASSIMP
		ClassDef(TDetector,1) //Abstract class for detector systems 
/// \endcond
};
/*! @} */
#endif
