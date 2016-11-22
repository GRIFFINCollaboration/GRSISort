#ifndef LABRHIT_H
#define LABRHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TLaBrHit
///
/// This is class that contains the information about a LaBr
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"

class TLaBrHit : public TGRSIDetectorHit {
   public:
      TLaBrHit();
      virtual ~TLaBrHit();
      TLaBrHit(const TLaBrHit&);
      TLaBrHit(const TFragment& frag) : TGRSIDetectorHit(frag) {}
      
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
      TVector3 GetPosition(Double_t dist) const;
      TVector3 GetPosition() const;
      
   private:
      Double_t GetDefaultDistance() const { return 0.; } //This needs to be updated
      
      /// \cond CLASSIMP
      ClassDef(TLaBrHit,2) //Stores the information for a LaBrrHit
      /// \endcond
};
/*! @} */
#endif
