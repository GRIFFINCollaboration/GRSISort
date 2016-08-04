#ifndef TZERODEGREE_H
#define TZERODEGREE_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TZeroDegree
///
/// The TZeroDegree class defines the observables and algorithms used
/// when analyzing GRIFFIN data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TZeroDegreeHit.h"

class TZeroDegree : public TGRSIDetector {
   public:
      TZeroDegree();
      virtual ~TZeroDegree();
      TZeroDegree(const TZeroDegree& rhs);
      
   public:
      TGRSIDetectorHit* GetHit(const Int_t& idx =0);
      void Copy(TObject &rhs) const;
      TZeroDegreeHit* GetZeroDegreeHit(const int& i);	//!<!
      Short_t GetMultiplicity() const	       {	return fZeroDegreeHits.size(); }	      //!<!
      
      static TVector3 GetPosition(double dist) { return TVector3(0,0,dist); }	//!<!
      
      void AddFragment(TFragment*, TChannel*); //!<!
      void BuildHits() {} //no need to build any hits, everything already done in AddFragment
      
      TZeroDegree& operator=(const TZeroDegree&);  //!<!
      
   private:
      std::vector <TZeroDegreeHit> fZeroDegreeHits;                             ///<   The set of zeroDegree hits
      
      static bool fSetWave;		                                               ///<  Flag for Waveforms ON/OFF
      
   public:
      static bool SetWave() { return fSetWave; }                                //!<!
      
   public:
      void Clear(Option_t *opt = "");		//!<!
      void Print(Option_t *opt = "") const;		//!<!
      
   protected:
      void PushBackHit(TGRSIDetectorHit*);
      
      /// \cond CLASSIMP
      ClassDef(TZeroDegree,2)  // ZeroDegree Physics structure
      /// \endcond
};
/*! @} */
#endif
