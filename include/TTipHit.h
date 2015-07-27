#ifndef TTIPHIT_H
#define TTIPHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
#include "TCrystalHit.h"

#include "TVector3.h"


#include "TGRSIDetectorHit.h"

class TTipHit : public TGRSIDetectorHit {
  public:
    TTipHit();
    virtual ~TTipHit();
    TTipHit(const TTipHit&);

  private:
    Int_t    filter;
	 Double_t PID;
   
  public:
		/////////////////////////		/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { filter   = x; }   //! 
  //    inline void SetPosition(TVector3 x)          { position = x; }   //!
      inline void SetPID(Double_t x)          		{ PID = x; }   //!

      inline Int_t    GetFiterPatter()         {   return filter;   }  //!
		inline Double_t GetPID()			        {	return PID;     }  //!
      TVector3 GetPosition() const {return TVector3();}

      bool   InFilter(Int_t);                                         //!

	public:
		void Clear(Option_t *opt = "");		                    //!
		void Print(Option_t *opt = "");		                    //!
      virtual void Copy(TTipHit&) const;        //!

		ClassDef(TTipHit,1);

};

#endif
