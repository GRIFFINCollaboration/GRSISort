#include "TTigress.h"

#include <iostream>

#include "TRandom.h"
#include "TMath.h"
#include "TClass.h"
#include "TInterpreter.h"

#include "TGRSIRunInfo.h"

/// \cond CLASSIMP
ClassImp(TTigress)
/// \endcond

bool TTigress::fSetSegmentHits = true;  // these are need to doppler shift properly.  
bool TTigress::fSetBGOHits = true;

bool TTigress::fSetCoreWave = false;
bool TTigress::fSetSegmentWave = false;
bool TTigress::fSetBGOWave = false;

bool DefaultAddback(TTigressHit& one, TTigressHit& two) {
  TVector3 res = one.GetPosition() - two.GetPosition();
                        // GetTime is in ns;  AddbackWindow is in 10's of ns.
  return ((std::abs(one.GetTime() - two.GetTime()) < (TGRSIRunInfo::AddBackWindow()*10.0)) &&
      ((((one.GetInitialHit() < 5 && two.GetInitialHit() < 5) || (one.GetInitialHit() > 4 && two.GetInitialHit() > 4)) && res.Mag() < 54) ||  //not front to back
       (((one.GetInitialHit() < 5 && two.GetInitialHit() > 4) || (one.GetInitialHit() > 4 && two.GetInitialHit() < 5)) && res.Mag() < 105))); //    front to back
}

std::function<bool(TTigressHit&, TTigressHit&)> TTigress::fAddbackCriterion = DefaultAddback;

TTigress::TTigress() : TGRSIDetector() {
  //Class()->IgnoreTObjectStreamer(true);
  Clear();
 
  SetBitNumber(TTigress::kSetCoreWave);
  SetBitNumber(TTigress::kSetSegWave,0);
  SetBitNumber(TTigress::kSetBGOWave,0);

}

TTigress::TTigress(const TTigress& rhs) : TGRSIDetector() {
  rhs.Copy(*this);
}

TTigress::~TTigress() {
}

void TTigress::Copy(TObject& rhs) const {
  TGRSIDetector::Copy(rhs);
  static_cast<TTigress&>(rhs).fTigressHits    = fTigressHits;
  static_cast<TTigress&>(rhs).fAddbackHits    = fAddbackHits;
  static_cast<TTigress&>(rhs).fAddbackFrags   = fAddbackFrags;
  static_cast<TTigress&>(rhs).fTigressBits    = 0;
  static_cast<TTigress&>(rhs).fSetSegmentHits = fSetSegmentHits;	
  static_cast<TTigress&>(rhs).fSetBGOHits     = fSetBGOHits;	
  static_cast<TTigress&>(rhs).fSetCoreWave    = fSetCoreWave;	
  static_cast<TTigress&>(rhs).fSetSegmentWave = fSetSegmentWave;	
  static_cast<TTigress&>(rhs).fSetBGOWave     = fSetBGOWave;	
}

void TTigress::Clear(Option_t *opt)	{
  //Clears the mother, and all of the hits
  if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
    ClearStatus();
  }
  TGRSIDetector::Clear(opt);
  fTigressHits.clear();
  fAddbackHits.clear();
  fAddbackFrags.clear();
}

void TTigress::Print(Option_t *opt)	const {
  printf("%lu tigress hits\n",fTigressHits.size());
  for(unsigned int i=0;i<GetMultiplicity();i++) 
     fTigressHits.at(i).Print(opt);

  return;
}

TTigress& TTigress::operator=(const TTigress& rhs) {
  rhs.Copy(*this);
  return *this;
}

void TTigress::PushBackHit(TGRSIDetectorHit *ghit){
  fTigressHits.push_back(*static_cast<TTigressHit*>(ghit));
}

TTigressHit* TTigress::GetTigressHit(const int& i) {
  try {
    return &(fTigressHits.at(i));
  } catch(const std::out_of_range& oor) {
    std::cerr<<ClassName()<<"Hits are out of range: "<<oor.what()<<std::endl;
    if(!gInterpreter)
      throw grsi::exit_exception(1);
  }
  return NULL;
}

