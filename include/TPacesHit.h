#ifndef PACESHIT_H
#define PACESHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TVirtualFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorHit.h"

class TPacesHit : public TGRSIDetectorHit {
	public:
		TPacesHit();
		TPacesHit(const TPacesHit&);
		virtual ~TPacesHit();

	private:
      Int_t fFilter;

	public:
		/////////////////////////  Setters	/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { fFilter = x;   }                  //!<! 
      //void SetHit();

		/////////////////////////  Getters	/////////////////////////////////////
      inline Int_t    GetFilterPattern() const         {   return fFilter;   }          //!<!

		/////////////////////////  TChannel Helpers /////////////////////////////////////
      bool   InFilter(Int_t);  //!<!

	public:
		virtual void Clear(Option_t *opt = "");		 //!<!
		virtual void Print(Option_t *opt = "") const; //!<!
      virtual void Copy(TObject&) const;        //!<!

   private:
      TVector3 GetChannelPosition(Double_t dist = 0) const; //!<!

/// \cond CLASSIMP
	ClassDef(TPacesHit,3);
/// \endcond
};
/*! @} */
#endif
