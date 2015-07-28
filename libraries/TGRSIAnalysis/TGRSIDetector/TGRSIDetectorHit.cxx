
#include "TGRSIDetectorHit.h"
#include <TClass.h>

ClassImp(TGRSIDetectorHit)

////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIDetectorHit                                           //
//                                                            //
// This is class that contains the basic info about detector  //
// hits. This is where the position of a detector is stored.  //
//                                                            //
////////////////////////////////////////////////////////////////

TGRSIDetectorHit::TGRSIDetectorHit(const int &fAddress):TObject()	{ 
  //Default constructor
  Clear();
  address = fAddress;
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::TGRSIDetectorHit(const TGRSIDetectorHit& rhs)	{ 
  //Default Copy constructor
  ((TGRSIDetectorHit&)rhs).Copy(*this);
  //((TGriffinHit&)rhs).cfd             = cfd;
  //rhs.time            = time;
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::~TGRSIDetectorHit()	{
//Default destructor
}

double TGRSIDetectorHit::GetEnergy(Option_t *opt) const {
   if(is_energy_set)
      return energy;

   TChannel *chan = GetChannel();
   if(!chan){
      printf("no TChannel set for this address\n");
      return 0.00;
   }
      return chan->CalibrateENG(GetCharge());
}
/* non const version
double TGRSIDetectorHit::GetEnergy(Option_t *opt){
   if(is_energy_set){
      return energy;
   }

   TChannel *chan = GetChannel();
   if(!chan){
      printf("no TChannel set for this address\n");
      return 0.00;
   }
      SetEnergy(chan->CalibrateENG(GetCharge()));
      return energy;

}

*/

void TGRSIDetectorHit::Copy(TObject &rhs) const {
  //if(!rhs.InheritsFrom("TGRSIDetectorHit")
  //   return;

  TObject::Copy(rhs);
  ((TGRSIDetectorHit&)rhs).address         = ((TGRSIDetectorHit&)*this).address;
  ((TGRSIDetectorHit&)rhs).position        = ((TGRSIDetectorHit&)*this).position;
  ((TGRSIDetectorHit&)rhs).waveform        = ((TGRSIDetectorHit&)*this).waveform;
  ((TGRSIDetectorHit&)rhs).cfd             = ((TGRSIDetectorHit&)*this).cfd;
  ((TGRSIDetectorHit&)rhs).time            = ((TGRSIDetectorHit&)*this).time;
  ((TGRSIDetectorHit&)rhs).charge          = ((TGRSIDetectorHit&)*this).charge;
  ((TGRSIDetectorHit&)rhs).detector        = ((TGRSIDetectorHit&)*this).detector;
  ((TGRSIDetectorHit&)rhs).energy          = ((TGRSIDetectorHit&)*this).energy;
  
  ((TGRSIDetectorHit&)rhs).is_energy_set   = false;
  ((TGRSIDetectorHit&)rhs).is_det_set      = false;
  ((TGRSIDetectorHit&)rhs).is_pos_set      = false;
//  ((TGRSIDetectorHit&)rhs).parent  = parent;  
}

void TGRSIDetectorHit::Print(Option_t *opt) const {
//General print statement for a TGRSIDetectorHit.
//Currently prints nothing.
}

void TGRSIDetectorHit::Clear(Option_t *opt) {
  //General clear statement for a TGRSIDetectorHit.
  address = 0xffffffff;    // -1
  position.SetXYZ(0,0,1);  // unit vector along the beam.
  waveform.clear();        // reset size to zero.
  charge          = 0;
  cfd             = -1;
  time            = -1;
  detector        = -1;
  energy          =  0.0;
  is_det_set = false;
  is_pos_set = false;
  is_energy_set = false;
}

UInt_t TGRSIDetectorHit::GetDetector() const {
   if(is_det_set)
      return detector;

   MNEMONIC mnemonic;
   TChannel *channel = GetChannel();
   if(!channel){
      Error("SetDetector","No TChannel exists for address %u",GetAddress());
      return -1;
   }
   ClearMNEMONIC(&mnemonic);
   ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
   return mnemonic.arrayposition;
}

/* non const version
UInt_t TGRSIDetectorHit::GetDetector() {
   if(is_det_set)
      return detector;

   MNEMONIC mnemonic;
   TChannel *channel = GetChannel();
   if(!channel){
      Error("SetDetector","No TChannel exists for address %u",GetAddress());
      return -1;
   }
   ClearMNEMONIC(&mnemonic);
   ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
   return SetDetector(mnemonic.arrayposition);
}
*/

UInt_t TGRSIDetectorHit::SetDetector(UInt_t det) {
   detector = det;
   is_det_set = true;
   return detector;
}

TVector3 TGRSIDetectorHit::SetPosition(Double_t dist) {
	position = TGRSIDetectorHit::GetPosition(dist); //Calls a general Hit GetPosition function
   return position;
}

TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) const{
   if(is_pos_set)
      return position;

   //if(is_det_set)// Dont necessarily need this (job of derived)
      return GetPosition(dist); //Calls the derivative GetPosition function

   return TVector3(0,0,1);

}
/* non const version
TVector3 TGRSIDetectorHit::GetPosition(Double_t dist) {
   if(is_pos_set)
      return position;

   if(is_det_set)
      return SetPosition(dist); //Calls the derivative GetPosition function

   return TVector3(0,0,1);

}
*/
bool TGRSIDetectorHit::CompareEnergy(TGRSIDetectorHit *lhs, TGRSIDetectorHit *rhs) {
   return (lhs->GetEnergy() > rhs->GetEnergy());
}


void TGRSIDetectorHit::CopyFragment(const TFragment &frag) {
  this->address  = frag.ChannelAddress;  
  this->charge   = frag.GetCharge();
  this->cfd      = frag.GetCfd();
  this->time     = frag.GetTime();
  this->position = TVector3(0,0,1); 
  this->energy   = frag.GetEnergy();

  this->SetDetector(this->GetDetector());
}


