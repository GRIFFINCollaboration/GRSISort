#include <TMath.h>

#include "TSharc.h"
#include <TClass.h>


ClassImp(TSharc)

// various sharc dimensions in mm

//const int TSharc::frontstripslist[16]     = {16,16,16,16,	24,24,24,24,	24,24,24,24,	16,16,16,16};
//const int TSharc::backstripslist[16]      = {24,24,24,24,	48,48,48,48,	48,48,48,48,	24,24,24,24};		

//const double TSharc::frontpitchlist[16]   = {2.0,2.0,2.0,2.0,	3.0,3.0,3.0,3.0,	3.0,3.0,3.0,3.0,	2.0,2.0,2.0,2.0};
//const double TSharc::backpitchlist[16]    = {PI/48,PI/48,PI/48,PI/48,	1.0,1.0,1.0,1.0,	1.0,1.0,1.0,1.0,	PI/48,PI/48,PI/48,PI/48};    // QQQ back pitches are angles
//const double TSharc::stripFpitch          = TSharc::Ydim / TSharc::frontstripslist[5]; // 72.0/24 = 3.0 mm
//const double TSharc::ringpitch            = TSharc::Rdim / TSharc::frontstripslist[1]; // 32.0/16 = 2.0 mm
//const double TSharc::stripBpitch          = TSharc::Zdim / TSharc::backstripslist[5] ; // 48.0/48 = 1.0 mm
//const double TSharc::segmentpitch         = TSharc::Pdim / TSharc::backstripslist[1] ; // 81.6/24 = 3.4 degrees (angular pitch)


//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

TSharc::TSharc() : data(0)	{
   Class()->IgnoreTObjectStreamer(true);
}

TSharc::~TSharc()	{
	if(data) delete data;
}


void TSharc::FillData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
	 if(!data)
			data = new TSharcData();
   if(mnemonic->arraysubposition.compare(0,1,"E")==0) {//PAD
      data->SetPAD(frag,channel,mnemonic);
   } else if(mnemonic->arraysubposition.compare(0,1,"D")==0) {//not a PAD
		if(mnemonic->collectedcharge.compare(0,1,"P")==0) { //front
			data->SetFront(frag,channel,mnemonic);
		} else {  //back
			data->SetBack(frag,channel,mnemonic);
		}
	}
	TSharcData::Set();
}

