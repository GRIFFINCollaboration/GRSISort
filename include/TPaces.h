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
     void BuildHits(TDetectorData *data =0,Option_t *opt = ""); //!

     TPacesHit *GetPacesHit(const int& i); //!
     TGRSIDetectorHit* GetHit(const Int_t& idx = 0);
     Short_t GetMultiplicity() const {return paces_hits.size();}
     
     static TVector3 GetPosition(int DetNbr);		//!
     void FillData(TFragment*,TChannel*,MNEMONIC*); //!
	  void BuildHits(TFragment*, MNEMONIC*); //!

     TPaces& operator=(const TPaces&);  //! 


   private: 
     TPacesData *pacesdata;                 //!  Used to build GRIFFIN Hits
     std::vector <TPacesHit> paces_hits; //  The set of crystal hits
		
     static bool fSetCoreWave;		         //!  Flag for Waveforms ON/OFF

   public:
     static bool SetCoreWave()        { return fSetCoreWave;  }	//!

   //  void AddHit(TGRSIDetectorHit *hit,Option_t *opt="");//!
   private:
    // static TVector3 gCloverPosition[17];               //! Position of each HPGe Clover

   public:         
     virtual void Copy(TObject&) const;                //!
     virtual void Clear(Option_t *opt = "all");		     //!
     virtual void Print(Option_t *opt = "") const;		  //!

   protected:
     void PushBackHit(TGRSIDetectorHit* phit);

   ClassDef(TPaces,3)  // Paces Physics structure


};

#endif


