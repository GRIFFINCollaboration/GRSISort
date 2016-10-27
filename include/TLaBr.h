#ifndef TLABR_H
#define TLABR_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TLaBr
///
/// The TLaBr class defines the observables and algorithms used
/// when analyzing LaBr data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TLaBrHit.h"

class TLaBr : public TGRSIDetector {
   public:
      TLaBr();
      virtual ~TLaBr();
      TLaBr(const TLaBr& rhs);
      
   public:
      TGRSIDetectorHit* GetHit(const Int_t& idx =0);
      void Copy(TObject &rhs) const;
      TLaBrHit* GetLaBrHit(const int& i);	//!<!
      Short_t GetMultiplicity() const	       {	return fLaBrHits.size(); }	      //!<!
#ifndef __CINT__
      void AddFragment(std::shared_ptr<const TFragment>, TChannel*); //!<!
#endif
      
      static TVector3 GetPosition(int DetNbr) { return gPosition[DetNbr]; }	//!<!
      
      TLaBr& operator=(const TLaBr&);  //!<!
      
   private:
      std::vector <TLaBrHit> fLaBrHits;                                  //   The set of LaBr hits
      
   private:
      static TVector3 gPosition[9];                                     //!<!  Position of each Paddle
      
   public:
      void Clear(Option_t *opt = "");		//!<!
      void Print(Option_t *opt = "") const;		//!<!
      
      /// \cond CLASSIMP
      ClassDef(TLaBr,1)  // LaBr Physics structure
      /// \endcond
};
/*! @} */
#endif