Int_t TTigress::GetAddbackMultiplicity() {
  // Automatically builds the addback hits using the addback_criterion (if the size of the addback_hits vector is zero) and return the number of addback hits.
  if(fTigressHits.size() == 0) {
    return 0;
  }
  //if the addback has been reset, clear the addback hits
  if((fTigressBits & kIsAddbackSet) == 0x0) {
    fAddbackHits.clear();
  }
  if(fAddbackHits.size() == 0) {
    // use the first tigress hit as starting point for the addback hits
    fAddbackHits.push_back(fTigressHits[0]);
    fAddbackHits.back().SumHit(&(fAddbackHits.back()));//this sets the last position
    fAddbackFrags.push_back(1);

    // loop over remaining tigress hits
    size_t i, j;
    for(i = 1; i < fTigressHits.size(); ++i) {
      // check for each existing addback hit if this tigress hit should be added
      for(j = 0; j < fAddbackHits.size(); ++j) {
        if(fAddbackCriterion(fAddbackHits[j], fTigressHits[i])) {
          fAddbackHits[j].SumHit(&(fTigressHits[i]));
          fAddbackFrags[j]++;
          break;
        }
      }
      if(j == fAddbackHits.size()) {
        fAddbackHits.push_back(fTigressHits[i]);
        fAddbackHits.back().SumHit(&(fAddbackHits.back()));//this sets the last position
        fAddbackFrags.push_back(1);
      }
    }
    SetBitNumber(kIsAddbackSet, true);
  }

  return fAddbackHits.size();
}

