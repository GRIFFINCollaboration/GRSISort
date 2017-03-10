#ifndef TGRIFFIN_H
#define TGRIFFIN_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>
#include <functional>
//#include <tuple>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TGriffinHit.h"
#include "TGRSIDetector.h"
#include "TGRSIRunInfo.h"
#include "TTransientBits.h"

class TGriffin : public TGRSIDetector {
	public:
		enum EGriffinBits {
			kIsLowGainAddbackSet			= 1<<0,
			kIsHighGainAddbackSet		= 1<<1,
			kIsLowGainCrossTalkSet   	= 1<<2,
			kIsHighGainCrossTalkSet   	= 1<<3,
			kBit4		= 1<<4,
			kBit5		= 1<<5,
			kBit6		= 1<<6,
			kBit7		= 1<<7
		};
		enum EGainBits {
			kLowGain,
			kHighGain
		};

		TGriffin();
		TGriffin(const TGriffin&);
		virtual ~TGriffin();

	public:
		TGriffinHit* GetGriffinLowGainHit(const int& i); //!<!
		TGriffinHit* GetGriffinHighGainHit(const int& i); //!<!
		TGriffinHit* GetGriffinHit(const Int_t &i) { return GetGriffinHit(i,GetDefaultGainType()); } //!<!
		TGRSIDetectorHit* GetHit(const Int_t& idx = 0);
		Short_t GetLowGainMultiplicity() const {return fGriffinLowGainHits.size();}
		Short_t GetHighGainMultiplicity() const {return fGriffinHighGainHits.size();}
		Int_t GetMultiplicity() const { return GetMultiplicity(GetDefaultGainType()); }

		static TVector3 GetPosition(int DetNbr, int CryNbr = 5, double distance = 110.0);    //!<!
      static const char* GetColorFromNumber(Int_t number);
#ifndef __CINT__
		void AddFragment(std::shared_ptr<const TFragment> frag, TChannel* chan); //!<!
#endif
		void ClearTransients() { fGriffinBits = 0; for(auto hit : fGriffinLowGainHits) hit.ClearTransients(); for(auto hit: fGriffinHighGainHits) hit.ClearTransients(); }
		void ResetFlags() const;

		TGriffin& operator=(const TGriffin&);  //!<!

#if !defined (__CINT__) && !defined (__CLING__)
		void SetAddbackCriterion(std::function<bool(TGriffinHit&, TGriffinHit&)> criterion) { fAddbackCriterion = criterion; }
		std::function<bool(TGriffinHit&, TGriffinHit&)> GetAddbackCriterion() const { return fAddbackCriterion; }
#endif

		Int_t GetAddbackLowGainMultiplicity();
		Int_t GetAddbackHighGainMultiplicity();
		Int_t GetAddbackMultiplicity() { return GetAddbackMultiplicity(GetDefaultGainType()); }
		TGriffinHit* GetAddbackLowGainHit(const int& i);
		TGriffinHit* GetAddbackHighGainHit(const int& i);
		TGriffinHit* GetAddbackHit(const int& i) { return GetAddbackHit(i,GetDefaultGainType()); }
		bool IsAddbackSet(const Int_t &gain_type) const;
		void ResetLowGainAddback();         //!<!
		void ResetHighGainAddback();         //!<!
		void ResetAddback() { ResetAddback(GetDefaultGainType()); }         //!<!
		UShort_t GetNHighGainAddbackFrags(const size_t &idx);
		UShort_t GetNLowGainAddbackFrags(const size_t &idx);
		UShort_t GetNAddbackFrags(const size_t &idx) { return GetNAddbackFrags(idx,GetDefaultGainType()); }

	private:
#if !defined (__CINT__) && !defined (__CLING__)
		static std::function<bool(TGriffinHit&, TGriffinHit&)> fAddbackCriterion;
#endif
		std::vector <TGriffinHit> fGriffinLowGainHits; //  The set of crystal hits
		std::vector <TGriffinHit> fGriffinHighGainHits; //  The set of crystal hits

