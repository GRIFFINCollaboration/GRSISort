#include "TTigress.h"

#include <iostream>

#include "TRandom.h"
#include "TMath.h"
#include "TClass.h"
#include "TInterpreter.h"

#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TTigress)
/// \endcond

double TTigress::fTargetOffset = 0.;

// Default tigress unpacking settings
TTransientBits<UShort_t> TTigress::fgTigressBits(TTigress::kSetCoreWave | TTigress::kSetBGOHits);

// Why arent these TTigress class functions?
bool DefaultAddback(TTigressHit& one, TTigressHit& two)
{

   // fTigressHits vector is sorted by descending energy during detector construction
   // Assumption for crystals and segments: higher energy = first interaction
   // Checking for Scattering FROM "one" TO "two"

   // GetTime is in ns;  AddbackWindow is in 10's of ns.
   if(std::abs(one.GetTime() - two.GetTime()) < (TGRSIOptions::AnalysisOptions()->AddbackWindow() * 10.0)) {

      // segments of crystals have been sorted by descending energy during detector construction
      // LastPosition is the position of lowest energy segment and GetPosition the highest energy segment (assumed
      // first)
      // Both return core position if no segments
      double res = (one.GetLastPosition() - two.GetPosition()).Mag();

      // In clover core separation 54.2564, 76.7367
      // Between clovers core separation 74.2400 91.9550 (high-eff mode)
      double seperation_limit = 93;

      // Important to avoid GetSegmentVec segfaults for no segment or when we have cores only efficiency calibration
      if(one.GetSegmentMultiplicity() > 0 && two.GetSegmentMultiplicity() > 0 && !TTigress::GetForceCrystal()) {
         int one_seg = one.GetSegmentVec().back().GetSegment();
         int two_seg = two.GetSegmentVec().front().GetSegment();

         // front segment to front segment OR back segment to back segment
         if((one_seg < 5 && two_seg < 5) || (one_seg > 4 && two_seg > 4)) {
            seperation_limit = 54;
            // front to back
         } else if((one_seg < 5 && two_seg > 4) || (one_seg > 4 && two_seg < 5)) {
            seperation_limit = 105;
         }
      }

      if(res < seperation_limit) {
         return true;
      }
   }

   return false;
}

std::function<bool(TTigressHit&, TTigressHit&)> TTigress::fAddbackCriterion = DefaultAddback;

bool DefaultSuppression(TTigressHit& tig, TBgoHit& bgo)
{
   Int_t dCfd = tig.GetCfd() - bgo.GetCfd();
   return ((dCfd > -400 && dCfd < -80) && (tig.GetDetector() == bgo.GetDetector()) && (bgo.GetCharge() > 100.) &&
           TTigress::BGOSuppression[tig.GetCrystal()][bgo.GetCrystal()][bgo.GetSegment() - 1]);
}

std::function<bool(TTigressHit&, TBgoHit&)> TTigress::fSuppressionCriterion = DefaultSuppression;

TTigress::TTigress() : TGRSIDetector()
{
   // Class()->IgnoreTObjectStreamer(true);
   Clear();

   // SetBit(TTigress::kSetCoreWave);
   // SetBit(TTigress::kSetBGOs,1);
   // SetBit(TTigress::kSetSegWave,0);
   // SetBit(TTigress::kSetBGOWave,0);
}

TTigress::TTigress(const TTigress& rhs) : TGRSIDetector()
{
   rhs.Copy(*this);
}

TTigress::~TTigress() = default;

void TTigress::Copy(TObject& rhs) const
{
   TGRSIDetector::Copy(rhs);
   static_cast<TTigress&>(rhs).fTigressHits  = fTigressHits;
   static_cast<TTigress&>(rhs).fAddbackHits  = fAddbackHits;
   static_cast<TTigress&>(rhs).fAddbackFrags = fAddbackFrags;
   static_cast<TTigress&>(rhs).fTigressBits  = 0;
}

void TTigress::Clear(Option_t* opt)
{
   // Clears the mother, and all of the hits
   TGRSIDetector::Clear(opt);
   fTigressHits.clear();
   fAddbackHits.clear();
   fAddbackFrags.clear();
   fBgos.clear();
   // fTigressBits.SetBit(TTigress::kAddbackSet,false);
   fTigressBits = 0;
}

void TTigress::Print(Option_t* opt) const
{
   printf("%lu tigress hits\n", fTigressHits.size());
   for(Short_t i = 0; i < GetMultiplicity(); i++) {
      fTigressHits.at(i).Print(opt);
   }
}

TTigress& TTigress::operator=(const TTigress& rhs)
{
   rhs.Copy(*this);
   return *this;
}

Int_t TTigress::GetAddbackMultiplicity()
{
   // Automatically builds the addback hits using the addback_criterion
   // (if the size of the addback_hits vector is zero) and return the number of addback hits.
   if(fTigressHits.empty()) {
      return 0;
   }
   // if the addback has been reset, clear the addback hits
   if(!fTigressBits.TestBit(kAddbackSet)) {
      fAddbackHits.clear();
   } else {
      return fAddbackHits.size();
   }

   // use the first (highest E) tigress hit as starting point for the addback hits
   fAddbackHits.push_back(fTigressHits[0]);
   fAddbackFrags.push_back(1);

   // loop over remaining tigress hits
   size_t i, j;
   for(i = 1; i < fTigressHits.size(); i++) {
      // check for each existing addback hit if this tigress hit should be added
      for(j = 0; j < fAddbackHits.size(); j++) {
         if(fAddbackCriterion(fAddbackHits[j], fTigressHits[i])) {
            // SumHit preserves time and position from first (highest E) hit, but adds segments so this hit becomes
            // LastPosition()
            fAddbackHits[j].SumHit(&(fTigressHits[i])); // Adds
            fAddbackFrags[j]++;
            break;
         }
      }
      // if hit[i] was not added to a higher energy hit, create its own addback hit
      if(j == fAddbackHits.size()) {
         fAddbackHits.push_back(fTigressHits[i]);
         fAddbackHits.back().SumHit(&(fAddbackHits.back())); // Does nothing
         fAddbackFrags.push_back(1);
      }
   }
   fTigressBits.SetBit(kAddbackSet, true);

   return fAddbackHits.size();
}

TTigressHit* TTigress::GetAddbackHit(const int& i)
{
   /// Get the ith addback hit. This function calls GetAddbackMultiplicity to check the range of the index.
   /// This automatically calculates all addback hits if they haven't been calculated before.
   if(i < GetAddbackMultiplicity()) {
      return &fAddbackHits.at(i);
   }
   std::cerr<<"Addback hits are out of range"<<std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

void TTigress::BuildHits()
{

   // for(size_t i = 0; i<GetMultiplicity(); i++){
   //  if(GetTigressHit(i)->GetCharge() <= 0.5)
   //    //DeleteTigressHit(i);
   //}
   // erasing elements of a vector in a loop is a bit tricker... pcb.
   std::vector<TTigressHit>::iterator it;
   for(it = fTigressHits.begin(); it != fTigressHits.end();) {
      // it->Print("all");
      if(it->GetNSegments() > 1) {
         it->SortSegments();
      }

      if(it->HasWave() && TGRSIOptions::AnalysisOptions()->IsWaveformFitting()) {
         it->SetWavefit();
      }
      it++;
   }
   if(fTigressHits.size() > 1) {
      std::sort(fTigressHits.begin(), fTigressHits.end());
   }

   // Label all hits as being suppressed or not
   for(auto& fTigressHit : fTigressHits) {
      bool suppressed = false;
      for(auto& fBgo : fBgos) {
         if(fSuppressionCriterion(fTigressHit, fBgo)) {
            suppressed = true;
            break;
         }
      }
      fTigressHit.SetBGOFired(suppressed);
   }
}

void TTigress::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   // if((chan->GetMnemonic()->subsystem.compare(0,1,"G")==0) &&
   if((chan->GetMnemonic()->SubSystem() == TMnemonic::EMnemonic::kG) &&
      (chan->GetSegmentNumber() == 0 || chan->GetSegmentNumber() == 9)) { // it is a core
      // if(frag->Charge.size() == 0 || (frag->Cfd.size() == 0 && frag->Led.size() == 0))   // sanity check, it has a
      // good energy and time (cfd or led).
      //  return;
      TTigressHit corehit; //(*frag);
      // loop over existing hits to see if this core was already created by a previously found segment
      // of course this means if we have a core in "coincidence" with itself we will overwrite the first hit
      for(size_t i = 0; i < fTigressHits.size(); ++i) {
         TTigressHit* hit = GetTigressHit(i);
         if((hit->GetDetector() == chan->GetDetectorNumber()) &&
            (hit->GetCrystal() == chan->GetCrystalNumber())) { // we have a match;
            // if(hit->Charge() == 0 || (frag->Cfd.size() == 0 && frag->Led.size() == 0))   // sanity check, it has a
            // good energy and time (cfd or led).
            // if(chan->GetMnemonic()->outputsensor.compare(0,1,"b")==0) {
            if(chan->GetMnemonic()->OutputSensor() == TMnemonic::EMnemonic::kB) {
               if(hit->GetName()[9] == 'a') {
                  return;
               }
               hit->CopyFragment(*frag);
               if(TestGlobalBit(kSetCoreWave)) {
                  frag->CopyWave(*hit);
               }
               return;
            }
            hit->CopyFragment(*frag);
            if(TestGlobalBit(kSetCoreWave)) {
               frag->CopyWave(*hit);
            }
            return;
         }
      }
      corehit.CopyFragment(*frag);
      if(TestGlobalBit(kSetCoreWave)) {
         frag->CopyWave(corehit);
      }
      fTigressHits.push_back(corehit);
      return;
   }
   if(chan->GetMnemonic()->SubSystem() == TMnemonic::EMnemonic::kG) { // its ge but its not a core...
      TGRSIDetectorHit temp(*frag);
      for(size_t i = 0; i < fTigressHits.size(); ++i) {
         TTigressHit* hit = GetTigressHit(i);
         if((hit->GetDetector() == chan->GetDetectorNumber()) &&
            (hit->GetCrystal() == chan->GetCrystalNumber())) { // we have a match;
            if(TestGlobalBit(kSetSegWave)) {
               frag->CopyWave(temp);
            }
            hit->AddSegment(temp);
            return;
         }
      }
      TTigressHit corehit;
      corehit.SetAddress((frag->GetAddress())); // fake it till you make it
      if(TestGlobalBit(kSetSegWave)) {
         frag->CopyWave(temp);
      }
      corehit.AddSegment(temp);
      fTigressHits.push_back(corehit);
      return;
   }
   if(chan->GetMnemonic()->SubSystem() == TMnemonic::EMnemonic::kS) {
      TBgoHit temp(*frag);
      fBgos.push_back(temp);
      return;
   }
   // if not suprress errors;
   printf(ALERTTEXT "failed to build!" RESET_COLOR "\n");
   frag->Print();
}