void	TSharc::BuildHits(TGRSIDetectorData *ddata,Option_t *opt)	{
   TSharcData *sdata = (TSharcData*)ddata;
   if(sdata==0)
     sdata = (this->data); 
	if(!sdata)
		return;


  //  after the data has been taken from the fragement tree, the data
  //  is stored/correlated breifly in by the tsharcdata class - these 
  //  function takes the data out of tsharcdata, and puts it into the 
  //  the tsharchits class.  These tsharchit objects are how we access
  //  the data stored in the tsharc branch in the analysis tree. 
  //
  //  pcb.
  //
  //int fCfdBuildDiff = 5; // largest acceptable time difference between events (clock ticks)  (50 ns)

	//printf("Building Hits!. options = %s\n",opt);


	//printf("frontsize = %i   |  backsize = %i  \n",sdata->GetMultiplicityFront(),sdata->GetMultiplicityBack());


	if((sdata->GetMultiplicityFront()!=1) || (sdata->GetMultiplicityBack()!=1) )   ///need to remove this soon and replaces with time gates in the loops below
		return;																							 //// !!! pcb. and add better building condition.
		

	//printf("Building sharc hits.\n");

   for(int i=0;i<sdata->GetMultiplicityFront();i++)	{	
      for(int j=0;j<sdata->GetMultiplicityBack();j++)	{	
         if(sdata->GetFront_DetectorNbr(i) != sdata->GetBack_DetectorNbr(j)) {
            continue;
         }
			   if(std::abs((long long)(sdata->GetFront_Charge(i) - sdata->GetBack_Charge(j))) > 6000)//naive charge cut keeps >99.9% of data.
				    continue;

		/*	
			printf("pair(%i,%i) Detector Number: %i\n",i,j,data->GetFront_DetectorNbr(i));
			printf("Front[%i]:%.02f  - Back[%i]:%.02f  = %.02f\n",
					data->GetFront_StripNbr(i),data->GetFront_Energy(i),
					data->GetBack_StripNbr(j),data->GetBack_Energy(j),
					data->GetFront_Energy(i) - data->GetBack_Energy(j) );
			printf("Front[%i] CSQ2 = %.02f    |     Back[%i] CSQ2 = %.02f\n",
					data->GetFront_StripNbr(i),data->GetFront_EngChi2(i),
					data->GetBack_StripNbr(j),data->GetBack_EngChi2(j) );
			TVector3 position = TSharc::GetPosition(data->GetFront_DetectorNbr(i),data->GetFront_StripNbr(i),data->GetBack_StripNbr(j));
			printf("X: %.02f   Y: %.02f   Z: %.02f\n",position.X(),position.Y(),position.Z());
		*/	
			TSharcHit hit; 

         hit.SetDetector(sdata->GetFront_DetectorNbr(i));				

         hit.SetDeltaFrontE(sdata->GetFront_Energy(i));
         hit.SetDeltaFrontT(sdata->GetFront_Time(i));
         hit.SetFrontCharge(sdata->GetFront_Charge(i));
         hit.SetFrontAddress(sdata->GetFront_ChannelAddress(i));

         hit.SetDeltaBackE(sdata->GetBack_Energy(j));
         hit.SetDeltaBackT(sdata->GetBack_Time(j));
         hit.SetBackCharge(sdata->GetBack_Charge(j));
         hit.SetBackAddress(sdata->GetBack_ChannelAddress(j));

			   hit.SetFrontStrip(sdata->GetFront_StripNbr(i));
			   hit.SetBackStrip(sdata->GetBack_StripNbr(j));

         hit.SetPosition(TSharc::GetPosition(hit.GetDetectorNumber(),
                                             hit.GetFrontStrip(),
                                             hit.GetBackStrip()));
         
        	this->sharc_hits.push_back(hit);

         //inline void SetPadE(const Double_t &tenergy)		{	p_energy = tenergy;	}	//!
         //inline void SetPadT(const Double_t &ttime)		{	p_time = ttime;	}		//!
         //inline void SetPadCharge(const Int_t &charge)	{ pad_charge = charge;}		//!

		}
		//printf("---------------------\n");
   }
	//printf("\n\n");

   for(int k=0;k<sdata->GetMultiplicityPAD();k++)	{	
      for(int l=0;l<sharc_hits.size();l++)	{
         if(sdata->GetPAD_DetectorNbr(k) != sharc_hits.at(l).GetDetectorNumber())
		      continue;
			sharc_hits.at(l).SetPadAddress(sdata->GetPAD_ChannelAddress(k)); 
			sharc_hits.at(l).SetPadE(sdata->GetPAD_Energy(k)); 
			sharc_hits.at(l).SetPadT(sdata->GetPAD_Time(k)); 
			sharc_hits.at(l).SetPadCharge(sdata->GetPAD_Charge(k)); 
     }
   }

}

