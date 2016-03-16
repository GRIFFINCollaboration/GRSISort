#ifndef TTIP_H
#define TTIP_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///                    
/// \class TTip
///
/// The TTip class defines the observables and algorithms used
/// when analyzing TIP data. It includes detector positions,
/// etc. 
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>
#include <iostream>
#include <set>
#include <stdio.h>

#include "TObject.h"
#include "TVector3.h" 

#include "Globals.h"
#include "TGRSIDetector.h" 
#include "TTipHit.h"

class TTip : public TGRSIDetector {
	public:
		virtual ~TTip();
		TTip();  
		TTip(const TTip& rhs);

		TTipHit* GetTipHit(const int& i) ;//!<!
		TGRSIDetectorHit* GetHit(const int& i);
		Short_t GetMultiplicity() const         {  return fTipHits.size();}  //!<!

		void AddFragment(TVirtualFragment*, MNEMONIC*); //!<!
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment
		void Copy(TObject &rhs) const;

		TTip& operator=(const TTip&);  //!<!

		void Clear(Option_t* opt = "");
		void Print(Option_t* opt = "") const;

	protected:
		void PushBackHit(TGRSIDetectorHit* tiphit);

	private:
		std::vector <TTipHit> fTipHits;                                  //   The set of detector hits

	public:
/// \cond CLASSIMP
		ClassDef(TTip,2);
/// \endcond
}; 
/*! @} */
#endif