void TTigress::ResetAddback()
{
   /// Used to clear the addback hits. When playing back a tree, this must
   /// be called before building the new addback hits, otherwise, a copy of
   /// the old addback hits will be stored instead.
   /// This should have changed now, we're using the stored tigress bits to reset the addback
   fTigressBits.SetBit(kAddbackSet, false);
   fAddbackHits.clear();
   fAddbackFrags.clear();
}

UShort_t TTigress::GetNAddbackFrags(size_t idx) const
{
   // Get the number of addback "fragments" contributing to the total addback hit
   // with index idx.
   if(idx < fAddbackFrags.size()) {
      return fAddbackFrags.at(idx);
   }
   return 0;
}

TVector3 TTigress::GetPosition(const TTigressHit& hit, double dist, bool smear)
{
   return TTigress::GetPosition(hit.GetDetector(), hit.GetCrystal(), hit.GetFirstSeg(), dist, smear);
}

TVector3 TTigress::GetPosition(int DetNbr, int CryNbr, int SegNbr, double dist, bool smear)
{
   TVector3 det_pos;
   double   xx = 0;
   double   yy = 0;
   double   zz = 0;

   if(dist > 140. || (GetArrayBackPos() && dist < 100.)) { // 145.0

      switch(CryNbr) {
      case -1: break;
      case 0:
         xx = GeBluePositionBack[DetNbr][SegNbr][0];
         yy = GeBluePositionBack[DetNbr][SegNbr][1];
         zz = GeBluePositionBack[DetNbr][SegNbr][2];
         break;
      case 1:
         xx = GeGreenPositionBack[DetNbr][SegNbr][0];
         yy = GeGreenPositionBack[DetNbr][SegNbr][1];
         zz = GeGreenPositionBack[DetNbr][SegNbr][2];
         break;
      case 2:
         xx = GeRedPositionBack[DetNbr][SegNbr][0];
         yy = GeRedPositionBack[DetNbr][SegNbr][1];
         zz = GeRedPositionBack[DetNbr][SegNbr][2];
         break;
      case 3:
         xx = GeWhitePositionBack[DetNbr][SegNbr][0];
         yy = GeWhitePositionBack[DetNbr][SegNbr][1];
         zz = GeWhitePositionBack[DetNbr][SegNbr][2];
         break;
      };
      det_pos.SetXYZ(xx, yy, zz);
   } else {
      switch(CryNbr) {
      case -1: break;
      case 0:
         xx = GeBluePosition[DetNbr][SegNbr][0];
         yy = GeBluePosition[DetNbr][SegNbr][1];
         zz = GeBluePosition[DetNbr][SegNbr][2];
         break;
      case 1:
         xx = GeGreenPosition[DetNbr][SegNbr][0];
         yy = GeGreenPosition[DetNbr][SegNbr][1];
         zz = GeGreenPosition[DetNbr][SegNbr][2];
         break;
      case 2:
         xx = GeRedPosition[DetNbr][SegNbr][0];
         yy = GeRedPosition[DetNbr][SegNbr][1];
         zz = GeRedPosition[DetNbr][SegNbr][2];
         break;
      case 3:
         xx = GeWhitePosition[DetNbr][SegNbr][0];
         yy = GeWhitePosition[DetNbr][SegNbr][1];
         zz = GeWhitePosition[DetNbr][SegNbr][2];
         break;
      };
   }

   det_pos.SetXYZ(xx, yy, zz - fTargetOffset);

   if(smear) {
      if(SegNbr == 0) {
         // Not perfect as it takes the perpendicular core vector, not the clover vector, but good enough for my
         // purposes
         TVector3 a(-yy, xx, 0);
         TVector3 b = TVector3(xx, yy, zz).Cross(a);
         double   x, y, r = sqrt(gRandom->Uniform(0, 400));
         gRandom->Circle(x, y, r);
         det_pos += a.Unit() * x + b.Unit() * y;
      }
   }

   return det_pos;
}

double TTigress::GeBluePosition[17][9][3] = {{{0., 0., 0.},
                                              {0., 0., 0.},
                                              {0., 0., 0.},
                                              {0., 0., 0.},
                                              {0., 0., 0.},
                                              {0., 0., 0.},
                                              {0., 0., 0.},
                                              {0., 0., 0.},
                                              {0., 0., 0.}},
                                             {{78.05, 61.70, 134.09},
                                              {47.83, 59.64, 119.06},
                                              {63.65, 65.78, 102.12},
                                              {72.14, 44.72, 103.15},
                                              {57.26, 37.61, 118.80},
                                              {72.73, 73.96, 152.77},
                                              {89.31, 81.09, 134.70},
                                              {99.39, 57.11, 134.51},
                                              {82.33, 50.30, 152.93}},
                                             {{-61.70, 78.05, 134.09},
                                              {-59.64, 47.83, 119.06},
                                              {-65.78, 63.65, 102.12},
                                              {-44.72, 72.14, 103.15},
                                              {-37.61, 57.26, 118.80},
                                              {-73.96, 72.73, 152.77},
                                              {-81.09, 89.31, 134.70},
                                              {-57.11, 99.39, 134.51},
                                              {-50.30, 82.33, 152.93}},
                                             {{-78.05, -61.70, 134.09},
                                              {-47.83, -59.64, 119.06},
                                              {-63.65, -65.78, 102.12},
                                              {-72.14, -44.72, 103.15},
                                              {-57.26, -37.61, 118.80},
                                              {-72.73, -73.96, 152.77},
                                              {-89.31, -81.09, 134.70},
                                              {-99.39, -57.11, 134.51},
                                              {-82.33, -50.30, 152.93}},
                                             {{61.70, -78.05, 134.09},
                                              {59.64, -47.83, 119.06},
                                              {65.78, -63.65, 102.12},
                                              {44.72, -72.14, 103.15},
                                              {37.61, -57.26, 118.80},
                                              {73.96, -72.73, 152.77},
                                              {81.09, -89.31, 134.70},
                                              {57.11, -99.39, 134.51},
                                              {50.30, -82.33, 152.93}},
                                             {{139.75, 87.25, 27.13},
                                              {107.47, 84.35, 36.80},
                                              {107.64, 84.01, 12.83},
                                              {116.86, 63.25, 13.71},
                                              {116.66, 62.21, 36.42},
                                              {146.69, 104.60, 40.50},
                                              {146.58, 104.81, 14.96},
                                              {156.50, 80.77, 14.73},
                                              {156.44, 80.99, 40.74}},
                                             {{37.12, 160.51, 27.13},
                                              {16.35, 135.64, 36.80},
                                              {16.71, 135.51, 12.83},
                                              {37.91, 127.36, 13.71},
                                              {38.50, 126.48, 36.42},
                                              {29.76, 177.69, 40.50},
                                              {29.53, 177.76, 14.96},
                                              {53.55, 167.78, 14.73},
                                              {53.35, 167.89, 40.74}},
                                             {{-87.25, 139.75, 27.13},
                                              {-84.35, 107.47, 36.80},
                                              {-84.01, 107.64, 12.83},
                                              {-63.25, 116.86, 13.71},
                                              {-62.21, 116.66, 36.42},
                                              {-104.60, 146.69, 40.50},
                                              {-104.81, 146.58, 14.96},
                                              {-80.77, 156.50, 14.73},
                                              {-80.99, 156.44, 40.74}},
                                             {{-160.51, 37.12, 27.13},
                                              {-135.64, 16.35, 36.80},
                                              {-135.51, 16.71, 12.83},
                                              {-127.36, 37.91, 13.71},
                                              {-126.48, 38.50, 36.42},
                                              {-177.69, 29.76, 40.50},
                                              {-177.76, 29.53, 14.96},
                                              {-167.78, 53.55, 14.73},
                                              {-167.89, 53.35, 40.74}},
                                             {{-139.75, -87.25, 27.13},
                                              {-107.47, -84.35, 36.80},
                                              {-107.64, -84.01, 12.83},
                                              {-116.86, -63.25, 13.71},
                                              {-116.66, -62.21, 36.42},
                                              {-146.69, -104.60, 40.50},
                                              {-146.58, -104.81, 14.96},
                                              {-156.50, -80.77, 14.73},
                                              {-156.44, -80.99, 40.74}},
                                             {{-37.12, -160.51, 27.13},
                                              {-16.35, -135.64, 36.80},
                                              {-16.71, -135.51, 12.83},
                                              {-37.91, -127.36, 13.71},
                                              {-38.50, -126.48, 36.42},
                                              {-29.76, -177.69, 40.50},
                                              {-29.53, -177.76, 14.96},
                                              {-53.55, -167.78, 14.73},
                                              {-53.35, -167.89, 40.74}},
                                             {{87.25, -139.75, 27.13},
                                              {84.35, -107.47, 36.80},
                                              {84.01, -107.64, 12.83},
                                              {63.25, -116.86, 13.71},
                                              {62.21, -116.66, 36.42},
                                              {104.60, -146.69, 40.50},
                                              {104.81, -146.58, 14.96},
                                              {80.77, -156.50, 14.73},
                                              {80.99, -156.44, 40.74}},
                                             {{160.51, -37.12, 27.13},
                                              {135.64, -16.35, 36.80},
                                              {135.51, -16.71, 12.83},
                                              {127.36, -37.91, 13.71},
                                              {126.48, -38.50, 36.42},
                                              {177.69, -29.76, 40.50},
                                              {177.76, -29.53, 14.96},
                                              {167.78, -53.55, 14.73},
                                              {167.89, -53.35, 40.74}},
                                             {{113.50, 76.38, -95.72},
                                              {95.91, 79.56, -67.01},
                                              {80.41, 72.73, -83.98},
                                              {90.05, 52.14, -83.76},
                                              {104.85, 57.32, -67.30},
                                              {125.64, 95.88, -95.49},
                                              {108.85, 89.19, -113.54},
                                              {118.64, 65.08, -113.68},
                                              {135.56, 72.34, -95.31}},
                                             {{-76.38, 113.50, -95.72},
                                              {-79.56, 95.91, -67.01},
                                              {-72.73, 80.41, -83.98},
                                              {-52.14, 90.05, -83.76},
                                              {-57.32, 104.85, -67.30},
                                              {-95.88, 125.64, -95.49},
                                              {-89.19, 108.85, -113.54},
                                              {-65.08, 118.64, -113.68},
                                              {-72.34, 135.56, -95.31}},
                                             {{-113.50, -76.38, -95.72},
                                              {-95.91, -79.56, -67.01},
                                              {-80.41, -72.73, -83.98},
                                              {-90.05, -52.14, -83.76},
                                              {-104.85, -57.32, -67.30},
                                              {-125.64, -95.88, -95.49},
                                              {-108.85, -89.19, -113.54},
                                              {-118.64, -65.08, -113.68},
                                              {-135.56, -72.34, -95.31}},
                                             {{76.38, -113.50, -95.72},
                                              {79.56, -95.91, -67.01},
                                              {72.73, -80.41, -83.98},
                                              {52.14, -90.05, -83.76},
                                              {57.32, -104.85, -67.30},
                                              {95.88, -125.64, -95.49},
                                              {89.19, -108.85, -113.54},
                                              {65.08, -118.64, -113.68},
                                              {72.34, -135.56, -95.31}}};

