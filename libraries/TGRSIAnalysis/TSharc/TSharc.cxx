#include "TSharc.h"
#include "TMnemonic.h"

#include <cstdio>
#include <iostream>
#include <cmath>

#include "TClass.h"
#include "TMath.h"

/// \cond CLASSIMP
ClassImp(TSharc)
   /// \endcond

   // various sharc dimensions in mm

   // const int TSharc::frontstripslist[16]     = {16,16,16,16,  24,24,24,24,  24,24,24,24,  16,16,16,16};
   // const int TSharc::backstripslist[16]      = {24,24,24,24,  48,48,48,48,  48,48,48,48,  24,24,24,24};

   // const double TSharc::frontpitchlist[16]   = {2.0,2.0,2.0,2.0,  3.0,3.0,3.0,3.0,  3.0,3.0,3.0,3.0,
   // 2.0,2.0,2.0,2.0};
   // const double TSharc::backpitchlist[16]    = {TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,
   // 1.0,1.0,1.0,1.0,  1.0,1.0,1.0,1.0,  TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48};    // QQQ back
   // pitches are angles
   // const double TSharc::stripFpitch          = TSharc::Ydim / TSharc::frontstripslist[5]; // 72.0/24 = 3.0 mm
   // const double TSharc::ringpitch            = TSharc::Rdim / TSharc::frontstripslist[1]; // 32.0/16 = 2.0 mm
   // const double TSharc::stripBpitch          = TSharc::Zdim / TSharc::backstripslist[5] ; // 48.0/48 = 1.0 mm
   // const double TSharc::segmentpitch         = TSharc::Pdim / TSharc::backstripslist[1] ; // 81.6/24 = 3.4 degrees
   // (angular pitch)

   //==========================================================================//
   //==========================================================================//
   //==========================================================================//
   //==========================================================================//
   //==========================================================================//

   double TSharc::fXoffset = +0.00; //
double TSharc::fYoffset    = +0.00; //
double TSharc::fZoffset    = +0.00; //

double TSharc::fXdim   = +72.0; // total X dimension of all boxes
double TSharc::fYdim   = +72.0; // total Y dimension of all boxes
double TSharc::fZdim   = +48.0; // total Z dimension of all boxes
double TSharc::fRdim   = +32.0; // Rmax-Rmin for all QQQs
double TSharc::fPdim   = +81.6; // QQQ quadrant angular range (degrees)
double TSharc::fXposUB = +42.5;
double TSharc::fYminUB = -36.0;
double TSharc::fZminUB = -5.00;
double TSharc::fXposDB = +40.5;
double TSharc::fYminDB = -36.0;
double TSharc::fZminDB = +9.00;
double TSharc::fZposUQ = -66.5;
double TSharc::fRmaxUQ = +41.00;
double TSharc::fRminUQ = +9.00;
double TSharc::fPminUQ = +2.00; // degrees
double TSharc::fZposDQ = +74.5;
double TSharc::fRmaxDQ = +41.00;
double TSharc::fRminDQ = +9.00;
double TSharc::fPminDQ = +6.40; // degrees

// const int TSharc::frontstripslist[16]     = {16,16,16,16,  24,24,24,24,  24,24,24,24,  16,16,16,16};
// const int TSharc::backstripslist[16]      = {24,24,24,24,  48,48,48,48,  48,48,48,48,  24,24,24,24};

// const double TSharc::frontpitchlist[16]   = {2.0,2.0,2.0,2.0,  3.0,3.0,3.0,3.0,  3.0,3.0,3.0,3.0,  2.0,2.0,2.0,2.0};
// const double TSharc::backpitchlist[16]    = {TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,
// 1.0,1.0,1.0,1.0,  1.0,1.0,1.0,1.0,  TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48};
// QQQ back pitches are angles
//
double TSharc::fStripFPitch = TSharc::fYdim / 24.0; // TSharc::frontstripslist[5]; // 72.0/24 = 3.0 mm
double TSharc::fRingPitch   = TSharc::fRdim / 16.0; // TSharc::frontstripslist[1]; // 32.0/16 = 2.0 mm
double TSharc::fStripBPitch = TSharc::fZdim / 48.0; // TSharc::backstripslist[5] ; // 48.0/48 = 1.0 mm
double TSharc::fSegmentPitch =
   TSharc::fPdim / 24.0; // TSharc::backstripslist[1] ; // 81.6/24 = 3.4 degrees (angular pitch)

// The dimensions are described for a single detector of each type UQ,UB,DB,DQ, and all other detectors can be
// calculated by rotating this

TSharc::TSharc()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TSharc::~TSharc() = default;

