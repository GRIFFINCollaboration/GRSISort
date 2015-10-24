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
#include "TPPG.h"
#include "TFile.h"

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
   // The bare bones; stuff all detectors need.  
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

	// 
   public:
   enum Ebitflag {
      kIsDetSet      = 1<<0,
      kIsEnergySet   = 1<<1,
      kIsPositionSet = 1<<2,
      kIsSubDetSet   = 1<<3,
		kIsPPGSet      = 1<<4,
		kIsTimeSet     = 1<<5,
		kBit6          = 1<<6,
		kBit7          = 1<<7,
      kIsAllSet      = 0xFF
   };


	public:
		TGRSIDetectorHit(const int &Address=0xffffffff);    //{ address=fAddress; }
		TGRSIDetectorHit(const TGRSIDetectorHit&);
	   TGRSIDetectorHit(const TFragment &frag)      { Class()->IgnoreTObjectStreamer(); this->CopyFragment(frag); }
      void CopyFragment(const TFragment&);
      virtual ~TGRSIDetectorHit();

      static void SetPPGPtr(TPPG* ptr) { fPPG = ptr; }

	public:
      virtual void Copy(TObject&) const;              //!
      virtual void Clear(Option_t* opt = "");         //!
      virtual void Print(Option_t* opt = "") const;	//!
      static bool CompareEnergy(TGRSIDetectorHit *lhs, TGRSIDetectorHit *rhs);
      //We need a common function for all detectors in here
		//static bool Compare(TGRSIDetectorHit *lhs,TGRSIDetectorHit *rhs); //!

      inline void SetPosition(const TVector3& temp_pos)           { fposition = temp_pos; SetFlag(kIsEnergySet,true); }    //!
      inline void SetAddress(const UInt_t &temp_address)          { faddress = temp_address; } //!
      inline void SetCharge(const Float_t &temp_charge)            { fcharge = temp_charge;} //!
  //    inline void SetParent(TGRSIDetector *fParent)               { parent = (TObject*)fParent ; } //!
      virtual inline void SetCfd(const Int_t &x)                  { fcfd    = x;   }                  //!
      inline void SetWaveform(const std::vector<Short_t> &x)             { fwaveform = x;    } //!
      virtual inline void SetTimeStamp(const ULong_t &x)               { ftimestamp   = x;   }                  //! Maybe make this abstract?

      virtual TVector3 SetPosition(Double_t temp_pos = 0);
      void SetEnergy(const double &en) { fenergy = en; SetFlag(kIsEnergySet,true);}
      virtual UInt_t SetDetector(const UInt_t &det);
      void SetTime(const Double_t &time) {ftime = time; SetFlag(kIsTimeSet,true); }

		TVector3 GetPosition(Double_t dist = 0) const; //!
      TVector3 GetPosition(Double_t dist = 0);
      virtual double GetEnergy(Option_t *opt="") const;
      virtual double GetEnergy(Option_t *opt="");
      virtual UInt_t GetDetector() const;
      //virtual double GetTime(Option_t *opt="")   const    { AbstractMethod("GetTime()"); return 0.00;   }  // Returns a time value to the nearest nanosecond!
      virtual ULong_t GetTimeStamp(Option_t *opt="")   const     { return ftimestamp;   }  // Returns a time value to the nearest nanosecond!
      virtual Double_t GetTime(Option_t *opt = "") const;
      virtual Double_t GetTime(Option_t *opt = "");
      virtual UInt_t GetDetector();
      virtual inline Int_t   GetCfd() const                          {   return fcfd;      }           //!
      virtual inline UInt_t GetAddress()     const                   { return faddress; }         //!
      virtual inline Float_t GetCharge() const                       { return fcharge;} //!
      inline TChannel *GetChannel() const                            { return TChannel::GetChannel(faddress); }  //!
      inline std::vector<Short_t> *GetWaveform()                     { return &fwaveform; } //!
    //  inline TGRSIDetector *GetParent() const               { return ((TGRSIDetector*)parent.GetObject()); } //!
      
      //The PPG is only stored in events that come out of the GRIFFIN DAQ
      uint16_t GetPPGStatus() const;
      uint16_t GetPPGStatus();
      uint16_t GetCycleTimeStamp() const;
      uint16_t GetCycleTimeStamp();

   private:
      virtual TVector3 GetChannelPosition(Double_t dist = 0) const { AbstractMethod("GetChannelPosition"); return TVector3(0.0,0.0,0.0); }

     // unsigned int GetHighestBitSet(UChar_t flag);

   protected:
      Bool_t IsDetSet() const {return (fbitflags & kIsDetSet);}
      Bool_t IsPosSet() const {return (fbitflags & kIsPositionSet);}
      Bool_t IsEnergySet() const {return (fbitflags & kIsEnergySet);} 
      Bool_t IsSubDetSet() const {return (fbitflags & kIsSubDetSet);}
      Bool_t IsPPGSet() const {return (fbitflags & kIsPPGSet);}
      Bool_t IsTimeSet() const {return (fbitflags & kIsTimeSet); }
      void SetFlag(enum Ebitflag,Bool_t set);

   private:
      UInt_t   faddress;    //address of the the channel in the DAQ.
      Float_t  fcharge;     //charge collected from the hit
      Int_t    fcfd;        // CFD time of the Hit
      ULong_t  ftimestamp; // Timestamp given to hit
      Double_t ftime;      //! Calibrated Time of the hit
      UInt_t   fdetector;   //! Detector Number
      TVector3 fposition;   //! Position of hit detector.
      Double_t fenergy;     //! Energy of the Hit.
   //   TRef      parent;   // pointer to the mother class;
      std::vector<Short_t> fwaveform;  //
      //Bool_t fHitSet;    //!
      uint16_t fPPGStatus; //! 
      ULong_t  fCycleTimeStamp; //!

   protected:
      static TPPG* fPPG;

   private:
   //flags   
      UChar_t fbitflags;
      
      //Bool_t fDetectorSet;//!
      //Bool_t fPosSet;//!
      //Bool_t fEnergySet;//!

	ClassDef(TGRSIDetectorHit,6) //Stores the information for a detector hit
};




#endif