// Assuming this is the 1
double TTigress::GeGreenPosition[17][9][3] = {{{0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.}},
                                              {{113.50, 76.38, 95.72},
                                               {95.91, 79.56, 67.01},
                                               {104.85, 57.32, 67.30},
                                               {90.05, 52.14, 83.76},
                                               {80.41, 72.73, 83.98},
                                               {125.64, 95.88, 95.49},
                                               {135.56, 72.34, 95.31},
                                               {118.64, 65.08, 113.68},
                                               {108.85, 89.19, 113.54}},
                                              {{-76.38, 113.50, 95.72},
                                               {-79.56, 95.91, 67.01},
                                               {-57.32, 104.85, 67.30},
                                               {-52.14, 90.05, 83.76},
                                               {-72.73, 80.41, 83.98},
                                               {-95.88, 125.64, 95.49},
                                               {-72.34, 135.56, 95.31},
                                               {-65.08, 118.64, 113.68},
                                               {-89.19, 108.85, 113.54}},
                                              {{-113.50, -76.38, 95.72},
                                               {-95.91, -79.56, 67.01},
                                               {-104.85, -57.32, 67.30},
                                               {-90.05, -52.14, 83.76},
                                               {-80.41, -72.73, 83.98},
                                               {-125.64, -95.88, 95.49},
                                               {-135.56, -72.34, 95.31},
                                               {-118.64, -65.08, 113.68},
                                               {-108.85, -89.19, 113.54}},
                                              {{76.38, -113.50, 95.72},
                                               {79.56, -95.91, 67.01},
                                               {57.32, -104.85, 67.30},
                                               {52.14, -90.05, 83.76},
                                               {72.73, -80.41, 83.98},
                                               {95.88, -125.64, 95.49},
                                               {72.34, -135.56, 95.31},
                                               {65.08, -118.64, 113.68},
                                               {89.19, -108.85, 113.54}},
                                              {{139.75, 87.25, -27.13},
                                               {107.47, 84.35, -36.80},
                                               {116.66, 62.21, -36.42},
                                               {116.86, 63.25, -13.71},
                                               {107.64, 84.01, -12.83},
                                               {146.69, 104.60, -40.50},
                                               {156.44, 80.99, -40.74},
                                               {156.50, 80.77, -14.73},
                                               {146.58, 104.81, -14.96}},
                                              {{37.12, 160.51, -27.13},
                                               {16.35, 135.64, -36.80},
                                               {38.50, 126.48, -36.42},
                                               {37.91, 127.36, -13.71},
                                               {16.71, 135.51, -12.83},
                                               {29.76, 177.69, -40.50},
                                               {53.35, 167.89, -40.74},
                                               {53.55, 167.78, -14.73},
                                               {29.53, 177.76, -14.96}},
                                              {{-87.25, 139.75, -27.13},
                                               {-84.35, 107.47, -36.80},
                                               {-62.21, 116.66, -36.42},
                                               {-63.25, 116.86, -13.71},
                                               {-84.01, 107.64, -12.83},
                                               {-104.60, 146.69, -40.50},
                                               {-80.99, 156.44, -40.74},
                                               {-80.77, 156.50, -14.73},
                                               {-104.81, 146.58, -14.96}},
                                              {{-160.51, 37.12, -27.13},
                                               {-135.64, 16.35, -36.80},
                                               {-126.48, 38.50, -36.42},
                                               {-127.36, 37.91, -13.71},
                                               {-135.51, 16.71, -12.83},
                                               {-177.69, 29.76, -40.50},
                                               {-167.89, 53.35, -40.74},
                                               {-167.78, 53.55, -14.73},
                                               {-177.76, 29.53, -14.96}},
                                              {{-139.75, -87.25, -27.13},
                                               {-107.47, -84.35, -36.80},
                                               {-116.66, -62.21, -36.42},
                                               {-116.86, -63.25, -13.71},
                                               {-107.64, -84.01, -12.83},
                                               {-146.69, -104.60, -40.50},
                                               {-156.44, -80.99, -40.74},
                                               {-156.50, -80.77, -14.73},
                                               {-146.58, -104.81, -14.96}},
                                              {{-37.12, -160.51, -27.13},
                                               {-16.35, -135.64, -36.80},
                                               {-38.50, -126.48, -36.42},
                                               {-37.91, -127.36, -13.71},
                                               {-16.71, -135.51, -12.83},
                                               {-29.76, -177.69, -40.50},
                                               {-53.35, -167.89, -40.74},
                                               {-53.55, -167.78, -14.73},
                                               {-29.53, -177.76, -14.96}},
                                              {{87.25, -139.75, -27.13},
                                               {84.35, -107.47, -36.80},
                                               {62.21, -116.66, -36.42},
                                               {63.25, -116.86, -13.71},
                                               {84.01, -107.64, -12.83},
                                               {104.60, -146.69, -40.50},
                                               {80.99, -156.44, -40.74},
                                               {80.77, -156.50, -14.73},
                                               {104.81, -146.58, -14.96}},
                                              {{160.51, -37.12, -27.13},
                                               {135.64, -16.35, -36.80},
                                               {126.48, -38.50, -36.42},
                                               {127.36, -37.91, -13.71},
                                               {135.51, -16.71, -12.83},
                                               {177.69, -29.76, -40.50},
                                               {167.89, -53.35, -40.74},
                                               {167.78, -53.55, -14.73},
                                               {177.76, -29.53, -14.96}},
                                              {{78.05, 61.70, -134.09},
                                               {47.83, 59.64, -119.06},
                                               {57.26, 37.61, -118.80},
                                               {72.14, 44.72, -103.15},
                                               {63.65, 65.78, -102.12},
                                               {72.73, 73.96, -152.77},
                                               {82.33, 50.30, -152.93},
                                               {99.39, 57.11, -134.51},
                                               {89.31, 81.09, -134.70}},
                                              {{-61.70, 78.05, -134.09},
                                               {-59.64, 47.83, -119.06},
                                               {-37.61, 57.26, -118.80},
                                               {-44.72, 72.14, -103.15},
                                               {-65.78, 63.65, -102.12},
                                               {-73.96, 72.73, -152.77},
                                               {-50.30, 82.33, -152.93},
                                               {-57.11, 99.39, -134.51},
                                               {-81.09, 89.31, -134.70}},
                                              {{-78.05, -61.70, -134.09},
                                               {-47.83, -59.64, -119.06},
                                               {-57.26, -37.61, -118.80},
                                               {-72.14, -44.72, -103.15},
                                               {-63.65, -65.78, -102.12},
                                               {-72.73, -73.96, -152.77},
                                               {-82.33, -50.30, -152.93},
                                               {-99.39, -57.11, -134.51},
                                               {-89.31, -81.09, -134.70}},
                                              {{61.70, -78.05, -134.09},
                                               {59.64, -47.83, -119.06},
                                               {37.61, -57.26, -118.80},
                                               {44.72, -72.14, -103.15},
                                               {65.78, -63.65, -102.12},
                                               {73.96, -72.73, -152.77},
                                               {50.30, -82.33, -152.93},
                                               {57.11, -99.39, -134.51},
                                               {81.09, -89.31, -134.70}}};

