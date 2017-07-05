
#include "TSiLi.h"
#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TSiLi)
   /// \endcond

   // Having these in Clear() caused issues as functions can be called abstract with out initialising a TSiLi
   int TSiLi::fRingNumber     = 10;
int    TSiLi::fSectorNumber   = 12;
double TSiLi::fOffsetPhi      = -165. * TMath::Pi() / 180.; // For SPICE. Sectors upstream.
double TSiLi::fOuterDiameter  = 94.;
double TSiLi::fInnerDiameter  = 16.;
double TSiLi::fTargetDistance = -117.8;

double TSiLi::sili_noise_fac        = 4;
double TSiLi::sili_default_decay    = 4616.18;
double TSiLi::sili_default_rise     = 20.90;
double TSiLi::sili_default_baseline = -4300;

int TSiLi::FitSiLiShape = 0; // 0 no. 1 try if normal fit fail. 2 yes

TSiLi::TSiLi()
{
   Clear();
}

TSiLi::~TSiLi() = default;

void TSiLi::Copy(TObject& rhs) const
{
   TGRSIDetector::Copy(rhs);
   static_cast<TSiLi&>(rhs).fSiLiHits    = fSiLiHits;
   static_cast<TSiLi&>(rhs).fAddbackHits = fAddbackHits;
   static_cast<TSiLi&>(rhs).fSiLiBits    = 0;
}

TSiLi::TSiLi(const TSiLi& rhs) : TGRSIDetector()
{
   rhs.Copy(*this);
}

void TSiLi::Clear(Option_t*)
{
   fSiLiHits.clear();
   fAddbackHits.clear();
   fSiLiBits.Clear();
}

TSiLi& TSiLi::operator=(const TSiLi& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TSiLi::Print(Option_t*) const
{
   printf("%lu sili_hits\n", fSiLiHits.size());
   printf("%lu sili_addback_hits\n", fAddbackHits.size());
}

TGRSIDetectorHit* TSiLi::GetHit(const Int_t& idx)
{
   return GetSiLiHit(idx);
}

TSiLiHit* TSiLi::GetSiLiHit(const int& i)
{
   try {
      return &fSiLiHits.at(i);
   } catch(const std::out_of_range& oor) {
      std::cerr<<ClassName()<<" is out of range: "<<oor.what()<<std::endl;
      throw grsi::exit_exception(1);
   }
   return nullptr;
}

void TSiLi::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   TSiLiHit hit(*frag); // Waveform fitting happens in ctor now
   fSiLiHits.push_back(std::move(hit));
}

// For mapping you may want to us -position.X() to account for looking upstream
TVector3 TSiLi::GetPosition(int ring, int sector, bool smear)
{

   double dist = fTargetDistance;

   double ring_width   = (fOuterDiameter - fInnerDiameter) * 0.5 / fRingNumber; // radial width!
   double inner_radius = fInnerDiameter / 2.0;
   double phi_width    = 2. * TMath::Pi() / fSectorNumber;
   double phi          = phi_width * sector; // the phi angle....
   phi += fOffsetPhi;
   double radius = inner_radius + ring_width * (ring + 0.5);
   if(smear) {
      double sep = ring_width * 0.025;
      double r1 = radius - ring_width * 0.5 + sep, r2 = radius + ring_width * 0.5 - sep;
      radius        = sqrt(gRandom->Uniform(r1 * r1, r2 * r2));
      double sepphi = sep / radius;
      phi           = gRandom->Uniform(phi - phi_width * 0.5 + sepphi, phi + phi_width * 0.5 - sepphi);
   }

   return TVector3(cos(phi) * radius, sin(phi) * radius, dist);
}

double TSiLi::GetSegmentArea(Int_t seg)
{
   int    r          = GetRing(seg);
   double ring_width = (fOuterDiameter - fInnerDiameter) * 0.5 / fRingNumber; // radial width!
   double r1         = fInnerDiameter + r * ring_width;
   double r2         = fInnerDiameter + (r + 1) * ring_width;

   return (TMath::Pi() * (r2 * r2 - r1 * r1)) / fSectorNumber;
}

TSiLiHit* TSiLi::GetAddbackHit(const int& i)
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

// How the make addback currently works
//
// First we itterate through all pixel hits
// If the hit has not yet been assigned to a "cluster" a new cluster is created containing it
// The hit is compared all following hits against fAddbackCriterion
// If they are acceptable neighbours the second hit is also added to the cluster
//
// fAddbackCriterion checks if hits are flat surface contact (no corners) neighbours
// and compares time to the global TGRSIOptions::AddBackWindow()
// some limited energy constraints are also added
//
// Clusters are then ordered by energy and summed
// All base information is taken from the highest energy hit

