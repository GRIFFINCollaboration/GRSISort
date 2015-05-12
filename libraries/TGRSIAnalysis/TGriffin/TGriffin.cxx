
#include <iostream>
#include "TGriffin.h"
#include <TRandom.h>
#include <TMath.h>

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


bool TGriffin::fSetCoreWave = false;
//bool TGriffin::fSetBGOHits  = false;
//bool TGriffin::fSetBGOWave  = false;

long TGriffin::fCycleStart  = 0;
long TGriffin::fLastPPG     = 0;

//bool     TGriffin::gCloverPositionSet = false;
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
   Clear();
}

TGriffin::TGriffin(const TGriffin& rhs) {
  ((TGriffin&)rhs).Copy(*this);
}

void TGriffin::Copy(TGriffin &rhs) const {
  TGRSIDetector::Copy((TGRSIDetector&)rhs);

  ((TGriffin&)rhs).grifdata     = 0;
  //((TGriffin&)rhs).bgodata      = 0;

  ((TGriffin&)rhs).griffin_hits        = griffin_hits;
  //((TGriffin&)rhs).addback_hits        = addback_hits;
  //((TGriffin&)rhs).addback_clover_hits = addback_clover_hits;
  //((TGriffin&)rhs).fSetBGOHits         = fSetBGOHits;
  ((TGriffin&)rhs).fSetCoreWave        = fSetCoreWave;
  ((TGriffin&)rhs).fGriffinBits        = fGriffinBits;
  //((TGriffin&)rhs).fSetBGOWave         = fSetBGOWave;
  //((TGriffin&)rhs).ftapemove           = ftapemove;
  //((TGriffin&)rhs).fbackground         = fbackground;
  //((TGriffin&)rhs).fbeamon             = fbeamon;      
  //((TGriffin&)rhs).fdecay              = fdecay;       
  return;                                      
}                                       

TGriffin::~TGriffin()	{
   //Default Destructor
   if(grifdata) delete grifdata;
   //if(bgodata)  delete bgodata;
}

//void TGriffin::InitCloverPositions() {
   ////Initiallizes the HPGe Clover positions as per the wiki <https://www.triumf.info/wiki/tigwiki/index.php/HPGe_Coordinate_Table>
   //gCloverPosition[0].SetMagThetaPhi(1.0,TMath::DegToRad()*(0.0),TMath::DegToRad()*(0.0));
//
   ////Downstream lampshade
   //gCloverPosition[1].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(67.5));
   //gCloverPosition[2].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(157.5));
   //gCloverPosition[3].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(247.5));
   //gCloverPosition[4].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(337.5));
//
   ////Corona
   //gCloverPosition[5].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(22.5));
   //gCloverPosition[6].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(67.5));
   //gCloverPosition[7].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(112.5));
   //gCloverPosition[8].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(157.5));
   //gCloverPosition[9].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(202.5));
   //gCloverPosition[10].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(247.5));
   //gCloverPosition[11].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(292.5));
   //gCloverPosition[12].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(337.5));
//
   ////Upstream lampshade
   //gCloverPosition[13].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(67.5));
   //gCloverPosition[14].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(157.5));
   //gCloverPosition[15].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(247.5));
   //gCloverPosition[16].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(337.5));
      //
//}

void TGriffin::Clear(Option_t *opt)	{
   //Clears the mother, all of the hits and any stored data
   //if(!strcmp(opt,"all") {
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
     TGRSIDetector::Clear(opt);
     if(grifdata) grifdata->Clear();
	  //if(bgodata)  bgodata->Clear();
     ClearStatus();
   }
	griffin_hits.clear();
	//addback_hits.clear();
	//addback_clover_hits.clear();
}