TTigressHit* TTigress::GetAddbackHit(const int& i) {
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

void TTigress::BuildHits(){
	
	//for(size_t i = 0; i<GetMultiplicity(); i++){
	//	if(GetTigressHit(i)->GetCharge() <= 0.5)
	//		//DeleteTigressHit(i);
	//}
  //erasing elements of a vector in a loop is a bit tricker... pcb.
  std::vector<TTigressHit>::iterator it;
  for( it=fTigressHits.begin();it!=fTigressHits.end();) {
    double largestsegment = 0.0;
    if((it->GetCharge()/125.0)<5)  {
       if(it->GetNBGOs()>0 && it->GetNSegments()<1) {  //bgo fired with no core.
         it = fTigressHits.erase(it);
         continue;
       }
    }
    //it->Print("all");
    for(int y=0;y<it->GetNSegments();y++) {
      if(it->GetSegment(y).GetCharge() > largestsegment) {
        it->SetInitalHit(it->GetSegment(y).GetSegment());
        largestsegment = it->GetSegment(y).GetCharge();
      }
    }  
		//it->SetPosition(TTigress::GetPosition(it->GetDetector(),it->GetCrystal(),it->GetSegment()));
    if(it->HasWave() &&TGRSIRunInfo::IsWaveformFitting() ) // this should really be moved to the grsioptions...  pcb.
      it->SetWavefit();
    it++;
  }

}

void TTigress::AddFragment(TFragment* frag, MNEMONIC* mnemonic) {
  if(frag == NULL || mnemonic == NULL) {
    return;
  }

  TChannel *channel = TChannel::GetChannel(frag->GetAddress());

  //printf("%s %s called.\n",__PRETTY_FUNCTION__,channel->GetChannelName());
  //fflush(stdout);

  if((mnemonic->subsystem.compare(0,1,"G")==0) && (channel->GetSegmentNumber()==0 || channel->GetSegmentNumber()==9) ) { // it is a core
    //if(frag->Charge.size() == 0 || (frag->Cfd.size() == 0 && frag->Led.size() == 0))   // sanity check, it has a good energy and time (cfd or led).
    //  return;
    TTigressHit corehit; //(*frag);
    //loop over existing hits to see if this core was already created by a previously found segment
    //of course this means if we have a core in "coincidence" with itself we will overwrite the first hit
    for(size_t i = 0; i < fTigressHits.size(); ++i)	{
      TTigressHit *hit = GetTigressHit(i);
      if((hit->GetDetector() == channel->GetDetectorNumber()) && (hit->GetCrystal() == channel->GetCrystalNumber())) { //we have a match;
        //if(hit->Charge() == 0 || (frag->Cfd.size() == 0 && frag->Led.size() == 0))   // sanity check, it has a good energy and time (cfd or led).
        if(mnemonic->outputsensor.compare(0,1,"b")==0) {
          if(hit->GetName()[9] == 'a') {
            return;
          } else  {
            hit->CopyFragment(*frag);
            if(TestBitNumber(kSetCoreWave))
              hit->CopyWaveform(*frag);
            return;
          }
        } else {
          hit->CopyFragment(*frag);
          if(TestBitNumber(kSetCoreWave))
             hit->CopyWaveform(*frag);
          return;
        }
      }
    }
    corehit.CopyFragment(*frag);
    if(TestBitNumber(kSetCoreWave))
      corehit.CopyWaveform(*frag);
    fTigressHits.push_back(corehit);
    return;
  } else if(mnemonic->subsystem.compare(0,1,"G")==0) { // its ge but its not a core...
    if(!SetSegmentHits()) 
      return;
    TGRSIDetectorHit temp(*frag);
    for(size_t i = 0; i < fTigressHits.size(); ++i)	{
      TTigressHit *hit = GetTigressHit(i);
      if((hit->GetDetector() == channel->GetDetectorNumber()) && (hit->GetCrystal() == channel->GetCrystalNumber())) { //we have a match;
        if(TestBitNumber(kSetSegWave))         
          temp.CopyWaveform(*frag);
        hit->AddSegment(temp);
        //printf(" I found a core !\t%i\n",hit->GetNSegments()); fflush(stdout);
        return;
      }
    }
    TTigressHit corehit;
    //corehit.SetAddress( (frag->ChannelAddress&0xfffffff0) );  // fake it till you make it.
    corehit.SetAddress( (frag->ChannelAddress) );  // the above only works if tigress is it's 'normal' setup
    if(TestBitNumber(kSetSegWave))         
       temp.CopyWaveform(*frag);
    corehit.AddSegment(temp);
    fTigressHits.push_back(corehit);
    //if(fTigressHits.size()>100) {
    //   printf("size is large!\t%i\n",fTigressHits.size());
    //   fflush(stdout);
    //}
    return;
  } else if(SetBGOHits() && mnemonic->subsystem.compare(0,1,"S")==0) {
    TGRSIDetectorHit temp(*frag);
    for(size_t i = 0; i < fTigressHits.size(); ++i)	{
      TTigressHit *hit = GetTigressHit(i);
      if((hit->GetDetector() == channel->GetDetectorNumber()) && (hit->GetCrystal() == channel->GetCrystalNumber())) { //we have a match;
        if(TestBitNumber(kSetBGOWave))         
          temp.CopyWaveform(*frag);
        GetTigressHit(i)->AddBGO(temp);
        return;
      }
    }
    TTigressHit corehit;
    corehit.SetAddress(frag->ChannelAddress);  // this makes me uncomfortable, though I have no slick solution.
    if(TestBitNumber(kSetBGOWave))         
      temp.CopyWaveform(*frag);
    corehit.AddBGO(temp);
    fTigressHits.push_back(corehit);
    return;
  }
  //if not suprress errors;
  frag->Print();
  printf(ALERTTEXT "failed to build!" RESET_COLOR "\n");

  return;
}



void TTigress::ResetAddback() {
  ///Used to clear the addback hits. When playing back a tree, this must
  ///be called before building the new addback hits, otherwise, a copy of
  ///the old addback hits will be stored instead.
  ///This should have changed now, we're using the stored tigress bits to reset the addback
  SetBitNumber(kIsAddbackSet, false);
  fAddbackHits.clear();
  fAddbackFrags.clear();
}

UShort_t TTigress::GetNAddbackFrags(size_t idx) const{
  //Get the number of addback "fragments" contributing to the total addback hit
  //with index idx.
  if(idx < fAddbackFrags.size())
    return fAddbackFrags.at(idx);
  else
    return 0;
}

void TTigress::SetBitNumber(enum ETigressBits bit,Bool_t set){
  //Used to set the flags that are stored in TTigress.
  if(set)
    fTigressBits |= bit;
  else
    fTigressBits &= (~bit);
}

//void TTigress::DopplerCorrect(TTigressHit *hit)	{
//	if(beta != 0.00)	{
//		double gamma = 1/(sqrt(1-pow(beta,2)));
//		double tmp = hit->GetEnergy()*gamma *(1 - beta*hit->GetPosition().CosTheta());

//		hit->SetDoppler(tmp);
//	}
//	else {
//		printf(DRED "\n\tWARNING!  Try to Doppler correct before setting beta!" RESET_COLOR "\n");
//	}
//}




//TVector3 TTigress::GetPosition(TTigressHit *hit, int dist)  {
//			return TTigress::GetPosition(hit->GetDetectorNumber(),hit->GetCrystalNumber(),hit->GetInitialHit());	
//}

TVector3 TTigress::GetPosition(int DetNbr,int CryNbr,int SegNbr, double dist)	{
  TVector3 det_pos;
  double xx = 0;
  double yy = 0;
  double zz = 0;

  //printf("xx = %f\nyy = %f\n zz = %f\n",GeBluePosition[DetNbr][SegNbr][0],GeBluePosition[DetNbr][SegNbr][1],GeBluePosition[DetNbr][SegNbr][2]);

  switch(CryNbr)	{
    case -1:
      break;
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
  //printf("xx = %f\nyy = %f\n zz = %f\n",xx,yy,zz);
  det_pos.SetXYZ(xx,yy,zz);

  return det_pos;
}

double TTigress::GeBluePosition[17][9][3] = { 
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 139.75, 87.25, 27.13 }, { 107.47, 84.35, 36.80 }, { 107.64, 84.01, 12.83 }, { 116.86, 63.25, 13.71 }, { 116.66, 62.21, 36.42 }, { 146.69, 104.60, 40.50 }, { 146.58, 104.81, 14.96 }, { 156.50, 80.77, 14.73 }, { 156.44, 80.99, 40.74 } },
  { { 37.12, 160.51, 27.13 }, { 16.35, 135.64, 36.80 }, { 16.71, 135.51, 12.83 }, { 37.91, 127.36, 13.71 }, { 38.50, 126.48, 36.42 }, { 29.76, 177.69, 40.50 }, { 29.53, 177.76, 14.96 }, { 53.55, 167.78, 14.73 }, { 53.35, 167.89, 40.74 } },
  { {-87.25, 139.75, 27.13}, {-84.35, 107.47, 36.80}, {-84.01, 107.64, 12.83}, {-63.25, 116.86, 13.71}, {-62.21, 116.66, 36.42}, {-104.60, 146.69, 40.50}, {-104.81, 146.58, 14.96}, {-80.77, 156.50, 14.73}, {-80.99, 156.44, 40.74} },
  { {-160.51, 37.12, 27.13}, {-135.64, 16.35, 36.80}, {-135.51, 16.71, 12.83}, {-127.36, 37.91, 13.71}, {-126.48, 38.50, 36.42}, {-177.69, 29.76, 40.50}, {-177.76, 29.53, 14.96}, {-167.78, 53.55, 14.73}, {-167.89, 53.35, 40.74} },
  { {-139.75, -87.25, 27.13}, {-107.47, -84.35, 36.80}, {-107.64, -84.01, 12.83}, {-116.86, -63.25, 13.71}, {-116.66, -62.21, 36.42}, {-146.69, -104.60, 40.50}, {-146.58, -104.81, 14.96}, {-156.50, -80.77, 14.73}, {-156.44, -80.99, 40.74} },
  { {-37.12, -160.51, 27.13}, {-16.35, -135.64, 36.80}, {-16.71, -135.51, 12.83}, {-37.91, -127.36, 13.71}, {-38.50, -126.48, 36.42}, {-29.76, -177.69, 40.50}, {-29.53, -177.76, 14.96}, {-53.55, -167.78, 14.73}, {-53.35, -167.89, 40.74} },
  { {87.25, -139.75, 27.13}, {84.35, -107.47, 36.80}, {84.01, -107.64, 12.83}, {63.25, -116.86, 13.71}, {62.21, -116.66, 36.42}, {104.60, -146.69, 40.50}, {104.81, -146.58, 14.96}, {80.77, -156.50, 14.73}, {80.99, -156.44, 40.74} },
  { {160.51, -37.12, 27.13}, {135.64, -16.35, 36.80}, {135.51, -16.71, 12.83}, {127.36, -37.91, 13.71}, {126.48, -38.50, 36.42}, {177.69, -29.76, 40.50}, {177.76, -29.53, 14.96}, {167.78, -53.55, 14.73}, {167.89, -53.35, 40.74} },
  { {113.50, 76.38, -95.72}, {95.91, 79.56, -67.01}, {80.41, 72.73, -83.98}, {90.05, 52.14, -83.76}, {104.85, 57.32, -67.30}, {125.64, 95.88, -95.49}, {108.85, 89.19, -113.54}, {118.64, 65.08, -113.68}, {135.56, 72.34, -95.31} },
  { {-76.38, 113.5, -95.72}, {-79.56, 95.91, -67.01}, {-72.73, 80.41, -83.98}, {-52.14, 90.05, -83.76}, {-57.32, 104.85, -67.30}, {-95.88, 125.64, -95.49}, {-89.19, 108.85, -113.54}, {-65.08, 118.64, -113.68}, {-72.34, 135.56, -95.31} },
  { {-113.50, -76.38, -95.72}, {-95.91, -79.56, -67.01}, {-80.41, -72.73, -83.98}, {-90.05, -52.14, -83.76}, {-104.85, -57.32, -67.30}, {-125.64, -95.88, -95.49}, {-108.85, -89.19, -113.54}, {-118.64, -65.08, -113.68}, {-135.56, -72.34, -95.31} },
  { {76.38, -113.50, -95.72}, {79.56, -95.91, -67.01}, {72.73, -80.41, -83.98}, {52.14, -90.05, -83.76}, {57.32, -104.85, -67.30}, {95.88, -125.64, -95.49}, {89.19, -108.85, -113.54}, {65.08, -118.64, -113.68}, {72.34, -135.56, -95.31} }
};

