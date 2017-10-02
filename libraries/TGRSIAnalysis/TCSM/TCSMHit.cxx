#include "TCSMHit.h"

/// \cond CLASSIMP
ClassImp(TCSMHit)
/// \endcond

TCSMHit::TCSMHit()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TCSMHit::~TCSMHit() = default;

void TCSMHit::Clear(Option_t*)
{
   fHorDStrip  = -1;
   fHorDCharge = 0.0;
   fHorDCfd    = 0.0;

   fVerDStrip  = -1;
   fVerDCharge = 0.0;
   fVerDCfd    = 0.0;

   fHorEStrip  = -1;
   fHorECharge = 0.0;
   fHorECfd    = 0.0;

   fVerEStrip  = -1;
   fVerECharge = 0.0;
   fVerECfd    = 0.0;

   fHorDEnergy = 0.0;
   fVerDEnergy = 0.0;
   fHorDTime   = 0.0;
   fVerDTime   = 0.0;
   fDPosition.SetXYZ(0, 0, 1);

   fHorEEnergy = 0.0;
   fVerEEnergy = 0.0;
   fHorETime   = 0.0;
   fVerETime   = 0.0;
   fEPosition.SetXYZ(0, 0, 1);

   fDetectorNumber = 0; //
}

Double_t TCSMHit::GetDthickness() const
{
   double CSMthick[2][16][16] = {
      {{38.35, 38.75, 36.35, 36.05, 35.85, 35.45, 35.15, 35.2, 34.5, 34.65, 35.05, 35.05, 35.55, 34.65, 34.45, 32.05},
       {39.25, 39.65, 38.85, 38.15, 37.65, 36.55, 35.95, 35.25, 34.35, 34.75, 35.35, 35.25, 36.45, 34.75, 33.55, 31.95},
       {38.85, 39.05, 38.45, 38.15, 37.45, 36.45, 35.15, 34.25, 33.45, 33.75, 34.15, 34.25, 35.25, 34.65, 32.85, 31.25},
       {38.15, 38.05, 37.55, 36.85, 35.85, 35.55, 34.55, 33.45, 32.95, 33.25, 33.45, 33.65, 34.15, 34.25, 32.95, 31.55},
       {36.75, 36.95, 35.95, 35.85, 34.65, 34.75, 34.85, 33.65, 33.15, 33.15, 33.15, 33.45, 33.65, 33.85, 33.25, 31.95},
       {36.25, 36.45, 35.75, 35.35, 34.35, 34.75, 34.65, 34.35, 33.55, 33.05, 33.15, 33.05, 33.35, 33.65, 33.95, 32.75},
       {36.75, 36.45, 35.75, 35.45, 34.95, 34.65, 34.95, 35.15, 34.65, 33.55, 33.55, 32.95, 33.55, 33.15, 33.85, 32.65},
       {37.05, 36.35, 35.55, 35.45, 35.05, 34.45, 34.75, 37.15, 37.05, 34.15, 33.85, 33.15, 33.35, 32.35, 32.95, 31.65},
       {37.45, 36.35, 35.25, 35.45, 34.85, 34.65, 34.95, 36.65, 36.45, 33.65, 33.25, 33.05, 32.95, 32.05, 32.35, 31.05},
       {37.05, 36.05, 35.05, 35.35, 35.05, 35.05, 34.75, 34.35, 33.45, 32.85, 33.25, 33.55, 33.05, 32.15, 32.55, 30.95},
       {37.25, 36.85, 35.15, 35.15, 34.95, 34.95, 34.55, 33.75, 32.95, 32.75, 33.55, 33.95, 33.55, 32.85, 33.25, 31.25},
       {36.95, 36.05, 35.05, 35.25, 35.35, 35.35, 35.15, 34.65, 33.75, 33.75, 34.65, 35.35, 34.85, 34.05, 34.75, 32.55},
       {37.05, 37.05, 36.05, 35.95, 36.15, 35.95, 35.95, 35.95, 35.75, 35.25, 35.15, 35.65, 35.55, 35.25, 35.35, 33.95},
       {37.95, 38.35, 37.35, 37.05, 36.65, 36.55, 36.75, 36.75, 36.45, 36.45, 35.35, 35.45, 36.05, 35.55, 35.85, 34.25},
       {38.65, 38.75, 37.75, 37.05, 36.25, 36.55, 36.55, 36.95, 36.65, 36.55, 36.05, 35.05, 35.75, 35.55, 35.95, 34.45},
       {39.35, 39.25, 38.35, 37.65, 36.55, 36.85, 36.85, 37.35, 36.85, 36.75, 35.85, 35.15, 35.05, 35.75, 35.25,
        34.05}},

      {{38.35, 38.75, 36.35, 36.05, 35.85, 35.45, 35.15, 35.2, 34.5, 34.65, 35.05, 35.05, 35.55, 34.65, 34.45, 32.05},
       {39.25, 39.65, 38.85, 38.15, 37.65, 36.55, 35.95, 35.25, 34.35, 34.75, 35.35, 35.25, 36.45, 34.75, 33.55, 31.95},
       {38.85, 39.05, 38.45, 38.15, 37.45, 36.45, 35.15, 34.25, 33.45, 33.75, 34.15, 34.25, 35.25, 34.65, 32.85, 31.25},
       {38.15, 38.05, 37.55, 36.85, 35.85, 35.55, 34.55, 33.45, 32.95, 33.25, 33.45, 33.65, 34.15, 34.25, 32.95, 31.55},
       {36.75, 36.95, 35.95, 35.85, 34.65, 34.75, 34.85, 33.65, 33.15, 33.15, 33.15, 33.45, 33.65, 33.85, 33.25, 31.95},
       {36.25, 36.45, 35.75, 35.35, 34.35, 34.75, 34.65, 34.35, 33.55, 33.05, 33.15, 33.05, 33.35, 33.65, 33.95, 32.75},
       {36.75, 36.45, 35.75, 35.45, 34.95, 34.65, 34.95, 35.15, 34.65, 33.55, 33.55, 32.95, 33.55, 33.15, 33.85, 32.65},
       {37.05, 36.35, 35.55, 35.45, 35.05, 34.45, 34.75, 37.15, 37.05, 34.15, 33.85, 33.15, 33.35, 32.35, 32.95, 31.65},
       {37.45, 36.35, 35.25, 35.45, 34.85, 34.65, 34.95, 36.65, 36.45, 33.65, 33.25, 33.05, 32.95, 32.05, 32.35, 31.05},
       {37.05, 36.05, 35.05, 35.35, 35.05, 35.05, 34.75, 34.35, 33.45, 32.85, 33.25, 33.55, 33.05, 32.15, 32.55, 30.95},
       {37.25, 36.85, 35.15, 35.15, 34.95, 34.95, 34.55, 33.75, 32.95, 32.75, 33.55, 33.95, 33.55, 32.85, 33.25, 31.25},
       {36.95, 36.05, 35.05, 35.25, 35.35, 35.35, 35.15, 34.65, 33.75, 33.75, 34.65, 35.35, 34.85, 34.05, 34.75, 32.55},
       {37.05, 37.05, 36.05, 35.95, 36.15, 35.95, 35.95, 35.95, 35.75, 35.25, 35.15, 35.65, 35.55, 35.25, 35.35, 33.95},
       {37.95, 38.35, 37.35, 37.05, 36.65, 36.55, 36.75, 36.75, 36.45, 36.45, 35.35, 35.45, 36.05, 35.55, 35.85, 34.25},
       {38.65, 38.75, 37.75, 37.05, 36.25, 36.55, 36.55, 36.95, 36.65, 36.55, 36.05, 35.05, 35.75, 35.55, 35.95, 34.45},
       {39.35, 39.25, 38.35, 37.65, 36.55, 36.85, 36.85, 37.35, 36.85, 36.75, 35.85, 35.15, 35.05, 35.75, 35.25,
        34.05}}};

   return CSMthick[GetDetectorNumber() - 1][GetDVerticalStrip()][GetDHorizontalStrip()];
}

