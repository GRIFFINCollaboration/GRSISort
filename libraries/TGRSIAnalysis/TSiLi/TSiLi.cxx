
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
double TSiLi::BaseFreq=4;

double  TSiLi::fSiLiCoincidenceTime = 200;
bool  TSiLi::fRejectPossibleCrosstalk = false;

std::string TSiLi::fPreAmpName[8]={"RG","RB","LG","LB","LW","LR","RW","RR"};

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

TSiLiHit* TSiLi::GetRejectHit(const int& i)
{
   if(i < GetRejectMultiplicity()) {
      return GetSiLiHit(fRejectHits[i]);
   }
   std::cerr<<"Reject hits are out of range"<<std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

Int_t TSiLi::GetRejectMultiplicity()
{
	GetAddbackMultiplicity();
	return fRejectHits.size();
}


// Currently Addback
//
// Itterate through all pixel hits
// If the hit has not yet been assigned to a "cluster" a new cluster is created containing it.
// The hit is compared all subsequent hits against fAddbackCriterion
// If they are accepted neighbours the second hit is also added to the cluster
//
// fAddbackCriterion checks if hits are within 2 pixels and compares times and energy
//
// Clusters >1 hit are accepted or rejected on a few criteria:
// >3 hits all rejected
// Clusters of 2 or 3 which are discontinuous (missing middle pixel) are rejected 
// Clusters spanning 3 sectors are rejected as impossible
// Clusters spanning 3 rings and a single sector are accepted IF energy of middle segment is large enough
// 
// Additionally clusters can be rejected if any of constituent hit is tagged as potential preamp-noise-crosstalk.
// This rules out many 3 hit events.
//
// Most of these rules favour clean spectra rather than efficiency, as currently noise dominates and addback events are not usable.


Int_t TSiLi::GetAddbackMultiplicity()
{
   // Automatically builds the addback hits using the addback_criterion (if the size of the addback_hits vector is zero)
   // and return the number of addback hits.
   short basehits = fSiLiHits.size();
   
   // if the addback has been reset, clear the addback hits
   if(fSiLiBits.TestBit(ESiLiBits::kAddbackSet)) {
	return fAddbackHits.size();
   }
   fAddbackHits.clear();
   fRejectHits.clear();
   
   if(basehits == 0) {
      fSiLiBits.SetBit(ESiLiBits::kAddbackSet, true);
      return 0;
   }
   
   //Do the addback if it hasnt been done 
   if(fAddbackHits.empty()&&fRejectHits.empty()) { 

	//Check if any of the initial hits could be preamp noise induction
	std::vector<bool> fPreampRejectedHit(basehits,false);
	for(int i = 0; i < basehits; i++) {
		for(int j = i + 1; j < basehits; j++) {   
			if(fRejectCriterion(GetSiLiHit(i),GetSiLiHit(j))){
				fPreampRejectedHit[i]=true;
				fPreampRejectedHit[j]=true;
			}
		}
	}

      std::vector<unsigned> clusters_id(basehits, 0);
      std::vector<std::vector<unsigned>> Clusters;
      std::vector<bool> hasreject;

      // In this loop we check all hits for pairing
      for(int i = 0; i < basehits; i++) {
	      
            if(clusters_id[i] < 1) { // If not yet paired start a new cluster
               std::vector<unsigned> newCluster(1, i);
               Clusters.push_back(newCluster);
               clusters_id[i] = Clusters.size();
	       hasreject.push_back(fPreampRejectedHit[i]);
            }
            unsigned clus_id = clusters_id[i];

            for(int j = i + 1; j < basehits; j++) {
                  if(fAddbackCriterion(GetSiLiHit(i), GetSiLiHit(j))) {
                     Clusters[clus_id - 1].push_back(j);
		     if(fPreampRejectedHit[j])hasreject[clus_id - 1]=true;
                     clusters_id[j] = clus_id;
                  }
            }
      }

      // Clusters are lists of hit index that have been identified as coincident near neighbours
      // Will be length 1 if no neighbours.
      // AddCluster applies additional rules
      for(unsigned int i=0;i<Clusters.size();i++) {
         TSiLi::AddCluster(Clusters[i],hasreject[i]); 
      }

      fSiLiBits.SetBit(ESiLiBits::kAddbackSet, true);
   }

   return fAddbackHits.size();
}

bool TSiLi::fAddbackCriterion(TSiLiHit* one, TSiLiHit* two)
{
   double e = one->GetEnergy() / two->GetEnergy();
   if(fCoincidenceTime(one,two)){
      if(e > 0.05 && e < 50) { // very basic energy gate to suppress noise issues
         int dring   = std::abs(one->GetRing() - two->GetRing());
         int dsector = std::abs(one->GetSector() - two->GetSector());
         if(dsector > 5)dsector =12-dsector;
	 
	 //if(dsector+dring==1)return true;
         //Changed to enable handing a missing middle pixel
	 if(dsector+dring<3)return true;
      }
   }

   return false;
}

// Intentionally no energy check as one of the two channels may have been calibrated to zero
// but we still want to know when they are coincident for this check.
bool TSiLi::fRejectCriterion(TSiLiHit* one, TSiLiHit* two)
{
   if(fCoincidenceTime(one,two)){
	if(one->GetPreamp()==two->GetPreamp()){
		if(std::abs(one->GetPin()-two->GetPin())<2){
			return true;
		}
	}
   }
   return false;
}


bool TSiLi::fCoincidenceTime(TSiLiHit* one, TSiLiHit* two)
{
   double T;
   if(one->GetTimeFit() > 0 && two->GetTimeFit() > 0) {
      T = (one->GetTimeFit() - two->GetTimeFit()) * 10;
   } else {
      T = one->GetTime() - two->GetTime();
   }

   return (std::abs(T) < fSiLiCoincidenceTime);
}


void TSiLi::AddCluster(std::vector<unsigned>& cluster,bool ContainsReject)
{
	if(cluster.empty())return;
	if(!fRejectPossibleCrosstalk)ContainsReject=false;
		
	if(cluster.size()>3){
		ContainsReject=true;
	}
	
	if(cluster.size()>1&&!ContainsReject){
		TSiLiHit* A=&fSiLiHits.at(cluster[0]);
		TSiLiHit* B=&fSiLiHits.at(cluster[1]);
		int rA=A->GetRing(),rB=B->GetRing();
		int sA=A->GetSector(),sB=B->GetSector();
		int rAB=std::abs(rA-rB);
		int sAB=std::abs(sA-sB);
		if(sAB>5)sAB=12-sAB;

		if(cluster.size()==2){
			//Reject events with missing middle pixel
			if(rAB+sAB>1)ContainsReject=true;
		}else{
			TSiLiHit* C=&fSiLiHits.at(cluster[2]);
			int rC=C->GetRing();
			int sC=C->GetSector();
			int rAC=std::abs(rA-rC),rBC=std::abs(rB-rC);
			int sAC=std::abs(sA-sC),sBC=std::abs(sB-sC);
			
			if(sAC>5)sAC=12-sAC;if(sBC>5)sBC=12-sBC;
			
			if(rA==rB&&rB==rC){
				//Reject events that cross 3 sectors
				ContainsReject=true;
			}else if(rAB+sAB>2||rAC+sAC>2||rBC+sBC>2){
				//Reject events that are too far apart (1 or more missing middles)
				ContainsReject=true;
			}else{
				//Must be an allowed L OR 3 rings same sector
				if(sA==sB&&sB==sC){
					//if event crosses 3 rings
					double midE;
					if(((rA-rB)*(rA-rC))<0){
						midE=A->GetEnergy();
					}else if(((rB-rA)*(rB-rC))<0){
						midE=B->GetEnergy();
					}else{
						midE=C->GetEnergy();
					}
					
					//Minimum possible energy
					if(midE<1400)ContainsReject=true;
				}
			}
		}
	}
	
	
	if(ContainsReject){
            for(unsigned int j : cluster) {
		fRejectHits.push_back(j);
	    }
	}else{
            uint s = fAddbackHits.size();
            // We have to add it and THEN do the SumHit because the push_back copies the charge but not the energy,
            // which is the bit we sum
            // This is desired behaviour of TGRSIDetectorHit for speed of sorts, but messy for the addback, which should
            // only be done "on the fly" not stored to TSiLi on disk
            fAddbackHits.emplace_back();
            for(unsigned int j : cluster) {
               fAddbackHits[s].SumHit(GetSiLiHit(j));       
            }
	}
	// Note: I got rid of ordering the cluster first.
	// There is really no way of knowing which is first for a double hits.
	// For a triple hit there are 2 possible & opposite orders, it would probably be best to take the middle. But why bother?
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