TSharc::TSharc(const TSharc& rhs) : TGRSIDetector()
{
   Class()->IgnoreTObjectStreamer(kTRUE);
   Clear("ALL");
   rhs.Copy(*this);
}

void TSharc::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   if(frag == nullptr || chan == nullptr) {
      return;
   }
   /*  if(GetMidasTimestamp() == -1) {
       SetMidasTimestamp(frag->GetMidasTimeStamp());
     }
   */
   switch(chan->GetMnemonic()->ArraySubPosition()) {
   case TMnemonic::kD:
      if(chan->GetMnemonic()->CollectedCharge() == TMnemonic::kP) {
         fFrontFragments.push_back(*frag);
      } else {
         fBackFragments.push_back(*frag);
      }
      break;
   case TMnemonic::kE: fPadFragments.push_back(*frag); break;
   };

   // if(frag->GetDetector()==11 && frag->GetSegment()==16)
   //   return;
   // printf("FRONT:  %s\n",frag->GetName());
}

void TSharc::BuildHits()
{
   std::vector<TFragment>::iterator front;
   std::vector<TFragment>::iterator back;
   std::vector<TFragment>::iterator pad;
   // static int total;
   // printf("\t%i:  front = %i; back = %i\n",total++,fFrontFragments.size(),fBackFragments.size()); fflush(stdout);

   for(front = fFrontFragments.begin(); front != fFrontFragments.end();) {
      bool front_used = false;
      bool back_used  = false;
      for(back = fBackFragments.begin(); back != fBackFragments.end(); back++) {
         if(front->GetDetector() == back->GetDetector()) {
            // if((TMath::Abs(front->GetCharge() - back->GetCharge()) <  6000) ){ // ||
            //(front->GetDetector()==5 && (front->GetSegment()%2)!=0))  {
            // time gate ?
            front_used = true;
            back_used  = true;
            break;
            //}
         }
      }
      if(front_used && back_used) {
         TSharcHit hit;
         hit.SetFront(*front);
         hit.SetBack(*back);
         fSharcHits.push_back(hit); // TODO: consider using std::move here
         front = fFrontFragments.erase(front);
         back  = fBackFragments.erase(back);
      } else {
         front++;
      }
   }

   for(auto& fSharcHit : fSharcHits) {
      for(pad = fPadFragments.begin(); pad != fPadFragments.end(); pad++) {
         if(fSharcHit.GetDetector() == pad->GetDetector()) {
            fSharcHit.SetPad(*pad);
            pad = fPadFragments.erase(pad);
            break;
         }
      }
   }
   // printf(DRED "built %i sharc hits!" RESET_COLOR "\n",fSharcHits.size()); fflush(stdout);
}

void TSharc::RemoveHits(std::vector<TSharcHit>* hits, std::set<int>* to_remove)
{
   for(auto iter = to_remove->rbegin(); iter != to_remove->rend(); ++iter) {
      if(*iter == -1) {
         continue;
      }
      hits->erase(hits->begin() + *iter);
   }
}

void TSharc::Clear(Option_t* option)
{
   TGRSIDetector::Clear(option);
   fSharcHits.clear();

   fFrontFragments.clear(); //!
   fBackFragments.clear();  //!
   fPadFragments.clear();   //!

   if(!strcmp(option, "ALL")) {
      fXoffset = 0.00;
      fYoffset = 0.00;
      fZoffset = 0.00;
   }
   return;
}

void TSharc::Print(Option_t*) const
{
   printf("not yet written...\n");
   return;
}

void TSharc::Copy(TObject& rhs) const
{
   // if(!rhs.InheritsFrom("TSharc"))
   //  return;
   TGRSIDetector::Copy(rhs);

   static_cast<TSharc&>(rhs).fSharcHits = fSharcHits;
   static_cast<TSharc&>(rhs).fXoffset   = fXoffset;
   static_cast<TSharc&>(rhs).fYoffset   = fYoffset;
   static_cast<TSharc&>(rhs).fZoffset   = fZoffset;
}

