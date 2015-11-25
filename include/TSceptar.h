#ifndef TSCEPTAR_H
#define TSCEPTAR_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TSceptarHit.h"

class TSceptar : public TGRSIDetector {
   public:
      TSceptar();
      virtual ~TSceptar();
      TSceptar(const TSceptar& rhs);
      
   public:
      TGRSIDetectorHit* GetHit(const Int_t& idx =0);
      void Copy(TObject &rhs) const;
      TSceptarHit* GetSceptarHit(const int& i);	//!<!
      Short_t GetMultiplicity() const	       {	return fSceptarHits.size(); }	      //!<!
      
      static TVector3 GetPosition(int DetNbr) { return gPaddlePosition[DetNbr]; }	//!<!
      
      void AddFragment(TFragment*, MNEMONIC*); //!<!
      void BuildHits() {} //no need to build any hits, everything already done in AddFragment
      
      TSceptar& operator=(const TSceptar&);  //!<!
      
   private:
      std::vector <TSceptarHit> fSceptarHits;                                  //   The set of sceptar hits
      
      static bool fSetWave;		                                                //  Flag for Waveforms ON/OFF
      
   public:
      static bool SetWave()      { return fSetWave;  }	                        //!<!
      
   private:
      static TVector3 gPaddlePosition[21];                                     //!<!  Position of each Paddle
      
   public:
      void Clear(Option_t *opt = "");		//!<!
      void Print(Option_t *opt = "") const;		//!<!
      
   protected:
      void PushBackHit(TGRSIDetectorHit*);
      
      /// \cond CLASSIMP
      ClassDef(TSceptar,2)  // Sceptar Physics structure
      /// \endcond
};
/*! @} */
#endif
