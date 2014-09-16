
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
bool TGriffin::fSetBGOHits  = false;
bool TGriffin::fSetBGOWave  = false;

bool     TGriffin::gCloverPositionSet = false;
TVector3 TGriffin::gCloverPosition[17];//This seems unnecessary, and why 17?;//  they are static members, and need
                                                                             //  to be defined outside the header
                                                                             //  17 is to have the detectors go from 1-16 
                                                                             //  plus we can use position zero 
                                                                             //  when the detector winds up back in 
                                                                             //  one of the stands like Alex used in the
                                                                             //  gps run. pcb.

TGriffin::TGriffin() : grifdata(0), bgodata(0)	{
   //Default Constructor
   Class()->IgnoreTObjectStreamer(true);
   if(!gCloverPositionSet) {
      gCloverPositionSet = true;
      InitCloverPositions();
   }
   Clear();
}

TGriffin::~TGriffin()	{
   //Default Destructor
   if(grifdata) delete grifdata;
   if(bgodata)  delete bgodata;
}

void TGriffin::InitCloverPositions() {
   //Initiallizes the HPGe Clover positions as per the wiki <https://www.triumf.info/wiki/tigwiki/index.php/HPGe_Coordinate_Table>
   gCloverPosition[0].SetMagThetaPhi(1.0,TMath::DegToRad()*(0.0),TMath::DegToRad()*(0.0));

   //Downstream lampshade
   gCloverPosition[1].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(67.5));
   gCloverPosition[2].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(157.5));
   gCloverPosition[3].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(247.5));
   gCloverPosition[4].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(337.5));

   //Corona
   gCloverPosition[5].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(22.5));
   gCloverPosition[6].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(67.5));
   gCloverPosition[7].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(112.5));
   gCloverPosition[8].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(157.5));
   gCloverPosition[9].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(202.5));
   gCloverPosition[10].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(247.5));
   gCloverPosition[11].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(292.5));
   gCloverPosition[12].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(337.5));

   //Upstream lampshade
   gCloverPosition[13].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(67.5));
   gCloverPosition[14].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(157.5));
   gCloverPosition[15].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(247.5));
   gCloverPosition[16].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(337.5));
      
}

void TGriffin::Clear(Option_t *opt)	{
//Clears all of the hits and data
	if(grifdata) grifdata->Clear();
	if(bgodata)  bgodata->Clear();

	griffin_hits.clear();
	addback_hits.clear();

}


void TGriffin::Print(Option_t *opt)	{
  //Prints out TGriffin members, currently does nothing.
  printf("not yet written...\n");
  return;
}


void TGriffin::FillData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
//Fills the "Data" structure for a specific channel with TFragment frag.
   if(!frag || !channel || !mnemonic)
      return;

   if(!grifdata)   
      grifdata = new TGriffinData();

   if(mnemonic->subsystem.compare(0,1,"F")==0) { 
      grifdata->SetCore(frag,channel,mnemonic);
   }   
   else if(mnemonic->subsystem.compare(0,1,"S")==0) {
      FillBGOData(frag,channel,mnemonic);
   }
   TGriffinData::Set();
}


void TGriffin::FillBGOData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
   //Fills the BGO "Data" Structure for a specific channel with the TFragment frag.
   if(SetBGOHits()) {
      if(!bgodata)
         bgodata = new TBGOData();
      bgodata->SetBGO(frag,channel,mnemonic);
   }
      TBGOData::Set();   
}



void TGriffin::BuildHits(TGRSIDetectorData *data,Option_t *opt)	{
//Builds the GRIFFIN Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
//This is done for both GRIFFIN and it's suppressors.
   TGriffinData *gdata = (TGriffinData*)data;
   if(gdata==0)
      gdata = (this->grifdata);
   TBGOData *bdata = (this->bgodata);

   if(!gdata)
      return;

   for(int i=0;i<gdata->GetMultiplicity();i++)	{
      TGriffinHit corehit;

      corehit.SetCharge(gdata->GetCoreCharge(i));
      corehit.SetEnergy(gdata->GetCoreEnergy(i));
      corehit.SetTime(gdata->GetCoreTime(i));
      corehit.SetCfd(gdata->GetCoreCFD(i));

      if(TGriffin::SetCoreWave()){
         corehit.SetWaveform(gdata->GetCoreWave(i));
      }
		
      corehit.SetDetectorNumber(gdata->GetCloverNumber(i));
      corehit.SetCrystalNumber(gdata->GetCoreNumber(i));
   
      corehit.SetPosition();

      griffin_hits.push_back(corehit);
   }

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

   if(griffin_hits.size()>1)
      BuildAddBack();

}


TVector3 TGriffin::GetPosition(int DetNbr,int CryNbr, double dist ){
   //Gets the position vector for a crystal specified by CryNbr within Clover DetNbr at a distance of dist mm away.
   //This is calculated to the most likely interaction point within the crystal.
   if(DetNbr>16)
      return TVector3(0,0,1);

   TVector3 temp_pos(gCloverPosition[DetNbr]);

   //Interaction points may eventually be set externally. May make these members of each crystal, or pass from waveforms.
   Double_t cp = 20.0; //Crystal Center Point  mm.
   Double_t id = 0.0;  //Crystal interaction depth mm.
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


   return (temp_pos + shift);

}



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

         int d_time = abs(addback_hits.at(j).GetTime() - this->GetGriffinHit(i)->GetTime());

         if( (res.Mag() < 105) && (d_time < 11) )    {    ///Still need to tune these values!! pcb.
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
