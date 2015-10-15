#ifndef TGRIFFIN_H
#define TGRIFFIN_H

#include "Globals.h"

#include <vector>
#include <cstdio>
#include <functional>

#include <TBits.h>

#include "TGriffinHit.h"
#ifndef __CINT__
#include "TGriffinData.h"
#else
class TGriffinData;
#endif
//#include "TBGOData.h"
#include "TVector3.h"

#include "TGriffinHit.h"
#include "TGRSIDetector.h"


class TGriffin : public TGRSIDetector {
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

   public:
      TGriffin();
      TGriffin(const TGriffin&);
      virtual ~TGriffin();

   public:
      void BuildHits(TDetectorData *data =0,Option_t *opt = ""); //!

      TGriffinHit *GetGriffinHit(const int i); //!
      TGRSIDetectorHit* GetHit(const Int_t idx = 0);
      Int_t GetMultiplicity() const {return griffin_hits.size();}

      static TVector3 GetPosition(int DetNbr ,int CryNbr = 5, double distance = 110.0);		//!
      void FillData(TFragment*,TChannel*,MNEMONIC*); //!

      TGriffin& operator=(const TGriffin&);  //!
      
#ifndef __CINT__
      void SetAddbackCriterion(std::function<bool(TGriffinHit&, TGriffinHit&)> criterion) { fAddback_criterion = criterion; }
      std::function<bool(TGriffinHit&, TGriffinHit&)> GetAddbackCriterion() const { return fAddback_criterion; }
#endif

      Int_t GetAddbackMultiplicity();
      TGriffinHit* GetAddbackHit(const int i);

   private:
#ifndef __CINT__
      static std::function<bool(TGriffinHit&, TGriffinHit&)> fAddback_criterion;
#endif
      TGriffinData *grifdata;                 //!  Used to build GRIFFIN Hits
      //TBGOData     *bgodata;                  //!  Used to build BGO Hits
      std::vector <TGriffinHit> griffin_hits; //  The set of crystal hits

      //static bool fSetBGOHits;		            //!  Flag that determines if BGOHits are being measured			 

      static bool fSetCoreWave;		         //!  Flag for Waveforms ON/OFF
      //static bool fSetBGOWave;		            //!  Flag for BGO Waveforms ON/OFF

      long fCycleStart;                //!  The start of the cycle
      UChar_t fGriffinBits;            // Transient member flags

      std::vector <TGriffinHit> fAddback_hits; //! Used to create addback hits on the fly
      std::vector <UShort_t> fAddback_frags; //! Number of crystals involved in creating in the addback hit

   public:
      static bool SetCoreWave()        { return fSetCoreWave;  }	//!
      //static bool SetBGOHits()       { return fSetBGOHits;   }	//!
      //static bool SetBGOWave()	    { return fSetBGOWave;   } //!

   private:
      static TVector3 gCloverPosition[17];               //! Position of each HPGe Clover
      void ClearStatus() { fGriffinBits = 0; } //!
      void SetBitNumber(enum EGriffinBits bit,Bool_t set);
      Bool_t TestBitNumber(enum EGriffinBits bit) const {return (bit & fGriffinBits);}

   public:
      virtual void Copy(TGriffin&) const;                //!
      virtual void Clear(Option_t *opt = "all");		     //!
      virtual void Print(Option_t *opt = "") const;		  //!
      void ResetAddback();		     //!
      UShort_t GetNAddbackFrags(size_t idx) const;

   protected:
      void PushBackHit(TGRSIDetectorHit* ghit);

      ClassDef(TGriffin,3)  // Griffin Physics structure
};

#endif
