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
      enum Ebitflag {
         kIsDetSet      = 1<<0,  //same as BIT(0);
         kIsEnergySet   = 1<<1,
         kIsPositionSet = 1<<2,
         kIsSubDetSet   = 1<<3,
         kIsPPGSet      = 1<<4,
         kIsTimeSet     = 1<<5,
         kIsSegSet      = 1<<6,
         kBit7          = 1<<7,
         kIsAllSet      = 0xFF
      };


   public:
      TGRSIDetectorHit(const int& Address=0xffffffff);
      TGRSIDetectorHit(const TGRSIDetectorHit&);
      TGRSIDetectorHit(const TFragment& frag)      { Class()->IgnoreTObjectStreamer(); this->CopyFragment(frag); }
      void CopyFragment(const TFragment&);
      void CopyWaveform(const TFragment&);
      virtual ~TGRSIDetectorHit();

      static void SetPPGPtr(TPPG* ptr) { fPPG = ptr; }

   public:
      virtual void Copy(TObject&) const;              //!<!
      virtual void Clear(Option_t* opt = "");         //!<!
      virtual void Print(Option_t* opt = "") const;	  //!<!
      virtual const char *GetName() const;                   //!<!
      virtual bool HasWave() const { return (fWaveform.size()>0) ?  true : false; } //!<!

      static bool CompareEnergy(TGRSIDetectorHit* lhs, TGRSIDetectorHit* rhs);
      //We need a common function for all detectors in here
      //static bool Compare(TGRSIDetectorHit* lhs,TGRSIDetectorHit* rhs); //!<!

      inline void SetPosition(const TVector3& temp_pos)           { fPosition = temp_pos; SetFlag(kIsPositionSet,true); }    //!<!
      inline void SetAddress(const UInt_t& temp_address)          { fAddress = temp_address; } //!<!
      inline void SetCharge(const Float_t& temp_charge)           { fCharge = temp_charge; }   //!<!
      //virtual Short_t SetSegment(const Short_t &seg);
      virtual inline void SetCfd(const Int_t& x)                  { fCfd    = x; }             //!<!
      inline void SetWaveform(const std::vector<Short_t>& x)      { fWaveform = x; }           //!<!
      virtual inline void SetTimeStamp(const ULong_t& x)          { fTimeStamp   = x; }        //!<! Maybe make this abstract?

      virtual TVector3 SetPosition(Double_t temp_pos = 0);
      void SetEnergy(const double& en) { fEnergy = en; SetFlag(kIsEnergySet,true);}
      //virtual UInt_t SetDetector(const UInt_t& det);
      void SetTime(const Double_t& time) {fTime = time; SetFlag(kIsTimeSet,true); }

      TVector3 GetPosition(Double_t dist = 0) const; //!<!
      TVector3 GetPosition(Double_t dist = 0);
      virtual double GetEnergy(Option_t* opt="") const;
      virtual double GetEnergy(Option_t* opt="");
      virtual Int_t  GetDetector() const;
      virtual Int_t  GetSegment() const;	
      virtual ULong_t GetTimeStamp(Option_t* opt="")   const     { return fTimeStamp;   }
      virtual Double_t GetTime(Option_t* opt = "") const;  ///< Returns a time value to the nearest nanosecond!
      virtual Double_t GetTime(Option_t* opt = "");
      //virtual UInt_t GetDetector();
      //virtual Short_t GetSegment();
      virtual inline Int_t   GetCfd() const                          { return fCfd;}           //!<!
      virtual inline UInt_t GetAddress()     const                   { return fAddress; }      //!<!
      virtual inline Float_t GetCharge() const                       { return fCharge;}        //!<!
      inline TChannel* GetChannel() const                            { return TChannel::GetChannel(fAddress); }  //!<!
      inline std::vector<Short_t>* GetWaveform()                     { return &fWaveform; }    //!<!

      //The PPG is only stored in events that come out of the GRIFFIN DAQ
      uint16_t GetPPGStatus() const;
      uint16_t GetPPGStatus();
      uint16_t GetCycleTimeStamp() const;
      uint16_t GetCycleTimeStamp();

      static TVector3 *GetBeamDirection() { return &fBeamDirection; }

   private:
      virtual TVector3 GetChannelPosition(Double_t dist = 0) const { AbstractMethod("GetChannelPosition"); return TVector3(0., 0., 0.); }

   protected:
     //Bool_t IsDetSet() const    { return (fBitflags & kIsDetSet); }
      Bool_t IsPosSet() const    { return (fBitflags & kIsPositionSet); }
      Bool_t IsEnergySet() const { return (fBitflags & kIsEnergySet); }
      Bool_t IsSubDetSet() const { return (fBitflags & kIsSubDetSet); }
      Bool_t IsPPGSet() const    { return (fBitflags & kIsPPGSet); }
      Bool_t IsTimeSet() const   { return (fBitflags & kIsTimeSet); }
      //Bool_t IsSegSet() const	 { return (fBitflags & kIsSegSet); }
      void SetFlag(enum Ebitflag,Bool_t set);

   protected:
      UInt_t   fAddress;    ///< address of the the channel in the DAQ.
      Float_t  fCharge;     ///< charge collected from the hit
      Int_t    fCfd;        ///< CFD time of the Hit
      ULong_t  fTimeStamp;  ///< Timestamp given to hit
      std::vector<Short_t> fWaveform;  ///<

   private:
      Double_t fTime;       //!<! Calibrated Time of the hit
      //UInt_t   fDetector;   //!<! Detector Number
      //Short_t  fSegment;	  //!<! Segment number
      TVector3 fPosition;   //!<! Position of hit detector.
      Double_t fEnergy;     //!<! Energy of the Hit.
      uint16_t fPPGStatus;  //!<! 
      ULong_t  fCycleTimeStamp; //!<!

   protected:
      static TPPG* fPPG;

   private:
      //flags   
      UChar_t fBitflags;

      static TVector3 fBeamDirection; //!

      /// \cond CLASSIMP
      ClassDef(TGRSIDetectorHit,7) //Stores the information for a detector hit
         /// \endcond
};
/*! @} */
#endif
