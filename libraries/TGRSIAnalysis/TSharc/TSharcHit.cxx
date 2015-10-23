

#include "TSharcHit.h"
#include "TSharc.h"
#include "TChannel.h"
#include <TClass.h>

ClassImp(TSharcHit)

TSharcHit::TSharcHit()	{	
#if MAJOR_ROOT_VERSION < 6
  Class()->IgnoreTObjectStreamer(kTRUE);
#endif
  Clear("ALL");
}

TSharcHit::~TSharcHit()	{	}

TSharcHit::TSharcHit(const TSharcHit &rhs) : TGRSIDetectorHit() {	
   Class()->IgnoreTObjectStreamer(kTRUE);
   Clear();
   ((TSharcHit&)rhs).Copy(*this);
}

void TSharcHit::Copy(TObject &rhs) const  {
  //if(!rhs.InheritsFrom("TSharcHit"))
  //   return;
  TGRSIDetectorHit::Copy(rhs);
  ((TGRSIDetectorHit&)backhit).Copy((TObject&)(((TSharcHit&)rhs).backhit));  
  ((TGRSIDetectorHit&)padhit).Copy((TObject&)(((TSharcHit&)rhs).padhit));  

  ((TSharcHit&)rhs).detectornumber = ((TSharcHit&)*this).detectornumber;
  ((TSharcHit&)rhs).front_strip    = ((TSharcHit&)*this).front_strip;     
  ((TSharcHit&)rhs).back_strip     = ((TSharcHit&)*this).back_strip;       
 
}                                       

void TSharcHit::Clear(Option_t *options)	{

  TGRSIDetectorHit::Clear(options); // 
  backhit.Clear(options);        //
  padhit.Clear(options);         //

  detectornumber = -1;    //
  front_strip    = -1;    //
  back_strip     = -1;    //

}

void TSharcHit::Print(Option_t *options) const {
  printf(DGREEN "[D/F/B] = %02i\t/%02i\t/%02i " RESET_COLOR "\n",GetDetectorNumber(),GetFrontStrip(),GetBackStrip());
  //printf("Sharc hit charge: %02f\n",GetFrontCharge());
  //printf("Sharc hit energy: %f\n",GetDeltaE());
  //printf("Sharc hit time:   %f\n",GetDeltaT());
  //printf( DGREEN "=	=	=	=	=	=	=	" RESET_COLOR "\n");
}

TVector3 TSharcHit::GetChannelPosition(Double_t dist) const {
 // return  fposition; // returned from this -> i.e front...
   //PC BENDER PLEASE LOOK AT THIS.
  return TSharc::GetPosition(detectornumber,front_strip,back_strip,TSharc::GetXOffset(),TSharc::GetYOffset(),TSharc::GetZOffset());  //! 
}

Double_t TSharcHit::GetTheta(double Xoff, double Yoff, double Zoff) {
  TVector3 posoff; 
  posoff.SetXYZ(Xoff,Yoff,Zoff);
  return (fposition+posoff).Theta();
}


//bool TSharcHit::Compare(TSharcHit *lhs, TSharcHit *rhs)	{
//	if(lhs->GetDetectorNumber() < rhs->GetDetectorNumber())	{
//		if(lhs->GetFrontStrip() < rhs->GetFrontStrip())	{
//			if(lhs->GetFrontCFD() < rhs->GetFrontCFD())	{
//				if(lhs->GetBackStrip() < rhs->GetBackStrip())	{
//					if(lhs->GetBackCFD() < rhs->GetBackCFD())	{
//						return true;
//					}
//				}
//			}	
//		}
//	}
//	return false;
//}


void TSharcHit::SetFront(const TFragment &frag) { 
  this->CopyFragment(frag);
  this->SetPosition(TSharc::GetPosition(detectornumber,front_strip,back_strip,TSharc::GetXOffset(),TSharc::GetYOffset(),TSharc::GetZOffset())); 
}

void TSharcHit::SetBack(const TFragment &frag) { 
  backhit.CopyFragment(frag);
}

void TSharcHit::SetPad(const TFragment &frag) { 
  padhit.CopyFragment(frag);
}



