TVector3 TSharc::GetPosition(int detector, int frontstrip, int backstrip, double X, double Y, double Z)
{
   int FrontDet = detector;
   int FrontStr = frontstrip;
   // int BackDet  = detector;
   int BackStr = backstrip;
   int nrots   = 0; // allows us to rotate into correct position

   TVector3 position;
   TVector3 position_offset;
   position_offset.SetXYZ(X, Y, Z);

   if(FrontDet >= 5 && FrontDet <= 8) { // forward box
      nrots    = FrontDet - 4; // edited to make box 5 on the ceiling.  assuming rotaing ccw around the +z axis!!
      double x = fXposDB;      // ?? x stays the same. first detector is aways defined in the y-z plane.
      double y = -(fYminDB + (FrontStr + 0.5) * fStripFPitch); // [(-36.0) - (+36.0)]        // ?? add minus sign,
                                                               // reversve the order of the strips on the ds section.
      double z = fZminDB + (BackStr + 0.5) * fStripBPitch;     // [(+9.0) - (+57.0)]
      position.SetXYZ(x, y, z);
   } else if(FrontDet >= 9 && FrontDet <= 12) { // backward box
      nrots    = FrontDet - 8; // edited to make box 5 on the ceiling.  assuming rotaing ccw around the +z axis!!
      double x = fXposUB;
      double y = fYminUB + (FrontStr + 0.5) * fStripFPitch; // [(-36.0) - (+36.0)]
      double z = fZminUB - (BackStr + 0.5) * fStripBPitch;  // [(-5.0) - (-53.0)]
      position.SetXYZ(x, y, z);
   } else if(FrontDet >= 13) { // backward (upstream) QQQ
      nrots      = FrontDet - 13;
      double z   = fZposUQ;
      double rho = fRmaxUQ - (FrontStr + 0.5) * fRingPitch;                           // [(+9.0) - (+41.0)]
      double phi = (fPminUQ + (BackStr + 0.5) * fSegmentPitch) * TMath::Pi() / 180.0; // [(+2.0) - (+83.6)]
      position.SetXYZ(rho * TMath::Sin(phi), rho * TMath::Cos(phi), z);
   } else if(FrontDet <= 4) { // forward (downstream) QQQ
      nrots      = FrontDet - 1;
      double z   = fZposDQ;
      double rho = fRmaxDQ - (FrontStr + 0.5) * fRingPitch;                           // [(+9.0) - (+41.0)]
      double phi = (fPminDQ + (BackStr + 0.5) * fSegmentPitch) * TMath::Pi() / 180.0; // [(+6.4) - (+88.0)]
      position.SetXYZ(rho * TMath::Sin(phi), rho * TMath::Cos(phi), z);
   }

   position.RotateZ(TMath::Pi() * nrots / 2);
   return (position + position_offset);
}

TGRSIDetectorHit* TSharc::GetHit(const Int_t& idx)
{
   return GetSharcHit(idx);
}

TSharcHit* TSharc::GetSharcHit(const int& i)
{
   try {
      return &fSharcHits.at(i);
   } catch(const std::out_of_range& oor) {
      std::cerr<<ClassName()<<" is out of range: "<<oor.what()<<std::endl;
      throw grsi::exit_exception(1);
   }
   return nullptr;
}

double TSharc::GetDetectorThickness(TSharcHit& hit, double dist)
{
   static double fDetectorThickness[16] = {998., 998.,  998.,  1001., 141., 142., 133., 143.,
                                           999., 1001., 1001., 1002., 390., 390., 383., 385.};
   if(dist < 0.0) {
      dist = fDetectorThickness[hit.GetDetector()];
   }

   double phi_90 = fmod(std::fabs(hit.GetPosition().Phi()), TMath::Pi() / 2);
   double phi_45 = phi_90;
   if(phi_90 > (TMath::Pi() / 4.)) {
      phi_45 = TMath::Pi() / 2 - phi_90;
   }

   if(hit.GetDetector() >= 5 && hit.GetDetector() <= 12) {
      return dist / (TMath::Sin(hit.GetPosition().Theta()) * TMath::Cos(phi_45));
   } else {
      return std::fabs(dist / (TMath::Cos(hit.GetPosition().Theta())));
   }
}

double TSharc::GetDeadLayerThickness(TSharcHit& hit)
{
   static double fDeadLayerThickness[16] = {0.7, 0.7, 0.7, 0.7, 0.1, 0.1, 0.1, 0.1,
                                            0.1, 0.1, 0.1, 0.1, 0.7, 0.7, 0.7, 0.7};
   return GetDetectorThickness(hit, fDeadLayerThickness[hit.GetDetector()]);
}

double TSharc::GetPadThickness(TSharcHit& hit)
{
   static double fPadThickness[16] = {0.0, 0.0, 0.0, 0.0, 1534, 1535, 1535, 1539,
                                      0.0, 0.0, 0.0, 0.0, 0.0,  0.0,  0.0,  0.0};
   return GetDetectorThickness(hit, fPadThickness[hit.GetDetector()]);
}

double TSharc::GetPadDeadLayerThickness(TSharcHit& hit)
{
   static double fPadDeadLayerThickness[16] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0,
                                               0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
   return GetDetectorThickness(hit, fPadDeadLayerThickness[hit.GetDetector()]);
}
