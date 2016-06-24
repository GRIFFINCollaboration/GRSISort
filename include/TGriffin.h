#ifndef TGRIFFIN_H
#define TGRIFFIN_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>
#include <functional>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TGriffinHit.h"
#include "TGRSIDetector.h"
#include "TGRSIRunInfo.h"

class TGriffin : public TGRSIDetector {
	public:
		enum EGriffinBits {
			kIsAddbackSet = 1<<0,
			kBit1         = 1<<1,
			kBit2         = 1<<2,
			kBit3         = 1<<3,
			kBit4         = 1<<4,
			kBit5         = 1<<5,
			kBit6         = 1<<6,
			kBit7         = 1<<7
		};

		TGriffin();
		TGriffin(const TGriffin&);
		virtual ~TGriffin();

	public:
		TGriffinHit* GetGriffinHit(const int& i); //!<!
		TGRSIDetectorHit* GetHit(const Int_t& idx = 0);
		Short_t GetMultiplicity() const {return fGriffinHits.size();}

		static TVector3 GetPosition(int DetNbr, int CryNbr = 5, double distance = TGRSIRunInfo::HPGeArrayPosition());		//!<!
		void AddFragment(TFragment*, MNEMONIC*); //!<!
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment

		TGriffin& operator=(const TGriffin&);  //!<!

#if !defined (__CINT__) && !defined (__CLING__)
		void SetAddbackCriterion(std::function<bool(TGriffinHit&, TGriffinHit&)> criterion) { fAddbackCriterion = criterion; }
		std::function<bool(TGriffinHit&, TGriffinHit&)> GetAddbackCriterion() const { return fAddbackCriterion; }
#endif

		Int_t GetAddbackMultiplicity();
		TGriffinHit* GetAddbackHit(const int& i);

	private:
#if !defined (__CINT__) && !defined (__CLING__)
		static std::function<bool(TGriffinHit&, TGriffinHit&)> fAddbackCriterion;
#endif
		std::vector <TGriffinHit> fGriffinHits; //  The set of crystal hits

		//static bool fSetBGOHits;		            //!<!  Flag that determines if BGOHits are being measured			 

		static bool fSetCoreWave;		         //!<!  Flag for Waveforms ON/OFF
		//static bool fSetBGOWave;		            //!<!  Flag for BGO Waveforms ON/OFF

		long fCycleStart;                //!<!  The start of the cycle
		UChar_t fGriffinBits;            // Transient member flags

		std::vector<TGriffinHit> fAddbackHits; //!<! Used to create addback hits on the fly
		std::vector<UShort_t> fAddbackFrags; //!<! Number of crystals involved in creating in the addback hit

	public:
		static bool SetCoreWave()        { return fSetCoreWave;  }	//!<!
		//static bool SetBGOHits()       { return fSetBGOHits;   }	//!<!
		//static bool SetBGOWave()	    { return fSetBGOWave;   } //!<!

	private:
		static TVector3 gCloverPosition[17];               //!<! Position of each HPGe Clover
		void ClearStatus() { fGriffinBits = 0; } //!<!
		void SetBitNumber(enum EGriffinBits bit,Bool_t set);
		Bool_t TestBitNumber(enum EGriffinBits bit) const {return (bit & fGriffinBits);}

	public:
		virtual void Copy(TObject&) const;                //!<!
		virtual void Clear(Option_t* opt = "all");		     //!<!
		virtual void Print(Option_t* opt = "") const;		  //!<!
		void ResetAddback();		     //!<!
		UShort_t GetNAddbackFrags(size_t idx) const;

	protected:
		void PushBackHit(TGRSIDetectorHit* ghit);

/// \cond CLASSIMP
		ClassDef(TGriffin,3)  // Griffin Physics structure
/// \endcond
};
/*! @} */
#endif
