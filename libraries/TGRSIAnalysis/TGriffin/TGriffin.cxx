#include "TGriffin.h"

#include <iostream>

#include "TRandom.h"
#include "TMath.h"
#include "TInterpreter.h"
#include "TMnemonic.h"

#include "TGRSIRunInfo.h"

////////////////////////////////////////////////////////////
//                    
// TGriffin
//
// The TGriffin class defines the observables and algorithms used
// when analyzing GRIFFIN data. It includes detector positions,
// add-back methods, etc. 
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TGriffin)
  /// \endcond

bool DefaultAddback(TGriffinHit& one, TGriffinHit& two) {
  return ((one.GetDetector() == two.GetDetector()) &&
	  (std::fabs(one.GetTime() - two.GetTime()) < TGRSIRunInfo::AddBackWindow()));
}

std::function<bool(TGriffinHit&, TGriffinHit&)> TGriffin::fAddbackCriterion = DefaultAddback;

bool TGriffin::fSetCoreWave = false;

//This seems unnecessary, and why 17?;//  they are static members, and need
//  to be defined outside the header
//  17 is to have the detectors go from 1-16 
//  plus we can use position zero 
//  when the detector winds up back in 
//  one of the stands like Alex used in the
//  gps run. pcb.
//Initiallizes the HPGe Clover positions as per the wiki <https://www.triumf.info/wiki/tigwiki/index.php/HPGe_Coordinate_Table>
//                                                                             theta                                 phi                                 theta                                phi                                 theta
TVector3 TGriffin::gCloverPosition[17] = { 
  TVector3(TMath::Sin(TMath::DegToRad()*(0.0))*TMath::Cos(TMath::DegToRad()*(0.0)), TMath::Sin(TMath::DegToRad()*(0.0))*TMath::Sin(TMath::DegToRad()*(0.0)), TMath::Cos(TMath::DegToRad()*(0.0))),
  //Downstream lampshade
  TVector3(TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Cos(TMath::DegToRad()*(67.5)), TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Sin(TMath::DegToRad()*(67.5)), TMath::Cos(TMath::DegToRad()*(45.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Cos(TMath::DegToRad()*(157.5)), TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Sin(TMath::DegToRad()*(157.5)), TMath::Cos(TMath::DegToRad()*(45.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Cos(TMath::DegToRad()*(247.5)), TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Sin(TMath::DegToRad()*(247.5)), TMath::Cos(TMath::DegToRad()*(45.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Cos(TMath::DegToRad()*(337.5)), TMath::Sin(TMath::DegToRad()*(45.0))*TMath::Sin(TMath::DegToRad()*(337.5)), TMath::Cos(TMath::DegToRad()*(45.0))),
  //Corona
  TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(22.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(22.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(67.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(67.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(112.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(112.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(157.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(157.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(202.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(202.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(247.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(247.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(292.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(292.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Cos(TMath::DegToRad()*(337.5)), TMath::Sin(TMath::DegToRad()*(90.0))*TMath::Sin(TMath::DegToRad()*(337.5)), TMath::Cos(TMath::DegToRad()*(90.0))),
  //Upstream lampshade
  TVector3(TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Cos(TMath::DegToRad()*(67.5)), TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Sin(TMath::DegToRad()*(67.5)), TMath::Cos(TMath::DegToRad()*(135.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Cos(TMath::DegToRad()*(157.5)), TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Sin(TMath::DegToRad()*(157.5)), TMath::Cos(TMath::DegToRad()*(135.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Cos(TMath::DegToRad()*(247.5)), TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Sin(TMath::DegToRad()*(247.5)), TMath::Cos(TMath::DegToRad()*(135.0))),
  TVector3(TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Cos(TMath::DegToRad()*(337.5)), TMath::Sin(TMath::DegToRad()*(135.0))*TMath::Sin(TMath::DegToRad()*(337.5)), TMath::Cos(TMath::DegToRad()*(135.0)))
};

//Cross Talk stuff
const Double_t TGriffin::gStrongCT[2] = { -0.02674, -0.000977 }; //This is for the 0-1 and 2-3 combination
const Double_t TGriffin::gWeakCT[2]   = { 0.005663, - 0.00028014};
const Double_t TGriffin::gCrossTalkPar[2][4][4] = {
   { {0.0, gStrongCT[0], gWeakCT[0], gWeakCT[0]}, {gStrongCT[0], 0.0, gWeakCT[0], gWeakCT[0]}, {gWeakCT[0], gWeakCT[0], 0.0, gStrongCT[0]}, {gWeakCT[0], gWeakCT[0], gStrongCT[0], 0.0}},
   { {0.0, gStrongCT[1], gWeakCT[1], gWeakCT[1]}, {gStrongCT[1], 0.0, gWeakCT[1], gWeakCT[1]}, {gWeakCT[1], gWeakCT[1], 0.0, gStrongCT[1]}, {gWeakCT[1], gWeakCT[1], gStrongCT[1], 0.0}}};

TGriffin::TGriffin() : TGRSIDetector() {
  //Default ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
  Class()->IgnoreTObjectStreamer(kTRUE);
#endif
  Clear();
}

TGriffin::TGriffin(const TGriffin& rhs) : TGRSIDetector() {
  //Copy ctor. Ignores TObjectStreamer in ROOT < 6
#if MAJOR_ROOT_VERSION < 6
  Class()->IgnoreTObjectStreamer(kTRUE);
#endif
  rhs.Copy(*this);
}

void TGriffin::Copy(TObject &rhs) const {
  //Copy function.
  TGRSIDetector::Copy(rhs);

  static_cast<TGriffin&>(rhs).fGriffinHits       = fGriffinHits;
  static_cast<TGriffin&>(rhs).fAddbackHits       = fAddbackHits;
  static_cast<TGriffin&>(rhs).fAddbackFrags      = fAddbackFrags;
  static_cast<TGriffin&>(rhs).fSetCoreWave       = fSetCoreWave;
  static_cast<TGriffin&>(rhs).fCycleStart        = fCycleStart;
  static_cast<TGriffin&>(rhs).fGriffinBits       = 0;

}                                       

TGriffin::~TGriffin()	{
  //Default Destructor
}

void TGriffin::Clear(Option_t *opt)	{
  //Clears the mother, and all of the hits
  //  if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
  ClearStatus();
  //  }
  TGRSIDetector::Clear(opt);
  fGriffinHits.clear();
  fAddbackHits.clear();
  fAddbackFrags.clear();
  fCycleStart = 0;
  //fGriffinBits.Class()->IgnoreTObjectStreamer(kTRUE);
}


void TGriffin::Print(Option_t *opt) const {
  //Prints out TGriffin members, currently does nothing.
  printf("%lu fGriffinHits\n",fGriffinHits.size());
  printf("%ld cycle start\n",fCycleStart);
  return;
}

TGriffin& TGriffin::operator=(const TGriffin& rhs) {
  rhs.Copy(*this);
  return *this;
}


TGRSIDetectorHit* TGriffin::GetHit(const Int_t& idx) {
  return GetGriffinHit(idx);
}

TGriffinHit* TGriffin::GetGriffinHit(const int& i) {
  try {
    if(!IsCrossTalkSet()){
      FixCrossTalk();
    }
    return &(fGriffinHits.at(i));
  } catch (const std::out_of_range& oor) {
    std::cerr << ClassName() << " Hits are out of range: " << oor.what() << std::endl;
    if(!gInterpreter)
      throw grsi::exit_exception(1);
  }
  return NULL;
}

Int_t TGriffin::GetAddbackMultiplicity() {
   // Automatically builds the addback hits using the fAddbackCriterion (if the size of the fAddbackHits vector is zero) and return the number of addback hits.
   if(!IsCrossTalkSet()){
      //Calculate Cross Talk on each hit
      FixCrossTalk();
   }
   if(fGriffinHits.size() == 0) {
      return 0;
   }
   //if the addback has been reset, clear the addback hits
   if((fGriffinBits & kIsAddbackSet) == 0x0) {
      fAddbackHits.clear();
   }
   if(fAddbackHits.size() == 0) {
      // use the first griffin hit as starting point for the addback hits
      fAddbackHits.push_back(fGriffinHits[0]);
      fAddbackFrags.push_back(1);

      // loop over remaining griffin hits
      size_t i, j;
      for(i = 1; i < fGriffinHits.size(); ++i) {
         // check for each existing addback hit if this griffin hit should be added
         for(j = 0; j < fAddbackHits.size(); ++j) {
            if(fAddbackCriterion(fAddbackHits[j], fGriffinHits[i])) {
               fAddbackHits[j].Add(&(fGriffinHits[i]));
               fAddbackFrags[j]++;
               break;
            }
         }
         if(j == fAddbackHits.size()) {
            fAddbackHits.push_back(fGriffinHits[i]);
            fAddbackFrags.push_back(1);
         }
      }
      SetBitNumber(kIsAddbackSet, true);
   }


   return fAddbackHits.size();
}

TGriffinHit* TGriffin::GetAddbackHit(const int& i) {
   if(i < GetAddbackMultiplicity()) {
      return &fAddbackHits.at(i);
   } else {
      std::cerr << "Addback hits are out of range" << std::endl;
      throw grsi::exit_exception(1);
      return NULL;
   }
}

void TGriffin::AddFragment(std::shared_ptr<TFragment> frag, TChannel* chan) {
   //Builds the GRIFFIN Hits directly from the TFragment. Basically, loops through the hits for an event and sets observables. 
   //This is done for both GRIFFIN and it's suppressors.
   if(frag == NULL || chan == NULL) {
      return;
   }
   //TODO: Fix kA to kB
   if(chan->GetMnemonic()->OutputSensor() == TMnemonic::kA) { return; }  //make this smarter.

   switch(chan->GetMnemonic()->SubSystem()){
      case TMnemonic::kG :
         TGriffinHit geHit(*frag);
         fGriffinHits.push_back(std::move(geHit));
         break;
         //     case TMnemonic::kS :
         //do supressor stuff
         //      break;
   };
   //  if(chan->GetMnemonic()->SubSystem() == TMnemonic::kG ) {
   //set griffin
   //    if(chan->GetMnemonic()->outputsensor[0] == 'B') { return; }  //make this smarter.
}

TVector3 TGriffin::GetPosition(int DetNbr,int CryNbr, double dist ) {
   //Gets the position vector for a crystal specified by CryNbr within Clover DetNbr at a distance of dist mm away.
   //This is calculated to the most likely interaction point within the crystal.
   if(DetNbr>16)
      return TVector3(0,0,1);

   TVector3 temp_pos(gCloverPosition[DetNbr]);

   //Interaction points may eventually be set externally. May make these members of each crystal, or pass from waveforms.
   Double_t cp = 26.0; //Crystal Center Point  mm.
   Double_t id = 45.0;//45.0;  //Crystal interaction depth mm.
   //Set Theta's of the center of each DETECTOR face
   ////Define one Detector position
   TVector3 shift;
   switch(CryNbr) {
      case 0:
         shift.SetXYZ(-cp,cp,id);
         break;
      case 1:
         shift.SetXYZ(cp,cp,id);
         break;
      case 2:
         shift.SetXYZ(cp,-cp,id);
         break;
      case 3:
         shift.SetXYZ(-cp,-cp,id);
         break;
      default:
         shift.SetXYZ(0,0,1);
         break;
   };
   shift.RotateY(temp_pos.Theta());
   shift.RotateZ(temp_pos.Phi());

   temp_pos.SetMag(dist);

   return (temp_pos + shift);

}

void TGriffin::ResetFlags(){
   fGriffinBits = 0;
}


void TGriffin::ResetAddback() {
   //Used to clear the addback hits. When playing back a tree, this must
   //be called before building the new addback hits, otherwise, a copy of
   //the old addback hits will be stored instead.
   //This should have changed now, we're using the stored griffin bits to reset the addback
   //unset the addback bit in fGriffinBits
   SetBitNumber(kIsAddbackSet, false);
   SetBitNumber(kIsCrossTalkSet,false);
   fAddbackHits.clear();
   fAddbackFrags.clear();
}

UShort_t TGriffin::GetNAddbackFrags(size_t idx) const{
   //Get the number of addback "fragments" contributing to the total addback hit
   //with index idx.
   if(idx < fAddbackFrags.size())
      return fAddbackFrags.at(idx);   
   else
      return 0;
}

void TGriffin::SetBitNumber(enum EGriffinBits bit,Bool_t set){
   //Used to set the flags that are stored in TGriffin.
   if(set)
      fGriffinBits |= bit;
   else
      fGriffinBits &= (~bit);
}

Double_t TGriffin::CTCorrectedEnergy(const TGriffinHit* const hit_to_correct, const TGriffinHit* const other_hit, Bool_t time_constraint){
   if(!hit_to_correct || !other_hit){
      printf("One of the hits is invalid in TGriffin::CTCorrectedEnergy\n");
      return 0;
   }

   if(time_constraint){
      //Figure out if this passes the selected window
      if(TMath::Abs(other_hit->GetTime() - hit_to_correct->GetTime()) > TGRSIRunInfo::AddBackWindow()) //placeholder
         return hit_to_correct->GetEnergy();
   }

   if(hit_to_correct->GetDetector() != other_hit->GetDetector() ){
      return hit_to_correct->GetEnergy();
   }
   return hit_to_correct->GetEnergy() - (gCrossTalkPar[0][hit_to_correct->GetCrystal()][other_hit->GetCrystal()] + gCrossTalkPar[1][hit_to_correct->GetCrystal()][other_hit->GetCrystal()]*other_hit->GetNoCTEnergy());

}

Bool_t TGriffin::IsCrossTalkSet() const{
   return (fGriffinBits & kIsCrossTalkSet);
}

void TGriffin::FixCrossTalk() {
   if(fGriffinHits.size() < 2) {
      SetBitNumber(kIsCrossTalkSet,true);
      return;
   }
   for(size_t i=0; i<fGriffinHits.size(); ++i)
      fGriffinHits[i].ClearEnergy();

   if(TGRSIRunInfo::Get()->IsCorrectingCrossTalk()){
      size_t i, j;
      for(i = 0; i < fGriffinHits.size(); ++i) {
         for(j = i+1; j < fGriffinHits.size(); ++j) {
            fGriffinHits[i].SetEnergy(TGriffin::CTCorrectedEnergy(&(fGriffinHits[i]),&(fGriffinHits[j])));
            fGriffinHits[j].SetEnergy(TGriffin::CTCorrectedEnergy(&(fGriffinHits[j]),&(fGriffinHits[i])));
         }
      }
   }
   SetBitNumber(kIsCrossTalkSet,true);

}