// Things to implement:
// -maybe dont use global TGRSIOptions::AddBackWindow() time
// -better energy restrictions could be placed on what is a pair to suppress crosstalk issues
// -instead of blindly summing entire clusters a decision could be made based on geometry of the pixels
// eg if an event claims to go through a row of 3 the middle one must have sufficient energy for an electron to have
// traversed
// this would obviously be less in the inner rings and basically 3 sectors in a row in the outer rings is completely
// absurd
// -the cluster energy sorting algorithm should probably be made more efficient
//
// On thinking about it we should probably have specific rules for clusters of 2 and 3 and if a cluster >3 either
// discard all events or use all as singles
//
// Possibly allow straight lines 3s only on the inner rings and then corner 3/4s further out straight lines
//
// Need to determine where the line is between throwing out bad reconstruction or saying it is just 2 very close singles

Int_t TSiLi::GetAddbackMultiplicity()
{
   // Automatically builds the addback hits using the addback_criterion (if the size of the addback_hits vector is zero)
   // and return the number of addback hits.
   short basehits = fSiLiHits.size();

   if(basehits == 0) {
      return 0;
   }

   // if the addback has been reset, clear the addback hits
   if(!(fSiLiBits.TestBit(kAddbackSet))) {
      fAddbackHits.clear();
   }

   if(fAddbackHits.empty()) {

      // Create a matrix of "pairs"
      std::vector<std::vector<bool>> pairs;
      for(int i = 0; i < basehits; i++) {
         pairs.emplace_back(basehits, false);
      }

      std::vector<unsigned>              clusters_id(basehits, 0);
      std::vector<std::vector<unsigned>> Clusters;

      // In this loop we check all hits for pairing
      for(int i = 0; i < basehits; i++) {
         if(fSiLiHits[i].GetEnergy() >
            10) { // We skip over any hits which have been calibrated to zero and some electronic noise

            if(clusters_id[i] < 1) { // If not yet paired start a new cluster
               std::vector<unsigned> newCluster(1, i);
               Clusters.push_back(newCluster);
               clusters_id[i] = Clusters.size();
            }
            unsigned clus_id = clusters_id[i];

            for(int j = i + 1; j < basehits; j++) {
               if(fSiLiHits[j].GetEnergy() > 10) {
                  // if a pair
                  if(fAddbackCriterion(GetSiLiHit(i), GetSiLiHit(j))) {
                     Clusters[clus_id - 1].push_back(j);
                     clusters_id[j] = clus_id;
                  }
               }
            }
         }
      }

      // Clusters are lists of hits that have been identified as coincident neighbours
      // Will be length 1 if no neighbours

      for(auto& Cluster : Clusters) {

         TSiLi::SortCluster(
            Cluster); // Energy sort the clusters also deletes any invalid hit numbers//Not an efficient function
         if(!Cluster.empty()) {
            uint s = fAddbackHits.size();
            // We have to add it and THEN do the SumHit because the push_back copies the charge but not the energy,
            // which is the bit we sum
            // This is desired behaviour of TGRSIDetectorHit for speed of sorts, but messy for the addback, which should
            // only be done "on the fly" not stored to TSiLi on disk
            fAddbackHits.emplace_back();
            for(unsigned int j : Cluster) {
               fAddbackHits[s].SumHit(GetSiLiHit(j));
            }
         }
      }

      fSiLiBits.SetBit(kAddbackSet, true);
   }

   return fAddbackHits.size();
}

bool TSiLi::fAddbackCriterion(TSiLiHit* one, TSiLiHit* two)
{

   double T;
   if(one->GetTimeFit() > 0 && two->GetTimeFit() > 0) {
      T = (one->GetTimeFit() - two->GetTimeFit()) * 16;
   } else {
      T = one->GetCfd() - two->GetCfd();
   }

   double e = one->GetEnergy() / two->GetEnergy();
   if(e > 0.1 && e < 10) { // very basic energy gate to suppress crosstalk noise issues
      if(std::abs(T) < (TGRSIOptions::AnalysisOptions()->AddbackWindow() * 10.0)) {
         int dring   = std::abs(one->GetRing() - two->GetRing());
         int dsector = std::abs(one->GetSector() - two->GetSector());
         if(dring == 1 && dsector == 0) {
            return true;
         }
         if(dring == 0 && (dsector == 1 || dsector == 11)) {
            return true;
         }
      }
   }

   // 	  TVector3 res = one.GetPosition() - two.GetPosition();
   //                         // GetTime is in ns;  AddbackWindow is in 10's of ns.
   //   return ((std::abs(one.GetTime() - two.GetTime()) < (TGRSIOptions::AddBackWindow()*10.0)) &&
   //       ((((one.GetInitialHit() < 5 && two.GetInitialHit() < 5) || (one.GetInitialHit() > 4 && two.GetInitialHit() >
   //       4)) && res.Mag() < 54) ||  //not front to back
   //        (((one.GetInitialHit() < 5 && two.GetInitialHit() > 4) || (one.GetInitialHit() > 4 && two.GetInitialHit() <
   //        5)) && res.Mag() < 105))); //    front to back

   return false;
}