/*  this really ought to be done with enregy,
int TSharc::CombineHits(TSharcHit *hit1,TSharcHit *hit2,int position1,int position2 )	{
	// used in the build hits routine to combine to hits 
	// in adcent pixels into one hit.

	//printf("here 1\n");	
	if(!hit1 || !hit2)
		return -1;

	double hit1_front_weight = hit1->GetFrontChgDbl()/(hit1->GetFrontChgDbl() + hit2->GetFrontChgDbl());
	if(hit1_front_weight > 0.051)
		return position1;
	double hit2_front_weight = hit2->GetFrontChgDbl()/(hit1->GetFrontChgDbl() + hit2->GetFrontChgDbl());
	if(hit2_front_weight > 0.051)
		return position2;

	double hit1_back_weight = hit1->GetBackChgDbl()/(hit1->GetBackChgDbl() + hit2->GetBackChgDbl());
	if(hit1_back_weight > 0.051)
		return position1;
	double hit2_back_weight = hit2->GetBackChgDbl()/(hit1->GetBackChgDbl() + hit2->GetBackChgDbl());
	if(hit2_back_weight > 0.051)
		return position2;


	if(hit1->GetFrontCharge() == hit2->GetFrontCharge()) {  //  same front strip;
		//printf("here!!\n");
		TVector3 newposition;
		newposition.SetX(hit1->GetPosition().X()*hit1_back_weight + hit2->GetPosition().X()*hit2_back_weight);
		newposition.SetY(hit1->GetPosition().X()*hit1_back_weight + hit2->GetPosition().X()*hit2_back_weight);
		newposition.SetZ(hit1->GetPosition().X()*hit1_back_weight + hit2->GetPosition().X()*hit2_back_weight);
		if( (hit1->GetFrontCharge()-hit1->GetBackCharge()) < (hit2->GetFrontCharge()-hit2->GetBackCharge()) )	{
		//if(hit1_back_weight > hit2_back_weight){
			//hit1->SetBackCharge(hit1->GetBackCharge() + hit2->GetBackCharge());
			hit1->SetPosition(newposition);
			return position2;
		}
		else {
			//hit2->SetBackCharge(hit1->GetBackCharge() + hit2->GetBackCharge());
			hit2->SetPosition(newposition);
			return position1;
		}
	}
	else if(hit1->GetBackCharge() == hit2->GetBackCharge()) {   //  same back strip;
		TVector3 newposition;
		newposition.SetX(hit1->GetPosition().X()*hit1_front_weight + hit2->GetPosition().X()*hit2_front_weight);
		newposition.SetY(hit1->GetPosition().X()*hit1_front_weight + hit2->GetPosition().X()*hit2_front_weight);
		newposition.SetZ(hit1->GetPosition().X()*hit1_front_weight + hit2->GetPosition().X()*hit2_front_weight);
		if( (hit1->GetFrontCharge()-hit1->GetBackCharge()) < (hit2->GetFrontCharge()-hit2->GetBackCharge()) )	{
		//if(hit1_front_weight > hit2_front_weight)	{
			//hit1->SetFrontCharge(hit1->GetFrontCharge() + hit2->GetFrontCharge());
			hit1->SetPosition(newposition);
			return position2;
		}
		else {
			//hit2->SetFrontCharge(hit1->GetFrontCharge() + hit2->GetFrontCharge());
			hit2->SetPosition(newposition);
			return position1;
		}
	}
	else	{
		return 0xffffffff;
	}

}
*/


void TSharc::RemoveHits(std::vector<TSharcHit> *hits,std::set<int> *to_remove)	{

	std::set<int>::reverse_iterator iter;
	for(iter= to_remove->rbegin(); iter != to_remove->rend(); iter++)	{
		if(*iter == 0xffffffff)
			continue;
		hits->erase(hits->begin()+*iter);

	}
}

void TSharc::Clear(Option_t *option)	{
  if(data) data->Clear();
  sharc_hits.clear();
  return;
}

void TSharc::Print(Option_t *option)	{
  printf("not yet written...\n");
  return;
}

double TSharc::Xdim        = +72.0; // total X dimension of all boxes
double TSharc::Ydim        = +72.0; // total Y dimension of all boxes
double TSharc::Zdim        = +48.0; // total Z dimension of all boxes
double TSharc::Rdim        = +32.0; // Rmax-Rmin for all QQQs 
double TSharc::Pdim        = +81.6; // QQQ quadrant angular range (degrees)
double TSharc::XposUB      = +42.5;
double TSharc::YminUB      = -36.0;
double TSharc::ZminUB      = -5.00;
double TSharc::XposDB      = +40.5;
double TSharc::YminDB      = -36.0;
double TSharc::ZminDB      = +9.00;
double TSharc::ZposUQ      = -66.5;
double TSharc::RminUQ      = +9.00;
double TSharc::PminUQ      = +2.00; // degrees
double TSharc::ZposDQ      = +74.5;
double TSharc::RminDQ      = +9.00;
double TSharc::PminDQ      = +6.40; // degrees

//const int TSharc::frontstripslist[16]     = {16,16,16,16,	24,24,24,24,	24,24,24,24,	16,16,16,16};
//const int TSharc::backstripslist[16]      = {24,24,24,24,	48,48,48,48,	48,48,48,48,	24,24,24,24};		

