#ifndef PACESHIT_H
#define PACESHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"


class TPacesHit : public TGRSIDetectorHit {
	public:
		TPacesHit();
		TPacesHit(const TPacesHit&);
		virtual ~TPacesHit();

	private:
      Int_t filter;
      Int_t ppg;

   //flags
   private:
      Bool_t is_crys_set; //!

	public:
		/////////////////////////  Setters	/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { filter = x;   }                  //! 
      inline void SetPPG(const int &x)             { ppg = x;   }                     //! 
      //void SetHit();
      virtual double GetTime(Option_t *opt = "") const;                                 //!

      TVector3 GetPosition(Double_t dist = 0.0) const; //!

		/////////////////////////  Getters	/////////////////////////////////////
      inline Int_t    GetFilterPattern() const         {   return filter;   }          //!
      inline Int_t    GetPPG() const                  {   return ppg;   }             //!

		/////////////////////////  TChannel Helpers /////////////////////////////////////
      bool   InFilter(Int_t);  //!

	public:
		virtual void Clear(Option_t *opt = "");		 //!
		virtual void Print(Option_t *opt = "") const; //!
      virtual void Copy(TPacesHit&) const;        //!

	ClassDef(TPacesHit,2);
};




#endif
