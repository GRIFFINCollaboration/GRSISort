#ifndef TPACES_H
#define TPACES_H

#include "Globals.h"

#include <vector>
#include <cstdio>

#include <TPacesHit.h>
#include <TPacesData.h>
#include <TVector3.h> 
#include <TClonesArray.h>

#include <TGRSIDetector.h> 


class TPaces : public TGRSIDetector {

  public:
     TPaces();
     TPaces(const TPaces&);
     virtual ~TPaces();

  public: 
     void BuildHits(TGRSIDetectorData *data =0,Option_t *opt = ""); //!

     TPacesHit *GetPacesHit(int i)           {	return (TPacesHit*)(paces_hits[i]);   }	//!
     Short_t GetMultiplicity() const	      {	return paces_hits.GetEntries();}	//!

     void FillData(TFragment*,TChannel*,MNEMONIC*); //!

     TPaces& operator=(const TPaces&);  //! 

   private: 
     TPacesData *pacesdata;                 //!  Used to build PACES Hits
     TClonesArray paces_hits;                //   The set of crystal hits
	
     static bool fSetCoreWave;		         //!  Flag for Waveforms ON/OFF
 
     static long fCycleStart;                //!  The start of the cycle
     static long fLastPPG;                   //!  value of the last ppg

     long fCycleStartTime;                   //   The start of the cycle as it's saved to the tree
     bool ftapemove;                         //
     bool fbackground;                       //
     bool fbeamon;                           // 
     bool fdecay;                            // 

   public:
     static bool SetCoreWave()      { return fSetCoreWave;  }	//!

     void SetTapeMove()     { ftapemove = kTRUE; }//!
     void SetBackground()   { fbackground = kTRUE;}//!
     void SetBeamOn()       { fbeamon = kTRUE;}//!
     void SetDecay()        { fdecay = kTRUE;}//!

     bool GetTapeMove()   const { return ftapemove;  }//!
     bool GetBackground() const { return fbackground;}//!
     bool GetBeamOn()     const { return fbeamon;    }//!
     bool GetDecay()      const { return fdecay;     }//!

     static int GetCycleTimeInMilliSeconds(long time) { return (int)((time-fCycleStart)/1e5); }//!

   private:
     void ClearStatus() { ftapemove = kFALSE; fbackground = kFALSE; fbeamon = kFALSE; fdecay = kFALSE;}//!     

   public:         
     virtual void Clear(Option_t *opt = "");		      //!
     virtual void Print(Option_t *opt = "");		//!

   ClassDef(TPaces,1)  // Paces Physics structure


};

#endif


