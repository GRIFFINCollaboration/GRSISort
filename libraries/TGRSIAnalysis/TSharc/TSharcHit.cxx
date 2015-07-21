
#include "TSharcHit.h"
#include "TChannel.h"
#include <TClass.h>

ClassImp(TSharcHit)

TSharcHit::TSharcHit()	{	
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TSharcHit::~TSharcHit()	{	}

TSharcHit::TSharcHit(const TSharcHit &rhs)	{	
   Class()->IgnoreTObjectStreamer(kTRUE);
   Clear();
   ((TSharcHit&)rhs).Copy(*this);
}

void TSharcHit::Copy(TSharcHit &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
((TSharcHit&)rhs).front_strip		=	front_strip;	  
((TSharcHit&)rhs).front_charge	=	front_charge;   
((TSharcHit&)rhs).back_strip		=	back_strip;	  
((TSharcHit&)rhs).back_charge		=	back_charge;	  
((TSharcHit&)rhs).pad_charge		=	pad_charge;	  
((TSharcHit&)rhs).d_energy_front =	d_energy_front;     
((TSharcHit&)rhs).d_time_front	=	d_time_front;       
((TSharcHit&)rhs).d_energy_back	=	d_energy_back;      
((TSharcHit&)rhs).d_time_back 	=	d_time_back;        
((TSharcHit&)rhs).p_energy			=  p_energy;		    
((TSharcHit&)rhs).p_time			=  p_time;		    
((TSharcHit&)rhs).p_address      =  p_address;     
((TSharcHit&)rhs).front_address  =  front_address; 
((TSharcHit&)rhs).back_address   =  back_address;  
((TSharcHit&)rhs).detectornumber	=	detectornumber;
}                                       

void TSharcHit::Clear(Option_t *options)	{

	front_strip		=	0;	//
	front_charge	=	0;	//  
	back_strip		=	0;	//
	back_charge		=	0;	//

	pad_charge		=	0;	//

	d_energy_front =	0;	//      
	d_time_front	=	0;	//      
	d_energy_back	=	0;	//      
	d_time_back 	=	0;	//      
	//d_cfd				=	0;	//      

	p_energy			=	0;	//          pad only;
	p_time			=	0;	//          pad only;
	//p_cfd				=	0;	//
	p_address      = 0xffffffff;

	front_address  = 0xffffffff;
	back_address   = 0xffffffff;

	detectornumber	=	0;	//
	//position.SetXYZ(0,0,1); // 

}

void TSharcHit::Print(Option_t *options)	{
			printf(DGREEN "[D/F/B] = %02i\t/%02i\t/%02i " RESET_COLOR "\n",GetDetectorNumber(),GetFrontStrip(),GetBackStrip());
			printf("Sharc hit charge: %f\t0x%08x\n",(double)front_charge/125.0,front_charge);
			printf("Sharc hit energy: %f\n",d_energy_front);
			printf("Sharc hit time:   %f\n",d_time_front);
			printf( DGREEN "=	=	=	=	=	=	=	" RESET_COLOR "\n");
}



Double_t  TSharcHit::GetFrontChgHeight() {
  int temp_add = back_address;
  if((0x00f00000&back_address) != 0x00200000) {
    temp_add += 0x00200000;
  }
  TChannel *chan = TChannel::GetChannel(temp_add);
  if(!chan) {
    printf("AHHHH, can't find sharc channel with address %d in TChannel\n",temp_add);
    return front_charge;
  }
  return chan->CalibrateENG(front_charge);
}

Double_t  TSharcHit::GetBackChgHeight() {
  int temp_add = back_address;
  if((0x00f00000&back_address) != 0x00200000) {
    temp_add += 0x00200000;
  }
  TChannel *chan = TChannel::GetChannel(temp_add);
  if(!chan) {
    printf("AHHHH, can't find sharc channel with address %d in TChannel\n",temp_add);
    return back_charge;
  }
  return chan->CalibrateENG(back_charge); //((double)(front_charge)+gRandom->Uniform())/((double)chan->GetIntegration());
}


Double_t  TSharcHit::GetPadChgHeight() {
  if(!p_address)
     return 0.0;
  TChannel *chan = TChannel::GetChannel(p_address);
  if(!chan) {
    printf("AHHHH, can't find sharc channel with address %d in TChannel\n",p_address);
    return pad_charge;
  }
  return ((double)(front_charge)+gRandom->Uniform())/((double)chan->GetIntegration());
}




Double_t TSharcHit::GetTheta(double Xoff, double Yoff, double Zoff) {
	TVector3 posoff; 
	posoff.SetXYZ(Xoff,Yoff,Zoff);
   //return (position+posoff).Theta();
   //FIX
   return 0.0;
}

Double_t TSharcHit::PadEnergyCal() {
  TChannel *chan = TChannel::GetChannel(p_address);
  if(!chan)
    return 0.0;
  return chan->CalibrateENG(pad_charge);

}

Double_t TSharcHit::FrontEnergyCal() { //!
  TChannel *chan = TChannel::GetChannel(front_address);
//  printf("\n\nchan = %p.\t front_address = %i\n",chan,front_address);
  if(!chan)
    chan = TChannel::GetChannelByNumber(front_address);
  if(!chan)
    return 0.0;
    
  return chan->CalibrateENG(front_charge);

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

//void TSharcHit::CalibrateSharcHit(TChannel *chan){
//	std::string name = chan->name;
//	if(name.compare(0,3,"PAD")==0)	
//		p_energy = chan->CalibrateENG((int)pad_charge);
//	else
//		d_energy =  chan->CalibrateENG((int)front_charge);
//}


