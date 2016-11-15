#ifndef PACESHIT_H
#define PACESHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorHit.h"

class TPacesHit : public TGRSIDetectorHit {
	public:
		TPacesHit();
		TPacesHit(const TPacesHit&);
      TPacesHit(const TFragment& frag) : TGRSIDetectorHit(frag) {} 
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
      TVector3 GetPosition(Double_t dist) const; //!<!
      TVector3 GetPosition() const; //!<!

   private:
      Double_t GetDefaultDistance() const { return 0.0; }

/// \cond CLASSIMP
	ClassDef(TPacesHit,3);
/// \endcond
};
/*! @} */
#endif
