#include <TMath.h>
#include "TCSM.h"


ClassImp(TCSM)

// various csm dimensions in mm
//const int TCSM::frontstripslist[16]     = {16,16,16,16,	24,24,24,24,	24,24,24,24,	16,16,16,16};
//const int TCSM::backstripslist[16]      = {24,24,24,24,	48,48,48,48,	48,48,48,48,	24,24,24,24};		
//const double TCSM::frontpitchlist[16]   = {2.0,2.0,2.0,2.0,	3.0,3.0,3.0,3.0,	3.0,3.0,3.0,3.0,	2.0,2.0,2.0,2.0};
//const double TCSM::backpitchlist[16]    = {PI/48,PI/48,PI/48,PI/48,	1.0,1.0,1.0,1.0,	1.0,1.0,1.0,1.0,	PI/48,PI/48,PI/48,PI/48};    // QQQ back pitches are angles
//const double TCSM::stripFpitch          = TCSM::Ydim / TCSM::frontstripslist[5]; // 72.0/24 = 3.0 mm
//const double TCSM::ringpitch            = TCSM::Rdim / TCSM::frontstripslist[1]; // 32.0/16 = 2.0 mm
//const double TCSM::stripBpitch          = TCSM::Zdim / TCSM::backstripslist[5] ; // 48.0/48 = 1.0 mm
//const double TCSM::segmentpitch         = TCSM::Pdim / TCSM::backstripslist[1] ; // 81.6/24 = 3.4 degrees (angular pitch)


//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//


int TCSM::fCfdBuildDiff = 5;

TCSM::TCSM() : data(0)	{
   Class()->IgnoreTObjectStreamer(true);
  //InitializeSRIMInputs();	
}

TCSM::~TCSM()	{
   if(data) delete data;
}




void TCSM::FillData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
   if(!data)
      data = new TCSMData();
   if(mnemonic->collectedcharge.compare(0,1,"N")==0 ) { //Horizontal Strips. aka "front"
      data->SetHorizontal(frag,channel,mnemonic);
      //cout<<":";
   } else if(mnemonic->collectedcharge.compare(0,1,"P")==0 ) { //Vertical Strips. aka "back"
      data->SetVertical(frag,channel,mnemonic);
      //cout<<".";
   }
}