// This function could really be replaced with some proper computing
void TSiLi::SortCluster(std::vector<unsigned>& cluster)
{
   unsigned cs = cluster.size();
   if(cs > 1) {
      std::vector<double> energy;
      double              Emax = 0;
      int                 Imax = -1;

      // Go through them all the referenced hits, get the energies and which one is the max
      for(unsigned i = 0; i < cs; i++) {
         if(cluster[i] < fSiLiHits.size()) {
            double e = fSiLiHits[cluster[i]].GetEnergy();
            energy.push_back(e);
            if(e > Emax) {
               Emax = e;
               Imax = cluster[i];
            }
         } else {
            energy.push_back(0);
         }
      }

      std::vector<unsigned> ordered;

      // Note if the cluster contained no valid hits Imax=-1 still and the loop will be skipped
      // cluster will effectively be deleted by the copy of blank order

      while(Imax >= 0) {
         ordered.push_back(Imax); // At start of loop because we want to do the previously determind max here
         if(cs == ordered.size()) {
            break;
         }
         Imax            = -1; // Will stay -1 unless there is a next highest
         double Emaxloop = 0;
         for(unsigned i = 0; i < cs; i++) {
            if(energy[i] > Emaxloop && energy[i] < Emax) {
               Imax     = cluster[i];
               Emaxloop = energy[i];
            };
         }
         Emax = Emaxloop;
      }

      cluster = ordered;
   }
   if(!(cluster[0] < fSiLiHits.size())) {
      cluster.clear();
   }
}

// Just a useful function for some dynamic tools
std::vector<TGraph> TSiLi::UpstreamShapes()
{
   std::vector<TGraph> ret;

   double inner_radius = fInnerDiameter / 2.0;
   double ring_width   = (fOuterDiameter - fInnerDiameter) * 0.5 / fRingNumber; // radial width!
   double phi_width    = 2. * TMath::Pi() / fSectorNumber;

   for(int ring = 0; ring < fRingNumber; ring++) {
      double r1 = inner_radius + ring_width * ring;
      double r2 = r1 + ring_width;

      for(int sector = 0; sector < fSectorNumber; sector++) {
         double phi = (phi_width * sector) + fOffsetPhi - (0.5 * phi_width);

         TGraph G;
         for(int i = 0; i <= 10; i++) {
            double x = r1 * TMath::Cos(phi + i * (phi_width / 10.0));
            double y = r1 * TMath::Sin(phi + i * (phi_width / 10.0));
            G.SetPoint(G.GetN(), x, y);
         }

         for(int i = 10; i >= 0; i--) {
            double x = r2 * TMath::Cos(phi + i * (phi_width / 10.0));
            double y = r2 * TMath::Sin(phi + i * (phi_width / 10.0));
            G.SetPoint(G.GetN(), x, y);
         }

         G.SetPoint(G.GetN(), (r1)*TMath::Cos(phi), (r1)*TMath::Sin(phi));

         ret.push_back(G);
      }
   }
   return ret;
}

// These numbers were determined for experiment of August 2015
// They may also be valid for 2016 experiments, however in March 2017 the FET board was changes and these numbers will
// probably be wrong

