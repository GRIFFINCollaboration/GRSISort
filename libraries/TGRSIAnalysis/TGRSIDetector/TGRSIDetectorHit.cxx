
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
  Class()->IgnoreTObjectStreamer(true);
}

TGRSIDetectorHit::TGRSIDetectorHit(const TGRSIDetectorHit& rhs)	{ 
  //Default Copy constructor
  ((TGRSIDetectorHit&)rhs).Copy(*this);
  //((TGriffinHit&)rhs).cfd             = cfd;
  //((TGriffinHit&)rhs).time            = time;
  Class()->IgnoreTObjectStreamer(true);
}

TGRSIDetectorHit::~TGRSIDetectorHit()	{
//Default destructor
}

double TGRSIDetectorHit::GetEnergy(Option_t *opt) const{
   TChannel *chan = GetChannel();
   if(!chan){
      printf("no TChannel set for this address\n");
      return 0.00;
   }
   return chan->CalibrateENG(GetCharge());
}

void TGRSIDetectorHit::Copy(TGRSIDetectorHit &rhs) const {
  TObject::Copy((TObject&)rhs);
  ((TGRSIDetectorHit&)rhs).address  = address;
  ((TGRSIDetectorHit&)rhs).position = position;
  ((TGRSIDetectorHit&)rhs).waveform = waveform;
  ((TGRSIDetectorHit&)rhs).cfd      = cfd;
  ((TGRSIDetectorHit&)rhs).time     = time;
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
  cfd             = -1;
  time            = -1;
  detector        = -1;
  is_det_set = false;
  is_pos_set = false;
}

Int_t TGRSIDetectorHit::GetDetector() const {
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

Int_t TGRSIDetectorHit::SetDetector() {
   detector = GetDetector();
   is_det_set = true;
   return detector;
}

void TGRSIDetectorHit::SetPosition(double dist) {
	position = TGRSIDetectorHit::GetPosition(dist); //Calls a general Hit GetPosition function
}

TVector3 TGRSIDetectorHit::GetPosition(Double_t dist){
   if(is_pos_set)
      return position;

   if(!is_det_set){
      SetDetector();
   }
   
   if(is_det_set)
      return GetPosition(dist); //Calls the derivative GetPosition function

}

