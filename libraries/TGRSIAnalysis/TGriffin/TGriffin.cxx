
#include <iostream>
#include "TGriffin.h"
#include <TRandom.h>
#include <TMath.h>

#include <TGRSIRunInfo.h>

////////////////////////////////////////////////////////////
//                    
// TGriffin
//
// The TGriffin class defines the observables and algorithms used
// when analyzing GRIFFIN data. It includes detector positions,
// add-back methods, etc. 
//
////////////////////////////////////////////////////////////

ClassImp(TGriffin)

bool DefaultAddback(TGriffinHit& one, TGriffinHit& two) {
	return ((one.GetDetector() == two.GetDetector()) &&
			  (std::abs(one.GetTime() - two.GetTime()) < TGRSIRunInfo::AddBackWindow()));
}
 
std::function<bool(TGriffinHit&, TGriffinHit&)> TGriffin::fAddback_criterion = DefaultAddback;

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


TGriffin::TGriffin() : TGRSIDetector(),grifdata(0) { //  ,bgodata(0)	{
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
  ((TGriffin&)rhs).Copy(*this);
}

void TGriffin::Copy(TObject &rhs) const {
  //Copy function.
  TGRSIDetector::Copy(rhs);

  static_cast<TGriffin&>(rhs).grifdata     = 0;
  //static_cast<TGriffin&>(rhs).bgodata      = 0;

  static_cast<TGriffin&>(rhs).griffin_hits        = griffin_hits;
  static_cast<TGriffin&>(rhs).fAddback_hits        = fAddback_hits;
  static_cast<TGriffin&>(rhs).fAddback_frags      = fAddback_frags;
  static_cast<TGriffin&>(rhs).fSetCoreWave        = fSetCoreWave;
  static_cast<TGriffin&>(rhs).fGriffinBits        = fGriffinBits;
  static_cast<TGriffin&>(rhs).fCycleStart         = fCycleStart;
  static_cast<TGriffin&>(rhs).fGriffinBits        = fGriffinBits;
   

  return;                                      
}                                       

TGriffin::~TGriffin()	{
   //Default Destructor
   if(grifdata) {
      delete grifdata;
      grifdata = 0;
   }
   //if(bgodata)  delete bgodata;
}

void TGriffin::Clear(Option_t *opt)	{
   //Clears the mother, all of the hits and any stored data
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
      if(grifdata) grifdata->Clear();
	  //if(bgodata)  bgodata->Clear();
     ClearStatus();
   }
   TGRSIDetector::Clear(opt);
   griffin_hits.clear();
   fAddback_hits.clear();
   fAddback_frags.clear();
   fCycleStart = 0;
   //fGriffinBits.Class()->IgnoreTObjectStreamer(kTRUE);
}


void TGriffin::Print(Option_t *opt) const {
  //Prints out TGriffin members, currently does nothing.
  printf("grifdata = %p\n",(void*) grifdata);
  if(grifdata) grifdata->Print();
  //printf("bgodata  = 0x%p\n",bgodata);
  //if(bgodata) bgodata->Print();
  printf("%lu griffin_hits\n",griffin_hits.size());
  printf("%ld cycle start\n",fCycleStart);
  return;
}

TGriffin& TGriffin::operator=(const TGriffin& rhs) {
     ((TGriffin&)rhs).Copy(*this);
     return *this;
}

void TGriffin::FillData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
//Fills the "Data" structure for a specific channel with TFragment frag.
   if(!frag || !channel || !mnemonic)
      return;
   if(!grifdata)   
      grifdata = new TGriffinData();

   //frag->Print();
   //channel->Print();
   //PrintMNEMONIC(mnemonic);

   if(mnemonic->subsystem.compare(0,1,"G")==0) { 
      grifdata->SetCore(frag,channel,mnemonic);
   }   
   else if(mnemonic->subsystem.compare(0,1,"S")==0) {
      //FillBGOData(frag,channel,mnemonic);
   }
   TGriffinData::Set();
}


//void TGriffin::FillBGOData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
   //Fills the BGO "Data" Structure for a specific channel with the TFragment frag.
//   if(SetBGOHits()) {
//      if(!bgodata)
//         bgodata = new TBGOData();
//      bgodata->SetBGO(frag,channel,mnemonic);
//   }
//      TBGOData::Set();   
//}

