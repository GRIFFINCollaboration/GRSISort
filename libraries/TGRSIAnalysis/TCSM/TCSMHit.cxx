
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

   detectornumber = 0;	//
}

void TCSMHit::Print(Option_t *options)	{
  std::cout<<"Printing TCSMHit:  Horizontal    Vertical"<<std::endl;
  std::cout<<"Detector number: "<<GetDetectorNumber()<<std::endl;
  //std::cout<<"Detector position: "<<GetDetectorPosition()<<std::endl;
  std::cout<<"D"<<std::endl;
  std::cout<<"Strip: "<<GetDHorizontalStrip()<<" "<<GetDVerticalStrip()<<std::endl;
  
  std::cout<<"Charge: "<<GetDHorizontalCharge()<<" "<<GetDVerticalCharge()<<std::endl;

  std::cout<<"CFD: "<<GetDHorizontalCFD()<<" "<<GetDVerticalCFD()<<std::endl;
  
  std::cout<<"Energy: "<<GetDHorizontalEnergy()<<" "<<GetDVerticalEnergy()<<std::endl;
  std::cout<<"E"<<std::endl;
  std::cout<<"Strip: "<<GetEHorizontalStrip()<<" "<<GetEVerticalStrip()<<std::endl;
  
  std::cout<<"Charge: "<<GetEHorizontalCharge()<<" "<<GetEVerticalCharge()<<std::endl;
  
  std::cout<<"CFD: "<<GetEHorizontalCFD()<<" "<<GetEVerticalCFD()<<std::endl;
  
  std::cout<<"Energy: "<<GetEHorizontalEnergy()<<" "<<GetEVerticalEnergy()<<std::endl;
  std::cout<<std::endl;
}

