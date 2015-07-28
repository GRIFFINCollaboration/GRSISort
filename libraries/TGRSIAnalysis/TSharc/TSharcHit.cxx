
#include "TSharcHit.h"
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

TSharcHit::TSharcHit(const TSharcHit &rhs)	{	
   Class()->IgnoreTObjectStreamer(kTRUE);
   Clear("ALL");
   ((TSharcHit&)rhs).Copy(*this);
}

void TSharcHit::Copy(TSharcHit &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
  backhit.Copy((TGRSIDetectorHit&)rhs.backhit);  
  padhit.Copy((TGRSIDetectorHit&)rhs.paddhit);  

  ((TSharcHit&)rhs).detectornumber = detectornumber;
  ((TSharcHit&)rhs).front_strip    = front_strip;	  
  ((TSharcHit&)rhs).back_strip     = back_strip;	  
  ((TSharcHit&)rhs).X_offset       = X_offset;
  ((TSharcHit&)rhs).Y_offset       = Y_offset;	  
  ((TSharcHit&)rhs).Z_offset       = Z_offset;	  

}                                       

void TSharcHit::Clear(Option_t *options)	{

  TGRSIDetector::Clear(); // 
  backhit.Clear();        //
  padhit.Clear();         //

  detectornumber = -1;    //
  front_strip    = -1;    //
  back_strip     = -1;    //

  if(strcmp(options,"ALL") { 
    X_offset = 0.00;
    Y_offset = 0.00;
    Z_offset = 0.00;
  }
}

void TSharcHit::Print(Option_t *options) const {
  printf(DGREEN "[D/F/B] = %02i\t/%02i\t/%02i " RESET_COLOR "\n",GetDetectorNumber(),GetFrontStrip(),GetBackStrip());
  printf("Sharc hit charge: %02f\n",GetFrontCharge());
  printf("Sharc hit energy: %f\n",GetDeltaE());
  printf("Sharc hit time:   %f\n",GetDeltaT());
  printf( DGREEN "=	=	=	=	=	=	=	" RESET_COLOR "\n");
}

TVector3 TSharcHit::GetPosition() const {
  return TSharc::GetPosition(detectornumber,front_strip,back_strip,TSharc::GetXOffset(),TSharc::GetYOffset(),TSharc::GetZOffset());  //! 
}

Double_t TSharcHit::GetTheta(double Xoff, double Yoff, double Zoff) {
  TVector3 posoff; 
  posoff.SetXYZ(Xoff,Yoff,Zoff);
  return (position+posoff).Theta();
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