double TSiLi::SiLiBaseLine[120] = {
   -4311.29, -4362.37, -4341.99, -4377.17, -4343.83, -4386.03, 0,        -4333.27, -4398.38, -4373.85, -4202.92,
   -4398.66, -4389.33, -4302.26, -4382.04, -4385.26, -4367.75, -4352.33, -4377.33, -4377.38, -4405.83, -4356.3,
   -4277.03, -4313.27, -4324.32, -4402.7,  -4261.66, -4419.27, -4338.73, -4393.92, -4404.18, -4347.65, -4363.82,
   -4372.2,  -4231.8,  -4350.03, -4319.36, -4314.3,  -4407.88, -4348.94, -4372.84, -4311.44, -4356.27, -4410.76,
   -4379.52, -4355.61, -4358.77, -4305.91, -4357.78, -4358.44, -4290.46, -4392.02, -4355.19, -4422.52, -4376.23,
   0,        -4414.29, -4350.79, -4263.77, -4375.99, -4332.29, -4301.94, 0,        -4377.86, -4364,    -4322.66,
   -4403.58, -4341.73, -4326.31, -4423.18, -4315.89, -4272.18, -4374.14, -4407.33, -4351.37, -4358.02, -4321.03,
   -4374.95, -4392.78, -4355.59, -4403.77, -4357.15, -4205.98, -4370.55, -4341.03, -4325.2,  -4379,    0,
   -4375.47, -4323.71, -4399.69, -4375.85, -4334.29, -4353.87, -4396.54, -4328.34, -4382.13, -4355.79, -4319.37,
   -4397.35, -4297.35, -4429.82, -4348.61, -4321.81, -4411.24, -4358.86, -4269.85, -4378.82, -4366.06, -4323.59,
   -4428.3,  -4386.68, -4342.15, -4294.11, -4368.67, 0,        -4365.09, -4376.49, -4411.39, -4329.4};

double TSiLi::SiLiRiseTime[120] = {
   17.7006, 18.1526, 18.4143, 21.0633, 20.0552, 21.1558, 0,       21.1022, 20.8279, 18.9485, 18.1788, 18.4856,
   17.0705, 20.5214, 17.7818, 20.2889, 21.8823, 24.7537, 21.7925, 20.3923, 21.0786, 18.3124, 17.9945, 18.7833,
   17.5301, 17.6973, 21.5441, 20.8638, 20.3361, 21.3943, 21.0507, 20.8244, 20.5086, 19.78,   17.7576, 18.2345,
   18.0943, 17.0828, 18.4387, 20.763,  21.5234, 20.9951, 21.1253, 20.7005, 20.8394, 18.6957, 18.0242, 18.3229,
   17.9074, 18.0461, 18.8957, 17.5421, 20.603,  18.6829, 20.1689, 0,       19.8335, 18.2445, 17.9673, 18.4463,
   18.3801, 17.7211, 0,       20.8042, 21.5408, 20.9313, 20.8545, 20.5488, 21.1608, 18.2187, 17.1329, 18.3299,
   18.1181, 18.0984, 17.8779, 20.3292, 20.0205, 20.498,  20.3131, 20.5419, 20.0218, 17.7342, 18.7776, 18.172,
   18.431,  17.8529, 18.9184, 0,       20.9641, 19.675,  20.9102, 20.4364, 20.5039, 17.8223, 17.9544, 18.4206,
   17.5164, 17.8366, 17.4624, 19.3626, 20.086,  17.4838, 20.2189, 21.0822, 19.472,  16.9091, 18.3167, 17.5306,
   17.81,   18.3091, 17.5707, 19.7937, 18.448,  20.3555, 18.8905, 0,       19.8282, 17.0713, 17.8352, 17.7503};

double TSiLi::SiLiDecayTime[120] = {
   5760.51, 5627.93, 5300.97, 4743.98, 5188.47, 5156.87, 0,       4857.08, 5353.57, 5292.52, 5418.59, 5385.06,
   5131.66, 5590.67, 5403.61, 5543.33, 4762.24, 5314,    4950.1,  5101.39, 5262.8,  5500.31, 5121.3,  5202.93,
   5321.21, 5479.6,  3176.26, 4961.65, 5253,    4847.6,  5481.86, 5115.23, 5117.01, 3462.89, 5197.63, 5307.52,
   5482.34, 5706.96, 5050.59, 5178.03, 4627.79, 5063.34, 4835.78, 5186.56, 3345.64, 5119.7,  5051.44, 5635.93,
   5351.65, 5311.44, 5213.41, 5328.86, 5131.71, 4892.7,  4122.25, 0,       5454.63, 5277.99, 5312.54, 4939.98,
   5239.82, 5208.5,  4084.02, 5086.21, 4871.68, 4923.12, 5126.62, 5180.82, 3643.19, 5296.4,  4956.38, 5008.98,
   5616.46, 5506.85, 5446.75, 4947.32, 4866.33, 4786.01, 4679.56, 4900.77, 5055.94, 5300.21, 5206.59, 4967.62,
   5229.4,  5468.41, 5116.12, 0,       4542.86, 5602.42, 4878.95, 4977.12, 5237.7,  5220.87, 5003.49, 5333.6,
   5231.87, 5367.88, 5226.58, 4997.02, 5057.7,  5161.1,  5225.2,  5191.29, 5155.01, 5226.23, 5261.57, 5029.67,
   5568.03, 5606.43, 5286.39, 5283.09, 5104.04, 5125.97, 5352.33, 0,       5158.13, 5319.02, 5228.2,  5524.42};
