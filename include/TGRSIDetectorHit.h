#ifndef TGRSIDETECTORHIT_H
#define TGRSIDETECTORHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>

#include "Globals.h"

#include "TChannel.h"
#include "TVector3.h" 
#include "TObject.h" 
#include "TRef.h"
#include "Rtypes.h"
#include "TPPG.h"
#include "TFile.h"
#include "TString.h"

class TGRSIDetector;

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIDetectorHit
///
/// This is class that contains the basic info about detector
/// hits:
/// 1. An address.         The whoami for the detector. This is the value used by TChannel::GetChannel(address);
///
/// 2. An "Energy value."  What this is left the parent class, but it is going to return a double.
///
/// 3. A   Time  value.     This should be a time value common for detectors (derived from the timestamp)
///                        Units matter here, I am adobting the ns as the standard.
///
/// 4. A   Position.       Tvector3s are nice, they make doing geometery trival.  Each hit needs one to determine
///                        where it is in space, the actual memory of the thing will be stored here.
///                        ***  This is not actually needed here unless we start do waveform analysis to 
///                        ***  better determine where the hit is.
///
/// 5. The waveform.       Since we are dealing with digital daqs, a waveform is a fairly common thing to have.  It
///                        may not always be present, put it is echoed enough that the storage for it belongs here.
///
/////////////////////////////////////////////////////////////////

class TGRSIDetectorHit : public TObject 	{

   // 
   public:
      enum EBitFlag {
         kIsEnergySet   = BIT(0),  //same as BIT(0);
         kIsChannelSet  = BIT(1),
         kBit2          = BIT(2),
         kBit3          = BIT(3),
         kIsPPGSet      = BIT(4),
         kIsTimeSet     = BIT(5),
         //reserved for derived class.
         kDetHitBit0    = BIT(6),
         kDetHitBit1    = BIT(7),
         kBase          = BIT(9),
         kIsAllSet      = 0xFFFF
      };

	enum ETimeFlag {
		kNone          = BIT(0),
		kCFD           = BIT(1),
		kWalk          = BIT(2),
		kOffset        = BIT(3),
		kAll           = 0xFFFF
	};


   public:
      TGRSIDetectorHit(const int& Address=0xffffffff);
      TGRSIDetectorHit(const TGRSIDetectorHit&, bool copywave=true);
      //TGRSIDetectorHit(const TFragment& frag) { Class()->IgnoreTObjectStreamer(); this->CopyFragment(frag); }
      //void CopyFragment(const TFragment&);
      //void CopyWaveform(const TFragment&);
      virtual ~TGRSIDetectorHit();

      static void SetPPGPtr(TPPG* ptr) { fPPG = ptr; }

      bool operator<(const TGRSIDetectorHit &rhs) const { return GetEnergy() > rhs.GetEnergy();} //sorts large->small


   public:
      virtual void Copy(TObject&) const;                //!<!
      virtual void Copy(TObject&,bool copywave) const;  //!<!
      virtual void CopyWave(TObject&)  const;  //!<!
      virtual void Clear(Option_t* opt = "");           //!<!
      virtual void ClearTransients() const { fBitflags = 0; }
      virtual void Print(Option_t* opt = "") const;     //!<!
      virtual bool HasWave() const { return (fWaveform.size()>0) ?  true : false; } //!<!

      static bool CompareEnergy(TGRSIDetectorHit* lhs, TGRSIDetectorHit* rhs);
      //We need a common function for all detectors in here
      //static bool Compare(TGRSIDetectorHit* lhs,TGRSIDetectorHit* rhs); //!<!

      void SetAddress(const UInt_t& temp_address)       { fAddress = temp_address; } //!<!
      void SetKValue(const Short_t& temp_kval)          { fKValue = temp_kval;   }   //!<!
      void SetCharge(const Float_t& temp_charge)        { fCharge = temp_charge; }   //!<!
      void SetCharge(const Int_t& temp_charge)          { fCharge = temp_charge + gRandom->Uniform(); }   //!<!
      virtual void SetCfd(const Int_t& x)               { fCfd    = x; }             //!<!
      void SetWaveform(const std::vector<Short_t>& x)   { fWaveform = x; }           //!<!
      virtual void SetTimeStamp(const Long_t& x)        { fTimeStamp   = x; }        //!<! 
      virtual void AppendTimeStamp(const Long_t& x)     { fTimeStamp   += x; }       //!<! 