// Assuming this is the 2
double TTigress::GeRedPosition[17][9][3] = {{{0., 0., 0.},
                                             {0., 0., 0.},
                                             {0., 0., 0.},
                                             {0., 0., 0.},
                                             {0., 0., 0.},
                                             {0., 0., 0.},
                                             {0., 0., 0.},
                                             {0., 0., 0.},
                                             {0., 0., 0.}},
                                            {{134.26, 26.25, 95.72},
                                             {124.08, 11.56, 67.01},
                                             {108.28, 5.43, 83.98},
                                             {100.55, 26.81, 83.76},
                                             {114.67, 33.61, 67.30},
                                             {156.64, 21.05, 95.49},
                                             {140.03, 13.91, 113.54},
                                             {129.91, 37.87, 113.68},
                                             {147.01, 44.70, 95.31}},
                                            {{-26.25, 134.26, 95.72},
                                             {-11.56, 124.08, 67.01},
                                             {-5.43, 108.28, 83.98},
                                             {-26.81, 100.55, 83.76},
                                             {-33.61, 114.67, 67.30},
                                             {-21.05, 156.64, 95.49},
                                             {-13.91, 140.03, 113.54},
                                             {-37.87, 129.91, 113.68},
                                             {-44.70, 147.01, 95.31}},
                                            {{-134.26, -26.25, 95.72},
                                             {-124.08, -11.56, 67.01},
                                             {-108.28, -5.43, 83.98},
                                             {-100.55, -26.81, 83.76},
                                             {-114.67, -33.61, 67.30},
                                             {-156.64, -21.05, 95.49},
                                             {-140.03, -13.91, 113.54},
                                             {-129.91, -37.87, 113.68},
                                             {-147.01, -44.70, 95.31}},
                                            {{26.25, -134.26, 95.72},
                                             {11.56, -124.08, 67.01},
                                             {5.43, -108.28, 83.98},
                                             {26.81, -100.55, 83.76},
                                             {33.61, -114.67, 67.30},
                                             {21.05, -156.64, 95.49},
                                             {13.91, -140.03, 113.54},
                                             {37.87, -129.91, 113.68},
                                             {44.70, -147.01, 95.31}},
                                            {{160.51, 37.12, -27.13},
                                             {135.64, 16.35, -36.80},
                                             {135.51, 16.71, -12.83},
                                             {127.36, 37.91, -13.71},
                                             {126.48, 38.50, -36.42},
                                             {177.69, 29.76, -40.50},
                                             {177.76, 29.53, -14.96},
                                             {167.78, 53.55, -14.73},
                                             {167.89, 53.35, -40.74}},
                                            {{87.25, 139.75, -27.13},
                                             {84.35, 107.47, -36.80},
                                             {84.01, 107.64, -12.83},
                                             {63.25, 116.86, -13.71},
                                             {62.21, 116.66, -36.42},
                                             {104.60, 146.69, -40.50},
                                             {104.81, 146.58, -14.96},
                                             {80.77, 156.50, -14.73},
                                             {80.99, 156.44, -40.74}},
                                            {{-37.12, 160.51, -27.13},
                                             {-16.35, 135.64, -36.80},
                                             {-16.71, 135.51, -12.83},
                                             {-37.91, 127.36, -13.71},
                                             {-38.50, 126.48, -36.42},
                                             {-29.76, 177.69, -40.50},
                                             {-29.53, 177.76, -14.96},
                                             {-53.55, 167.78, -14.73},
                                             {-53.35, 167.89, -40.74}},
                                            {{-139.75, 87.25, -27.13},
                                             {-107.47, 84.35, -36.80},
                                             {-107.64, 84.01, -12.83},
                                             {-116.86, 63.25, -13.71},
                                             {-116.66, 62.21, -36.42},
                                             {-146.69, 104.60, -40.50},
                                             {-146.58, 104.81, -14.96},
                                             {-156.50, 80.77, -14.73},
                                             {-156.44, 80.99, -40.74}},
                                            {{-160.51, -37.12, -27.13},
                                             {-135.64, -16.35, -36.80},
                                             {-135.51, -16.71, -12.83},
                                             {-127.36, -37.91, -13.71},
                                             {-126.48, -38.50, -36.42},
                                             {-177.69, -29.76, -40.50},
                                             {-177.76, -29.53, -14.96},
                                             {-167.78, -53.55, -14.73},
                                             {-167.89, -53.35, -40.74}},
                                            {{-87.25, -139.75, -27.13},
                                             {-84.35, -107.47, -36.80},
                                             {-84.01, -107.64, -12.83},
                                             {-63.25, -116.86, -13.71},
                                             {-62.21, -116.66, -36.42},
                                             {-104.60, -146.69, -40.50},
                                             {-104.81, -146.58, -14.96},
                                             {-80.77, -156.50, -14.73},
                                             {-80.99, -156.44, -40.74}},
                                            {{37.12, -160.51, -27.13},
                                             {16.35, -135.64, -36.80},
                                             {16.71, -135.51, -12.83},
                                             {37.91, -127.36, -13.71},
                                             {38.50, -126.48, -36.42},
                                             {29.76, -177.69, -40.50},
                                             {29.53, -177.76, -14.96},
                                             {53.55, -167.78, -14.73},
                                             {53.35, -167.89, -40.74}},
                                            {{139.75, -87.25, -27.13},
                                             {107.47, -84.35, -36.80},
                                             {107.64, -84.01, -12.83},
                                             {116.86, -63.25, -13.71},
                                             {116.66, -62.21, -36.42},
                                             {146.69, -104.60, -40.50},
                                             {146.58, -104.81, -14.96},
                                             {156.50, -80.77, -14.73},
                                             {156.44, -80.99, -40.74}},
                                            {{98.82, 11.57, -134.09},
                                             {75.99, -8.35, -119.06},
                                             {91.52, -1.51, -102.12},
                                             {82.63, 19.39, -103.15},
                                             {67.08, 13.90, -118.80},
                                             {103.72, -0.87, -152.77},
                                             {120.49, 5.81, -134.70},
                                             {110.66, 29.90, -134.51},
                                             {93.78, 22.65, -152.93}},
                                            {{-11.57, 98.82, -134.09},
                                             {8.35, 75.99, -119.06},
                                             {1.51, 91.52, -102.12},
                                             {-19.39, 82.63, -103.15},
                                             {-13.90, 67.08, -118.80},
                                             {0.87, 103.72, -152.77},
                                             {-5.81, 120.49, -134.70},
                                             {-29.90, 110.66, -134.51},
                                             {-22.65, 93.78, -152.93}},
                                            {{-98.82, -11.57, -134.09},
                                             {-75.99, 8.35, -119.06},
                                             {-91.52, 1.51, -102.12},
                                             {-82.63, -19.39, -103.15},
                                             {-67.08, -13.90, -118.80},
                                             {-103.72, 0.87, -152.77},
                                             {-120.49, -5.81, -134.70},
                                             {-110.66, -29.90, -134.51},
                                             {-93.78, -22.65, -152.93}},
                                            {{11.57, -98.82, -134.09},
                                             {-8.35, -75.99, -119.06},
                                             {-1.51, -91.52, -102.12},
                                             {19.39, -82.63, -103.15},
                                             {13.90, -67.08, -118.80},
                                             {-0.87, -103.72, -152.77},
                                             {5.81, -120.49, -134.70},
                                             {29.90, -110.66, -134.51},
                                             {22.65, -93.78, -152.93}}};

