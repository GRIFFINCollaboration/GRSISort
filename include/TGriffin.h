#ifndef TGRIFFIN_H
#define TGRIFFIN_H

#include "Globals.h"

#include <vector>
#include <cstdio>

#include "TGriffinHit.h"
#ifndef __CINT__
#include "TGriffinData.h"
#else
class TGriffinData;
#endif
#include "TBGOData.h"
#include "TVector3.h" 

#include "TGriffinHit.h"
#include "TGRSIDetector.h" 


class TGriffin : public TGRSIDetector {

  public:
     TGriffin();
     TGriffin(const TGriffin&);
     virtual ~TGriffin();

  public: 
     void BuildHits(TGRSIDetectorData *data =0,Option_t *opt = ""); //!
     //void BuildAddBack(Option_t *opt="");	//!
     //void BuildAddBackClover(Option_t *opt=""); //!

     TGriffinHit *GetGriffinHit(const int i) { return &griffin_hits.at(i);   }  //!
     TGriffinHit &GetGriffinHit(const int i) { return  griffin_hits.at(i);   }  //!
     Int_t        GetMultiplicity() const    { return griffin_hits.size();   }  //!


     //TGriffinHit *GetAddBackHit(int i)        {	return &addback_hits[i];   }	//!
     //Short_t GetAddBackMultiplicity() const   {	return addback_hits.size();}	//!

     //TGriffinHit *GetAddBackCloverHit(int i)       { return &addback_clover_hits[i]; }   //!
     //Short_t GetAddBackCloverMultiplicity() const  { return addback_clover_hits.size();} //!

		//TVector3 GetPosition(TGriffinHit *,int distance=0);						//!

     static TVector3 GetPosition(int DetNbr ,int CryNbr = 5, double distance = 110.0);		//!
     void FillData(TFragment*,TChannel*,MNEMONIC*); //!
     //void FillBGOData(TFragment*,TChannel*,MNEMONIC*); //!

     TGriffin& operator=(const TGriffin&);  //! 

     //void AddHit(TGriffinHit *hit) { griffin_hits.push_back(*hit); }

   private: 
     TGriffinData *grifdata;                 //!  Used to build GRIFFIN Hits
     TBGOData     *bgodata;                  //!  Used to build BGO Hits
     std::vector <TGriffinHit> griffin_hits; //   The set of crystal hits
     //std::vector <TGriffinHit> addback_hits; //   The set of add-back hits		
     //std::vector <TGriffinHit> addback_clover_hits; //  The set of add-back2 hits

     //static bool fSetBGOHits;		            //!  Flag that determines if BGOHits are being measured			 
		
     static bool fSetCoreWave;		         //!  Flag for Waveforms ON/OFF
     //static bool fSetBGOWave;		            //!  Flag for BGO Waveforms ON/OFF

     static long fCycleStart;                //!  The start of the cycle
     static long fLastPPG;                   //!  value of the last ppg

     enum  Bit{"CycleStartTime","TapeMove","BackGround","BeamOn","Decay"};
     TBits fbits;

   public:
     static bool SetCoreWave()        { return fSetCoreWave;  }	//!
     //static bool SetBGOHits()       { return fSetBGOHits;   }	//!
     //static bool SetBGOWave()	    { return fSetBGOWave;   } //!

     void SetTapeMove(Bool_t flag=kTRUE)   { fbits.SetBitNumber("TapeMove",flag); }  //!
     void SetBackground(Bool_t flag=kTRUE) { fbits.SetBitNumber("BackGround",flag);} //!
     void SetBeamOn(Bool_t flag=kTRUE)     { fbits.SetBitNumber("BeamOn",flag);}     //!
     void SetDecay(Bool_t flag=kTRUE)      { fbits.SetBitNumber("Decay",flag);}      //!

     bool GetTapeMove()   const { fbits.TestBitNumber("TapeMove");}//!
     bool GetBackground() const { fbits.TestBitNumber("BackGround");}//!
     bool GetBeamOn()     const { fbits.TestBitNumber("BeamOn");}//!
     bool GetDecay()      const { fbits.TestBitNumber("Decay");}//!

     static int GetCycleTimeInMilliSeconds(long time) { return (int)((time-fCycleStart)/1e5); }//!

   private:
     static TVector3 gCloverPosition[17];               //! Position of each HPGe Clover
     void ClearStatus() { fbits.ResetAllBits(kFALSE); } //!     

   public:         
     virtual void Copy(TGriffin&) const;                //!
     virtual void Clear(Option_t *opt = "");		        //!
     virtual void Print(Option_t *opt = "") const;		  //!

   ClassDef(TGriffin,2)  // Griffin Physics structure


};

#endif


