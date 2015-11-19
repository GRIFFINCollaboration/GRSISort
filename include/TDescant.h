#ifndef TDESCANT_H
#define TDESCANT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///                    
/// \class TDescant
///
/// The TDescant class defines the observables and algorithms used
/// when analyzing GRIFFIN data. It includes detector positions,
/// etc. 
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h" 

#include "Globals.h"
#include "TGRSIDetector.h" 
#include "TDescantHit.h"

class TDescant : public TGRSIDetector {
	public:
		TDescant();
		TDescant(const TDescant&);
		virtual ~TDescant();

	public: 
		TGRSIDetectorHit* GetHit(const Int_t& idx =0);
		TDescantHit* GetDescantHit(const Int_t& idx = 0);

		Short_t GetMultiplicity() const	       {	return fDescantHits.size(); }	//!<!

      static TVector3 GetPosition(int DetNbr, double dist=222);	//!<!

      void AddFragment(TFragment*, MNEMONIC*);           //!<!
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment

		TDescant& operator=(const TDescant&);  // 

	private: 
		std::vector<TDescantHit> fDescantHits;                                  ///<  The set of crystal hits
		static bool fSetWave;		                                             ///<  Flag for Waveforms ON/OFF
		bool fHitFlag;                                                          ///<   Is there a Descant hit?

	public:
		static bool SetWave()      { return fSetWave;  }	                     //!<!
		bool Hit()                {return fHitFlag;}                            //!<!  
		void SetHit(bool flag = true) { fHitFlag = flag; }
	private:
		static TVector3 gPosition[2][71];                                          //!<!
   public:
      void Copy(TObject&) const;                                              //!<!
      void Clear(Option_t* opt = "");                                         //!<!
      void Print(Option_t* opt = "") const;                                   //!<!

      void PushBackHit(TGRSIDetectorHit* deshit);

/// \cond CLASSIMP
		ClassDef(TDescant,1)  // Descant Physics structure
/// \endcond
};
/*! @} */
#endif