// Assuming this is the 3
double TTigress::GeWhitePosition[17][9][3] = {{{0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.},
                                               {0., 0., 0.}},
                                              {{98.82, 11.57, 134.09},
                                               {75.99, -8.35, 119.06},
                                               {67.08, 13.90, 118.80},
                                               {82.63, 19.39, 103.15},
                                               {91.52, -1.51, 102.12},
                                               {103.72, -0.87, 152.77},
                                               {93.78, 22.65, 152.93},
                                               {110.66, 29.90, 134.51},
                                               {120.49, 5.81, 134.70}},
                                              {{-11.57, 98.82, 134.09},
                                               {8.35, 75.99, 119.06},
                                               {-13.90, 67.08, 118.80},
                                               {-19.39, 82.63, 103.15},
                                               {1.51, 91.52, 102.12},
                                               {0.87, 103.72, 152.77},
                                               {-22.65, 93.78, 152.93},
                                               {-29.90, 110.66, 134.51},
                                               {-5.81, 120.49, 134.70}},
                                              {{-98.82, -11.57, 134.09},
                                               {-75.99, 8.35, 119.06},
                                               {-67.08, -13.90, 118.80},
                                               {-82.63, -19.39, 103.15},
                                               {-91.52, 1.51, 102.12},
                                               {-103.72, 0.87, 152.77},
                                               {-93.78, -22.65, 152.93},
                                               {-110.66, -29.90, 134.51},
                                               {-120.49, -5.81, 134.70}},
                                              {{11.57, -98.82, 134.09},
                                               {-8.35, -75.99, 119.06},
                                               {13.90, -67.08, 118.80},
                                               {19.39, -82.63, 103.15},
                                               {-1.51, -91.52, 102.12},
                                               {-0.87, -103.72, 152.77},
                                               {22.65, -93.78, 152.93},
                                               {29.90, -110.66, 134.51},
                                               {5.81, -120.49, 134.70}},
                                              {{160.51, 37.12, 27.13},
                                               {135.64, 16.35, 36.80},
                                               {126.48, 38.50, 36.42},
                                               {127.36, 37.91, 13.71},
                                               {135.51, 16.71, 12.83},
                                               {177.69, 29.76, 40.50},
                                               {167.89, 53.35, 40.74},
                                               {167.78, 53.55, 14.73},
                                               {177.76, 29.53, 14.96}},
                                              {{87.25, 139.75, 27.13},
                                               {84.35, 107.47, 36.80},
                                               {62.21, 116.66, 36.42},
                                               {63.25, 116.86, 13.71},
                                               {84.01, 107.64, 12.83},
                                               {104.60, 146.69, 40.50},
                                               {80.99, 156.44, 40.74},
                                               {80.77, 156.50, 14.73},
                                               {104.81, 146.58, 14.96}},
                                              {{-37.12, 160.51, 27.13},
                                               {-16.35, 135.64, 36.80},
                                               {-38.50, 126.48, 36.42},
                                               {-37.91, 127.36, 13.71},
                                               {-16.71, 135.51, 12.83},
                                               {-29.76, 177.69, 40.50},
                                               {-53.35, 167.89, 40.74},
                                               {-53.55, 167.78, 14.73},
                                               {-29.53, 177.76, 14.96}},
                                              {{-139.75, 87.25, 27.13},
                                               {-107.47, 84.35, 36.80},
                                               {-116.66, 62.21, 36.42},
                                               {-116.86, 63.25, 13.71},
                                               {-107.64, 84.01, 12.83},
                                               {-146.69, 104.60, 40.50},
                                               {-156.44, 80.99, 40.74},
                                               {-156.50, 80.77, 14.73},
                                               {-146.58, 104.81, 14.96}},
                                              {{-160.51, -37.12, 27.13},
                                               {-135.64, -16.35, 36.80},
                                               {-126.48, -38.50, 36.42},
                                               {-127.36, -37.91, 13.71},
                                               {-135.51, -16.71, 12.83},
                                               {-177.69, -29.76, 40.50},
                                               {-167.89, -53.35, 40.74},
                                               {-167.78, -53.55, 14.73},
                                               {-177.76, -29.53, 14.96}},
                                              {{-87.25, -139.75, 27.13},
                                               {-84.35, -107.47, 36.80},
                                               {-62.21, -116.66, 36.42},
                                               {-63.25, -116.86, 13.71},
                                               {-84.01, -107.64, 12.83},
                                               {-104.60, -146.69, 40.50},
                                               {-80.99, -156.44, 40.74},
                                               {-80.77, -156.50, 14.73},
                                               {-104.81, -146.58, 14.96}},
                                              {{37.12, -160.51, 27.13},
                                               {16.35, -135.64, 36.80},
                                               {38.50, -126.48, 36.42},
                                               {37.91, -127.36, 13.71},
                                               {16.71, -135.51, 12.83},
                                               {29.76, -177.69, 40.50},
                                               {53.35, -167.89, 40.74},
                                               {53.55, -167.78, 14.73},
                                               {29.53, -177.76, 14.96}},
                                              {{139.75, -87.25, 27.13},
                                               {107.47, -84.35, 36.80},
                                               {116.66, -62.21, 36.42},
                                               {116.86, -63.25, 13.71},
                                               {107.64, -84.01, 12.83},
                                               {146.69, -104.60, 40.50},
                                               {156.44, -80.99, 40.74},
                                               {156.50, -80.77, 14.73},
                                               {146.58, -104.81, 14.96}},
                                              {{134.26, 26.25, -95.72},
                                               {124.08, 11.56, -67.01},
                                               {114.67, 33.61, -67.30},
                                               {100.55, 26.81, -83.76},
                                               {108.28, 5.43, -83.98},
                                               {156.64, 21.05, -95.49},
                                               {147.01, 44.70, -95.31},
                                               {129.91, 37.87, -113.68},
                                               {140.03, 13.91, -113.54}},
                                              {{-26.25, 134.26, -95.72},
                                               {-11.56, 124.08, -67.01},
                                               {-33.61, 114.67, -67.30},
                                               {-26.81, 100.55, -83.76},
                                               {-5.43, 108.28, -83.98},
                                               {-21.05, 156.64, -95.49},
                                               {-44.70, 147.01, -95.31},
                                               {-37.87, 129.91, -113.68},
                                               {-13.91, 140.03, -113.54}},
                                              {{-134.26, -26.25, -95.72},
                                               {-124.08, -11.56, -67.01},
                                               {-114.67, -33.61, -67.30},
                                               {-100.55, -26.81, -83.76},
                                               {-108.28, -5.43, -83.98},
                                               {-156.64, -21.05, -95.49},
                                               {-147.01, -44.70, -95.31},
                                               {-129.91, -37.87, -113.68},
                                               {-140.03, -13.91, -113.54}},
                                              {{26.25, -134.26, -95.72},
                                               {11.56, -124.08, -67.01},
                                               {33.61, -114.67, -67.30},
                                               {26.81, -100.55, -83.76},
                                               {5.43, -108.28, -83.98},
                                               {21.05, -156.64, -95.49},
                                               {44.70, -147.01, -95.31},
                                               {37.87, -129.91, -113.68},
                                               {13.91, -140.03, -113.54}}};