//Assuming this is the 1
double TTigress::GeGreenPosition[17][9][3] = { 
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 139.75, 87.25, -27.13 }, { 107.47, 84.35, -36.80 }, { 116.66, 62.21, -36.42 }, { 116.86, 63.25, -13.71 }, { 107.64, 84.01, -12.83 }, { 146.69, 104.60, -40.50 }, { 156.44, 80.99, -40.74 }, { 156.50, 80.77, -14.73 }, { 146.58, 104.81, -14.96 } },
  { { 37.12, 160.51, -27.13 }, { 16.35, 135.64, -36.80 }, { 38.50, 126.48, -36.42 }, { 37.91, 127.36, -13.71 }, { 16.71, 135.51, -12.83 }, { 29.76, 177.69, -40.50 }, { 53.35, 167.89, -40.74 }, { 53.55, 167.78, -14.73 }, { 29.53, 177.76, -14.96 } },
  { {-87.25, 139.75, -27.13}, {-84.35, 107.47, -36.80}, {-62.21, 116.66, -36.42}, {-63.25, 116.86, -13.71}, {-84.01, 107.64, -12.83}, {-104.60, 146.69, -40.50}, {-80.99, 156.44, -40.74}, {-80.77, 156.50, -14.73}, {-104.81, 146.58, -14.96} },
  { {-160.51, 37.12, -27.13}, {-135.64, 16.35, -36.80}, {-126.48, 38.50, -36.42}, {-127.36, 37.91, -13.71}, {-135.51, 16.71, -12.83}, {-177.69, 29.76, -40.50}, {-167.89, 53.35, -40.74}, {-167.78, 53.55, -14.73}, {-177.76, 29.53, -14.96} },
  { {-139.75, -87.25, -27.13}, {-107.47, -84.35, -36.80}, {-116.66, -62.21, -36.42}, {-116.86, -63.25, -13.71}, {-107.64, -84.01, -12.83}, {-146.69, -104.60, -40.50}, {-156.44, -80.99, -40.74}, {-156.50, -80.77, -14.73}, {-146.58, -104.81, -14.96} },
  { {-37.12, -160.51, -27.13}, {-16.35, -135.64, -36.80}, {-38.50, -126.48, -36.42}, {-37.91, -127.36, -13.71}, {-16.71, -135.51, -12.83}, {-29.76, -177.69, -40.50}, {-53.35, -167.89, -40.74}, {-53.55, -167.78, -14.73}, {-29.53, -177.76, -14.96} },
  { {87.25, -139.75, -27.13}, {84.35, -107.47, -36.80}, {62.21, -116.66, -36.42}, {63.25, -116.86, -13.71}, {84.01, -107.64, -12.83}, {104.60, -146.69, -40.50}, {80.99, -156.44, -40.74}, {80.77, -156.50, -14.73}, {104.81, -146.58, -14.96} },
  { {160.51, -37.12, -27.13}, {135.64, -16.35, -36.80}, {126.48, -38.50, -36.42}, {127.36, -37.91, -13.71}, {135.51, -16.71, -12.83}, {177.69, -29.76, -40.50}, {167.89, -53.35, -40.74}, {167.78, -53.55, -14.73}, {177.76, -29.53, -14.96} },
  { {78.05, 61.70, -134.09}, {47.83, 59.64, -119.06}, {57.26, 37.61, -118.80}, {72.14, 44.72, -103.15}, {63.65, 65.78, -102.12}, {72.73, 73.96, -152.77}, {82.33, 50.30, -152.93}, {99.39, 57.11, -134.51}, {89.31, 81.09, -134.70} },
  { {-61.7, 78.05, -134.09}, {-59.64, 47.83, -119.06}, {-37.61, 57.26, -118.80}, {-44.72, 72.14, -103.15}, {-65.78, 63.65, -102.12}, {-73.96, 72.73, -152.77}, {-50.30, 82.33, -152.93}, {-57.11, 99.39, -134.51}, {-81.09, 89.31, -134.70} },
  { {-78.05, -61.7, -134.09}, {-47.83, -59.64, -119.06}, {-57.26, -37.61, -118.80}, {-72.14, -44.72, -103.15}, {-63.65, -65.78, -102.12}, {-72.73, -73.96, -152.77}, {-82.33, -50.30, -152.93}, {-99.39, -57.11, -134.51}, {-89.31, -81.09, -134.70} },
  { {61.7, -78.05, -134.09}, {59.64, -47.83, -119.06}, {37.61, -57.26, -118.80}, {44.72, -72.14, -103.15}, {65.78, -63.65, -102.12}, {73.96, -72.73, -152.77}, {50.30, -82.33, -152.93}, {57.11, -99.39, -134.51}, {81.09, -89.31, -134.70} }
};

