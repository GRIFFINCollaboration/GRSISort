#ifndef TDETECTOR_H
#define TDETECTOR_H

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
		TDetector();
		TDetector(const TDetector&);
		virtual ~TDetector();
		TDetector &operator= (const TDetector& other) {
			if(this != &other) 
				other.Copy(*this);
			return *this;
		}

	public: 
		virtual void BuildHits()                                        { AbstractMethod("BuildHits()"); } //!<!
#ifndef __CINT__
		virtual void AddFragment(std::shared_ptr<const TFragment>, TChannel*) { AbstractMethod("AddFragment()"); } //!<!
#endif

		virtual void Copy(TObject&) const;              //!<!
		virtual void Clear(Option_t* opt = "");         //!<!
      virtual void ClearTransients() {}               //!<!
		virtual void Print(Option_t* opt = "") const;   //!<!

/// \cond CLASSIMP
		ClassDef(TDetector,1) //Abstract class for detector systems 
/// \endcond
};
/*! @} */
#endif
