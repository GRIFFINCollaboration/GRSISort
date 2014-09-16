
#include "TCSMHit.h"

ClassImp(TCSMHit)

TCSMHit::TCSMHit()	{	
   Class()->IgnoreTObjectStreamer(true);
   Clear();
}

TCSMHit::~TCSMHit()	{	}


void TCSMHit::Clear(Option_t *options)	{

   hor_d_strip 	= -1;
   hor_d_charge = 0;
   hor_d_cfd    = 0.0;
	
   ver_d_strip  = -1;
   ver_d_charge	= 0;
   ver_d_cfd	= 0.0;      
	
   hor_e_strip  = -1;
   hor_e_charge = 0;
   hor_e_cfd    = 0.0;
	
   ver_e_strip  = -1;
   ver_e_charge = 0;
   ver_e_cfd    = 0.0;      

   hor_d_energy = 0.0;   
   ver_d_energy = 0.0;   
   hor_d_time   = 0.0;   
   ver_d_time   = 0.0;   
   d_position.SetXYZ(0,0,1);
	
   hor_e_energy = 0.0;
   ver_e_energy = 0.0;
   hor_e_time   = 0.0;
   ver_e_time   = 0.0;
   e_position.SetXYZ(0,0,1);
}

void TCSMHit::Print(Option_t *options)	{
   //printf(DGREEN "[D/F/B] = %02i\t/%02i\t/%02i " RESET_COLOR "\n",GetDetectorNumber(),GetHorizontalStrip(),GetVerticalStrip());
   //printf("CSM hit charge: %f\t0x%08x\n",(double)hor_charge/125.0,hor_charge);
   //printf("CSM hit energy: %f\n",d_energy);
   //printf("CSM hit time:   %f\n",d_time);
   //printf( DGREEN "=	=	=	=	=	=	=	" RESET_COLOR "\n");
}

/*
  bool TCSMHit::Compare(TCSMHit *lhs, TCSMHit *rhs)	{
  if(lhs->GetDetectorNumber() < rhs->GetDetectorNumber())	{
  if(lhs->GetHorizontalStrip() < rhs->GetHorizontalStrip())	{
  if(lhs->GetHorizontalCFD() < rhs->GetHorizontalCFD())	{
  if(lhs->GetVerticalStrip() < rhs->GetVerticalStrip())	{
  if(lhs->GetVerticalCFD() < rhs->GetVerticalCFD())	{
  return true;
  }
  }
  }	
  }
  }
	
  return false;
	
  }*/