//Assuming this is the 2
double TTigress::GeRedPosition[17][9][3] = { 
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 160.51, 37.12, -27.13 }, { 135.64, 16.35, -36.80 }, { 135.51, 16.71, -12.83 }, { 127.36, 37.91, -13.71 }, { 126.48, 38.50, -36.42 }, { 177.69, 29.76, -40.50 }, { 177.76, 29.53, -14.96 }, { 167.78, 53.55, -14.73 }, { 167.89, 53.35, -40.74 } },
  { { 87.25, 139.75, -27.13 }, { 84.35, 107.47, -36.80 }, { 84.01, 107.64, -12.83 }, { 63.25, 116.86, -13.71 }, { 62.21, 116.66, -36.42 }, { 104.60, 146.69, -40.50 }, { 104.81, 146.58, -14.96 }, { 80.77, 156.50, -14.73 }, { 80.99, 156.44, -40.74 } },
  { {-37.12, 160.51, -27.13}, {-16.35, 135.64, -36.80}, {-16.71, 135.51, -12.83}, {-37.91, 127.36, -13.71}, {-38.50, 126.48, -36.42}, {-29.76, 177.69, -40.50}, {-29.53, 177.76, -14.96}, {-53.55, 167.78, -14.73}, {-53.35, 167.89, -40.74} },
  { {-139.75, 87.25, -27.13}, {-107.47, 84.35, -36.80}, {-107.64, 84.01, -12.83}, {-116.86, 63.25, -13.71}, {-116.66, 62.21, -36.42}, {-146.69, 104.60, -40.50}, {-146.58, 104.81, -14.96}, {-156.50, 80.77, -14.73}, {-156.44, 80.99, -40.74} },
  { {-160.51, -37.12, -27.13}, {-135.64, -16.35, -36.80}, {-135.51, -16.71, -12.83}, {-127.36, -37.91, -13.71}, {-126.48, -38.50, -36.42}, {-177.69, -29.76, -40.50}, {-177.76, -29.53, -14.96}, {-167.78, -53.55, -14.73}, {-167.89, -53.35, -40.74} },
  { {-87.25, -139.75, -27.13}, {-84.35, -107.47, -36.80}, {-84.01, -107.64, -12.83}, {-63.25, -116.86, -13.71}, {-62.21, -116.66, -36.42}, {-104.60, -146.69, -40.50}, {-104.81, -146.58, -14.96}, {-80.77, -156.50, -14.73}, {-80.99, -156.44, -40.74} },
  { {37.12, -160.51, -27.13}, {16.35, -135.64, -36.80}, {16.71, -135.51, -12.83}, {37.91, -127.36, -13.71}, {38.50, -126.48, -36.42}, {29.76, -177.69, -40.50}, {29.53, -177.76, -14.96}, {53.55, -167.78, -14.73}, {53.35, -167.89, -40.74} },
  { {139.75, -87.25, -27.13}, {107.47, -84.35, -36.80}, {107.64, -84.01, -12.83}, {116.86, -63.25, -13.71}, {116.66, -62.21, -36.42}, {146.69, -104.60, -40.50}, {146.58, -104.81, -14.96}, {156.50, -80.77, -14.73}, {156.44, -80.99, -40.74} },
  { {98.82, 11.57, -134.09}, {75.99, -8.35, -119.06}, {91.52, -1.51, -102.12}, {82.63, 19.39, -103.15}, {67.08, 13.90, -118.80}, {103.72, -0.87, -152.77}, {120.49, 5.81, -134.70}, {110.66, 29.90, -134.51}, {93.78, 22.65, -152.93} },
  { {-11.57, 98.82, -134.09}, {8.35, 75.99, -119.06}, {1.51, 91.52, -102.12}, {-19.39, 82.63, -103.15}, {-13.90, 67.08, -118.80}, {0.87, 103.72, -152.77}, {-5.81, 120.49, -134.70}, {-29.90, 110.66, -134.51}, {-22.65, 93.78, -152.93} },
  { {-98.82, -11.57, -134.09}, {-75.99, 8.35, -119.06}, {-91.52, 1.51, -102.12}, {-82.63, -19.39, -103.15}, {-67.08, -13.90, -118.80}, {-103.72, 0.87, -152.77}, {-120.49, -5.81, -134.70}, {-110.66, -29.90, -134.51}, {-93.78, -22.65, -152.93} },
  { {11.57, -98.82, -134.09}, {-8.35, -75.99, -119.06}, {-1.51, -91.52, -102.12}, {19.39, -82.63, -103.15}, {13.90, -67.08, -118.80}, {-0.87, -103.72, -152.77}, {5.81, -120.49, -134.70}, {29.90, -110.66, -134.51}, {22.65, -93.78, -152.93} }
};

