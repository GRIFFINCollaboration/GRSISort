
#include <iostream>
#include "TPaces.h"
#include <TRandom.h>
#include <TMath.h>

////////////////////////////////////////////////////////////
//                    
// TPaces
//
// The TPaces class defines the observables and algorithms used
// when analyzing PACES data.
//  
//
////////////////////////////////////////////////////////////

ClassImp(TPaces)


bool TPaces::fSetCoreWave = false;

long TPaces::fCycleStart  = 0;
long TPaces::fLastPPG     = 0;

TPaces::TPaces() : pacesdata(0)	{
   //Default Constructor
   //Class()->IgnoreTObjectStreamer(true);
   Clear();
}

TPaces::TPaces(const TPaces& rhs) {
     pacesdata     = 0;
     paces_hits = rhs.paces_hits;
     fSetCoreWave = rhs.fSetCoreWave;
     ftapemove    = rhs.ftapemove;
     fbackground  = rhs.fbackground;
     fbeamon      = rhs.fbeamon;
     fdecay       = rhs.fdecay;
}

TPaces::~TPaces()	{
   //Default Destructor
   if(pacesdata) delete pacesdata;
}

void TPaces::Clear(Option_t *opt)	{
//Clears all of the hits and data
	if(pacesdata) pacesdata->Clear();

	paces_hits.clear();

}


void TPaces::Print(Option_t *opt) {
  //Prints out TPaces members, currently does nothing.
  printf("pacesdata = 0x%p\n",pacesdata);
  if(pacesdata) pacesdata->Print();
  printf("%lu paces_hits\n",paces_hits.size());
  return;
}

TPaces& TPaces::operator=(const TPaces& rhs) {
     pacesdata    = 0;
     paces_hits   = rhs.paces_hits;
     fSetCoreWave = rhs.fSetCoreWave;
     ftapemove    = rhs.ftapemove;
     fbackground  = rhs.fbackground;
     fbeamon      = rhs.fbeamon;
     fdecay       = rhs.fdecay;

     return *this;
}

void TPaces::FillData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
//Fills the "Data" structure for a specific channel with TFragment frag.
   if(!frag || !channel || !mnemonic)
      return;

   if(!pacesdata)   
      pacesdata = new TPacesData();

   //frag->Print();
   //channel->Print();
   //PrintMNEMONIC(mnemonic);

   if(mnemonic->subsystem.compare(0,1,"C")==0) { 
      //frag->Print();
      pacesdata->SetCore(frag,channel,mnemonic);
   }   
   TPacesData::Set();
}


void TPaces::BuildHits(TGRSIDetectorData *data,Option_t *opt)	{
//Builds the PACES Hits from the "data" structure. Basically, loops through the data for and event and sets observables. 
   TPacesData *pdata = (TPacesData*)data;
   if(pdata==0)
      pdata = (this->pacesdata);

   if(!pdata)
      return;

   paces_hits.clear();

   for(int i=0;i<pdata->GetMultiplicity();i++)	{
      TPacesHit corehit;

      corehit.SetAddress(pdata->GetCoreAddress(i));
      
//      if(pdata->GetIsHighGain(i)) {
         //corehit.SetEnergyHigh(pdata->GetCoreEnergy(i));

      corehit.SetChargeHigh(pdata->GetCoreCharge(i));
//      }
//      else {
//         corehit.SetEnergyLow(pdata->GetCoreEnergy(i));
//         corehit.SetChargeLow(pdata->GetCoreCharge(i));
//      }

      corehit.SetTime(pdata->GetCoreTime(i));
      corehit.SetCfd(pdata->GetCoreCFD(i));

      if(TPaces::SetCoreWave()){
         corehit.SetWaveform(pdata->GetCoreWave(i));
      }
      corehit.SetCrystalNumber(pdata->GetCoreNumber(i));
   
      //corehit.SetPPG(pdata->GetPPG(i));

//      if((pdata->GetPPG(i) == 0xd000 && pdata->GetPPG(i) != fLastPPG) || fCycleStart == 0.) { //this is a background event
//         fCycleStart = corehit.GetTime();
//      }
//      fLastPPG = pdata->GetPPG(i);
      fCycleStartTime = fCycleStart;

      paces_hits.push_back(corehit);

      //printf(RED "pdata->GetCoreNbrHitsMidasId(%i)    = %i" RESET_COLOR "\n",i, pdata->GetCoreNbrHits(i)); 
      //printf("pdata->GetCoreMidasId(%i)    = %i\n",i, pdata->GetCoreMidasId(i));
      //printf("pdata->GetIsHighGain(%i)    = %s\n",i, pdata->GetIsHighGain(i) ? "true":"false");
      //printf("corehit.GetCrystalNumber()  = %i\n", corehit.GetCrystalNumber());
      //printf("corehit.GetEnergyLow()     = %.02f | %.02f\n", corehit.GetEnergyLow() , corehit.GetTime());
   }
      
}
