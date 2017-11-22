#ifndef TCSMHIT_H
#define TCSMHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <utility>
#include <iostream>

#include "TChannel.h"
#include "TVector3.h"

#include "Globals.h"

#include "TGRSIDetectorHit.h"

class TCSMHit : public TGRSIDetectorHit {
public:
   TCSMHit();
   ~TCSMHit() override;

private:
   Short_t fHorDStrip;  //
   Float_t fHorDCharge; //
   Int_t   fHorDCfd;    //

   Short_t fVerDStrip;  //
   Float_t fVerDCharge; //
   Int_t   fVerDCfd;    //

   Short_t fHorEStrip;  //
   Float_t fHorECharge; //
   Int_t   fHorECfd;    //

   Short_t fVerEStrip;  //
   Float_t fVerECharge; //
   Int_t   fVerECfd;    //

   Double_t fHorDEnergy; //
   Double_t fVerDEnergy; //
   Double_t fHorDTime;   //
   Double_t fVerDTime;   //
   TVector3 fDPosition;    //

   Double_t fHorEEnergy; //
   Double_t fVerEEnergy; //
   Double_t fHorETime;   //
   Double_t fVerETime;   //

   TVector3 fEPosition; //

   UShort_t fDetectorNumber; //

public:
   bool IsEmpty();

   void Clear(Option_t* = "") override;       //!<!
   void Print(Option_t* = "") const override; //!<!

   // static bool Compare(TCSMHit *lhs,TCSMHit *rhs); //!<!

   Double_t GetEHorizontalEnergy() const { return fHorEEnergy; }; //!<!
   Double_t GetEVerticalEnergy() const { return fVerEEnergy; };   //!<!
   Double_t GetEEnergy() const;
   Double_t GetETime() const { return fHorETime; } //!<!

   Double_t GetDHorizontalEnergy() const { return fHorDEnergy; }; //!<!
   Double_t GetDVerticalEnergy() const { return fVerDEnergy; };   //!<!
   Double_t GetDEnergy() const;
   Double_t GetDTime() const { return fVerDTime; } //!<!

   UShort_t GetDetectorNumber() const { return fDetectorNumber; } //!<!

   Float_t GetEHorizontalCharge() const { return fHorECharge; } //!<!
   Float_t GetEVerticalCharge() const { return fVerECharge; }   //!<!

   Float_t GetDHorizontalCharge() const { return fHorDCharge; } //!<!
   Float_t GetDVerticalCharge() const { return fVerDCharge; }   //!<!

   Double_t GetEHorizontalTime() const { return fHorETime; } //!<!
   Double_t GetEVerticalTime() const { return fVerETime; }   //!<!

   Double_t GetDHorizontalTime() const { return fHorDTime; } //!<!
   Double_t GetDVerticalTime() const { return fVerDTime; }   //!<!

   Int_t GetEHorizontalStrip() const { return fHorEStrip; } //!<!
   Int_t GetEVerticalStrip() const { return fVerEStrip; }   //!<!

   Int_t GetDHorizontalStrip() const { return fHorDStrip; } //!<!
   Int_t GetDVerticalStrip() const { return fVerDStrip; }   //!<!

   Int_t GetEHorizontalCFD() const { return fHorECfd; } //!<!
   Int_t GetEVerticalCFD() const { return fVerECfd; }   //!<!

   Int_t GetDHorizontalCFD() const { return fHorDCfd; } //!<!
   Int_t GetDVerticalCFD() const { return fVerDCfd; }   //!<!

   TVector3 GetEPosition() const { return fEPosition; } //!<!
   TVector3 GetDPosition() const { return fDPosition; } //!<!

   Double_t GetDdE_dx() const { return GetDEnergy() / GetDthickness(); }
   Double_t GetDthickness() const;

   TVector3 GetPosition(Double_t = 0) const override { return fDPosition; }                  //!<!
   Double_t GetEnergy(Option_t* = "") const override { return GetDEnergy() + GetEEnergy(); } //!<!

   Double_t GetTime(const ETimeFlag& = ETimeFlag::kAll, Option_t* = "") const override
   {
      return fVerDTime;
   } ///< Returns fVerDTime

   inline void SetDetectorNumber(const Int_t& tempnum) { fDetectorNumber = tempnum; } //!<!

   inline void SetEHorizontalCharge(const Float_t temp) { fHorECharge = temp; } //!<!
   inline void SetEVerticalCharge(const Float_t temp) { fVerECharge = temp; }   //!<!

   inline void SetDHorizontalCharge(const Float_t temp) { fHorDCharge = temp; } //!<!
   inline void SetDVerticalCharge(const Float_t temp) { fVerDCharge = temp; }   //!<!

   inline void SetEHorizontalStrip(const Int_t temp) { fHorEStrip = temp; } //!<!
   inline void SetEVerticalStrip(const Int_t temp) { fVerEStrip = temp; }   //!<!

   inline void SetDHorizontalStrip(const Int_t temp) { fHorDStrip = temp; } //!<!
   inline void SetDVerticalStrip(const Int_t temp) { fVerDStrip = temp; }   //!<!

   inline void SetEHorizontalCFD(const Int_t tempd) { fHorECfd = tempd; } //!<!
   inline void SetEVerticalCFD(const Int_t tempd) { fVerECfd = tempd; }   //!<!

   inline void SetDHorizontalCFD(const Int_t tempd) { fHorDCfd = tempd; } //!<!
   inline void SetDVerticalCFD(const Int_t tempd) { fVerDCfd = tempd; }   //!<!

   inline void SetEHorizontalTime(const Int_t tempd) { fHorETime = tempd; } //!<!
   inline void SetEVerticalTime(const Int_t tempd) { fVerETime = tempd; }   //!<!

   inline void SetDHorizontalTime(const Int_t tempd) { fHorDTime = tempd; } //!<!
   inline void SetDVerticalTime(const Int_t tempd) { fVerDTime = tempd; }   //!<!

   inline void SetEPosition(const TVector3& tempp) { fEPosition = tempp; } //!<!
   inline void SetDPosition(const TVector3& tempp) { fDPosition = tempp; } //!<!

   inline void SetDHorizontalEnergy(const Double_t tempd) { fHorDEnergy = tempd; }
   inline void SetDVerticalEnergy(const Double_t tempd) { fVerDEnergy = tempd; }

   inline void SetEHorizontalEnergy(const Double_t tempd) { fHorEEnergy = tempd; }
   inline void SetEVerticalEnergy(const Double_t tempd) { fVerEEnergy = tempd; }

   /// \cond CLASSIMP
   ClassDefOverride(TCSMHit, 4)
   /// \endcond
};
/*! @} */
#endif