void TGriffin::PushBackHit(TGRSIDetectorHit *ghit){
   griffin_hits.push_back(*((TGriffinHit*)ghit));
}


TGRSIDetectorHit* TGriffin::GetHit(const Int_t idx) {
   return GetGriffinHit(idx);
}


TGriffinHit* TGriffin::GetGriffinHit(const int i) {
   if(i < GetMultiplicity())
      return &griffin_hits.at(i);   
   else
      return 0;
}

Int_t TGriffin::GetAddbackMultiplicity() {
   // Automatically builds the addback hits using the addback_criterion (if the size of the addback_hits vector is zero) and return the number of addback hits.
   if(griffin_hits.size() == 0) {
      return 0;
   }
	//if the addback has been reset, clear the addback hits
	if((fGriffinBits & kIsAddbackSet) == 0x0) {
		fAddback_hits.clear();
	}
   if(fAddback_hits.size() == 0) {
      // use the first griffin hit as starting point for the addback hits
      fAddback_hits.push_back(griffin_hits[0]);
      fAddback_frags.push_back(1);

      // loop over remaining griffin hits
      size_t i, j;
      for(i = 1; i < griffin_hits.size(); ++i) {
	      // check for each existing addback hit if this griffin hit should be added
	      for(j = 0; j < fAddback_hits.size(); ++j) {
	         if(fAddback_criterion(fAddback_hits[j], griffin_hits[i])) {
	            fAddback_hits[j].Add(&(griffin_hits[i]));
               fAddback_frags[j]++;
	            break;
	         }
	      }
	      if(j == fAddback_hits.size()) {
	         fAddback_hits.push_back(griffin_hits[i]);
            fAddback_frags.push_back(1);
	      }
      }
	   SetBitNumber(kIsAddbackSet, true);
   }

   return fAddback_hits.size();
}

TGriffinHit* TGriffin::GetAddbackHit(const int i) {
   if(i < GetAddbackMultiplicity()) {
      return &fAddback_hits.at(i);
   } else {
      return NULL;
   }
}