void	TCSM::BuildHits(TGRSIDetectorData *ddata, Option_t *opt)	{
  TCSMData *cdata = (TCSMData*)ddata;
  if(cdata==0)
     cdata = (this->data); 
  if(!cdata)
     return;

  //cdata->Print();


  //  after the data has been taken from the fragement tree, the data
  //  is stored/correlated breifly in by the tcsmdata class - these 
  //  function takes the data out of tcsmdata, and puts it into the 
  //  the tcsmhits class.  These tcsmhit objects are how we access
  //  the data stored in the tcsm branch in the analysis tree. 
  //
  //  pcb.
  //
  std::string option = opt;
  TCSMHit csmhit;
  std::vector<TCSMHit> D_Hits;
  std::vector<TCSMHit> E_Hits;

  //int fCfdBuildDiff = 5; // largest acceptable time difference between events (clock ticks)  (50 ns)
  /*
    if(GetMultiplicityHorizontal()>1 || GetMultiplicityVertical()>1)
    return;
  */
  std::vector<bool> HorUsed;
  std::vector<bool> VerUsed;


  HorUsed.assign(cdata->GetMultiplicityHorizontal(), false);
  VerUsed.assign(cdata->GetMultiplicityVertical(), false);

  double total_ver_energy = 0.0;
  double total_hor_energy = 0.0;

  int total_ver_hits = 0;
  int total_hor_hits = 0;



  for(int i=0;i<cdata->GetMultiplicityHorizontal();i++)	{	
    if(HorUsed.at(i)) continue;
    total_hor_energy += cdata->GetHorizontal_Energy(i);
    total_hor_hits++;
    for(int j=0;j<cdata->GetMultiplicityVertical();j++)	{	
      if(VerUsed.at(j)) continue;
      total_ver_energy += cdata->GetVertical_Energy(i);
      total_ver_hits++;
      if(cdata->GetHorizontal_DetectorNbr(i) == cdata->GetVertical_DetectorNbr(j))	{ //check if same detector
			if(cdata->GetHorizontal_DetectorPos(i) == cdata->GetVertical_DetectorPos(j)) { //check the are from the same position!		
	  			//if(abs(cdata->GetHorizontal_Energy(i)-cdata->GetVertical_Energy(j))>.200 ) // && ((total_hor_hits+total_ver_hits)==3) ) {
	    			//continue;

			  //if(abs(GetHorizontal_TimeCFD(i)-GetVertical_TimeCFD(j)) > fCfdBuildDiff) {
	  			//	continue; // ensure there is front-back time correlation to protect against noise/false coinc.
	  			//}
	  			csmhit.Clear();
	  			if(cdata->GetHorizontal_DetectorPos(i) == 'D') {  //i decided this is going to mean Delta.
	    //printf ("D%i  Hor[%i] - Ver[%i] = " DBLUE  "%.02f - %02f " RESET_COLOR " = " DYELLOW " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Time(i),GetVertical_Time(j),GetHorizontal_Time(i)-GetVertical_Time(j));
	    //printf ("D%i  Hor[%i] - Ver[%i] = " DRED  "%.02f - %02f " RESET_COLOR " = " DGREEN " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Energy(i),GetVertical_Energy(j),GetHorizontal_Energy(i)-GetVertical_Energy(j));
			   	csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(i));		//!

				   csmhit.SetDHorizontalCharge(cdata->GetHorizontal_Charge(i)); 				//! 
			      csmhit.SetDVerticalCharge(cdata->GetVertical_Charge(j));    			//!

			      csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(i)); 			//!
			      csmhit.SetDVerticalStrip(cdata->GetVertical_StripNbr(j));   			//!
						
			      csmhit.SetDHorizontalCFD(cdata->GetHorizontal_TimeCFD(i));					//!
			      csmhit.SetDVerticalCFD(cdata->GetVertical_TimeCFD(j));					//!
					
			      csmhit.SetDHorizontalTime(cdata->GetHorizontal_Time(i));	//!
			      csmhit.SetDVerticalTime(cdata->GetVertical_Time(j));		//!
			
			      csmhit.SetDHorizontalEnergy(cdata->GetHorizontal_Energy(i));				//!
			      csmhit.SetDVerticalEnergy(cdata->GetVertical_Energy(j));				//!
			
						if(cdata->GetHorizontal_DetectorNbr(i)==2)
						{
						  if(cdata->GetHorizontal_StripNbr(i)==9 || 
						     cdata->GetHorizontal_StripNbr(i)==10 || 
						     cdata->GetHorizontal_StripNbr(i)==11)
						     {
						        csmhit.SetDHorizontalCharge(cdata->GetVertical_Charge(j));
						        csmhit.SetDHorizontalEnergy(cdata->GetVertical_Energy(j));
						     }
			      }
			      
			      if(cdata->GetHorizontal_DetectorNbr(i)==3)
						{
						  if(cdata->GetHorizontal_StripNbr(i)==12 || 
						     cdata->GetHorizontal_StripNbr(i)==15)
						     {
						        csmhit.SetDHorizontalCharge(cdata->GetVertical_Charge(j));
						        csmhit.SetDHorizontalEnergy(cdata->GetVertical_Energy(j));
						     }
			      }
			      
			      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i),
                                                     cdata->GetHorizontal_DetectorPos(i),
                                                     cdata->GetHorizontal_StripNbr(i),
                                                     cdata->GetVertical_StripNbr(j)));

  	          VerUsed.at(j) = true;
	            HorUsed.at(i) = true;
	            D_Hits.push_back(csmhit);
	  			} else if(cdata->GetHorizontal_DetectorPos(i) == 'E') {
	    //printf ("E%i  Hor[%i] - Ver[%i] = " DBLUE  "%.02f - %02f " RESET_COLOR " = " DYELLOW " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Time(i),GetVertical_Time(j),GetHorizontal_Time(i)-GetVertical_Time(j));
	    //printf ("E%i  Hor[%i] - Ver[%i] = " DRED  "%.02f - %02f " RESET_COLOR " = " DGREEN " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Energy(i),GetVertical_Energy(j),GetHorizontal_Energy(i)-GetVertical_Energy(j));
	    // printf("Here.\n");
	    			csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(i));		//!
							
	    			csmhit.SetEHorizontalCharge(cdata->GetHorizontal_Charge(i)); 				//! 
	    			//cout<<"  E horiz charge: "<< cdata->GetHorizontal_Charge(i)<<endl;
	    			csmhit.SetEVerticalCharge(cdata->GetVertical_Charge(j));    			//!
						//cout<<"  E vertical charge: "<< cdata->GetVertical_Charge(i)<<endl;
						
	    			csmhit.SetEHorizontalStrip(cdata->GetHorizontal_StripNbr(i)); 			//!
	    			csmhit.SetEVerticalStrip(cdata->GetVertical_StripNbr(j));   			//!
					
	    			csmhit.SetEHorizontalCFD(cdata->GetHorizontal_TimeCFD(i));					//!
	    			csmhit.SetEVerticalCFD(cdata->GetVertical_TimeCFD(j));					//!
				
	    			csmhit.SetEHorizontalTime(cdata->GetHorizontal_Time(i)); //!
	    			csmhit.SetEVerticalTime(cdata->GetVertical_Time(j));     //!

	    			csmhit.SetEHorizontalEnergy(cdata->GetHorizontal_Energy(i));				//!
	    			//cout<<"E Horiz Energy: "<<cdata->GetHorizontal_Energy(i)<<endl;
	    			csmhit.SetEVerticalEnergy(cdata->GetVertical_Energy(j));				//!
	    			//cout<<"E Verti Energy: "<<cdata->GetVertical_Energy(i)<<endl;
	    			
	    			csmhit.SetEPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i),
                                                     cdata->GetHorizontal_DetectorPos(i),
                                                     cdata->GetHorizontal_StripNbr(i),
                                                     cdata->GetVertical_StripNbr(j)));
  	    		VerUsed.at(j) = true;
  	    		HorUsed.at(i) = true;
	    			E_Hits.push_back(csmhit);
			  }
			}
      }
    }
  }


   std::vector<int>HorStrpFree;
  std::vector<int> VerStrpFree;
  

  for(int i=0;i<cdata->GetMultiplicityHorizontal();i++)	{	
     if(!HorUsed.at(i))  		
      HorStrpFree.push_back(i);
  }	
  for(int j=0;j<cdata->GetMultiplicityVertical();j++)	{	
     if(!VerUsed.at(j)) //&& GetVertical_Energy(j)>.100 )  		
  		VerStrpFree.push_back(j);
  }


    for(int iter=0;iter<VerStrpFree.size();iter++)
    {
      int addr = VerStrpFree.at(iter);
      //cout<<"addr: "<<addr<<endl;
      if(cdata->GetVertical_DetectorNbr(addr)==1 && cdata->GetVertical_DetectorPos(addr)=='D')
      {
            //cout<<"Here1"<<endl;
    		   	csmhit.SetDetectorNumber(cdata->GetVertical_DetectorNbr(addr));		//!

				    csmhit.SetDHorizontalCharge(cdata->GetVertical_Charge(addr)); 				//! 
			      csmhit.SetDVerticalCharge(cdata->GetVertical_Charge(addr));    			//!
						
			      csmhit.SetDHorizontalStrip(6); 			//!
			      csmhit.SetDVerticalStrip(cdata->GetVertical_StripNbr(addr));   			//!
						
			      csmhit.SetDHorizontalCFD(cdata->GetVertical_TimeCFD(addr));					//!
			      csmhit.SetDVerticalCFD(cdata->GetVertical_TimeCFD(addr));					//!
					
			      csmhit.SetDHorizontalTime(cdata->GetVertical_Time(addr));	//!
			      csmhit.SetDVerticalTime(cdata->GetVertical_Time(addr));		//!
			
			      csmhit.SetDHorizontalEnergy(cdata->GetVertical_Energy(addr));				//!
			      csmhit.SetDVerticalEnergy(cdata->GetVertical_Energy(addr));				//!
			            //cout<<"Here2"<<endl;
			      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetVertical_DetectorNbr(addr),
                                                     cdata->GetVertical_DetectorPos(addr),
                                                     6,
                                                     cdata->GetVertical_StripNbr(addr)));
                                                                 //cout<<"Here3"<<endl;
	            D_Hits.push_back(csmhit);
	                        //cout<<"Here4"<<endl;
	    }

	  }


    for(int iter=0;iter<HorStrpFree.size();iter++)
    {
      int addr = HorStrpFree.at(iter);
      //cout<<"addr: "<<addr<<endl;
      if(cdata->GetHorizontal_DetectorPos(addr)=='D')
      {
	if(cdata->GetHorizontal_DetectorNbr(addr)==2 && cdata->GetHorizontal_StripNbr(addr)!=9)
	{
	  csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(addr));		//!
	  
	  csmhit.SetDHorizontalCharge(cdata->GetHorizontal_Charge(addr)); 				//!
	  csmhit.SetDVerticalCharge(cdata->GetHorizontal_Charge(addr));    			//!
	  
	  csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(addr)); 			//!
	  csmhit.SetDVerticalStrip(6);   			//!
	  
	  csmhit.SetDHorizontalCFD(cdata->GetHorizontal_TimeCFD(addr));					//!
	  csmhit.SetDVerticalCFD(cdata->GetHorizontal_TimeCFD(addr));					//!
	  
	  csmhit.SetDHorizontalTime(cdata->GetHorizontal_Time(addr));	//!
	  csmhit.SetDVerticalTime(cdata->GetHorizontal_Time(addr));		//!
	  
	  csmhit.SetDHorizontalEnergy(cdata->GetHorizontal_Energy(addr));				//!
	  csmhit.SetDVerticalEnergy(cdata->GetHorizontal_Energy(addr));				//!
	  
	  csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(addr),
						cdata->GetHorizontal_DetectorPos(addr),
						cdata->GetHorizontal_StripNbr(addr),
						6));
	  D_Hits.push_back(csmhit);
	}
        if(cdata->GetHorizontal_DetectorNbr(addr)==3)
        {
    		   	csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(addr));		//!

				    csmhit.SetDHorizontalCharge(cdata->GetHorizontal_Charge(addr)); 				//! 
			      csmhit.SetDVerticalCharge(cdata->GetHorizontal_Charge(addr));    			//!
						
			      csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(addr)); 			//!
			      csmhit.SetDVerticalStrip(11);   			//!
						
			      csmhit.SetDHorizontalCFD(cdata->GetHorizontal_TimeCFD(addr));					//!
			      csmhit.SetDVerticalCFD(cdata->GetHorizontal_TimeCFD(addr));					//!
					
			      csmhit.SetDHorizontalTime(cdata->GetHorizontal_Time(addr));	//!
			      csmhit.SetDVerticalTime(cdata->GetHorizontal_Time(addr));		//!
			
			      csmhit.SetDHorizontalEnergy(cdata->GetHorizontal_Energy(addr));				//!
			      csmhit.SetDVerticalEnergy(cdata->GetHorizontal_Energy(addr));				//!

			      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(addr),
                                                     cdata->GetHorizontal_DetectorPos(addr),
                                                     cdata->GetHorizontal_StripNbr(addr),
                                                     11));
	            D_Hits.push_back(csmhit);
	      }
	      
	      else if(cdata->GetHorizontal_DetectorNbr(addr)==4)
        {
    		   	csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(addr));		//!

				    csmhit.SetDHorizontalCharge(cdata->GetHorizontal_Charge(addr)); 				//! 
			      csmhit.SetDVerticalCharge(cdata->GetHorizontal_Charge(addr));    			//!
						
			      csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(addr)); 			//!
			      csmhit.SetDVerticalStrip(15);   			//!
						
			      csmhit.SetDHorizontalCFD(cdata->GetHorizontal_TimeCFD(addr));					//!
			      csmhit.SetDVerticalCFD(cdata->GetHorizontal_TimeCFD(addr));					//!
					
			      csmhit.SetDHorizontalTime(cdata->GetHorizontal_Time(addr));	//!
			      csmhit.SetDVerticalTime(cdata->GetHorizontal_Time(addr));		//!
			
			      csmhit.SetDHorizontalEnergy(cdata->GetHorizontal_Energy(addr));				//!
			      csmhit.SetDVerticalEnergy(cdata->GetHorizontal_Energy(addr));				//!

			      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(addr),
                                                     cdata->GetHorizontal_DetectorPos(addr),
                                                     cdata->GetHorizontal_StripNbr(addr),
                                                     15));
	            D_Hits.push_back(csmhit);
	      }
	    }
	  }
  //if(HorStrpFree.size()+VerStrpFree.size()>0)
    //cout<<" HorStrpFree: "<<HorStrpFree.size()<<" VerStrpFree: "<<VerStrpFree.size()<<endl;