      Double_t SetEnergy(const double& en) const { fEnergy = en; SetBit(kIsEnergySet,true); return fEnergy;}
      void SetTime(const Double_t& time) {fTime = time; SetBit(kIsTimeSet,true); }

      virtual TVector3 GetPosition(Double_t dist)    const { return TVector3(0.,0.,0.); } //!<!
      virtual TVector3 GetPosition()    const { return TVector3(0.,0.,0.); } //!<!
      virtual double GetEnergy(Option_t* opt="")     const;
      virtual Long_t GetTimeStamp(Option_t* opt="") const;
      Long_t GetRawTimeStamp(Option_t* opt="") const { return fTimeStamp; }
      virtual Double_t GetTime(const UInt_t& correct_flag = kAll, Option_t* opt = "")   const;  ///< Returns a time value to the nearest nanosecond!
      virtual Int_t   GetCfd()    const      { return fCfd;}                 //!<!
      virtual UInt_t GetAddress() const      { return fAddress; }            //!<!
      virtual Int_t  GetCharge()  const      ;                               //!<!
      virtual Float_t Charge()    const      { return fCharge; }             //!<!
      virtual Short_t GetKValue() const      { return fKValue; }             //!<!
      TChannel* GetChannel()      const      { if(!IsChannelSet()) { fChannel = TChannel::GetChannel(fAddress); SetBit(kIsChannelSet, true); } return fChannel; }
      //TODO: Fix Getters to have non-const types
	std::vector<Short_t>* GetWaveform()    { return &fWaveform; }          //!<!

      //stored in the tchannel (things common to all hits of this address)
      virtual Int_t  GetDetector()  const; //!<!
      virtual Int_t  GetSegment()   const; //!<!	
      virtual Int_t  GetCrystal()   const; //!<!
      virtual const char *GetName() const; //!<!
      virtual UShort_t  GetArrayNumber() const; //!<!

      //virtual void GetSegment() const; 

      //The PPG is only stored in events that come out of the GRIFFIN DAQ
      uint16_t GetPPGStatus() const;
      uint16_t GetCycleTimeStamp() const;

      void ClearEnergy()  { fEnergy  = 0.0;  SetBit(kIsEnergySet,false); }
      void ClearChannel() { fChannel = NULL; SetBit(kIsChannelSet,false); }

      static TVector3 *GetBeamDirection() { return &fBeamDirection; }

   private:
 //     virtual TVector3 GetChannelPosition(Double_t dist) const { AbstractMethod("GetChannelPosition"); return TVector3(0., 0., 0.); }

   protected:
      Bool_t IsEnergySet()  const { return (fBitflags & kIsEnergySet); }
      Bool_t IsChannelSet() const { return (fBitflags & kIsChannelSet); }
      Bool_t IsTimeSet()    const { return (fBitflags & kIsTimeSet); }
      Bool_t IsPPGSet()     const { return (fBitflags & kIsPPGSet); }


      
      // Forcing use of enum EBitFlag made it unpleasant to use own names in inheriting detectors 
      void SetBit(UInt_t,Bool_t set=true) const; //const here is dirty
      bool TestBit(UInt_t flag) const { return fBitflags & flag; }
      
   protected:
      UInt_t   fAddress;    ///< address of the the channel in the DAQ.
      Float_t  fCharge;     ///< charge collected from the hit
      Short_t  fKValue;      ///< integration value.
      Int_t    fCfd;        ///< CFD time of the Hit
      Long_t   fTimeStamp;  ///< Timestamp given to hit
      std::vector<Short_t> fWaveform;  ///<

   private:
      mutable Double_t fTime;       //!<! Calibrated Time of the hit
      //UInt_t   fDetector;   //!<! Detector Number
      //Short_t  fSegment;	  //!<! Segment number
     // mutable TVector3 fPosition;   //!<! Position of hit detector.
      mutable Double_t fEnergy;     //!<! Energy of the Hit.
      mutable uint16_t fPPGStatus;  //!<! 
      mutable ULong_t  fCycleTimeStamp; //!<!
      mutable TChannel* fChannel; //!<!

   protected:
      static TPPG* fPPG;

   private:
      //flags   
      mutable UChar_t fBitflags;
      static TVector3 fBeamDirection; //!

      /// \cond CLASSIMP
      ClassDef(TGRSIDetectorHit,9) //Stores the information for a detector hit
      /// \endcond
};
/*! @} */
#endif
