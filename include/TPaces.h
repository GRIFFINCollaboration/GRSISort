#ifndef TPACES_H
#define TPACES_H

#include "Globals.h"

#include <vector>
#include <cstdio>

#include <TBits.h>

#include "TPacesHit.h"
#ifndef __CINT__
#include "TPacesData.h"
#else
class TPacesData;
#endif
#include "TVector3.h" 

#include "TPacesHit.h"
#include "TGRSIDetector.h" 


class TPaces : public TGRSIDetector {

  public:
     TPaces();
     TPaces(const TPaces&);
     virtual ~TPaces();

  public: 
     void BuildHits(TGRSIDetectorData *data =0,Option_t *opt = ""); //!

     TPacesHit *GetPacesHit(const int i); //!
     TGRSIDetectorHit* GetHit(const Int_t idx = 0);
     Int_t GetMultiplicity() const {return paces_hits.size();}
     
     static TVector3 GetPosition(int DetNbr);		//!
     void FillData(TFragment*,TChannel*,MNEMONIC*); //!

     TPaces& operator=(const TPaces&);  //! 


   private: 
     TPacesData *pacesdata;                 //!  Used to build GRIFFIN Hits
     std::vector <TPacesHit> paces_hits; //  The set of crystal hits
		
     static bool fSetCoreWave;		         //!  Flag for Waveforms ON/OFF

     static long fCycleStart;                //!  The start of the cycle
     static long fLastPPG;                   //!  value of the last ppg

     enum  PacesFlags{kCycleStartTime,kTapeMove,kBackGround,kBeamOn,kDecay};
     TBits fPacesBits;

   public:
     static bool SetCoreWave()        { return fSetCoreWave;  }	//!

     void SetTapeMove(Bool_t flag=kTRUE)   { fPacesBits.SetBitNumber(kTapeMove,flag); }  //!
     void SetBackground(Bool_t flag=kTRUE) { fPacesBits.SetBitNumber(kBackGround,flag);} //!
     void SetBeamOn(Bool_t flag=kTRUE)     { fPacesBits.SetBitNumber(kBeamOn,flag);}     //!
     void SetDecay(Bool_t flag=kTRUE)      { fPacesBits.SetBitNumber(kDecay,flag);}      //!

     bool GetTapeMove()   const { return fPacesBits.TestBitNumber(kTapeMove);}//!
     bool GetBackground() const { return fPacesBits.TestBitNumber(kBackGround);}//!
     bool GetBeamOn()     const { return fPacesBits.TestBitNumber(kBeamOn);}//!
     bool GetDecay()      const { return fPacesBits.TestBitNumber(kDecay);}//!

     static int GetCycleTimeInMilliSeconds(long time) { return (int)((time-fCycleStart)/1e5); }//!

   //  void AddHit(TGRSIDetectorHit *hit,Option_t *opt="");//!
   private:
    // static TVector3 gCloverPosition[17];               //! Position of each HPGe Clover
     void ClearStatus() { fPacesBits.ResetAllBits(kFALSE); } //!     

   public:         
     virtual void Copy(TPaces&) const;                //!
     virtual void Clear(Option_t *opt = "all");		     //!
     virtual void Print(Option_t *opt = "") const;		  //!

   protected:
     void PushBackHit(TGRSIDetectorHit* phit);

   ClassDef(TPaces,2)  // Paces Physics structure


};

#endif