Double_t TCSMHit::GetDEnergy() const
{
   bool debug           = false;
   bool trustVertical   = true;
   bool trustHoriztonal = true;

   switch(GetDetectorNumber()) {
   case 1:
      switch(GetDHorizontalStrip()) {
      case 6:
      case 9: trustHoriztonal = false; break;
      }
      break;

   case 2:
      switch(GetDHorizontalStrip()) {
      // case 8:
      case 9:
      case 10: trustHoriztonal = false; break;
      }
      switch(GetDVerticalStrip()) {
      case 5: trustVertical = false; break;
      }
      break;

   case 3:
      switch(GetDHorizontalStrip()) {
      case 12:
      case 15: trustHoriztonal = false; break;
      }
      break;

   case 4: break;

   default: std::cerr<<"  ERROR, Trying to get energy from a D detector that doesn't exist!"<<std::endl;
   }

   if((!trustVertical || !trustHoriztonal) && debug) {
      std::cout<<std::endl<<"  GetDEnergy() output: (V,H)"<<std::endl;
      std::cout<<"  Detector: "<<GetDetectorNumber()<<std::endl;
      std::cout<<"  Strip: "<<GetDVerticalStrip()<<" "<<GetDHorizontalStrip()<<std::endl;
      std::cout<<"  Trustworthy: "<<trustVertical<<" "<<trustHoriztonal<<std::endl;
      std::cout<<"  Energy: "<<GetDVerticalEnergy()<<" "<<GetDHorizontalEnergy()<<std::endl;
   }

   if(trustVertical && trustHoriztonal) {
      // if(debug) std::cout<<"  Returning: "<<(GetDVerticalEnergy() + GetDHorizontalEnergy())/2.<<std::endl;
      return ((GetDVerticalEnergy() + GetDHorizontalEnergy()) / 2.);
   }
   if(trustVertical && !trustHoriztonal) {
      if(debug) {
         std::cout<<"**Returning: "<<GetDVerticalEnergy()<<std::endl;
      }
      return (GetDVerticalEnergy());
   }
   if(!trustVertical && trustHoriztonal) {
      if(debug) {
         std::cout<<"**Returning: "<<GetDHorizontalEnergy()<<std::endl;
      }
      return (GetDHorizontalEnergy());
   } else if(!trustVertical && !trustVertical) { // Are these correct??? RD
      return (0.);
   } else {
      std::cerr<<"  ERROR, I don't know who to trust in GetDEnergy()"<<std::endl;
      return -1; // I added this here so that there is guaranteed a return at the end of the function RD
   }
}

