#ifndef TACHIT_H
#define TACHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TTACHit
///
/// This is class that contains the information about a TAC
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"

class TTACHit : public TGRSIDetectorHit {
   public:
      TTACHit();
      virtual ~TTACHit();
      TTACHit(const TTACHit&);
      
   private:
      Int_t    fFilter;
      
   public:
      /////////////////////////		/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { fFilter   = x; }   //!<!
      
      /////////////////////////		/////////////////////////////////////
      inline Int_t    GetFilterPattern()    const     { return fFilter;   }  //!<!
      
      bool   InFilter(Int_t);                                          //!<!
      
   public:
      void Clear(Option_t *opt = "");		                    //!<!
      void Print(Option_t *opt = "") const;		                    //!<!
      virtual void Copy(TObject&) const;        //!<!
      
   private:
      TVector3 GetChannelPosition(Double_t dist = 0) const; //!<!
      
      /// \cond CLASSIMP
      ClassDef(TTACHit,2) //Stores the information for a TACrHit
      /// \endcond
};
/*! @} */
#endif