/*
  if(HorStrpFree.size() == 2 && VerStrpFree.size() == 1) { 
	  int i1 = HorStrpFree.at(0);
	  int i2 = HorStrpFree.at(1);
	  int j1 = VerStrpFree.at(0);

	  if((cdata->GetHorizontal_DetectorNbr(i1) == cdata->GetVertical_DetectorNbr(j1)) && 
		  (cdata->GetHorizontal_DetectorNbr(i2) == cdata->GetVertical_DetectorNbr(j1))    )	{ //check if same detector
	  	if((cdata->GetHorizontal_DetectorPos(i2) == cdata->GetVertical_DetectorPos(j1)) &&
		   (cdata->GetHorizontal_DetectorPos(i2) == cdata->GetVertical_DetectorPos(j1))) { //check the are from the same position!		
	  			if(abs((cdata->GetHorizontal_Energy(i1)+cdata->GetHorizontal_Energy(i2))-cdata->GetVertical_Energy(j1))<.500 ) {
				//printf( BLUE "hits are good!" RESET_COLOR "\n");
	  			csmhit.Clear();
	  			if(cdata->GetHorizontal_DetectorPos(i1) == 'D') {  //i decided this is going to mean Delta.
	    //printf ("D%i  Hor[%i] - Ver[%i] = " DBLUE  "%.02f - %02f " RESET_COLOR " = " DYELLOW " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Time(i),GetVertical_Time(j),GetHorizontal_Time(i)-GetVertical_Time(j));
	    //printf ("D%i  Hor[%i] - Ver[%i] = " DRED  "%.02f - %02f " RESET_COLOR " = " DGREEN " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Energy(i),GetVertical_Energy(j),GetHorizontal_Energy(i)-GetVertical_Energy(j));
			   	csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(i1));		//!

				   csmhit.SetDHorizontalCharge(cdata->GetHorizontal_Charge(i1)); 				//! 
			      csmhit.SetDVerticalCharge(cdata->GetHorizontal_Charge(i1));    			//!
						
			      csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(i1)); 			//!
			      csmhit.SetDVerticalStrip(cdata->GetVertical_StripNbr(j1));   			//!
						
			      csmhit.SetDHorizontalCFD(cdata->GetHorizontal_TimeCFD(i1));					//!
			      csmhit.SetDVerticalCFD(cdata->GetHorizontal_TimeCFD(i1));					//!
					
			      csmhit.SetDHorizontalTime(cdata->GetHorizontal_Time(i1));	//!
			      csmhit.SetDVerticalTime(cdata->GetHorizontal_Time(i1));		//!
			
			      csmhit.SetDHorizontalEnergy(cdata->GetHorizontal_Energy(i1));				//!
			      csmhit.SetDVerticalEnergy(cdata->GetHorizontal_Energy(i1));				//!
			
			      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i1),
                                                     cdata->GetHorizontal_DetectorPos(i1),
                                                     cdata->GetHorizontal_StripNbr(i1),
                                                     cdata->GetVertical_StripNbr(j1)));
	            D_Hits.push_back(csmhit);

					csmhit.Clear();
			   	csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(i2));		//!

				   csmhit.SetDHorizontalCharge(cdata->GetHorizontal_Charge(i2)); 				//! 
			      csmhit.SetDVerticalCharge(cdata->GetHorizontal_Charge(i2));    			//!
						
			      csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(i2)); 			//!
			      csmhit.SetDVerticalStrip(cdata->GetVertical_StripNbr(j1));   			//!
						
			      csmhit.SetDHorizontalCFD(cdata->GetHorizontal_TimeCFD(i2));					//!
			      csmhit.SetDVerticalCFD(cdata->GetHorizontal_TimeCFD(i2));					//!
					
			      csmhit.SetDHorizontalTime(cdata->GetHorizontal_Time(i2));	//!
			      csmhit.SetDVerticalTime(cdata->GetHorizontal_Time(i2));		//!
			
			      csmhit.SetDHorizontalEnergy(cdata->GetHorizontal_Energy(i2));				//!
			      csmhit.SetDVerticalEnergy(cdata->GetHorizontal_Energy(i2));				//!
			
			      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i2),
                                                     cdata->GetHorizontal_DetectorPos(i2),
                                                     cdata->GetHorizontal_StripNbr(i2),
                                                     cdata->GetVertical_StripNbr(j1)));
	            D_Hits.push_back(csmhit);

	  			} else if(cdata->GetHorizontal_DetectorPos(i1) == 'E') {
	    //printf ("E%i  Hor[%i] - Ver[%i] = " DBLUE  "%.02f - %02f " RESET_COLOR " = " DYELLOW " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Time(i),GetVertical_Time(j),GetHorizontal_Time(i)-GetVertical_Time(j));
	    //printf ("E%i  Hor[%i] - Ver[%i] = " DRED  "%.02f - %02f " RESET_COLOR " = " DGREEN " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Energy(i),GetVertical_Energy(j),GetHorizontal_Energy(i)-GetVertical_Energy(j));
	    // printf("Here.\n");
	    			csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(i1));		//!
							
	    			csmhit.SetEHorizontalCharge(cdata->GetHorizontal_Charge(i1)); 				//! 
	    			csmhit.SetEVerticalCharge(cdata->GetHorizontal_Charge(i1));    			//!
						
	    			csmhit.SetEHorizontalStrip(cdata->GetHorizontal_StripNbr(i1)); 			//!
	    			csmhit.SetEVerticalStrip(cdata->GetVertical_StripNbr(j1));   			//!
					
	    			csmhit.SetEHorizontalCFD(cdata->GetHorizontal_TimeCFD(i1));					//!
	    			csmhit.SetEVerticalCFD(cdata->GetHorizontal_TimeCFD(i1));					//!
				
	    			csmhit.SetEHorizontalTime(cdata->GetHorizontal_Time(i1)); //!
	    			csmhit.SetEVerticalTime(cdata->GetHorizontal_Time(i1));     //!

	    			csmhit.SetEHorizontalEnergy(cdata->GetHorizontal_Energy(i1));				//!
	    			csmhit.SetEVerticalEnergy(cdata->GetHorizontal_Energy(i1));				//!

	    			csmhit.SetEPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i1),
                                                     cdata->GetHorizontal_DetectorPos(i1),
                                                     cdata->GetHorizontal_StripNbr(i1),
                                                     cdata->GetVertical_StripNbr(j1)));
	    			E_Hits.push_back(csmhit);
		
					csmhit.Clear();

	    			csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(i2));		//!
							
	    			csmhit.SetEHorizontalCharge(cdata->GetHorizontal_Charge(i2)); 				//! 
	    			csmhit.SetEVerticalCharge(cdata->GetHorizontal_Charge(i2));    			//!
						
	    			csmhit.SetEHorizontalStrip(cdata->GetHorizontal_StripNbr(i2)); 			//!
	    			csmhit.SetEVerticalStrip(cdata->GetVertical_StripNbr(j1));   			//!
					
	    			csmhit.SetEHorizontalCFD(cdata->GetHorizontal_TimeCFD(i2));					//!
	    			csmhit.SetEVerticalCFD(cdata->GetHorizontal_TimeCFD(i2));					//!
				
	    			csmhit.SetEHorizontalTime(cdata->GetHorizontal_Time(i2)); //!
	    			csmhit.SetEVerticalTime(cdata->GetHorizontal_Time(i2));     //!

	    			csmhit.SetEHorizontalEnergy(cdata->GetHorizontal_Energy(i2));				//!
	    			csmhit.SetEVerticalEnergy(cdata->GetHorizontal_Energy(i2));				//!

	    			csmhit.SetEPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i2),
                                                     cdata->GetHorizontal_DetectorPos(i2),
                                                     cdata->GetHorizontal_StripNbr(i2),
                                                     cdata->GetVertical_StripNbr(j1)));
	    			E_Hits.push_back(csmhit);

			  }
			}
		}
		}
	} else if(HorStrpFree.size() == 1 && VerStrpFree.size() == 2) {


	  int i1 = HorStrpFree.at(0);
	  int j1 = VerStrpFree.at(0);
	  int j2 = VerStrpFree.at(1);

	  if((cdata->GetHorizontal_DetectorNbr(i1) == cdata->GetVertical_DetectorNbr(j1)) && 
		  (cdata->GetHorizontal_DetectorNbr(i1) == cdata->GetVertical_DetectorNbr(j2))    )	{ //check if same detector
	  	if((cdata->GetHorizontal_DetectorPos(i1) == cdata->GetVertical_DetectorPos(j1)) &&
		   (cdata->GetHorizontal_DetectorPos(i1) == cdata->GetVertical_DetectorPos(j2))) { //check the are from the same position!		
	  			if(abs(cdata->GetHorizontal_Energy(i1)-(cdata->GetVertical_Energy(j1)+cdata->GetVertical_Energy(j2)))<.500 ) { 
				//printf( BLUE "hits are good!" RESET_COLOR "\n");
	  				csmhit.Clear();
	  				if(cdata->GetHorizontal_DetectorPos(i1) == 'D') {  //i decided this is going to mean Delta.
	    //printf ("D%i  Hor[%i] - Ver[%i] = " DBLUE  "%.02f - %02f " RESET_COLOR " = " DYELLOW " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Time(i),GetVertical_Time(j),GetHorizontal_Time(i)-GetVertical_Time(j));
	    //printf ("D%i  Hor[%i] - Ver[%i] = " DRED  "%.02f - %02f " RESET_COLOR " = " DGREEN " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Energy(i),GetVertical_Energy(j),GetHorizontal_Energy(i)-GetVertical_Energy(j));
			   	csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(i1));		//!

				   csmhit.SetDHorizontalCharge(cdata->GetVertical_Charge(j1)); 				//! 
			      csmhit.SetDVerticalCharge(cdata->GetVertical_Charge(j1));    			//!
						
			      csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(i1)); 			//!
			      csmhit.SetDVerticalStrip(cdata->GetVertical_StripNbr(j1));   			//!
						
			      csmhit.SetDHorizontalCFD(cdata->GetVertical_TimeCFD(j1));					//!
			      csmhit.SetDVerticalCFD(cdata->GetVertical_TimeCFD(j1));					//!
					
			      csmhit.SetDHorizontalTime(cdata->GetVertical_Time(j1));	//!
			      csmhit.SetDVerticalTime(cdata->GetVertical_Time(j1));		//!
			
			      csmhit.SetDHorizontalEnergy(cdata->GetVertical_Energy(j1));				//!
			      csmhit.SetDVerticalEnergy(cdata->GetVertical_Energy(j1));				//!
			
			      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i1),
                                                     cdata->GetHorizontal_DetectorPos(i1),
                                                     cdata->GetHorizontal_StripNbr(i1),
                                                     cdata->GetVertical_StripNbr(j1)));
	            D_Hits.push_back(csmhit);

					csmhit.Clear();
			   	csmhit.SetDetectorNumber(cdata->GetVertical_DetectorNbr(j2));		//!

				   csmhit.SetDHorizontalCharge(cdata->GetVertical_Charge(j2)); 				//! 
			      csmhit.SetDVerticalCharge(cdata->GetVertical_Charge(j2));    			//!
						
			      csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(i1)); 			//!
			      csmhit.SetDVerticalStrip(cdata->GetVertical_StripNbr(j2));   			//!
						
			      csmhit.SetDHorizontalCFD(cdata->GetVertical_TimeCFD(j2));					//!
			      csmhit.SetDVerticalCFD(cdata->GetVertical_TimeCFD(j2));					//!
					
			      csmhit.SetDHorizontalTime(cdata->GetVertical_Time(j2));	//!
			      csmhit.SetDVerticalTime(cdata->GetVertical_Time(j2));		//!
			
			      csmhit.SetDHorizontalEnergy(cdata->GetVertical_Energy(j2));				//!
			      csmhit.SetDVerticalEnergy(cdata->GetVertical_Energy(j2));				//!
			
			      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i1),
                                                     cdata->GetHorizontal_DetectorPos(i1),
                                                     cdata->GetHorizontal_StripNbr(i1),
                                                     cdata->GetVertical_StripNbr(j2)));
	            D_Hits.push_back(csmhit);

	  			} else if(cdata->GetHorizontal_DetectorPos(i1) == 'E') {
	    //printf ("E%i  Hor[%i] - Ver[%i] = " DBLUE  "%.02f - %02f " RESET_COLOR " = " DYELLOW " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Time(i),GetVertical_Time(j),GetHorizontal_Time(i)-GetVertical_Time(j));
	    //printf ("E%i  Hor[%i] - Ver[%i] = " DRED  "%.02f - %02f " RESET_COLOR " = " DGREEN " %.02f" RESET_COLOR "\n",GetHorizontal_DetectorNbr(i),i,j,GetHorizontal_Energy(i),GetVertical_Energy(j),GetHorizontal_Energy(i)-GetVertical_Energy(j));
	    // printf("Here.\n");
	    			csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(i1));		//!
							
	    			csmhit.SetEHorizontalCharge(cdata->GetVertical_Charge(j1)); 				//! 
	    			csmhit.SetEVerticalCharge(cdata->GetVertical_Charge(j1));    			//!
						
	    			csmhit.SetEHorizontalStrip(cdata->GetHorizontal_StripNbr(i1)); 			//!
	    			csmhit.SetEVerticalStrip(cdata->GetVertical_StripNbr(j1));   			//!
					
	    			csmhit.SetEHorizontalCFD(cdata->GetVertical_TimeCFD(j1));					//!
	    			csmhit.SetEVerticalCFD(cdata->GetVertical_TimeCFD(j1));					//!
				
	    			csmhit.SetEHorizontalTime(cdata->GetVertical_Time(j1)); //!
	    			csmhit.SetEVerticalTime(cdata->GetVertical_Time(j1));     //!

	    			csmhit.SetEHorizontalEnergy(cdata->GetVertical_Energy(j1));				//!
	    			csmhit.SetEVerticalEnergy(cdata->GetVertical_Energy(j1));				//!

	    			csmhit.SetEPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i1),
                                                     cdata->GetHorizontal_DetectorPos(i1),
                                                     cdata->GetHorizontal_StripNbr(i1),
                                                     cdata->GetVertical_StripNbr(j1)));
	    			E_Hits.push_back(csmhit);
		
					csmhit.Clear();

	    			csmhit.SetDetectorNumber(cdata->GetVertical_DetectorNbr(j2));		//!
							
	    			csmhit.SetEHorizontalCharge(cdata->GetVertical_Charge(j2)); 				//! 
	    			csmhit.SetEVerticalCharge(cdata->GetVertical_Charge(j2));    			//!
						
	    			csmhit.SetEHorizontalStrip(cdata->GetHorizontal_StripNbr(i1)); 			//!
	    			csmhit.SetEVerticalStrip(cdata->GetVertical_StripNbr(j2));   			//!
					
	    			csmhit.SetEHorizontalCFD(cdata->GetVertical_TimeCFD(j2));					//!
	    			csmhit.SetEVerticalCFD(cdata->GetVertical_TimeCFD(j2));					//!
				
	    			csmhit.SetEHorizontalTime(cdata->GetVertical_Time(j2)); //!
	    			csmhit.SetEVerticalTime(cdata->GetVertical_Time(j2));     //!

	    			csmhit.SetEHorizontalEnergy(cdata->GetVertical_Energy(j2));				//!
	    			csmhit.SetEVerticalEnergy(cdata->GetVertical_Energy(j2));				//!

	    			csmhit.SetEPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(i1),
                                                     cdata->GetHorizontal_DetectorPos(i1),
                                                     cdata->GetHorizontal_StripNbr(i1),
                                                     cdata->GetVertical_StripNbr(j2)));
	    			E_Hits.push_back(csmhit);

			  }
			}
		}
		
	}




	 	
  }
*/
  //now we will try to match front and back detectors.
  std::vector<bool> usedpixel(E_Hits.size(), false);

  for(int i=0; i<D_Hits.size();i++) {
    //D_Hits.at(i).Print(); 
    for(int j=0;j<E_Hits.size();j++) {
      //E_Hits.at(j).Print(); 
      if(usedpixel.at(j)) {
        //printf(" I AM HERE 1\n");
        continue; 
      }
      if(D_Hits.at(i).GetDetectorNumber() == E_Hits.at(j).GetDetectorNumber()) {
    		if((D_Hits.at(i).GetDPosition() - E_Hits.at(j).GetEPosition()).Mag()>10.0) {
          //printf(" I AM HERE 2\n");
      		continue;
        }
    		usedpixel.at(j) = true;
    
    		D_Hits.at(i).SetEHorizontalCharge(E_Hits.at(j).GetEHorizontalCharge()); 
    		D_Hits.at(i).SetEVerticalCharge(E_Hits.at(j).GetEVerticalCharge());
    
    		D_Hits.at(i).SetEHorizontalStrip(E_Hits.at(j).GetEHorizontalStrip()); 
    		D_Hits.at(i).SetEVerticalStrip(E_Hits.at(j).GetEVerticalStrip()); 
    
    		D_Hits.at(i).SetEHorizontalCFD(E_Hits.at(j).GetEHorizontalCFD());
    		D_Hits.at(i).SetEVerticalCFD(E_Hits.at(j).GetEVerticalCFD());
    
    		D_Hits.at(i).SetEHorizontalEnergy(E_Hits.at(j).GetEHorizontalEnergy());
    		D_Hits.at(i).SetEVerticalEnergy(E_Hits.at(j).GetEVerticalEnergy());
    
    		D_Hits.at(i).SetEHorizontalTime(E_Hits.at(j).GetEHorizontalTime());
    		D_Hits.at(i).SetEVerticalTime(E_Hits.at(j).GetEVerticalTime());                  
    		D_Hits.at(i).SetEPosition(E_Hits.at(j).GetEPosition());
    
        //D_Hits.at(i).Print(); 
      } // comparison of detector numbers
    } // loop over e hits
    csm_hits.push_back(D_Hits.at(i));
  }
  for(int k=0;k<usedpixel.size();k++) { 
    if(!usedpixel.at(k)) {
      csm_hits.push_back(E_Hits.at(k));
    }
  }
}



