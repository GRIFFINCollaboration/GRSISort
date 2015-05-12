#ifndef TGRSIDETECTORHIT_H
#define TGRSIDETECTORHIT_H


#include "Globals.h"

//#include <cstdio>
//#include <utility>
#include <vector>

#include "TChannel.h"
#include "TVector3.h" 
#include "TObject.h" 
#include "TRef.h"
#include "Rtypes.h"

class TGRSIDetector;


////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIDetectorHit                                           //
//                                                            //
// This is class that contains the basic info about detector  //
// hits. This is where the position of a detector is stored.
//                                                            //
////////////////////////////////////////////////////////////////

class TGRSIDetectorHit : public TObject 	{
   // The bare bones; stuff all detecotors need.  
   // 1. An address.         The whoami for the detector. This is the value used by TChannel::GetChannel(address);
   // 2. An "Energy value."  What this is left the parent class, but it is going to return a double.
   // 3. A   Time  value.     This should be a time value common for detectors (derived from the timestamp)
   //                        Units matter here, I am adobting the ns as the standard.
                           
   // 4. A   Position.       Tvector3s are nice, they make doing geometery trival.  Each hit needs one to determine
   //                        where it is in space, the actual memory of the thing will be stored here.
   //                        ***  This is not actually needed here unless we start do waveform analysis to 
   //                        ***  better determine where the hit is.
                           
   // 5. The waveform.       Since we are dealing with digital daqs, a waveform is a fairly common thing to have.  It
   //                        may not allows be present, put it is echoed enough that the storage for it belongs here.


	public:
		TGRSIDetectorHit(const int &fAddress=0xffffffff);    //{ address=fAddress; }
		TGRSIDetectorHit(const TGRSIDetectorHit&);
		virtual ~TGRSIDetectorHit();

	public:
      virtual void Copy(TGRSIDetectorHit &) const;    //!
		virtual void Clear(Option_t* opt = "");         //!
		virtual void Print(Option_t* opt = "") const;	//!
      //We need a common function for all detectors in here
		//static bool Compare(TGRSIDetectorHit *lhs,TGRSIDetectorHit *rhs); //!

		inline void SetPosition(const TVector3& temp_pos) { position = temp_pos; } //!
		inline TVector3 GetPosition() const { return position; }	//!
      
      //Abstract methods. These are required in all derived classes
      virtual double GetEnergy(Option_t *opt="") const;
      virtual double GetTime(Option_t *opt="")   const { AbstractMethod("GetTime()");   }  // Returns a time value to the nearest nanosecond!
      virtual inline void SetTime(const Long_t &x)         { time   = x;   }                  //! Maybe make this abstract?
      //virtual void SetHit() { AbstractMethod("SetHit()");}
      //We need a common function for all detectors in here
		//static bool Compare(TGRSIDetectorHit *lhs,TGRSIDetectorHit *rhs); //!


      inline void  SetAddress(const Int_t &temp_address) { address = temp_address; } //!
      inline Int_t GetAddress()     const                { return address; }         //!

      inline void SetCharge(const Int_t &temp_charge) { charge = temp_charge;} //!
      inline Int_t GetCharge() const                    { return charge;} //!

      virtual inline void SetCfd(const int &x)             { cfd    = x;   }                  //!
      virtual inline Int_t    GetCfd() const                 {   return cfd;      }           //!
 

      inline TChannel *GetChannel() const                { return TChannel::GetChannel(address); }  //!

      inline void SetWaveform(std::vector<Short_t> x) { waveform = x;    } //!
      inline std::vector<Short_t> GetWaveForm() const { return waveform; } //!

      inline void           SetParent(TGRSIDetector *fParent)   { parent = (TObject*)fParent ; } //!
      inline TGRSIDetector *GetParent() const                   { return ((TGRSIDetector*)parent.GetObject()); } //!

   protected:
      Int_t     address;  //address of the the channel in the DAQ.
      Int_t     charge;   //charge collected from the hit
      Int_t     cfd;
      Long_t    time;
      TVector3  position; //! Position of hit detector.
      Double_t  energy;   //! Energy of the Hit.
      TRef      parent;   //pointer to the mother class;
      std::vector<Short_t> waveform;  
      //Bool_t fHitSet;    //!
      //Bool_t fDetectorSet;//!
      //Bool_t fPosSet;//!
      //Bool_t fEnergySet;//!

	ClassDef(TGRSIDetectorHit,2) //Stores the information for a detector hit
};




#endif