double TTigress::GeBluePositionBack[17][9][3] = {{{0., 0., 0.},
                                                  {0., 0., 0.},
                                                  {0., 0., 0.},
                                                  {0., 0., 0.},
                                                  {0., 0., 0.},
                                                  {0., 0., 0.},
                                                  {0., 0., 0.},
                                                  {0., 0., 0.},
                                                  {0., 0., 0.}},
                                                 {{100.92, 71.17, 158.84},
                                                  {70.69, 69.11, 143.80},
                                                  {86.51, 75.25, 126.87},
                                                  {95.01, 54.19, 127.90},
                                                  {80.13, 47.08, 143.55},
                                                  {95.59, 83.43, 177.52},
                                                  {112.17, 90.56, 159.45},
                                                  {122.26, 66.58, 159.26},
                                                  {105.20, 59.77, 177.67}},
                                                 {{-71.17, 100.92, 158.84},
                                                  {-69.11, 70.69, 143.80},
                                                  {-75.25, 86.51, 126.87},
                                                  {-54.19, 95.01, 127.90},
                                                  {-47.08, 80.13, 143.55},
                                                  {-83.43, 95.59, 177.52},
                                                  {-90.56, 112.17, 159.45},
                                                  {-66.58, 122.26, 159.26},
                                                  {-59.77, 105.20, 177.67}},
                                                 {{-100.92, -71.17, 158.84},
                                                  {-70.69, -69.11, 143.80},
                                                  {-86.51, -75.25, 126.87},
                                                  {-95.01, -54.19, 127.90},
                                                  {-80.13, -47.08, 143.55},
                                                  {-95.59, -83.43, 177.52},
                                                  {-112.17, -90.56, 159.45},
                                                  {-122.26, -66.58, 159.26},
                                                  {-105.20, -59.77, 177.67}},
                                                 {{71.17, -100.92, 158.84},
                                                  {69.11, -70.69, 143.80},
                                                  {75.25, -86.51, 126.87},
                                                  {54.19, -95.01, 127.90},
                                                  {47.08, -80.13, 143.55},
                                                  {83.43, -95.59, 177.52},
                                                  {90.56, -112.17, 159.45},
                                                  {66.58, -122.26, 159.26},
                                                  {59.77, -105.20, 177.67}},
                                                 {{172.08, 100.64, 27.13},
                                                  {139.81, 97.74, 36.80},
                                                  {139.97, 97.40, 12.83},
                                                  {149.20, 76.64, 13.71},
                                                  {149.00, 75.60, 36.42},
                                                  {179.02, 117.99, 40.50},
                                                  {178.91, 118.21, 14.96},
                                                  {188.84, 94.16, 14.73},
                                                  {188.78, 94.39, 40.74}},
                                                 {{50.52, 192.85, 27.13},
                                                  {29.74, 167.97, 36.80},
                                                  {30.10, 167.85, 12.83},
                                                  {51.31, 159.69, 13.71},
                                                  {51.90, 158.82, 36.42},
                                                  {43.16, 210.02, 40.50},
                                                  {42.93, 210.09, 14.96},
                                                  {66.95, 200.11, 14.73},
                                                  {66.75, 200.23, 40.74}},
                                                 {{-100.64, 172.08, 27.13},
                                                  {-97.74, 139.81, 36.80},
                                                  {-97.40, 139.97, 12.83},
                                                  {-76.64, 149.20, 13.71},
                                                  {-75.60, 149.00, 36.42},
                                                  {-117.99, 179.02, 40.50},
                                                  {-118.21, 178.91, 14.96},
                                                  {-94.16, 188.84, 14.73},
                                                  {-94.39, 188.78, 40.74}},
                                                 {{-192.85, 50.52, 27.13},
                                                  {-167.97, 29.74, 36.80},
                                                  {-167.85, 30.10, 12.83},
                                                  {-159.69, 51.31, 13.71},
                                                  {-158.82, 51.90, 36.42},
                                                  {-210.02, 43.16, 40.50},
                                                  {-210.09, 42.93, 14.96},
                                                  {-200.11, 66.95, 14.73},
                                                  {-200.23, 66.75, 40.74}},
                                                 {{-172.08, -100.64, 27.13},
                                                  {-139.81, -97.74, 36.80},
                                                  {-139.97, -97.40, 12.83},
                                                  {-149.20, -76.64, 13.71},
                                                  {-149.00, -75.60, 36.42},
                                                  {-179.02, -117.99, 40.50},
                                                  {-178.91, -118.21, 14.96},
                                                  {-188.84, -94.16, 14.73},
                                                  {-188.78, -94.39, 40.74}},
                                                 {{-50.52, -192.85, 27.13},
                                                  {-29.74, -167.97, 36.80},
                                                  {-30.10, -167.85, 12.83},
                                                  {-51.31, -159.69, 13.71},
                                                  {-51.90, -158.82, 36.42},
                                                  {-43.16, -210.02, 40.50},
                                                  {-42.93, -210.09, 14.96},
                                                  {-66.95, -200.11, 14.73},
                                                  {-66.75, -200.23, 40.74}},
                                                 {{100.64, -172.08, 27.13},
                                                  {97.74, -139.81, 36.80},
                                                  {97.40, -139.97, 12.83},
                                                  {76.64, -149.20, 13.71},
                                                  {75.60, -149.00, 36.42},
                                                  {117.99, -179.02, 40.50},
                                                  {118.21, -178.91, 14.96},
                                                  {94.16, -188.84, 14.73},
                                                  {94.39, -188.78, 40.74}},
                                                 {{192.85, -50.52, 27.13},
                                                  {167.97, -29.74, 36.80},
                                                  {167.85, -30.10, 12.83},
                                                  {159.69, -51.31, 13.71},
                                                  {158.82, -51.90, 36.42},
                                                  {210.02, -43.16, 40.50},
                                                  {210.09, -42.93, 14.96},
                                                  {200.11, -66.95, 14.73},
                                                  {200.23, -66.75, 40.74}},
                                                 {{136.36, 85.85, -120.47},
                                                  {118.78, 89.03, -91.76},
                                                  {103.27, 82.20, -108.72},
                                                  {112.92, 61.61, -108.51},
                                                  {127.71, 66.79, -92.04},
                                                  {148.51, 105.35, -120.24},
                                                  {131.72, 98.66, -138.29},
                                                  {141.50, 74.56, -138.43},
                                                  {158.43, 81.81, -120.06}},
                                                 {{-85.85, 136.36, -120.47},
                                                  {-89.03, 118.78, -91.76},
                                                  {-82.20, 103.27, -108.72},
                                                  {-61.61, 112.92, -108.51},
                                                  {-66.79, 127.71, -92.04},
                                                  {-105.35, 148.51, -120.24},
                                                  {-98.66, 131.72, -138.29},
                                                  {-74.56, 141.50, -138.43},
                                                  {-81.81, 158.43, -120.06}},
                                                 {{-136.36, -85.85, -120.47},
                                                  {-118.78, -89.03, -91.76},
                                                  {-103.27, -82.20, -108.72},
                                                  {-112.92, -61.61, -108.51},
                                                  {-127.71, -66.79, -92.04},
                                                  {-148.51, -105.35, -120.24},
                                                  {-131.72, -98.66, -138.29},
                                                  {-141.50, -74.56, -138.43},
                                                  {-158.43, -81.81, -120.06}},
                                                 {{85.85, -136.36, -120.47},
                                                  {89.03, -118.78, -91.76},
                                                  {82.20, -103.27, -108.72},
                                                  {61.61, -112.92, -108.51},
                                                  {66.79, -127.71, -92.04},
                                                  {105.35, -148.51, -120.24},
                                                  {98.66, -131.72, -138.29},
                                                  {74.56, -141.50, -138.43},
                                                  {81.81, -158.43, -120.06}}};

// Assuming this is the 1
double TTigress::GeGreenPositionBack[17][9][3] = {{{0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.}},
                                                  {{136.36, 85.85, 120.47},
                                                   {118.78, 89.03, 91.76},
                                                   {127.71, 66.79, 92.04},
                                                   {112.92, 61.61, 108.51},
                                                   {103.27, 82.20, 108.72},
                                                   {148.51, 105.35, 120.24},
                                                   {158.43, 81.81, 120.06},
                                                   {141.50, 74.56, 138.43},
                                                   {131.72, 98.66, 138.29}},
                                                  {{-85.85, 136.36, 120.47},
                                                   {-89.03, 118.78, 91.76},
                                                   {-66.79, 127.71, 92.04},
                                                   {-61.61, 112.92, 108.51},
                                                   {-82.20, 103.27, 108.72},
                                                   {-105.35, 148.51, 120.24},
                                                   {-81.81, 158.43, 120.06},
                                                   {-74.56, 141.50, 138.43},
                                                   {-98.66, 131.72, 138.29}},
                                                  {{-136.36, -85.85, 120.47},
                                                   {-118.78, -89.03, 91.76},
                                                   {-127.71, -66.79, 92.04},
                                                   {-112.92, -61.61, 108.51},
                                                   {-103.27, -82.20, 108.72},
                                                   {-148.51, -105.35, 120.24},
                                                   {-158.43, -81.81, 120.06},
                                                   {-141.50, -74.56, 138.43},
                                                   {-131.72, -98.66, 138.29}},
                                                  {{85.85, -136.36, 120.47},
                                                   {89.03, -118.78, 91.76},
                                                   {66.79, -127.71, 92.04},
                                                   {61.61, -112.92, 108.51},
                                                   {82.20, -103.27, 108.72},
                                                   {105.35, -148.51, 120.24},
                                                   {81.81, -158.43, 120.06},
                                                   {74.56, -141.50, 138.43},
                                                   {98.66, -131.72, 138.29}},
                                                  {{172.08, 100.64, -27.13},
                                                   {139.81, 97.74, -36.80},
                                                   {149.00, 75.60, -36.42},
                                                   {149.20, 76.64, -13.71},
                                                   {139.97, 97.40, -12.83},
                                                   {179.02, 117.99, -40.50},
                                                   {188.78, 94.39, -40.74},
                                                   {188.84, 94.16, -14.73},
                                                   {178.91, 118.21, -14.96}},
                                                  {{50.52, 192.85, -27.13},
                                                   {29.74, 167.97, -36.80},
                                                   {51.90, 158.82, -36.42},
                                                   {51.31, 159.69, -13.71},
                                                   {30.10, 167.85, -12.83},
                                                   {43.16, 210.02, -40.50},
                                                   {66.75, 200.23, -40.74},
                                                   {66.95, 200.11, -14.73},
                                                   {42.93, 210.09, -14.96}},
                                                  {{-100.64, 172.08, -27.13},
                                                   {-97.74, 139.81, -36.80},
                                                   {-75.60, 149.00, -36.42},
                                                   {-76.64, 149.20, -13.71},
                                                   {-97.40, 139.97, -12.83},
                                                   {-117.99, 179.02, -40.50},
                                                   {-94.39, 188.78, -40.74},
                                                   {-94.16, 188.84, -14.73},
                                                   {-118.21, 178.91, -14.96}},
                                                  {{-192.85, 50.52, -27.13},
                                                   {-167.97, 29.74, -36.80},
                                                   {-158.82, 51.90, -36.42},
                                                   {-159.69, 51.31, -13.71},
                                                   {-167.85, 30.10, -12.83},
                                                   {-210.02, 43.16, -40.50},
                                                   {-200.23, 66.75, -40.74},
                                                   {-200.11, 66.95, -14.73},
                                                   {-210.09, 42.93, -14.96}},
                                                  {{-172.08, -100.64, -27.13},
                                                   {-139.81, -97.74, -36.80},
                                                   {-149.00, -75.60, -36.42},
                                                   {-149.20, -76.64, -13.71},
                                                   {-139.97, -97.40, -12.83},
                                                   {-179.02, -117.99, -40.50},
                                                   {-188.78, -94.39, -40.74},
                                                   {-188.84, -94.16, -14.73},
                                                   {-178.91, -118.21, -14.96}},
                                                  {{-50.52, -192.85, -27.13},
                                                   {-29.74, -167.97, -36.80},
                                                   {-51.90, -158.82, -36.42},
                                                   {-51.31, -159.69, -13.71},
                                                   {-30.10, -167.85, -12.83},
                                                   {-43.16, -210.02, -40.50},
                                                   {-66.75, -200.23, -40.74},
                                                   {-66.95, -200.11, -14.73},
                                                   {-42.93, -210.09, -14.96}},
                                                  {{100.64, -172.08, -27.13},
                                                   {97.74, -139.81, -36.80},
                                                   {75.60, -149.00, -36.42},
                                                   {76.64, -149.20, -13.71},
                                                   {97.40, -139.97, -12.83},
                                                   {117.99, -179.02, -40.50},
                                                   {94.39, -188.78, -40.74},
                                                   {94.16, -188.84, -14.73},
                                                   {118.21, -178.91, -14.96}},
                                                  {{192.85, -50.52, -27.13},
                                                   {167.97, -29.74, -36.80},
                                                   {158.82, -51.90, -36.42},
                                                   {159.69, -51.31, -13.71},
                                                   {167.85, -30.10, -12.83},
                                                   {210.02, -43.16, -40.50},
                                                   {200.23, -66.75, -40.74},
                                                   {200.11, -66.95, -14.73},
                                                   {210.09, -42.93, -14.96}},
                                                  {{100.92, 71.17, -158.84},
                                                   {70.69, 69.11, -143.80},
                                                   {80.13, 47.08, -143.55},
                                                   {95.01, 54.19, -127.90},
                                                   {86.51, 75.25, -126.87},
                                                   {95.59, 83.43, -177.52},
                                                   {105.20, 59.77, -177.67},
                                                   {122.26, 66.58, -159.26},
                                                   {112.17, 90.56, -159.45}},
                                                  {{-71.17, 100.92, -158.84},
                                                   {-69.11, 70.69, -143.80},
                                                   {-47.08, 80.13, -143.55},
                                                   {-54.19, 95.01, -127.90},
                                                   {-75.25, 86.51, -126.87},
                                                   {-83.43, 95.59, -177.52},
                                                   {-59.77, 105.20, -177.67},
                                                   {-66.58, 122.26, -159.26},
                                                   {-90.56, 112.17, -159.45}},
                                                  {{-100.92, -71.17, -158.84},
                                                   {-70.69, -69.11, -143.80},
                                                   {-80.13, -47.08, -143.55},
                                                   {-95.01, -54.19, -127.90},
                                                   {-86.51, -75.25, -126.87},
                                                   {-95.59, -83.43, -177.52},
                                                   {-105.20, -59.77, -177.67},
                                                   {-122.26, -66.58, -159.26},
                                                   {-112.17, -90.56, -159.45}},
                                                  {{71.17, -100.92, -158.84},
                                                   {69.11, -70.69, -143.80},
                                                   {47.08, -80.13, -143.55},
                                                   {54.19, -95.01, -127.90},
                                                   {75.25, -86.51, -126.87},
                                                   {83.43, -95.59, -177.52},
                                                   {59.77, -105.20, -177.67},
                                                   {66.58, -122.26, -159.26},
                                                   {90.56, -112.17, -159.45}}};