void TCSM::Clear(Option_t *option)	{

  //cout << "clearing " << endl;
  if(data) data->Clear();
  csm_hits.clear();
  //cout <<" size: " << csm_hits.size() << endl;
  return;
}

void TCSM::Print(Option_t *option)	{
  printf("not yet written...\n");
  return;
}


TVector3 TCSM::GetPosition(int detector,char pos, int horizontalstrip, int verticalstrip, double X, double Y, double Z)	{


  //horizontal strips collect N charge!
  //vertical strips collect P charge!

  TVector3 Pos;
  double SideX = 68;
  double SideZ = -4.8834;

  double dEX = 54.9721;
  double dEZ = 42.948977;

  double EX = 58.062412;
  double EZ = 48.09198;

  double detTheta = 31. * (TMath::Pi()/180.);

  double x = 0.0,y = 0.0,z = 0.0;

  if(detector==3&&pos=='D') {
    //Right Side
    verticalstrip=15-verticalstrip;

    x = SideX;
    z = SideZ + (50./32.)*(2*verticalstrip+1);
  } else if(detector==4&&pos=='D') {
    //Left Side


    x = -SideX;
    z = SideZ + (50./32.)*(2*verticalstrip+1);
  } else if(detector==1&&pos=='D') {
    //Right dE
    verticalstrip=15-verticalstrip;

    x = dEX - (50./32.)*cos(detTheta)*(2*verticalstrip+1);
    z = dEZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
  } else if(detector==2&&pos=='D') {
    //Left dE


    x = -dEX + (50./32.)*cos(detTheta)*(2*verticalstrip+1);
    z = dEZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
  } else if(detector==1&&pos=='E') {
    //Right E


    x = EX - (50./32.)*cos(detTheta)*(2*verticalstrip+1);
    z = EZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
  } else if(detector==2&&pos=='E') {
    //Left E
    verticalstrip=15-verticalstrip;

    x = -EX + (50./32.)*cos(detTheta)*(2*verticalstrip+1);
    z = EZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
  } else {
    printf("***Error, unrecognized detector and position combo!***\n");
  }

  y = (50./32.)*(2*horizontalstrip+1) - (50/16.)*8;
  Pos.SetX(x + X); 
  Pos.SetY(y + Y);
  Pos.SetZ(z+ Z);

  return(Pos);
}