void TGriffin::Print(Option_t *opt) const {
  //Prints out TGriffin members, currently does nothing.
  printf("grifdata = 0x%p\n",grifdata);
  if(grifdata) grifdata->Print();
  //printf("bgodata  = 0x%p\n",bgodata);
  //if(bgodata) bgodata->Print();
  printf("%lu griffin_hits\n",griffin_hits.size());
  //printf("%lu addback_hits\n",addback_hits.size());
  //printf("%lu addback_clover_hits\n",addback_clover_hits.size());
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

void TGriffin::BuildHits(TGRSIDetectorData *data,Option_t *opt)	{
//Builds the GRIFFIN Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
//This is done for both GRIFFIN and it's suppressors.
   TGriffinData *gdata = (TGriffinData*)data;
   if(gdata==0)
      gdata = (this->grifdata);
   //TBGOData *bdata = (this->bgodata);

   if(!gdata)
      return;


   
   //griffin_hits.clear();
   Clear("");
   griffin_hits.reserve(gdata->GetMultiplicity());

  // std::vector<TGriffinHit> temp_hits;
  // std::map<std::pair<int,int>,std::pair<int,int> > address_gain_map;  // < <det,core>, <high gain , low gain> >

   //printf("=========================================================\n");
   //printf("=========================================================\n");
   //printf("gdata->GetMultiplicity() = %i\n",gdata->GetMultiplicity());

   for(int i=0;i<gdata->GetMultiplicity();i++)	{
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

      corehit.SetTime(gdata->GetCoreTime(i));
      corehit.SetCfd(gdata->GetCoreCFD(i));

      if(TGriffin::SetCoreWave()){
         corehit.SetWaveform(gdata->GetCoreWave(i));
      }
		
      //corehit.SetDetectorNumber(gdata->GetCloverNumber(i));
      //corehit.SetCrystalNumber(gdata->GetCoreNumber(i));
   
//      corehit.SetPosition();
 //FIX     
      corehit.SetPPG(gdata->GetPPG(i));

      if((gdata->GetPPG(i) == 0xd000 && gdata->GetPPG(i) != fLastPPG) || fCycleStart == 0.) { //this is a background event
         fCycleStart = corehit.GetTime();
      }
      fLastPPG = gdata->GetPPG(i);
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
//   BuildAddBack();
//   BuildAddBackClover();
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


/*
void TGriffin::BuildAddBack(Option_t *opt) { 
   //Builds the addback for the GRIFFIN Event. This is based on a resolution set within the function. This will have to be
   //tuned in order to make add-back the most efficient. 
   if(this->griffin_hits.size() == 0)
      return;
   //We may have angular correlation add-back algorithms eventaully too.
   addback_hits.clear();
   addback_hits.push_back(*(this->GetGriffinHit(0)));
   //	addback_hits.at(0).Add(&(addback_hits.at(0)));

   for(int i = 1; i<this->GetMultiplicity(); i++) {
      bool used = false;
      for(int j =0; j<addback_hits.size();j++) {
         TVector3 res = addback_hits.at(j).GetPosition() - this->GetGriffinHit(i)->GetPosition();

         int d_time = std::abs(addback_hits.at(j).GetTime() - this->GetGriffinHit(i)->GetTime());

         if( (res.Mag() < 105) && (d_time < 20) )    {    ///Still need to tune these values!! pcb.
            used = true;
            addback_hits.at(j).Add(this->GetGriffinHit(i));
            break;
	      }
      }
      if(!used) {
         addback_hits.push_back(*(this->GetGriffinHit(i)));
        	//addback_hits.back().Add(&(addback_hits.back()));
      }
   }
}
*/
/*
void TGriffin::BuildAddBackClover(Option_t *opt) { 
   //Builds the addback for the GRIFFIN Event. This is based on a resolution set within the function. This will have to be
   //tuned in order to make add-back the most efficient. 
   if(this->griffin_hits.size() == 0)
      return;
   //We may have angular correlation add-back algorithms eventaully too.
   addback_clover_hits.clear();
   addback_clover_hits.push_back(*(this->GetGriffinHit(0)));

   for(int i = 1; i<this->GetMultiplicity(); i++) {
      bool used = false;
      for(int j =0; j<addback_clover_hits.size();j++) {
//         TVector3 res = addback_hits.at(j).GetPosition() - this->GetGriffinHit(i)->GetPosition();
         if(addback_clover_hits.at(j).GetDetectorNumber() != griffin_hits.at(i).GetDetectorNumber())
            continue;
         int d_time = std::abs(addback_clover_hits.at(j).GetTime() - this->GetGriffinHit(i)->GetTime());

         if(  (d_time < 20)  )    {    ///Still need to tune these values!! pcb.
            used = true;
            addback_clover_hits.at(j).Add(this->GetGriffinHit(i));
            break;
	      }
      }
      if(!used) {
         addback_clover_hits.push_back(*(this->GetGriffinHit(i)));
      }
   }
}
*/