// Assuming this is the 2
double TTigress::GeRedPositionBack[17][9][3] = {{{0., 0., 0.},
                                                 {0., 0., 0.},
                                                 {0., 0., 0.},
                                                 {0., 0., 0.},
                                                 {0., 0., 0.},
                                                 {0., 0., 0.},
                                                 {0., 0., 0.},
                                                 {0., 0., 0.},
                                                 {0., 0., 0.}},
                                                {{157.13, 35.72, 120.47},
                                                 {146.94, 21.03, 91.76},
                                                 {131.15, 14.90, 108.72},
                                                 {123.41, 36.28, 108.51},
                                                 {137.53, 43.08, 92.04},
                                                 {179.50, 30.52, 120.24},
                                                 {162.90, 23.38, 138.29},
                                                 {152.78, 47.34, 138.43},
                                                 {169.87, 54.17, 120.06}},
                                                {{-35.72, 157.13, 120.47},
                                                 {-21.03, 146.94, 91.76},
                                                 {-14.90, 131.15, 108.72},
                                                 {-36.28, 123.41, 108.51},
                                                 {-43.08, 137.53, 92.04},
                                                 {-30.52, 179.50, 120.24},
                                                 {-23.38, 162.90, 138.29},
                                                 {-47.34, 152.78, 138.43},
                                                 {-54.17, 169.87, 120.06}},
                                                {{-157.13, -35.72, 120.47},
                                                 {-146.94, -21.03, 91.76},
                                                 {-131.15, -14.90, 108.72},
                                                 {-123.41, -36.28, 108.51},
                                                 {-137.53, -43.08, 92.04},
                                                 {-179.50, -30.52, 120.24},
                                                 {-162.90, -23.38, 138.29},
                                                 {-152.78, -47.34, 138.43},
                                                 {-169.87, -54.17, 120.06}},
                                                {{35.72, -157.13, 120.47},
                                                 {21.03, -146.94, 91.76},
                                                 {14.90, -131.15, 108.72},
                                                 {36.28, -123.41, 108.51},
                                                 {43.08, -137.53, 92.04},
                                                 {30.52, -179.50, 120.24},
                                                 {23.38, -162.90, 138.29},
                                                 {47.34, -152.78, 138.43},
                                                 {54.17, -169.87, 120.06}},
                                                {{192.85, 50.52, -27.13},
                                                 {167.97, 29.74, -36.80},
                                                 {167.85, 30.10, -12.83},
                                                 {159.69, 51.31, -13.71},
                                                 {158.82, 51.90, -36.42},
                                                 {210.02, 43.16, -40.50},
                                                 {210.09, 42.93, -14.96},
                                                 {200.11, 66.95, -14.73},
                                                 {200.23, 66.75, -40.74}},
                                                {{100.64, 172.08, -27.13},
                                                 {97.74, 139.81, -36.80},
                                                 {97.40, 139.97, -12.83},
                                                 {76.64, 149.20, -13.71},
                                                 {75.60, 149.00, -36.42},
                                                 {117.99, 179.02, -40.50},
                                                 {118.21, 178.91, -14.96},
                                                 {94.16, 188.84, -14.73},
                                                 {94.39, 188.78, -40.74}},
                                                {{-50.52, 192.85, -27.13},
                                                 {-29.74, 167.97, -36.80},
                                                 {-30.10, 167.85, -12.83},
                                                 {-51.31, 159.69, -13.71},
                                                 {-51.90, 158.82, -36.42},
                                                 {-43.16, 210.02, -40.50},
                                                 {-42.93, 210.09, -14.96},
                                                 {-66.95, 200.11, -14.73},
                                                 {-66.75, 200.23, -40.74}},
                                                {{-172.08, 100.64, -27.13},
                                                 {-139.81, 97.74, -36.80},
                                                 {-139.97, 97.40, -12.83},
                                                 {-149.20, 76.64, -13.71},
                                                 {-149.00, 75.60, -36.42},
                                                 {-179.02, 117.99, -40.50},
                                                 {-178.91, 118.21, -14.96},
                                                 {-188.84, 94.16, -14.73},
                                                 {-188.78, 94.39, -40.74}},
                                                {{-192.85, -50.52, -27.13},
                                                 {-167.97, -29.74, -36.80},
                                                 {-167.85, -30.10, -12.83},
                                                 {-159.69, -51.31, -13.71},
                                                 {-158.82, -51.90, -36.42},
                                                 {-210.02, -43.16, -40.50},
                                                 {-210.09, -42.93, -14.96},
                                                 {-200.11, -66.95, -14.73},
                                                 {-200.23, -66.75, -40.74}},
                                                {{-100.64, -172.08, -27.13},
                                                 {-97.74, -139.81, -36.80},
                                                 {-97.40, -139.97, -12.83},
                                                 {-76.64, -149.20, -13.71},
                                                 {-75.60, -149.00, -36.42},
                                                 {-117.99, -179.02, -40.50},
                                                 {-118.21, -178.91, -14.96},
                                                 {-94.16, -188.84, -14.73},
                                                 {-94.39, -188.78, -40.74}},
                                                {{50.52, -192.85, -27.13},
                                                 {29.74, -167.97, -36.80},
                                                 {30.10, -167.85, -12.83},
                                                 {51.31, -159.69, -13.71},
                                                 {51.90, -158.82, -36.42},
                                                 {43.16, -210.02, -40.50},
                                                 {42.93, -210.09, -14.96},
                                                 {66.95, -200.11, -14.73},
                                                 {66.75, -200.23, -40.74}},
                                                {{172.08, -100.64, -27.13},
                                                 {139.81, -97.74, -36.80},
                                                 {139.97, -97.40, -12.83},
                                                 {149.20, -76.64, -13.71},
                                                 {149.00, -75.60, -36.42},
                                                 {179.02, -117.99, -40.50},
                                                 {178.91, -118.21, -14.96},
                                                 {188.84, -94.16, -14.73},
                                                 {188.78, -94.39, -40.74}},
                                                {{121.68, 21.04, -158.84},
                                                 {98.86, 1.12, -143.80},
                                                 {114.39, 7.96, -126.87},
                                                 {105.50, 28.86, -127.90},
                                                 {89.95, 23.37, -143.55},
                                                 {126.59, 8.60, -177.52},
                                                 {143.35, 15.28, -159.45},
                                                 {133.53, 39.37, -159.26},
                                                 {116.65, 32.12, -177.67}},
                                                {{-21.04, 121.68, -158.84},
                                                 {-1.12, 98.86, -143.80},
                                                 {-7.96, 114.39, -126.87},
                                                 {-28.86, 105.50, -127.90},
                                                 {-23.37, 89.95, -143.55},
                                                 {-8.60, 126.59, -177.52},
                                                 {-15.28, 143.35, -159.45},
                                                 {-39.37, 133.53, -159.26},
                                                 {-32.12, 116.65, -177.67}},
                                                {{-121.68, -21.04, -158.84},
                                                 {-98.86, -1.12, -143.80},
                                                 {-114.39, -7.96, -126.87},
                                                 {-105.50, -28.86, -127.90},
                                                 {-89.95, -23.37, -143.55},
                                                 {-126.59, -8.60, -177.52},
                                                 {-143.35, -15.28, -159.45},
                                                 {-133.53, -39.37, -159.26},
                                                 {-116.65, -32.12, -177.67}},
                                                {{21.04, -121.68, -158.84},
                                                 {1.12, -98.86, -143.80},
                                                 {7.96, -114.39, -126.87},
                                                 {28.86, -105.50, -127.90},
                                                 {23.37, -89.95, -143.55},
                                                 {8.60, -126.59, -177.52},
                                                 {15.28, -143.35, -159.45},
                                                 {39.37, -133.53, -159.26},
                                                 {32.12, -116.65, -177.67}}};

