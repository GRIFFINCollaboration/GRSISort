
#include "TSiLi.h"
#include <TGRSIRunInfo.h>

/// \cond CLASSIMP
ClassImp(TSiLi)
/// \endcond

int    TSiLi::fRingNumber;
int    TSiLi::fSectorNumber;

double TSiLi::fOffsetPhi;
double TSiLi::fOuterDiameter;
double TSiLi::fInnerDiameter;
double TSiLi::fTargetDistance;

TRandom2 TSiLi::sili_rand;
double TSiLi::sili_noise_fac;

TSiLi::TSiLi() {
   Clear();	
}

TSiLi::~TSiLi()  {
}

void TSiLi::Copy(TObject &rhs) const {
  TGRSIDetector::Copy(rhs);
  static_cast<TSiLi&>(rhs).fSiLiHits     = fSiLiHits;
  static_cast<TSiLi&>(rhs).fAddbackHits     = fAddbackHits;
  return;                                      
} 

TSiLi::TSiLi(const TSiLi& rhs) : TGRSIDetector() {
  rhs.Copy(*this);
} 

void TSiLi::Clear(Option_t *opt)  {
  fSiLiHits.clear();
  fAddbackHits.clear();
  fRingNumber=10;
  fSectorNumber=12;
  fOffsetPhi=-165.*TMath::Pi()/180.; // For SPICE, sectors upstream.
  fOuterDiameter=94.;
  fInnerDiameter=16.;
  fTargetDistance=-117.8;
  
  sili_rand.SetSeed();
  sili_noise_fac=4.;  
}