Double_t TCSMHit::GetEEnergy() const
{
   bool trustVertical   = true;
   bool trustHoriztonal = true;

   switch(GetDetectorNumber()) {
   case 1: break;

   case 2:
      switch(GetEHorizontalStrip()) {
      case 0:
      case 2:
      case 3:
      case 6:
      // case 8:
      case 12:
         // case 15:
         trustHoriztonal = false;
         break;
      }
      switch(GetEVerticalStrip()) {
      case 0: trustVertical = false; break;
      }
      break;

   case 3: break;

   case 4: break;

   default: std::cerr<<"  ERROR, Trying to get energy from a E detector that doesn't exist!"<<std::endl;
   }

   if(trustVertical && trustHoriztonal) {
      return ((GetEVerticalEnergy() + GetEHorizontalEnergy()) / 2.);
   }
   if(trustVertical && !trustHoriztonal) {
      return (GetEVerticalEnergy());
   }
   if(!trustVertical && trustHoriztonal) {
      return (GetEHorizontalEnergy());
   } else if(!trustVertical && !trustVertical) { // Are these correct? RD
      return (0.);
   } else {
      return -1; // I added this here so that there is guaranteed a return at the end of the function RD
      std::cerr<<"  ERROR, I don't know who to trust in GetEEnergy()"<<std::endl;
   }
}

void TCSMHit::Print(Option_t*) const
{
   std::cout<<"Printing TCSMHit:  Horizontal    Vertical"<<std::endl;
   std::cout<<"Detector number: "<<GetDetectorNumber()<<std::endl;
   // std::cout<<"Detector position: "<<GetDetectorPosition()<<std::endl;
   std::cout<<"D"<<std::endl;
   std::cout<<"Strip: "<<GetDHorizontalStrip()<<" "<<GetDVerticalStrip()<<std::endl;

   std::cout<<"Charge: "<<GetDHorizontalCharge()<<" "<<GetDVerticalCharge()<<std::endl;

   std::cout<<"CFD: "<<GetDHorizontalCFD()<<" "<<GetDVerticalCFD()<<std::endl;

   std::cout<<"Energy: "<<GetDHorizontalEnergy()<<" "<<GetDVerticalEnergy()<<std::endl;

   std::cout<<"Theta: "<<GetDPosition().Theta() * 180. / 3.14159
            <<" Phi: "<<GetDPosition().Phi() * 180. / 3.14159<<std::endl;

   std::cout<<"E"<<std::endl;
   std::cout<<"Strip: "<<GetEHorizontalStrip()<<" "<<GetEVerticalStrip()<<std::endl;

   std::cout<<"Charge: "<<GetEHorizontalCharge()<<" "<<GetEVerticalCharge()<<std::endl;

   std::cout<<"CFD: "<<GetEHorizontalCFD()<<" "<<GetEVerticalCFD()<<std::endl;

   std::cout<<"Energy: "<<GetEHorizontalEnergy()<<" "<<GetEVerticalEnergy()<<std::endl;

   std::cout<<"Theta: "<<GetEPosition().Theta() * 180. / 3.14159
            <<" Phi: "<<GetEPosition().Phi() * 180. / 3.14159<<std::endl;

   std::cout<<std::endl;
}

bool TCSMHit::IsEmpty()
{
   return (fHorDStrip == -1 && fHorDCharge < 1. && fHorDCfd == 0. && fVerDStrip == -1 && fVerDCharge < 1. &&
           fVerDCfd == 0. && fHorEStrip == -1 && fHorECharge < 1. && fHorECfd == 0. && fVerEStrip == -1 &&
           fVerECharge < 1. && fVerECfd == 0. && fHorDEnergy == 0.0 && fVerDEnergy == 0.0 && fHorDTime == 0.0 &&
           fVerDTime == 0.0 && fHorEEnergy == 0.0 && fVerEEnergy == 0.0 && fHorETime == 0.0 && fVerETime == 0.0 &&
           fDetectorNumber == 0);
}