// Assuming this is the 3
double TTigress::GeWhitePositionBack[17][9][3] = {{{0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.},
                                                   {0., 0., 0.}},
                                                  {{121.68, 21.04, 158.84},
                                                   {98.86, 1.12, 143.80},
                                                   {89.95, 23.37, 143.55},
                                                   {105.50, 28.86, 127.90},
                                                   {114.39, 7.96, 126.87},
                                                   {126.59, 8.60, 177.52},
                                                   {116.65, 32.12, 177.67},
                                                   {133.53, 39.37, 159.26},
                                                   {143.35, 15.28, 159.45}},
                                                  {{-21.04, 121.68, 158.84},
                                                   {-1.12, 98.86, 143.80},
                                                   {-23.37, 89.95, 143.55},
                                                   {-28.86, 105.50, 127.90},
                                                   {-7.96, 114.39, 126.87},
                                                   {-8.60, 126.59, 177.52},
                                                   {-32.12, 116.65, 177.67},
                                                   {-39.37, 133.53, 159.26},
                                                   {-15.28, 143.35, 159.45}},
                                                  {{-121.68, -21.04, 158.84},
                                                   {-98.86, -1.12, 143.80},
                                                   {-89.95, -23.37, 143.55},
                                                   {-105.50, -28.86, 127.90},
                                                   {-114.39, -7.96, 126.87},
                                                   {-126.59, -8.60, 177.52},
                                                   {-116.65, -32.12, 177.67},
                                                   {-133.53, -39.37, 159.26},
                                                   {-143.35, -15.28, 159.45}},
                                                  {{21.04, -121.68, 158.84},
                                                   {1.12, -98.86, 143.80},
                                                   {23.37, -89.95, 143.55},
                                                   {28.86, -105.50, 127.90},
                                                   {7.96, -114.39, 126.87},
                                                   {8.60, -126.59, 177.52},
                                                   {32.12, -116.65, 177.67},
                                                   {39.37, -133.53, 159.26},
                                                   {15.28, -143.35, 159.45}},
                                                  {{192.85, 50.52, 27.13},
                                                   {167.97, 29.74, 36.80},
                                                   {158.82, 51.90, 36.42},
                                                   {159.69, 51.31, 13.71},
                                                   {167.85, 30.10, 12.83},
                                                   {210.02, 43.16, 40.50},
                                                   {200.23, 66.75, 40.74},
                                                   {200.11, 66.95, 14.73},
                                                   {210.09, 42.93, 14.96}},
                                                  {{100.64, 172.08, 27.13},
                                                   {97.74, 139.81, 36.80},
                                                   {75.60, 149.00, 36.42},
                                                   {76.64, 149.20, 13.71},
                                                   {97.40, 139.97, 12.83},
                                                   {117.99, 179.02, 40.50},
                                                   {94.39, 188.78, 40.74},
                                                   {94.16, 188.84, 14.73},
                                                   {118.21, 178.91, 14.96}},
                                                  {{-50.52, 192.85, 27.13},
                                                   {-29.74, 167.97, 36.80},
                                                   {-51.90, 158.82, 36.42},
                                                   {-51.31, 159.69, 13.71},
                                                   {-30.10, 167.85, 12.83},
                                                   {-43.16, 210.02, 40.50},
                                                   {-66.75, 200.23, 40.74},
                                                   {-66.95, 200.11, 14.73},
                                                   {-42.93, 210.09, 14.96}},
                                                  {{-172.08, 100.64, 27.13},
                                                   {-139.81, 97.74, 36.80},
                                                   {-149.00, 75.60, 36.42},
                                                   {-149.20, 76.64, 13.71},
                                                   {-139.97, 97.40, 12.83},
                                                   {-179.02, 117.99, 40.50},
                                                   {-188.78, 94.39, 40.74},
                                                   {-188.84, 94.16, 14.73},
                                                   {-178.91, 118.21, 14.96}},
                                                  {{-192.85, -50.52, 27.13},
                                                   {-167.97, -29.74, 36.80},
                                                   {-158.82, -51.90, 36.42},
                                                   {-159.69, -51.31, 13.71},
                                                   {-167.85, -30.10, 12.83},
                                                   {-210.02, -43.16, 40.50},
                                                   {-200.23, -66.75, 40.74},
                                                   {-200.11, -66.95, 14.73},
                                                   {-210.09, -42.93, 14.96}},
                                                  {{-100.64, -172.08, 27.13},
                                                   {-97.74, -139.81, 36.80},
                                                   {-75.60, -149.00, 36.42},
                                                   {-76.64, -149.20, 13.71},
                                                   {-97.40, -139.97, 12.83},
                                                   {-117.99, -179.02, 40.50},
                                                   {-94.39, -188.78, 40.74},
                                                   {-94.16, -188.84, 14.73},
                                                   {-118.21, -178.91, 14.96}},
                                                  {{50.52, -192.85, 27.13},
                                                   {29.74, -167.97, 36.80},
                                                   {51.90, -158.82, 36.42},
                                                   {51.31, -159.69, 13.71},
                                                   {30.10, -167.85, 12.83},
                                                   {43.16, -210.02, 40.50},
                                                   {66.75, -200.23, 40.74},
                                                   {66.95, -200.11, 14.73},
                                                   {42.93, -210.09, 14.96}},
                                                  {{172.08, -100.64, 27.13},
                                                   {139.81, -97.74, 36.80},
                                                   {149.00, -75.60, 36.42},
                                                   {149.20, -76.64, 13.71},
                                                   {139.97, -97.40, 12.83},
                                                   {179.02, -117.99, 40.50},
                                                   {188.78, -94.39, 40.74},
                                                   {188.84, -94.16, 14.73},
                                                   {178.91, -118.21, 14.96}},
                                                  {{157.13, 35.72, -120.47},
                                                   {146.94, 21.03, -91.76},
                                                   {137.53, 43.08, -92.04},
                                                   {123.41, 36.28, -108.51},
                                                   {131.15, 14.90, -108.72},
                                                   {179.50, 30.52, -120.24},
                                                   {169.87, 54.17, -120.06},
                                                   {152.78, 47.34, -138.43},
                                                   {162.90, 23.38, -138.29}},
                                                  {{-35.72, 157.13, -120.47},
                                                   {-21.03, 146.94, -91.76},
                                                   {-43.08, 137.53, -92.04},
                                                   {-36.28, 123.41, -108.51},
                                                   {-14.90, 131.15, -108.72},
                                                   {-30.52, 179.50, -120.24},
                                                   {-54.17, 169.87, -120.06},
                                                   {-47.34, 152.78, -138.43},
                                                   {-23.38, 162.90, -138.29}},
                                                  {{-157.13, -35.72, -120.47},
                                                   {-146.94, -21.03, -91.76},
                                                   {-137.53, -43.08, -92.04},
                                                   {-123.41, -36.28, -108.51},
                                                   {-131.15, -14.90, -108.72},
                                                   {-179.50, -30.52, -120.24},
                                                   {-169.87, -54.17, -120.06},
                                                   {-152.78, -47.34, -138.43},
                                                   {-162.90, -23.38, -138.29}},
                                                  {{35.72, -157.13, -120.47},
                                                   {21.03, -146.94, -91.76},
                                                   {43.08, -137.53, -92.04},
                                                   {36.28, -123.41, -108.51},
                                                   {14.90, -131.15, -108.72},
                                                   {30.52, -179.50, -120.24},
                                                   {54.17, -169.87, -120.06},
                                                   {47.34, -152.78, -138.43},
                                                   {23.38, -162.90, -138.29}}};

bool TTigress::BGOSuppression[4][4][5] = {{{true, true, true, true, true},
                                           {true, false, false, false, false},
                                           {false, false, false, false, false},
                                           {false, false, false, false, true}},
                                          {{false, false, false, false, true},
                                           {true, true, true, true, true},
                                           {true, false, false, false, false},
                                           {false, false, false, false, false}},
                                          {{false, false, false, false, false},
                                           {false, false, false, false, true},
                                           {true, true, true, true, true},
                                           {true, false, false, false, false}},
                                          {{true, false, false, false, false},
                                           {false, false, false, false, false},
                                           {false, false, false, false, true},
                                           {true, true, true, true, true}}};
