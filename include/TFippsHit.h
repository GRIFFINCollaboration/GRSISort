#ifndef FIPPSHIT_H
#define FIPPSHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorHit.h"
#include "TGRSIRunInfo.h"

class TFippsHit : public TGRSIDetectorHit {
public:
   TFippsHit();
   TFippsHit(const TFippsHit&);
   TFippsHit(const TFragment&);
   virtual ~TFippsHit();

public:
   /////////////////////////  Getters	/////////////////////////////////////
   Double_t GetNoCTEnergy(Option_t* opt = "") const;

   /////////////////////////  Recommended Functions/////////////////////////

   /////////////////////////		/////////////////////////////////////

   inline UShort_t GetArrayNumber() const { return (4 * (GetDetector() - 1) + (GetCrystal() + 1)); } //!<!
   // returns a number 1-64 ( 1 = Detector 1 blue;  64 =  Detector 16 white; )

   static bool CompareEnergy(const TFippsHit*, const TFippsHit*); //!<!
   void        Add(const TFippsHit*);                             //!<!

public:
   virtual void Clear(Option_t* opt = "");       //!<!
   virtual void Print(Option_t* opt = "") const; //!<!
   virtual void Copy(TObject&) const;            //!<!
   virtual void Copy(TObject&, bool) const;      //!<!

   TVector3 GetPosition(double dist) const; //!<!
   TVector3 GetPosition() const;

private:
   Double_t GetDefaultDistance() const { return 110.; }

   /// \cond CLASSIMP
   ClassDef(TFippsHit, 1); // Information about a FIPPS Hit
   /// \endcond
};
/*! @} */
#endif
