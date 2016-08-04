#ifndef TTIGRESS_H
#define TTIGRESS_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <iostream>
#include <set>
#include <stdio.h>
#include <functional>

#include "TMath.h"
#include "TVector3.h" 
#include "TObject.h"
#include "TClonesArray.h"

#include "TGRSIDetector.h" 
#include "TTigressHit.h"
#include "TBgoHit.h"

class TTigress : public TGRSIDetector {
	public:
		enum ETigressBits {
			kIsAddbackSet = BIT(0),
			kSuppression  = BIT(1),
			kBit2         = BIT(2),
			kBit3         = BIT(3),
			kBit4         = BIT(4),
			kSetCoreWave  = BIT(5),
			kSetSegWave   = BIT(6),
			kSetBGOWave   = BIT(7)
		};

    std::vector<TBgoHit> fBgos;
		void AddBGO(TBgoHit& bgo) 		    { fBgos.push_back(bgo);	}	   //!<!
		int GetBGOMultiplicity()			    const      { return fBgos.size();     }   //!<!
		int GetNBGOs()			              const      { return fBgos.size();     }   //!<!
		TBgoHit GetBGO( int &i)	   const { return fBgos.at(i);	     }   //!<!
		TBgoHit& GetBGO( int &i)	         { return fBgos.at(i);	     }   //!<!

		std::vector<std::vector<TFragment*> > SegmentFragments;

		TTigress();
		TTigress(const TTigress&);
		virtual ~TTigress();

		//These Getters of hits throw "ROOT Errors"
		TTigressHit* GetTigressHit(const int& i);
		TGRSIDetectorHit* GetHit(const int& i)   { return GetTigressHit(i);       } //!<!
		size_t GetMultiplicity() const	        { return fTigressHits.size(); }	//!<!
		static TVector3 GetPosition(int DetNbr ,int CryNbr, int SegNbr, double distance = 110.);		//!<!

		// Delete tigress hit from vector (for whatever reason)
		//void DeleteTigressHit(const int& i) { fTigressHits.erase(fTigressHits.begin()+i); } 

		Int_t GetAddbackMultiplicity();
		TTigressHit* GetAddbackHit(const int&);
		void ResetAddback();		     //!<!
		void SetDoSuppression(bool flag = true);
		UShort_t GetNAddbackFrags(size_t idx) const;

		void AddFragment(TFragment*, TChannel*); //!<!
		void BuildHits();

		TTigress& operator=(const TTigress&); //!<!

#if !defined (__CINT__) && !defined (__CLING__)
		void SetAddbackCriterion(std::function<bool(TTigressHit&, TTigressHit&)> criterion) { fAddbackCriterion = criterion; }
		std::function<bool(TTigressHit&, TTigressHit&)> GetAddbackCriterion() const         { return fAddbackCriterion; }

		void SetSuppressionCriterion(std::function<bool(TTigressHit&, TBgoHit&)> criterion) { fSuppressionCriterion = criterion; }
		std::function<bool(TTigressHit&, TBgoHit&)> GetSuppressionCriterion() const     { return fSuppressionCriterion; }
#endif

	private: 
#if !defined (__CINT__) && !defined (__CLING__)
		static std::function<bool(TTigressHit&, TTigressHit&)> fAddbackCriterion;

		static std::function<bool(TTigressHit&, TBgoHit&)> fSuppressionCriterion;
#endif
		std::vector<TTigressHit> fTigressHits;

		static bool fSetSegmentHits;			   //!<!
		static bool fSetBGOHits;					//!<!

		static bool fSetCoreWave;					//!<!
		static bool fSetSegmentWave;			   //!<!
		static bool fSetBGOWave;					//!<!

		static double GeBluePosition[17][9][3];	//!<!	detector segment XYZ
		static double GeGreenPosition[17][9][3];	//!<!
		static double GeRedPosition[17][9][3];	//!<!
		static double GeWhitePosition[17][9][3];	//!<!

		static double GeBluePositionBack[17][9][3];	//!<!	detector segment XYZ
		static double GeGreenPositionBack[17][9][3];	//!<!
		static double GeRedPositionBack[17][9][3];	//!<!
		static double GeWhitePositionBack[17][9][3];	//!<!

		UChar_t fTigressBits;                  // flags for transient members
		void ClearStatus() { fTigressBits = 0; }
		void SetBitNumber(enum ETigressBits bit,Bool_t set=true);
		Bool_t TestBitNumber(enum ETigressBits bit) const {return (bit & fTigressBits);}

		std::vector<TTigressHit> fAddbackHits; //!<! Used to create addback hits on the fly
		std::vector<UShort_t> fAddbackFrags; //!<! Number of crystals involved in creating in the addback hit

	public:
		static bool SetSegmentHits() 	 { return fSetSegmentHits;	}	//!<!
		static bool SetBGOHits()     	 { return fSetBGOHits;	    }	//!<!

		static bool SetCoreWave()    { return fSetCoreWave;	    }	//!<!
		static bool SetSegmentWave() { return fSetSegmentWave;  }	//!<!
		static bool SetBGOWave()	 { return fSetBGOWave;		}     //!<!
		static bool BGOSuppression[4][4][5]; //!<!

	public:         
		virtual void Clear(Option_t *opt = "");		 //!<!
		virtual void Print(Option_t *opt = "") const; //!<!
		virtual void Copy(TObject&) const;           //!<!

	protected:
		void PushBackHit(TGRSIDetectorHit* ghit);

/// \cond CLASSIMP
		ClassDef(TTigress,5)  // Tigress Physics structure
/// \endcond
};
/*! @} */
#endif
