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

   enum EPPG {
      kTapeMove      = 0x1,
      kBackGround    = 0x2,
      kBeamOn        = 0x4,
      kDecay         = 0x8,
      kCycleStartTime= 0x10,
      //Room for 0x20
      //Room for 0x40
      //Room for 0x80
   };

   public:
      TGriffin();
      TGriffin(const TGriffin&);
      virtual ~TGriffin();

   public:
      void BuildHits(TGRSIDetectorData *data =0,Option_t *opt = ""); //!

      TGriffinHit *GetGriffinHit(const int i); //!
      TGRSIDetectorHit* GetHit(const Int_t idx = 0);
      Int_t GetMultiplicity() const {return griffin_hits.size();}

      static TVector3 GetPosition(int DetNbr ,int CryNbr = 5, double distance = 110.0);		//!
      void FillData(TFragment*,TChannel*,MNEMONIC*); //!

      TGriffin& operator=(const TGriffin&);  //!

      //void AddHit(TGriffinHit *hit) { griffin_hits.push_back(*hit); }

      
      static bool addback_criterion_singleclover(TGriffinHit&, TGriffinHit&);
      //static bool addback_criterion_array(TGriffinHit&, TGriffinHit&);
      void SetAddbackCriterion(bool (*criterion)(TGriffinHit&, TGriffinHit&));
#ifndef __CINT__
      void SetAddbackCriterion(std::function<bool(TGriffinHit&, TGriffinHit&)> criterion) { addback_criterion = criterion; }
      std::function<bool(TGriffinHit&, TGriffinHit&)> GetAddbackCriterion() const { return addback_criterion; }
      static std::function<bool(TGriffinHit&, TGriffinHit&)> addback_criterion;
#endif

      Int_t GetAddbackMultiplicity();
      TGriffinHit* GetAddbackHit(const int i);

   private:
      TGriffinData *grifdata;                 //!  Used to build GRIFFIN Hits
      //TBGOData     *bgodata;                  //!  Used to build BGO Hits
      std::vector <TGriffinHit> griffin_hits; //  The set of crystal hits

      //static bool fSetBGOHits;		            //!  Flag that determines if BGOHits are being measured			 

      static bool fSetCoreWave;		         //!  Flag for Waveforms ON/OFF
      //static bool fSetBGOWave;		            //!  Flag for BGO Waveforms ON/OFF

      long fCycleStart;                //!  The start of the cycle
      static long fLastPPG;                   //!  value of the last ppg

      UChar_t fGriffinBits;

      std::vector <TGriffinHit> addback_hits; //! Used to create addback hits on the fly
      std::vector <UShort_t> faddback_frags; //! Number of crystals involved in creating in the addback hit

   public:
      static bool SetCoreWave()        { return fSetCoreWave;  }	//!
      //static bool SetBGOHits()       { return fSetBGOHits;   }	//!
      //static bool SetBGOWave()	    { return fSetBGOWave;   } //!

      void SetTapeMove(Bool_t flag=kTRUE)   { SetBitNumber(kTapeMove,flag); }  //!
      void SetBackground(Bool_t flag=kTRUE) { SetBitNumber(kBackGround,flag);} //!
      void SetBeamOn(Bool_t flag=kTRUE)     { SetBitNumber(kBeamOn,flag);}     //!
      void SetDecay(Bool_t flag=kTRUE)      { SetBitNumber(kDecay,flag);}      //!

      bool GetTapeMove()   const { return TestBitNumber(kTapeMove);}//!
      bool GetBackground() const { return TestBitNumber(kBackGround);}//!
      bool GetBeamOn()     const { return TestBitNumber(kBeamOn);}//!
      bool GetDecay()      const { return TestBitNumber(kDecay);}//!

      int GetCycleTimeInMilliSeconds(long time) { return (int)((time-fCycleStart)/1e5); }//!

      //  void AddHit(TGRSIDetectorHit *hit,Option_t *opt="");//!
   private:
      static TVector3 gCloverPosition[17];               //! Position of each HPGe Clover
      void ClearStatus() { fGriffinBits = 0; } //!
      void SetBitNumber(enum EPPG ppg,Bool_t set);
      Bool_t TestBitNumber(enum EPPG ppg) const {return (ppg & fGriffinBits);}

   public:
      virtual void Copy(TGriffin&) const;                //!
      virtual void Clear(Option_t *opt = "all");		     //!
      virtual void Print(Option_t *opt = "") const;		  //!
      void ResetAddback();		     //!
      UShort_t GetNAddbackFrags(int idx) const;

   protected:
      void PushBackHit(TGRSIDetectorHit* ghit);

      ClassDef(TGriffin,2)  // Griffin Physics structure


};

#endif
