
#include <iostream>

#include "TGriffin.h"

#include <TRandom.h>
#include <TMath.h>

ClassImp(TGriffin)


bool TGriffin::fSetCoreWave = false;
bool TGriffin::fSetBGOHits  = false;
bool TGriffin::fSetBGOWave  = false;

bool     TGriffin::gCloverPositionSet = false;
TVector3 TGriffin::gCloverPosition[17];

TGriffin::TGriffin() : grifdata(0), bgodata(0)	{
   if(!gCloverPositionSet) {
      gCloverPositionSet = true;
      InitCloverPositions();
   }
   Clear();
}

TGriffin::~TGriffin()	{
  	if(grifdata) delete grifdata;
   if(bgodata)  delete bgodata;
}

void TGriffin::InitCloverPositions() {
   gCloverPosition[0].SetMagThetaPhi(1.0,TMath::DegToRad()*(0.0),TMath::DegToRad()*(0.0));

   gCloverPosition[1].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(67.5));
   gCloverPosition[2].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(157.5));
   gCloverPosition[3].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(247.5));
   gCloverPosition[4].SetMagThetaPhi(1.0,TMath::DegToRad()*(45.0),TMath::DegToRad()*(337.5));

   gCloverPosition[5].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(22.5));
   gCloverPosition[6].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(67.5));
   gCloverPosition[7].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(112.5));
   gCloverPosition[8].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(157.5));
   gCloverPosition[9].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(202.5));
   gCloverPosition[10].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(247.5));
   gCloverPosition[11].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(292.5));
   gCloverPosition[12].SetMagThetaPhi(1.0,TMath::DegToRad()*(90.0),TMath::DegToRad()*(337.5));


   gCloverPosition[13].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(67.5));
   gCloverPosition[14].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(157.5));
   gCloverPosition[15].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(247.5));
   gCloverPosition[16].SetMagThetaPhi(1.0,TMath::DegToRad()*(135.0),TMath::DegToRad()*(337.5));
      
}

void TGriffin::Clear(Option_t *opt)	{

	if(grifdata) grifdata->Clear();
	if(bgodata)  bgodata->Clear();

	griffin_hits.clear();
	addback_hits.clear();

}


void TGriffin::Print(Option_t *opt)	{
  printf("not yet written...\n");
  return;
}


void TGriffin::FillData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
	 if(!frag || !channel || !mnemonic)
	   return;

   if(!grifdata)   
      grifdata = new TGriffinData();

   if(mnemonic->subsystem.compare(0,1,"F")==0) { 
 	   grifdata->SetCore(frag,channel,mnemonic);
	} else if(mnemonic->subsystem.compare(0,1,"S")==0) {
      FillBGOData(frag,channel,mnemonic);
   }
	TGriffinData::Set();
}


void TGriffin::FillBGOData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
   if(SetBGOHits()) {
	   if(!bgodata)
  			bgodata = new TBGOData();
      bgodata->SetBGO(frag,channel,mnemonic);
   }
	TBGOData::Set();   
}



void	TGriffin::BuildHits(TGRSIDetectorData *data,Option_t *opt)	{
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

		if(TGriffin::SetCoreWave())	{
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


TVector3 TGriffin::GetPosition(int DetNbr,int CryNbr, double dist)	{
   if(DetNbr>16)
      return TVector3(0,0,1);

   TVector3 temp_pos(gCloverPosition[DetNbr]);

   Double_t cp = 20.0; //Crystal Center Point Might have adepth eventually//  cm ??  now mm.
   //Set Theta's of the center of each DETECTOR face
   ////Define one Detector position
   TVector3 shift;
   switch(CryNbr) {
      case 0:
         shift.SetXYZ(-cp,cp,0);
         break;
      case 1:
         shift.SetXYZ(cp,cp,0);
         break;
      case 2:
         shift.SetXYZ(cp,-cp,0);
         break;
      case 3:
         shift.SetXYZ(-cp,-cp,0);
         break;
      default:
         shift.SetXYZ(0,0,1);
         break;
   };
   shift.RotateY(temp_pos.Theta());
   shift.RotateZ(temp_pos.Phi());


   return (temp_pos + shift);

}



void TGriffin::BuildAddBack(Option_t *opt)	{ 

	if(this->griffin_hits.size() == 0)
    	return;

	addback_hits.clear();
	addback_hits.push_back(*(this->GetGriffinHit(0)));
   //	addback_hits.at(0).Add(&(addback_hits.at(0)));

	for(int i = 1; i<this->GetMultiplicity(); i++)   {
    	bool used = false;
	    for(int j =0; j<addback_hits.size();j++)    {
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