void TGriffin::BuildHits(TDetectorData *data,Option_t *opt)	{
//Builds the GRIFFIN Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
//This is done for both GRIFFIN and it's suppressors.
   TGriffinData *gdata = (TGriffinData*)data;
   if(gdata==0)
      gdata = (this->grifdata);
   //TBGOData *bdata = (this->bgodata);

   if(!gdata)
      return;

   griffin_hits.clear();
   Clear("");
   griffin_hits.reserve(gdata->GetMultiplicity());

  // std::vector<TGriffinHit> temp_hits;
  // std::map<std::pair<int,int>,std::pair<int,int> > address_gain_map;  // < <det,core>, <high gain , low gain> >

   //printf("=========================================================\n");
   //printf("=========================================================\n");
   //printf("gdata->GetMultiplicity() = %i\n",gdata->GetMultiplicity());

   for(size_t i=0;i<gdata->GetMultiplicity();i++)	{
      TGriffinHit corehit;

      corehit.SetAddress(gdata->GetCoreAddress(i));
      
      //if(gdata->GetIsHighGain(i)) {
      //   corehit.SetEnergyHigh(gdata->GetCoreEnergy(i));
      //   corehit.SetChargeHigh(gdata->GetCoreCharge(i));
      //}
      //else {
      //   corehit.SetEnergyLow(gdata->GetCoreEnergy(i));
      //   corehit.SetChargeLow(gdata->GetCoreCharge(i));
      //}

      corehit.SetTimeStamp(gdata->GetCoreTime(i));
      corehit.SetCfd(gdata->GetCoreCFD(i));
      corehit.SetCharge(gdata->GetCoreCharge(i));
      corehit.SetNPileUps((UChar_t)(gdata->GetCoreNbrHits(i)));
      corehit.SetPUHit((UChar_t)(gdata->GetCorePUHit(i)));
/*
      if(TGriffin::SetCoreWave()){
         corehit.SetWaveform(gdata->GetCoreWave(i));
      }
*/		
      //corehit.SetDetectorNumber(gdata->GetCloverNumber(i));
      //corehit.SetCrystalNumber(gdata->GetCoreNumber(i));
   
     // corehit.SetPosition(TGRSIRunInfo::HPGeArrayPosition());
      
      //fCycleStartTime = fCycleStart;

      //temp_hits.push_back(corehit);  
      //griffin_hits.push_back(corehit);
      AddHit(&corehit);

      //printf(RED "gdata->GetCoreNbrHitsMidasId(%i)    = %i" RESET_COLOR "\n",i, gdata->GetCoreNbrHits(i)); 
      //printf("gdata->GetCoreMidasId(%i)    = %i\n",i, gdata->GetCoreMidasId(i));
      //printf("gdata->GetIsHighGain(%i)    = %s\n",i, gdata->GetIsHighGain(i) ? "true":"false");
      //printf("corehit.GetDetectorNumber() = %i\n", corehit.GetDetectorNumber());
      //printf("corehit.GetCrystalNumber()  = %i\n", corehit.GetCrystalNumber());
      //printf("corehit.GetEnergyLow()     = %.02f | %.02f\n", corehit.GetEnergyLow() , corehit.GetTime());
      

//      std::pair<int,int> det_cry = std::make_pair(corehit.GetDetectorNumber(),corehit.GetCrystalNumber());
/*
      if(address_gain_map.count(det_cry)==0) {
         if(gdata->GetIsHighGain(i)) 
            address_gain_map.insert(std::make_pair(det_cry,std::make_pair(temp_hits.size()-1,0)));
         else
            address_gain_map.insert(std::make_pair(det_cry,std::make_pair(0,temp_hits.size()-1)));
      } else {
         if(gdata->GetIsHighGain(i)) 
            address_gain_map.at(det_cry).first = temp_hits.size()-1;
         else
            address_gain_map.at(det_cry).second = temp_hits.size()-1;
      }
  */    

   }

   //printf("created %ld hits\n",griffin_hits.size());


/*
   std::map<std::pair<int,int>,std::pair<int,int> >::iterator iter;
   for(iter = address_gain_map.begin(); iter != address_gain_map.end(); iter++) {
     temp_hits.at(iter->second.first).SetChargeLow(temp_hits.at(iter->second.second).GetChargeLow());
     temp_hits.at(iter->second.first).SetEnergyLow(temp_hits.at(iter->second.second).GetEnergyLow());
     griffin_hits.push_back(temp_hits.at(iter->second.first));

   }
*/ 

/*
   if(TGriffin::SetBGOHits() && bdata)  {
      TCrystalHit temp_crystal; temp_crystal.Clear();
      for(int i=0;i<griffin_hits.size();i++)	{
        for(int j=0;j< bdata->GetBGOMultiplicity();j++)  {
            if((griffin_hits[i].GetDetectorNumber() == bdata->GetBGOCloverNumber(j))  && (griffin_hits[i].GetCrystalNumber() == bdata->GetBGOCoreNumber(j))) {
               temp_crystal.Clear();
               temp_crystal.SetSegmentNumber(bdata->GetBGOPmNbr(j));
               temp_crystal.SetCharge(bdata->GetBGOCharge(j));
               //temp_crystal.SetEnergy(bdata->GetBGOEnergy(j));
               temp_crystal.SetTime(bdata->GetBGOTime(j));
               temp_crystal.SetCfd(bdata->GetBGOCFD(j));
               if(TGriffin::SetBGOWave()) {
                  temp_crystal.SetWave(bdata->GetBGOWave(j));
               }			
	            griffin_hits.at(i).SetBGO(temp_crystal);	
            }
         }
      }
   }
*/


   //for(int z=0;z<griffin_hits.size();z++)
      //griffin_hits.at(z).Print();

   //printf(DGREEN "|||||||||||||||||||||||||||||||||||||||||||||||||||||" RESET_COLOR "\n");

   //if(griffin_hits.size()>1)
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


void TGriffin::ResetAddback() {
//Used to clear the addback hits. When playing back a tree, this must
//be called before building the new addback hits, otherwise, a copy of
//the old addback hits will be stored instead.
//This should have changed now, we're using the stored griffin bits to reset the addback
	//unset the addback bit in fGriffinBits
	SetBitNumber(kIsAddbackSet, false);
   fAddback_hits.clear();
   fAddback_frags.clear();
}

UShort_t TGriffin::GetNAddbackFrags(size_t idx) const{
 //Get the number of addback "fragments" contributing to the total addback hit
 //with index idx.
  if(idx < fAddback_frags.size())
      return fAddback_frags.at(idx);   
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
