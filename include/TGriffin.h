#ifndef TGRIFFIN_H
#define TGRIFFIN_H

#include "Globals.h"

#include <vector>
#include <cstdio>

#include <TGriffinHit.h>
#include <TGriffinData.h>
#include <TBGOData.h>
#include <TVector3.h> 

#include <TGriffinHit.h>
#include <TGRSIDetector.h> 


class TGriffin : public TGRSIDetector {

  public:
     TGriffin();
     ~TGriffin();

  public: 
     void BuildHits(TGRSIDetectorData *data =0,Option_t *opt = ""); //!
     //void BuildHits(TGriffinData *data = 0,TBGOData *bdata = 0,Option_t *opt="");	//!
     void BuildAddBack(Option_t *opt="");	//!

     TGriffinHit *GetGriffinHit(int i)        {	return &griffin_hits[i];   }	//!
     Short_t GetMultiplicity() const	      {	return griffin_hits.size();}	//!

     TGriffinHit *GetAddBackHit(int i)        {	return &addback_hits[i];   }	//!
     Short_t GetAddBackMultiplicity() const   {	return addback_hits.size();}	//!

		//TVector3 GetPosition(TGriffinHit *,int distance=0);						//!

     static TVector3 GetPosition(int DetNbr ,int CryNbr = 5, double distance = 110.0);		//!

     void FillData(TFragment*,TChannel*,MNEMONIC*); //!
     void FillBGOData(TFragment*,TChannel*,MNEMONIC*); //!

   private: 
     TGriffinData *grifdata;                 //!  Used to build GRIFFIN Hits
     TBGOData     *bgodata;                  //!  Used to build BGO Hits
     std::vector <TGriffinHit> griffin_hits; //   The set of crystal hits
     std::vector <TGriffinHit> addback_hits; //   The set of add-back hits		

     static bool fSetBGOHits;		            //!  Flag that determines if BGOHits are being measured			 
		
     static bool fSetCoreWave;		         //  Flag for Waveforms ON/OFF
     static bool fSetBGOWave;		            //  Flag for BGO Waveforms ON/OFF

     bool ftapemove;
     bool fbackground;
     bool fbeamon;
     bool fdecay;       

   public:
     static bool SetBGOHits()       { return fSetBGOHits;   }	//!
     static bool SetCoreWave()      { return fSetCoreWave;  }	//!
     static bool SetBGOWave()	    { return fSetBGOWave;   } //!

     void SetTapeMove()     { ftapemove = kTRUE; }
     void SetBackground()   { fbackground = kTRUE;}
     void SetBeamOn()       { fbeamon = kTRUE;}
     void SetDecay()        { fdecay = kTRUE;}

     bool GetTapeMove()   const { return ftapemove;  }
     bool GetBackground() const { return fbackground;}
     bool GetBeamOn()     const { return fbeamon;    }
     bool GetDecay()      const { return fdecay;     }

    // bool GetTapeMove()     { fcyclestatus

   private:
     static TVector3 gCloverPosition[17];     //Position of each HPGe Clover
     void ClearStatus() { ftapemove = kFALSE; fbackground = kFALSE; fbeamon = kFALSE; fdecay = kFALSE;}     

   public:         
     virtual void Clear(Option_t *opt = "");		      //!
     virtual void Print(Option_t *opt = "") const;		//!

   ClassDef(TGriffin,1)  // Griffin Physics structure


};

#endif