TSiLi& TSiLi::operator=(const TSiLi& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TSiLi::Print(Option_t *opt) const  {  
  printf("%lu sili_hits\n",fSiLiHits.size());
  printf("%lu sili_addback_hits\n",fAddbackHits.size());  
}

TGRSIDetectorHit* TSiLi::GetHit(const Int_t& idx){
   return GetSiLiHit(idx);
}

TSiLiHit * TSiLi::GetSiLiHit(const int& i)   {  
   try{
      return &fSiLiHits.at(i);   
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return 0;
}  

void TSiLi::AddFragment(TFragment* frag, TChannel* chan) {
  if(frag == NULL || chan == NULL) {
	 return;
  }

  TSiLiHit hit(*frag);//Waveform fitting happens in ctor now
  fSiLiHits.push_back(std::move(hit));
}

//For mapping you may want to us -position.X() to account for looking upstream
TVector3 TSiLi::GetPosition(int ring, int sector, bool smear)  {

  double dist = fTargetDistance;
	
  double ring_width=(fOuterDiameter-fInnerDiameter)*0.5/fRingNumber; // radial width!
  double inner_radius=fInnerDiameter/2.0;
  double phi_width=2.*TMath::Pi()/fSectorNumber;
  double phi    =  phi_width * sector;   //the phi angle....
  phi+=fOffsetPhi;
  double radius =  inner_radius + ring_width * (ring + 0.5) ;
  if(smear){	
	double sep=ring_width*0.025;
	double r1=radius-ring_width*0.5+sep,r2=radius+ring_width*0.5-sep;
	radius=sqrt(sili_rand.Uniform(r1*r1,r2*r2));
	double sepphi=sep/radius;
	phi=sili_rand.Uniform(phi-phi_width*0.5+sepphi,phi+phi_width*0.5-sepphi);	
  }
  
  return TVector3(cos(phi)*radius,sin(phi)*radius,dist);
}

double TSiLi::GetSegmentArea(Int_t seg){
	int r=GetRing(seg);
	double ring_width=(fOuterDiameter-fInnerDiameter)*0.5/fRingNumber; // radial width!
	double r1=fInnerDiameter+r*ring_width;
	double r2=fInnerDiameter+(r+1)*ring_width;
	
	return (TMath::Pi()*(r2*r2-r1*r1))/fSectorNumber;
}

TSiLiHit* TSiLi::GetAddbackHit(const int& i) {
  ///Get the ith addback hit. This function calls GetAddbackMultiplicity to check the range of the index.
  ///This automatically calculates all addback hits if they haven't been calculated before.
  if(i < GetAddbackMultiplicity()) {
    return &fAddbackHits.at(i);
  } else {
    std::cerr << "Addback hits are out of range" << std::endl;
    throw grsi::exit_exception(1);
    return NULL;
  }
}


// How the make addback currently works
//
// First we itterate through all pixel hits
// If the hit has not yet been assigned to a "cluster" a new cluster is created containing it
// The hit is compared all following hits against fAddbackCriterion
// If they are acceptable neighbours the second hit is also added to the cluster
//
// fAddbackCriterion checks if hits are flat surface contact (no corners) neighbours
// and compares time to the global TGRSIRunInfo::AddBackWindow()
// some limited energy constraints are also added
//
// Clusters are then ordered by energy and summed
// All base information is taken from the highest energy hit

// Things to implement:
// -maybe dont use global TGRSIRunInfo::AddBackWindow() time
// -better energy restrictions could be placed on what is a pair to suppress crosstalk issues
// -instead of blindly summing entire clusters a decision could be made based on geometry of the pixels
// eg if an event claims to go through a row of 3 the middle one must have sufficient energy for an electron to have traversed
// this would obviously be less in the inner rings and basically 3 sectors in a row in the outer rings is completely absurd 
// -the cluster energy sorting algorithm should probably be made more efficient 
// 
// On thinking about it we should probably have specific rules for clusters of 2 and 3 and if a cluster >3 either discard all events or use all as singles
//
// Possibly allow straight lines 3s only on the inner rings and then corner 3/4s further out straight lines
//
// Need to determine where the line is between throwing out bad reconstruction or saying it is just 2 very close singles 

Int_t TSiLi::GetAddbackMultiplicity() {
  // Automatically builds the addback hits using the addback_criterion (if the size of the addback_hits vector is zero) and return the number of addback hits.
  short basehits=fSiLiHits.size();
	
  if(basehits == 0) {
    return 0;
  }

  //if the addback has been reset, clear the addback hits
  if(TestBit(kAddbackSet) == 0x0) {
    fAddbackHits.clear();
  }

  if(fAddbackHits.size() == 0) {
	  
	// Create a matrix of "pairs"
	std::vector < std::vector<bool> > pairs; 
	for(int i=0;i<basehits;i++)pairs.push_back(std::vector<bool>(basehits,false));

	std::vector<unsigned> clusters_id(basehits,0);
	std::vector < std::vector<unsigned> > Clusters; 

	
	//In this loop we check all hits for pairing
	for(int i=0;i<basehits;i++){
		if(fSiLiHits[i].GetEnergy()>10){//We skip over any hits which have been calibrated to zero and some electronic noise
			
			if(clusters_id[i]<1){// If not yet paired start a new cluster
				std::vector<unsigned> newCluster(1,i);
				Clusters.push_back(newCluster);	
				clusters_id[i]=Clusters.size();
			}
			unsigned clus_id=clusters_id[i];

			for(int j=i+1;j<basehits;j++){
				if(fSiLiHits[j].GetEnergy()>10){
					//if a pair
					if(fAddbackCriterion(GetSiLiHit(i),GetSiLiHit(j))){
						Clusters[clus_id-1].push_back(j);
						clusters_id[j]=clus_id;
					}
				}
			}
		}
	}
	
	//Clusters are lists of hits that have been identified as coincident neighbours
	//Will be length 1 if no neighbours	
	
	for(unsigned i=0;i<Clusters.size();i++){
	
 		TSiLi::SortCluster(Clusters[i]);//Energy sort the clusters also deletes any invalid hit numbers//Not an efficient function
		if(Clusters[i].size()>0){
			uint s=fAddbackHits.size();
			//We have to add it and THEN do the SumHit because the push_back copies the charge but not the energy, which is the bit we sum
			//This is desired behaviour of TGRSIDetectorHit for speed of sorts, but messy for the addback, which should only be done "on the fly" not stored to TSiLi on disk
			fAddbackHits.push_back(TSiLiHit());
			for(unsigned j=0;j<Clusters[i].size();j++){
				fAddbackHits[s].SumHit(GetSiLiHit(Clusters[i][j]));
			}
		}
	}
	
	
     SetBit(kAddbackSet, true);
  }

  return fAddbackHits.size();
}




bool TSiLi::fAddbackCriterion(TSiLiHit* one, TSiLiHit* two){
	
	double T;
	if(one->GetTimeFit()>0 && two->GetTimeFit()>0){
		T=(one->GetTimeFit()-two->GetTimeFit())*16;
	} else{
		T=one->GetCfd()-two->GetCfd();		
	}

	double e=one->GetEnergy()/two->GetEnergy();
	if(e>0.1&&e<10){//very basic energy gate to suppress crosstalk noise issues
		if(std::abs(T)<(TGRSIRunInfo::AddBackWindow()*10.0)){
			int dring = std::abs(one->GetRing()-two->GetRing());
			int dsector = std::abs(one->GetSector()-two->GetSector());
			if(dring==1 && dsector==0) return true;
			if(dring==0 && (dsector==1||dsector==11)) return true;
		}
	}	

// 	  TVector3 res = one.GetPosition() - two.GetPosition();
//                         // GetTime is in ns;  AddbackWindow is in 10's of ns.
//   return ((std::abs(one.GetTime() - two.GetTime()) < (TGRSIRunInfo::AddBackWindow()*10.0)) &&
//       ((((one.GetInitialHit() < 5 && two.GetInitialHit() < 5) || (one.GetInitialHit() > 4 && two.GetInitialHit() > 4)) && res.Mag() < 54) ||  //not front to back
//        (((one.GetInitialHit() < 5 && two.GetInitialHit() > 4) || (one.GetInitialHit() > 4 && two.GetInitialHit() < 5)) && res.Mag() < 105))); //    front to back

	return false;
}



//This function could really be replaced with some proper computing
void TSiLi::SortCluster(std::vector<unsigned>& cluster){
	unsigned cs=cluster.size();
	if(cs>1){
		std::vector<double> energy;
		double Emax=0;
		int Imax=-1;
		
		//Go through them all the referenced hits, get the energies and which one is the max
		for(unsigned i=0;i<cs;i++){
			if(cluster[i]<fSiLiHits.size()){
				double e=fSiLiHits[cluster[i]].GetEnergy();
				energy.push_back(e);
				if(e>Emax){Emax=e;Imax=cluster[i];}
			}else{energy.push_back(0);}
		}
		
		std::vector<unsigned> ordered;
		
		//Note if the cluster contained no valid hits Imax=-1 still and the loop will be skipped
		//cluster will effectively be deleted by the copy of blank order
		
		while(Imax>=0){
			ordered.push_back(Imax);//At start of loop because we want to do the previously determind max here
			if(cs==ordered.size())break;
			Imax=-1;//Will stay -1 unless there is a next highest
			double Emaxloop=0;
			for(unsigned i=0;i<cs;i++){
				if(energy[i]>Emaxloop&&energy[i]<Emax){Imax=cluster[i];Emaxloop=energy[i];};
			}
			Emax=Emaxloop;
		}
		
		cluster=ordered;
	}
	if(!(cluster[0]<fSiLiHits.size()))cluster.clear();
}


//Just a useful function for some dynamic tools
std::vector< TGraph > TSiLi::UpstreamShapes()  {
	std::vector< TGraph > ret;
	
	double inner_radius=fInnerDiameter/2.0;
	double ring_width=(fOuterDiameter-fInnerDiameter)*0.5/fRingNumber; //radial width!
	double phi_width=2.*TMath::Pi()/fSectorNumber;
  	
	for(int ring=0;ring<fRingNumber;ring++){
		double r1=inner_radius + ring_width * ring;
		double r2=r1+ring_width;
		
		for(int sector=0;sector<fSectorNumber;sector++){
			double phi    =  (phi_width*sector) +fOffsetPhi -(0.5*phi_width);
			
			TGraph G;
			for(int i=0;i<=10;i++){
				double x=r1*TMath::Cos(phi+i*(phi_width/10.0));
				double y=r1*TMath::Sin(phi+i*(phi_width/10.0));
				G.SetPoint(G.GetN(),x,y);
			}
			
			for(int i=10;i>=0;i--){
				double x=r2*TMath::Cos(phi+i*(phi_width/10.0));
				double y=r2*TMath::Sin(phi+i*(phi_width/10.0));
				G.SetPoint(G.GetN(),x,y);
			}
			
			G.SetPoint(G.GetN(),(r1)*TMath::Cos(phi),(r1)*TMath::Sin(phi));			
			
			ret.push_back(G);	
		}
	}
	return ret;
}