		//static bool fSetBGOHits;                //!<!  Flag that determines if BGOHits are being measured       

		static bool fSetCoreWave;             //!<!  Flag for Waveforms ON/OFF
		//static bool fSetBGOWave;                //!<!  Flag for BGO Waveforms ON/OFF

		long fCycleStart;                //!<!  The start of the cycle
		mutable TTransientBits<UChar_t> fGriffinBits; // Transient member flags

		mutable std::vector<TGriffinHit> fAddbackLowGainHits; //!<! Used to create addback hits on the fly
		mutable std::vector<TGriffinHit> fAddbackHighGainHits; //!<! Used to create addback hits on the fly
		mutable std::vector<UShort_t> fAddbackLowGainFrags; //!<! Number of crystals involved in creating in the addback hit
		mutable std::vector<UShort_t> fAddbackHighGainFrags; //!<! Number of crystals involved in creating in the addback hit

		static Int_t fDefaultGainType;

	public:
		static bool SetCoreWave()        { return fSetCoreWave;  }  //!<!
		//static bool SetBGOHits()       { return fSetBGOHits;   }  //!<!
		//static bool SetBGOWave()      { return fSetBGOWave;   } //!<!
		static void SetDefaultGainType(const Int_t &gain_type); 
		static Int_t GetDefaultGainType() { return fDefaultGainType; }

	private:
		static TVector3 gCloverPosition[17];               //!<! Position of each HPGe Clover
		void ClearStatus() const { fGriffinBits = 0; } //!<!
		void SetBitNumber(enum EGriffinBits bit,Bool_t set) const;
		Bool_t TestBitNumber(enum EGriffinBits bit) const {return fGriffinBits.TestBit(bit);}

     // const std::tuple<std::vector<TGriffinHit> *, std::vector<TGriffinHit> *, std::vector<UShort_t>*> fLowGainTuple = std::make_tuple(&fGriffinLowGainHits,&fAddbackLowGainHits,&fAddbackLowGainFrags);

		//Cross-Talk stuff
	public:
		static const Double_t gStrongCT[2];   //!<!
		static const Double_t gWeakCT[2]; //!<!
		static const Double_t gCrossTalkPar[2][4][4]; //!<! 
		static Double_t CTCorrectedEnergy(const TGriffinHit* const energy_to_correct, const TGriffinHit* const other_energy, Bool_t time_constraint = true);
		Bool_t IsCrossTalkSet(const Int_t &gain_type) const;
		void FixLowGainCrossTalk();
		void FixHighGainCrossTalk();

	private:
		//This is where the general untouchable functions live.
		std::vector<TGriffinHit> 	*GetHitVector(const Int_t &gain_type); //!<!
		std::vector<TGriffinHit> 	*GetAddbackVector(const Int_t &gain_type); //!<!
		std::vector<UShort_t> 		*GetAddbackFragVector(const Int_t &gain_type); //!<!
		TGriffinHit* GetGriffinHit(const Int_t &i,const Int_t &gain_type); //!<!
		Int_t GetMultiplicity(const Int_t &gain_type) const;
		TGriffinHit* GetAddbackHit(const int& i, const Int_t &gain_type);
		Int_t GetAddbackMultiplicity(const Int_t &gain_type);
		void SetAddback(const Int_t &gain_type,bool flag = true) const;
		void ResetAddback(const Int_t &gain_type);         //!<!
		UShort_t GetNAddbackFrags(const size_t &idx, const Int_t &gain_type);
		void FixCrossTalk(const Int_t &gain_type);
		void SetCrossTalk(const Int_t &gain_type, bool flag = true) const;

	public:
		virtual void Copy(TObject&) const;                //!<!
		virtual void Clear(Option_t* opt = "all");         //!<!
		virtual void Print(Option_t* opt = "") const;      //!<!

/// \cond CLASSIMP
	ClassDef(TGriffin,5)  // Griffin Physics structure
/// \endcond
};
/*! @} */
#endif