//const double TSharc::frontpitchlist[16]   = {2.0,2.0,2.0,2.0,	3.0,3.0,3.0,3.0,	3.0,3.0,3.0,3.0,	2.0,2.0,2.0,2.0};
//const double TSharc::backpitchlist[16]    = {PI/48,PI/48,PI/48,PI/48,	1.0,1.0,1.0,1.0,	1.0,1.0,1.0,1.0,	PI/48,PI/48,PI/48,PI/48}; 
// QQQ back pitches are angles
//
double TSharc::stripFpitch          = TSharc::Ydim / 24.0;  //TSharc::frontstripslist[5]; // 72.0/24 = 3.0 mm
double TSharc::ringpitch            = TSharc::Rdim / 16.0;  //TSharc::frontstripslist[1]; // 32.0/16 = 2.0 mm
double TSharc::stripBpitch          = TSharc::Zdim / 48.0;  //TSharc::backstripslist[5] ; // 48.0/48 = 1.0 mm
double TSharc::segmentpitch         = TSharc::Pdim / 24.0;  //TSharc::backstripslist[1] ; // 81.6/24 = 3.4 degrees (angular pitch)

// The dimensions are described for a single detector of each type UQ,UB,DB,DQ, and all other detectors can be calculated by rotating this
TVector3 TSharc::GetPosition(int detector, int frontstrip, int backstrip, double X, double Y, double Z)	{
  int FrontDet = detector;
  int FrontStr = frontstrip;
  int BackDet  = detector;
  int BackStr  = backstrip;
  int nrots = 0; // allows us to rotate into correct position
  double x = 0;
  double y = 0;
  double z = 0;

  TVector3 position;
  TVector3 position_offset;
  position_offset.SetXYZ(X,Y,Z);

  if(FrontDet>=5 && FrontDet<=8){ //forward box
    nrots = FrontDet-4;                                // edited to make box 5 on the ceiling.  assuming rotaing ccw around the +z axis!!
    x = XposDB;                                                                      // ?? x stays the same. first detector is aways defined in the y-z plane.
    y = - (YminDB + (FrontStr+0.5)*stripFpitch);       // [(-36.0) - (+36.0)]        // ?? add minus sign, reversve the order of the strips on the ds section.
    z = ZminDB + (BackStr+0.5)*stripBpitch;            // [(+9.0) - (+57.0)]    
		position.SetXYZ(x,y,z);
  }
  else if(FrontDet>=9 && FrontDet<=12){ //backward box
    nrots = FrontDet-8; 							   										       // edited to make box 5 on the ceiling.  assuming rotaing ccw around the +z axis!!
    x = XposUB;                                             
    y = YminUB + (FrontStr+0.5)*stripFpitch;           // [(-36.0) - (+36.0)] 
    z = ZminUB - (BackStr+0.5)*stripBpitch;            // [(-5.0) - (-53.0)]
		position.SetXYZ(x,y,z);
  }
  else if(FrontDet>=13){ // backward (upstream) QQQ
    nrots = FrontDet-13;
    double z = ZposUQ;
    double rho = RminUQ + (FrontStr+0.5)*ringpitch;    // [(+9.0) - (+41.0)] 
    double phi = (PminUQ + (BackStr+0.5)*segmentpitch)*PI/180.0;  // [(+2.0) - (+83.6)] 
    position.SetXYZ(rho*TMath::Sin(phi),rho*TMath::Cos(phi),z);   
  }
  else if(FrontDet<=4){ // forward (downstream) QQQ
    nrots = FrontDet-1;
    double z = ZposDQ;
    double rho = RminDQ + (FrontStr+0.5)*ringpitch;    // [(+9.0) - (+41.0)] 
    double phi = (PminDQ + (BackStr+0.5)*segmentpitch)*PI/180.0;  // [(+6.4) - (+88.0)] 
    position.SetXYZ(rho*TMath::Sin(phi),rho*TMath::Cos(phi),z);    
  }  

  position.RotateZ(PI*nrots/2);
  return (position + position_offset);
}










