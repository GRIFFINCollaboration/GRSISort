#ifndef TSCEPTAR_H
#define TSCEPTAR_H

#include "Globals.h"

#include <vector>
#include <cstdio>

#include <TSceptarHit.h>
#include <TSceptarData.h>
#include <TVector3.h> 

#include <TSceptarHit.h>
#include <TGRSIDetector.h> 


class TSceptar : public TGRSIDetector {

  public:
     TSceptar();
     ~TSceptar();

  public: 
     void BuildHits(TGRSIDetectorData *data =0,Option_t *opt = ""); //!
     //void BuildHits(TSceptarData *data = 0,TBGOData *bdata = 0,Option_t *opt="");	//!
     void BuildAddBack(Option_t *opt="");	//!

     TSceptarHit *GetSceptarHit(int i)        {	return &sceptar_hits[i];   }	//!
     Short_t GetMultiplicity() const	      {	return sceptar_hits.size();}	//!

      //Need to fix this stuff
     static TVector3 GetPosition(int DetNbr ,int CryNbr = 5, double distance = 110.0);		//!

     void FillData(TFragment*,TChannel*,MNEMONIC*); //!
     void FillBGOData(TFragment*,TChannel*,MNEMONIC*); //!

   private: 
     TSceptarData *sceptardata;                 //!  Used to build GRIFFIN Hits
     std::vector <TSceptarHit> sceptar_hits; //   The set of crystal hits
     std::vector <TSceptarHit> addback_hits; //   The set of add-back hits		

     static bool fSetCoreWave;		         //  Flag for Waveforms ON/OFF

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
     static TVector3 gPaddlePosition[17];     //Position of each Paddle
     void ClearStatus() { ftapemove = kFALSE; fbackground = kFALSE; fbeamon = kFALSE; fdecay = kFALSE;}     

   public:         
     virtual void Clear(Option_t *opt = "");		//!
     virtual void Print(Option_t *opt = "");		//!

   ClassDef(TSceptar,1)  // Sceptar Physics structure


};

#endif