//Assuming this is the 3
double TTigress::GeWhitePosition[17][9][3] = { 
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { { 160.51, 37.12, 27.13 }, { 135.64, 16.35, 36.80 }, { 126.48, 38.50, 36.42 }, { 127.36, 37.91, 13.71 }, { 135.51, 16.71, 12.83 }, { 177.69, 29.76, 40.50 }, { 167.89, 53.35, 40.74 }, { 167.78, 53.55, 14.73 }, { 177.76, 29.53, 14.96 } },
  { { 87.25, 139.75, 27.13 }, { 84.35, 107.47, 36.80 }, { 62.21, 116.66, 36.42 }, { 63.25, 116.86, 13.71 }, { 84.01, 107.64, 12.83 }, { 104.60, 146.69, 40.50 }, { 80.99, 156.44, 40.74 }, { 80.77, 156.50, 14.73 }, { 104.81, 146.58, 14.96 } },
  { {-37.12, 160.51, 27.13}, {-16.35, 135.64, 36.80}, {-38.50, 126.48, 36.42}, {-37.91, 127.36, 13.71}, {-16.71, 135.51, 12.83}, {-29.76, 177.69, 40.50}, {-53.35, 167.89, 40.74}, {-53.55, 167.78, 14.73}, {-29.53, 177.76, 14.96} },
  { {-139.75, 87.25, 27.13}, {-107.47, 84.35, 36.80}, {-116.66, 62.21, 36.42}, {-116.86, 63.25, 13.71}, {-107.64, 84.01, 12.83}, {-146.69, 104.60, 40.50}, {-156.44, 80.99, 40.74}, {-156.50, 80.77, 14.73}, {-146.58, 104.81, 14.96} },
  { {-160.51, -37.12, 27.13}, {-135.64, -16.35, 36.80}, {-126.48, -38.50, 36.42}, {-127.36, -37.91, 13.71}, {-135.51, -16.71, 12.83}, {-177.69, -29.76, 40.50}, {-167.89, -53.35, 40.74}, {-167.78, -53.55, 14.73}, {-177.76, -29.53, 14.96} },
  { {-87.25, -139.75, 27.13}, {-84.35, -107.47, 36.80}, {-62.21, -116.66, 36.42}, {-63.25, -116.86, 13.71}, {-84.01, -107.64, 12.83}, {-104.60, -146.69, 40.50}, {-80.99, -156.44, 40.74}, {-80.77, -156.50, 14.73}, {-104.81, -146.58, 14.96} },
  { {37.12, -160.51, 27.13}, {16.35, -135.64, 36.80}, {38.50, -126.48, 36.42}, {37.91, -127.36, 13.71}, {16.71, -135.51, 12.83}, {29.76, -177.69, 40.50}, {53.35, -167.89, 40.74}, {53.55, -167.78, 14.73}, {29.53, -177.76, 14.96} },
  { {139.75, -87.25, 27.13}, {107.47, -84.35, 36.80}, {116.66, -62.21, 36.42}, {116.86, -63.25, 13.71}, {107.64, -84.01, 12.83}, {146.69, -104.60, 40.50}, {156.44, -80.99, 40.74}, {156.50, -80.77, 14.73}, {146.58, -104.81, 14.96} },
  { {134.26, 26.25, -95.72}, {124.08, 11.56, -67.01}, {114.67, 33.61, -67.30}, {100.55, 26.81, -83.76}, {108.28, 5.43, -83.98}, {156.64, 21.05, -95.49}, {147.01, 44.70, -95.31}, {129.91, 37.87, -113.68}, {140.03, 13.91, -113.54} },
  { {-26.25, 134.26, -95.72}, {-11.56, 124.08, -67.01}, {-33.61, 114.67, -67.30}, {-26.81, 100.55, -83.76}, {-5.43, 108.28, -83.98}, {-21.05, 156.64, -95.49}, {-44.70, 147.01, -95.31}, {-37.87, 129.91, -113.68}, {-13.91, 140.03, -113.54} },
  { {-134.26, -26.25, -95.72}, {-124.08, -11.56, -67.01}, {-114.67, -33.61, -67.30}, {-100.55, -26.81, -83.76}, {-108.28, -5.43, -83.98}, {-156.64, -21.05, -95.49}, {-147.01, -44.70, -95.31}, {-129.91, -37.87, -113.68}, {-140.03, -13.91, -113.54} },
  { {26.25, -134.26, -95.72}, {11.56, -124.08, -67.01}, {33.61, -114.67, -67.30}, {26.81, -100.55, -83.76}, {5.43, -108.28, -83.98}, {21.05, -156.64, -95.49}, {44.70, -147.01, -95.31}, {37.87, -129.91, -113.68}, {13.91, -140.03, -113.54} }
};









