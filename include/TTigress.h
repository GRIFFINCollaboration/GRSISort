#ifndef TTIGRESS_H
#define TTIGRESS_H


#include <vector>
#include <iostream>
#include <set>
#include <stdio.h>
#include <functional>

#include "TTigressHit.h"

#include "TMath.h"
#include "TVector3.h" 
#include "TObject.h"
#include "TClonesArray.h"

#include "TGRSIDetector.h" 

class TTigress : public TGRSIDetector {
	public:
		enum ETigressBits {
			kIsAddbackSet = 1<<0,
			kBit1         = 1<<1,
			kBit2         = 1<<2,
			kBit3         = 1<<3,
			kBit4         = 1<<4,
			kBit5         = 1<<5,
			kBit6         = 1<<6,
			kBit7         = 1<<7
		};

		TTigress();
		TTigress(const TTigress&);
		virtual ~TTigress();

		//These Getters of hits throw "ROOT Errors"
		TTigressHit *GetTigressHit(const int& i);
		TGRSIDetectorHit *GetHit(const int& i)   { return GetTigressHit(i);       } //!
		size_t GetMultiplicity() const	       { return fTigressHits.size(); }	//!
		static TVector3 GetPosition(int DetNbr ,int CryNbr, int SegNbr, double distance = 110.);		//!

		Int_t GetAddbackMultiplicity();
		TTigressHit* GetAddbackHit(const int&);
		void ResetAddback();		     //!
		UShort_t GetNAddbackFrags(size_t idx) const;

		void AddFragment(TFragment*, MNEMONIC*); //!
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment

		TTigress& operator=(const TTigress&); //!

#ifndef __CINT__
		void SetAddbackCriterion(std::function<bool(TTigressHit&, TTigressHit&)> criterion) { fAddbackCriterion = criterion; }
		std::function<bool(TTigressHit&, TTigressHit&)> GetAddbackCriterion() const { return fAddbackCriterion; }
#endif

	private: 
#ifndef __CINT__
		static std::function<bool(TTigressHit&, TTigressHit&)> fAddbackCriterion;
#endif
		std::vector<TTigressHit> fTigressHits;

		//static double beta;

		static bool fSetSegmentHits;			   //!
		static bool fSetBGOHits;					//!

		static bool fSetCoreWave;					//!
		static bool fSetSegmentWave;			   //!
		static bool fSetBGOWave;					//!

		static double GeBluePosition[17][9][3];	//!	detector segment XYZ
		static double GeGreenPosition[17][9][3];	//!
		static double GeRedPosition[17][9][3];	//!
		static double GeWhitePosition[17][9][3];	//!

		UChar_t fTigressBits;                  // flags for transient members
		void ClearStatus() { fTigressBits = 0; }
		void SetBitNumber(enum ETigressBits bit,Bool_t set);
		Bool_t TestBitNumber(enum ETigressBits bit) const {return (bit & fTigressBits);}

		std::vector<TTigressHit> fAddbackHits; //! Used to create addback hits on the fly
		std::vector<UShort_t> fAddbackFrags; //! Number of crystals involved in creating in the addback hit

	public:
		static bool SetSegmentHits() 	 { return fSetSegmentHits;	}	//!
		static bool SetBGOHits()     	 { return fSetBGOHits;	    }	//!

		static bool SetCoreWave()    { return fSetCoreWave;	    }	//!
		static bool SetSegmentWave() { return fSetSegmentWave;  }	//!
		static bool SetBGOWave()	 { return fSetBGOWave;		}     //!

	public:         
		virtual void Clear(Option_t *opt = "");		 //!
		virtual void Print(Option_t *opt = "") const; //!
		virtual void Copy(TObject&) const;           //!

	protected:
		void PushBackHit(TGRSIDetectorHit* ghit);

		ClassDef(TTigress,5)  // Tigress Physics structure
};
#endif


