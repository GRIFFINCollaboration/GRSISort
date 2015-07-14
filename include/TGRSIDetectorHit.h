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
      static bool CompareEnergy(TGRSIDetectorHit *lhs, TGRSIDetectorHit *rhs);
      //We need a common function for all detectors in here
		//static bool Compare(TGRSIDetectorHit *lhs,TGRSIDetectorHit *rhs); //!

		inline void SetPosition(const TVector3& temp_pos)           { position = temp_pos; } //!
      inline void SetAddress(const UInt_t &temp_address)          { address = temp_address; } //!
      inline void SetCharge(const Int_t &temp_charge)            { charge = temp_charge;} //!
  //    inline void SetParent(TGRSIDetector *fParent)               { parent = (TObject*)fParent ; } //!
      virtual inline void SetCfd(const Int_t &x)           { cfd    = x;   }                  //!
      inline void SetWaveform(std::vector<Short_t> x)             { waveform = x;    } //!
      virtual inline void SetTime(const ULong_t &x)               { time   = x;   }                  //! Maybe make this abstract?
 
      void SetPosition(Double_t temp_pos = 0);
      void SetEnergy(double en) { energy = en; is_energy_set = true; }
      virtual UInt_t SetDetector(UInt_t det);
      
      //Abstract methods. These are required in all derived classes
		virtual TVector3 GetPosition(Double_t dist = 0) const; //!
      virtual double GetEnergy(Option_t *opt="") const;
      virtual double GetEnergy(Option_t *opt="");
      virtual UInt_t GetDetector() const;
      virtual UInt_t GetDetector();
      virtual double GetTime(Option_t *opt="")   const      {return 0.0; } //AbstractMethod("GetTime()"); return 0.00;   }  // Returns a time value to the nearest nanosecond!
      virtual inline Int_t   GetCfd() const             {   return cfd;      }           //!
      inline UInt_t GetAddress()     const                  { return address; }         //!
      inline Double_t GetCharge() const                       { return charge;} //!
      inline TChannel *GetChannel() const                   { return TChannel::GetChannel(address); }  //!
      inline std::vector<Short_t> GetWaveform() const       { return waveform; } //!
   //   inline TGRSIDetector *GetParent() const               { return ((TGRSIDetector*)parent.GetObject()); } //!
      //virtual void SetHit() { AbstractMethod("SetHit()");}
      //We need a common function for all detectors in here
		//static bool Compare(TGRSIDetectorHit *lhs,TGRSIDetectorHit *rhs); //!

   protected:
      UInt_t     address;  //address of the the channel in the DAQ.
      Double_t  charge;   //charge collected from the hit
      Int_t     cfd;     // CFD time of the Hit
      ULong_t    time;    // Timsstamp given to hit
      UInt_t    detector; //! Detector Number
      TVector3  position; //! Position of hit detector.
      Double_t  energy;   //! Energy of the Hit.
   //   TRef      parent;   // pointer to the mother class;
     std::vector<Short_t> waveform;  //
      //Bool_t fHitSet;    //!
 
   //flags   
   protected:  
      Bool_t is_det_set;   //!
      Bool_t is_pos_set;   //!
      Bool_t is_energy_set;   //!
      
      //Bool_t fDetectorSet;//!
      //Bool_t fPosSet;//!
      //Bool_t fEnergySet;//!

	ClassDef(TGRSIDetectorHit,2) //Stores the information for a detector hit
};




#endif
