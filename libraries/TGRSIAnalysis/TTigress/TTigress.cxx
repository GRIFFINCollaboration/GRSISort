
#include <iostream>

#include "TTigress.h"

#include <TRandom.h>
#include <TMath.h>
#include <TClass.h>

ClassImp(TTigress)

//double TTigress::beta = 0.00;

bool TTigress::fSetSegmentHits = false;//true;
bool TTigress::fSetBGOHits = true;

bool TTigress::fSetCoreWave = false;
bool TTigress::fSetSegmentWave = false;
bool TTigress::fSetBGOWave = false;


TTigress::TTigress() : tigdata(0), bgodata(0)	{
   Class()->IgnoreTObjectStreamer(true);
   Clear();
}

TTigress::~TTigress()	{
  	if(tigdata) delete tigdata;
   if(bgodata) delete bgodata;
}

void TTigress::Clear(Option_t *opt)	{

	if(tigdata) tigdata->Clear();
	if(bgodata) bgodata->Clear();

	tigress_hits.clear();
	addback_hits.clear();
	clover_addback_hits.clear();

}


void TTigress::Print(Option_t *opt)	{
  //printf("not yet written...\n");
  //printf(DYELLOW "TTigress::beta  =  %.04f" RESET_COLOR  "\n",beta);
  return;
}


void TTigress::FillData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
	 if(!frag || !channel || !mnemonic)
	   return;

   if(!tigdata)   
      tigdata = new TTigressData();
   UShort_t color =5;   
   if(mnemonic->arraysubposition.compare(0,1,"B")==0) {
	    color = 0;      
   } else if(mnemonic->arraysubposition.compare(0,1,"G")==0) {
	    color = 1;
   } else if(mnemonic->arraysubposition.compare(0,1,"R")==0) {
	    color = 2;
   } else if(mnemonic->arraysubposition.compare(0,1,"W")==0) {
	    color = 3;
   } 
   if(mnemonic->subsystem.compare(0,1,"G")==0) { 
	  	if((mnemonic->segment==0) || (mnemonic->segment==9 ))	{
 				tigdata->SetCore(frag,channel,mnemonic);
			} else {                         
				//if(SetSegmentHits()) {
 					tigdata->SetSegment(frag,channel,mnemonic);
				//}
			}
  } else if(mnemonic->subsystem.compare(0,1,"S")==0) {
      FillBGOData(frag,channel,mnemonic);
   }
	TTigressData::Set();
}


void TTigress::FillBGOData(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic) {
   if(SetBGOHits()) {
	   if(!bgodata)
  			bgodata = new TBGOData();
      bgodata->SetBGO(frag,channel,mnemonic);
   }
	TBGOData::Set();   
}



//void	TTigress::BuildHits(TTigressData *tdata,TBGOData *bdata,Option_t *opt)	{
void	TTigress::BuildHits(TGRSIDetectorData *data,Option_t *opt)	{
   TTigressData *tdata = (TTigressData*)data;
   if(tdata==0)
      tdata = (this->tigdata);
//   if(bdata==0)
   TBGOData *bdata = (this->bgodata);

   if(!tdata)
      return;

	TCrystalHit temp_crystal;

	//First build the core hits.
	for(int i=0;i<tdata->GetCoreMultiplicity();i++)	{

		TTigressHit corehit;
		temp_crystal.Clear();

		temp_crystal.SetCharge(tdata->GetCoreCharge(i));
    temp_crystal.SetEnergy(tdata->GetCoreEnergy(i));
    temp_crystal.SetTime(tdata->GetCoreTime(i));
    temp_crystal.SetCfd(tdata->GetCoreCFD(i));

		if(TTigress::SetCoreWave())	{
        	temp_crystal.SetWave(tdata->GetCoreWave(i));
		}
		
		corehit.SetCore(temp_crystal);	
		corehit.SetDetectorNumber(tdata->GetCloverNumber(i));
    corehit.SetCrystalNumber(tdata->GetCoreNumber(i));


		tigress_hits.push_back(corehit);
	}
	
	for(int i=0;i<tigress_hits.size();i++)	{
	   for(int j=0;j<tdata->GetSegmentMultiplicity();j++)	{
	      if((tigress_hits[i].GetDetectorNumber() == tdata->GetSegCloverNumber(j))  && (tigress_hits[i].GetCrystalNumber() == tdata->GetSegCoreNumber(j))) {
	         tigress_hits[i].CheckFirstHit(tdata->GetSegmentCharge(j),tdata->GetSegmentNumber(j));

				if(!SetSegmentHits()) 
					continue;
			
           temp_crystal.Clear();
           temp_crystal.SetSegmentNumber(tdata->GetSegmentNumber(j));
           temp_crystal.SetCharge(tdata->GetSegmentCharge(j));
           temp_crystal.SetEnergy(tdata->GetSegmentEnergy(j));
           temp_crystal.SetTime(tdata->GetSegmentTime(j));
           temp_crystal.SetCfd(tdata->GetSegmentCFD(j));

           if(TTigress::SetSegmentWave()) {
             temp_crystal.SetWave(tdata->GetSegmentWave(j));
           }
           tigress_hits.at(i).SetSegment(temp_crystal);
        }	
	   }  // all segments set.  

    if(TTigress::SetBGOHits() && bdata)  {
      for(int j=0;j< bdata->GetBGOMultiplicity();j++)  {
        if((tigress_hits[i].GetDetectorNumber() == bdata->GetBGOCloverNumber(j))  && (tigress_hits[i].GetCrystalNumber() == bdata->GetBGOCoreNumber(j))) {
          temp_crystal.Clear();
          temp_crystal.SetSegmentNumber(bdata->GetBGOPmNbr(j));
          temp_crystal.SetCharge(bdata->GetBGOCharge(j));
          //temp_crystal.SetEnergy(bdata->GetBGOEnergy(j));
          temp_crystal.SetTime(bdata->GetBGOTime(j));
          temp_crystal.SetCfd(bdata->GetBGOCFD(j));
          if(TTigress::SetBGOWave()) {
            temp_crystal.SetWave(bdata->GetBGOWave(j));
          }			
	        tigress_hits.at(i).SetBGO(temp_crystal);	
        }
      }
    }  // all bgo's set.

	//	tigress_hits.at(i).SetPosition(GetPosition(&(tigress_hits.at(i))));
//FIX

		//DopplerCorrect(&(tigress_hits.at(i)));
	}

	//if(tigress_hits.size()>1){
		BuildAddBack();
		BuildCloverAddBack();
	//}
}


//void TTigress::DopplerCorrect(TTigressHit *hit)	{
//	if(beta != 0.00)	{
//		double gamma = 1/(sqrt(1-pow(beta,2)));
//		double tmp = hit->GetEnergy()*gamma *(1 - beta*hit->GetPosition().CosTheta());

//		hit->SetDoppler(tmp);
//	}
//	else {
//		printf(DRED "\n\tWARNING!  Try to Doppler correct before setting beta!" RESET_COLOR "\n");
//	}
//}




TVector3 TTigress::GetPosition(TTigressHit *hit, int dist)  {
			return TTigress::GetPosition(hit->GetDetectorNumber(),hit->GetCrystalNumber(),hit->GetInitialHit());	
}

TVector3 TTigress::GetPosition(int DetNbr,int CryNbr,int SegNbr, int dist)	{

			TVector3 det_pos;
			double xx = 0;
			double yy = 0;
			double zz = 0;
			
			//printf("xx = %f\nyy = %f\n zz = %f\n",GeBlue_Position[DetNbr][SegNbr][0],GeBlue_Position[DetNbr][SegNbr][1],GeBlue_Position[DetNbr][SegNbr][2]);
				

			switch(CryNbr)	{
				case -1:
					break;
				case 0:
					xx = GeBlue_Position[DetNbr][SegNbr][0];
					yy = GeBlue_Position[DetNbr][SegNbr][1];
					zz = GeBlue_Position[DetNbr][SegNbr][2];
					break;
				case 1:
					xx = GeGreen_Position[DetNbr][SegNbr][0]; 
					yy = GeGreen_Position[DetNbr][SegNbr][1]; 
					zz = GeGreen_Position[DetNbr][SegNbr][2]; 
					break;
				case 2:
					xx = GeRed_Position[DetNbr][SegNbr][0]; 
					yy = GeRed_Position[DetNbr][SegNbr][1]; 
					zz = GeRed_Position[DetNbr][SegNbr][2];  
					break;
				case 3:
					xx = GeWhite_Position[DetNbr][SegNbr][0]; 
					yy = GeWhite_Position[DetNbr][SegNbr][1]; 
					zz = GeWhite_Position[DetNbr][SegNbr][2]; 
					break;
			};
			//printf("xx = %f\nyy = %f\n zz = %f\n",xx,yy,zz);
			det_pos.SetXYZ(xx,yy,zz);

			return det_pos;


			/// this function is 

		/*
			double x_offset = 0;
			double y_offset = 0;
			double z_offset = 0;

			switch(SegNbr)	{
				case -1:
					break;
				case 0:
				case 9:
					x_offset = 27.2;	
					y_offset = 27.2;
					z_offset = 45.0;
					break;
				case 1:
					x_offset = 20.9+10.45;
					y_offset = 20.9+10.45;
					z_offset = 18.1;
					break;
				case 2:
					x_offset = 10.45;
					y_offset = 20.9+10.45;
					z_offset = 18.1;
					break;
				case 3:
					x_offset = 10.45;
					y_offset = 10.45;
					z_offset = 18.1;	
					break;
				case 4:
					x_offset =  20.9 + 10.45;	
					y_offset = 10.45;
					z_offset = 18.1;
					break;
				case 5:
					x_offset = 27.2 + 13.6;
					y_offset = 27.2 + 13.6;
					z_offset = 63.1;
					break;
				case 6:
					x_offset = 13.6;
					y_offset = 27.2 + 13.6;
					z_offset = 63.1;	
					break;
				case 7:
					x_offset = 13.6;
					y_offset = 13.6;
					z_offset = 63.1;
					break;
				case 8:
					x_offset = 27.2 + 13.6;
					y_offset = 13.6;
					z_offset = 63.1;
					break;
				default:
					break;
			};


			if(CryNbr<0 || CryNbr >3) 
				return det_pos;

			TRotation Cry_Rot;	
			Cry_Rot.RotateZ(90*CryNbr*TMath::DegToRad());

			TRotation rot;
			TRotation rot_theta;
			TRotation rot_phi;
			bool need_to_be_mirrored = false;
			double theta;
			double phi;
			switch(DetNbr)	{
				case 1:
					phi = 22.5;
					theta = 45.0;
					break;
				case 2:
					phi = 112.5;
					theta = 45.0;
					need_to_be_mirrored = true;
					break;
				case 3:
					phi = 202.5;
					theta = 45.0;
					need_to_be_mirrored = true;
					break;

				case 4:
					phi = 292.5;
					theta = 45.0;
					break;
				case 5:
					phi 	= 22.5;
					theta = 90.0;
					break;
				case 6:
					phi = 67.5;
					theta = 90.0;
					break;
				case 7:
					phi = 112.5;
					theta = 90.0;
					need_to_be_mirrored = true;
					break;
				case 8:
					phi = 157.5;
					theta = 90.0;
					need_to_be_mirrored = true;
					break;
				case 9:
					phi = 202.5;		
					theta = 90.0;
					need_to_be_mirrored = true;
					break;
				case 10:
					phi = 247.5;		
					theta = 90.0;
					need_to_be_mirrored = true;
					break;
				case 11:
					phi = 292.5;		
					theta = 90.0;
					break;
				case 12:
					phi = 337.5;		
					theta = 90.0;
					break;
					break;
				case 13:
					phi = 22.5;
					theta = 145.0;
					break;
				case 14:
					phi = 112.5;
					theta = 145.0;
					need_to_be_mirrored = true;
					break;
				case 15:
					phi = 202.5;
					theta = 145.0;
					need_to_be_mirrored = true;
					break;
				case 16:
					phi = 292.5;
					theta = 145.0;
					break;
			};

			if(need_to_be_mirrored)	{
				Cry_Rot.RotateZ(180*TMath::DegToRad());
			}

			det_pos.SetXYZ(x_offset,y_offset,R+z_offset);
		//	rot_theta.RotateY(theta*TMath::DegToRad());
			rot_theta.RotateX(theta*TMath::DegToRad());
		//	rot_phi.RotateZ((phi)*TMath::DegToRad());
			rot_phi.RotateZ((phi)*TMath::DegToRad());
			rot =   Cry_Rot * rot_phi * rot_theta; //

			//det_pos.Transform(rot_theta);
			//det_pos.Transform(rot);
		*/

		/*
			std::cout << det_pos.X() <<"\t" << det_pos.Y() << "\t" << det_pos.Z() << std::endl;
			det_pos.SetXYZ(x_offset,y_offset,R+z_offset);
			rot_theta.RotateY(theta*TMath::DegToRad());
			rot_phi.RotateZ(-phi*TMath::DegToRad());
			rot =    rot_phi * rot_theta; //Cry_Rot *
			*/
		//	det_pos.Transform(rot_phi);
		//	det_pos.Transform(rot_theta);

			/*
		*/
			//det_pos.SetZ(-det_pos.Z());
		//	std::cout << det_pos.X() <<"\t" << det_pos.Y() << "\t" << det_pos.Z() << std::endl;

		//	return det_pos;

		}


void TTigress::BuildCloverAddBack(Option_t *opt)	{ 

	if(this->tigress_hits.size() == 0)
    	return;

	clover_addback_hits.clear();

	if(this->GetMultiplicity() == 1) 
      clover_addback_hits.push_back(*(this->GetTigressHit(0)));
	else{
      clover_addback_hits.push_back(*(this->GetTigressHit(0)));
		clover_addback_hits.at(0).Add(&(clover_addback_hits.at(0)));

      for(int i = 1; i<this->GetMultiplicity(); i++)   {
		 	bool used = false;
			 for(int j =0; j<clover_addback_hits.size();j++)    {
		     	int d_time = abs(clover_addback_hits.at(j).GetTime() - this->GetTigressHit(i)->GetTime());

		
				if( clover_addback_hits.at(j).GetDetectorNumber() == this->GetTigressHit(i)->GetDetectorNumber() )	{
				     if( (d_time < 11) )    { // gate hard coded to 110ns.
		 		        used = true;
		     		     clover_addback_hits.at(j).Add(this->GetTigressHit(i));
		         	  break;
			     	}
				}

		 	}
			 if(!used) {
		 	    clover_addback_hits.push_back(*(this->GetTigressHit(i)));
		     	 clover_addback_hits.back().Add(&(clover_addback_hits.back()));
			 }
		}

	}

}

void TTigress::BuildAddBack(Option_t *opt)	{ 

	if(this->tigress_hits.size() == 0)
    	return;

	addback_hits.clear();

	if(this->GetMultiplicity() == 1) 
      addback_hits.push_back(*(this->GetTigressHit(0)));
	else{

		addback_hits.push_back(*(this->GetTigressHit(0)));
		addback_hits.at(0).Add(&(addback_hits.at(0)));

		for(int i = 1; i<this->GetMultiplicity(); i++)   {
		 	bool used = false;
			 for(int j =0; j<addback_hits.size();j++)    {
		 	   TVector3 res = addback_hits.at(j).GetLastHit() - this->GetTigressHit(i)->GetPosition();
		     	int d_time = abs(addback_hits.at(j).GetTime() - this->GetTigressHit(i)->GetTime());

				int seg1 = std::get<2>(addback_hits.at(j).GetLastPosition());
				int seg2 = this->GetTigressHit(i)->GetInitialHit();
		
				if( (seg1<5 && seg2<5) || (seg1>4 && seg2>4) )	{   // not front to back
				     if( (res.Mag() < 54) && (d_time < 110) )    {  // time gate == 110  ns  pos gate == 54mm
		 		        used = true;
		     		    addback_hits.at(j).Add(this->GetTigressHit(i));
		         		break;
			     	}
				}
				else if( (seg1<5 && seg2>4) || (seg1>4 && seg2<5) )	{ // front to back
				     if( (res.Mag() < 105) && (d_time < 110) )    {     // time gate == 110 ns pos gate == 105mm.
		 		        used = true;
		     		    addback_hits.at(j).Add(this->GetTigressHit(i));
		         		break;
			     	}
				}




		 	}
			 if(!used) {
		 	    addback_hits.push_back(*(this->GetTigressHit(i)));
		     	addback_hits.back().Add(&(addback_hits.back()));
			 }
		}

	}

}







		double TTigress::GeBlue_Position[17][9][3] = { 
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 139.75, 87.25, 27.13 }, { 107.47, 84.35, 36.80 }, { 107.64, 84.01, 12.83 }, { 116.86, 63.25, 13.71 }, { 116.66, 62.21, 36.42 }, { 146.69, 104.60, 40.50 }, { 146.58, 104.81, 14.96 }, { 156.50, 80.77, 14.73 }, { 156.44, 80.99, 40.74 } },
		{ { 37.12, 160.51, 27.13 }, { 16.35, 135.64, 36.80 }, { 16.71, 135.51, 12.83 }, { 37.91, 127.36, 13.71 }, { 38.50, 126.48, 36.42 }, { 29.76, 177.69, 40.50 }, { 29.53, 177.76, 14.96 }, { 53.55, 167.78, 14.73 }, { 53.35, 167.89, 40.74 } },
		{ {-87.25, 139.75, 27.13}, {-84.35, 107.47, 36.80}, {-84.01, 107.64, 12.83}, {-63.25, 116.86, 13.71}, {-62.21, 116.66, 36.42}, {-104.60, 146.69, 40.50}, {-104.81, 146.58, 14.96}, {-80.77, 156.50, 14.73}, {-80.99, 156.44, 40.74} },
		{ {-160.51, 37.12, 27.13}, {-135.64, 16.35, 36.80}, {-135.51, 16.71, 12.83}, {-127.36, 37.91, 13.71}, {-126.48, 38.50, 36.42}, {-177.69, 29.76, 40.50}, {-177.76, 29.53, 14.96}, {-167.78, 53.55, 14.73}, {-167.89, 53.35, 40.74} },
		{ {-139.75, -87.25, 27.13}, {-107.47, -84.35, 36.80}, {-107.64, -84.01, 12.83}, {-116.86, -63.25, 13.71}, {-116.66, -62.21, 36.42}, {-146.69, -104.60, 40.50}, {-146.58, -104.81, 14.96}, {-156.50, -80.77, 14.73}, {-156.44, -80.99, 40.74} },
		{ {-37.12, -160.51, 27.13}, {-16.35, -135.64, 36.80}, {-16.71, -135.51, 12.83}, {-37.91, -127.36, 13.71}, {-38.50, -126.48, 36.42}, {-29.76, -177.69, 40.50}, {-29.53, -177.76, 14.96}, {-53.55, -167.78, 14.73}, {-53.35, -167.89, 40.74} },
		{ {87.25, -139.75, 27.13}, {84.35, -107.47, 36.80}, {84.01, -107.64, 12.83}, {63.25, -116.86, 13.71}, {62.21, -116.66, 36.42}, {104.60, -146.69, 40.50}, {104.81, -146.58, 14.96}, {80.77, -156.50, 14.73}, {80.99, -156.44, 40.74} },
		{ {160.51, -37.12, 27.13}, {135.64, -16.35, 36.80}, {135.51, -16.71, 12.83}, {127.36, -37.91, 13.71}, {126.48, -38.50, 36.42}, {177.69, -29.76, 40.50}, {177.76, -29.53, 14.96}, {167.78, -53.55, 14.73}, {167.89, -53.35, 40.74} },
		{ {113.50, 76.38, -95.72}, {95.91, 79.56, -67.01}, {80.41, 72.73, -83.98}, {90.05, 52.14, -83.76}, {104.85, 57.32, -67.30}, {125.64, 95.88, -95.49}, {108.85, 89.19, -113.54}, {118.64, 65.08, -113.68}, {135.56, 72.34, -95.31} },
		{ {-76.38, 113.5, -95.72}, {-79.56, 95.91, -67.01}, {-72.73, 80.41, -83.98}, {-52.14, 90.05, -83.76}, {-57.32, 104.85, -67.30}, {-95.88, 125.64, -95.49}, {-89.19, 108.85, -113.54}, {-65.08, 118.64, -113.68}, {-72.34, 135.56, -95.31} },
		{ {-113.50, -76.38, -95.72}, {-95.91, -79.56, -67.01}, {-80.41, -72.73, -83.98}, {-90.05, -52.14, -83.76}, {-104.85, -57.32, -67.30}, {-125.64, -95.88, -95.49}, {-108.85, -89.19, -113.54}, {-118.64, -65.08, -113.68}, {-135.56, -72.34, -95.31} },
		{ {76.38, -113.50, -95.72}, {79.56, -95.91, -67.01}, {72.73, -80.41, -83.98}, {52.14, -90.05, -83.76}, {57.32, -104.85, -67.30}, {95.88, -125.64, -95.49}, {89.19, -108.85, -113.54}, {65.08, -118.64, -113.68}, {72.34, -135.56, -95.31} }
		};

		//Assuming this is the 1
		double TTigress::GeGreen_Position[17][9][3] = { 
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 139.75, 87.25, -27.13 }, { 107.47, 84.35, -36.80 }, { 116.66, 62.21, -36.42 }, { 116.86, 63.25, -13.71 }, { 107.64, 84.01, -12.83 }, { 146.69, 104.60, -40.50 }, { 156.44, 80.99, -40.74 }, { 156.50, 80.77, -14.73 }, { 146.58, 104.81, -14.96 } },
		{ { 37.12, 160.51, -27.13 }, { 16.35, 135.64, -36.80 }, { 38.50, 126.48, -36.42 }, { 37.91, 127.36, -13.71 }, { 16.71, 135.51, -12.83 }, { 29.76, 177.69, -40.50 }, { 53.35, 167.89, -40.74 }, { 53.55, 167.78, -14.73 }, { 29.53, 177.76, -14.96 } },
		{ {-87.25, 139.75, -27.13}, {-84.35, 107.47, -36.80}, {-62.21, 116.66, -36.42}, {-63.25, 116.86, -13.71}, {-84.01, 107.64, -12.83}, {-104.60, 146.69, -40.50}, {-80.99, 156.44, -40.74}, {-80.77, 156.50, -14.73}, {-104.81, 146.58, -14.96} },
		{ {-160.51, 37.12, -27.13}, {-135.64, 16.35, -36.80}, {-126.48, 38.50, -36.42}, {-127.36, 37.91, -13.71}, {-135.51, 16.71, -12.83}, {-177.69, 29.76, -40.50}, {-167.89, 53.35, -40.74}, {-167.78, 53.55, -14.73}, {-177.76, 29.53, -14.96} },
		{ {-139.75, -87.25, -27.13}, {-107.47, -84.35, -36.80}, {-116.66, -62.21, -36.42}, {-116.86, -63.25, -13.71}, {-107.64, -84.01, -12.83}, {-146.69, -104.60, -40.50}, {-156.44, -80.99, -40.74}, {-156.50, -80.77, -14.73}, {-146.58, -104.81, -14.96} },
		{ {-37.12, -160.51, -27.13}, {-16.35, -135.64, -36.80}, {-38.50, -126.48, -36.42}, {-37.91, -127.36, -13.71}, {-16.71, -135.51, -12.83}, {-29.76, -177.69, -40.50}, {-53.35, -167.89, -40.74}, {-53.55, -167.78, -14.73}, {-29.53, -177.76, -14.96} },
		{ {87.25, -139.75, -27.13}, {84.35, -107.47, -36.80}, {62.21, -116.66, -36.42}, {63.25, -116.86, -13.71}, {84.01, -107.64, -12.83}, {104.60, -146.69, -40.50}, {80.99, -156.44, -40.74}, {80.77, -156.50, -14.73}, {104.81, -146.58, -14.96} },
		{ {160.51, -37.12, -27.13}, {135.64, -16.35, -36.80}, {126.48, -38.50, -36.42}, {127.36, -37.91, -13.71}, {135.51, -16.71, -12.83}, {177.69, -29.76, -40.50}, {167.89, -53.35, -40.74}, {167.78, -53.55, -14.73}, {177.76, -29.53, -14.96} },
		{ {78.05, 61.70, -134.09}, {47.83, 59.64, -119.06}, {57.26, 37.61, -118.80}, {72.14, 44.72, -103.15}, {63.65, 65.78, -102.12}, {72.73, 73.96, -152.77}, {82.33, 50.30, -152.93}, {99.39, 57.11, -134.51}, {89.31, 81.09, -134.70} },
		{ {-61.7, 78.05, -134.09}, {-59.64, 47.83, -119.06}, {-37.61, 57.26, -118.80}, {-44.72, 72.14, -103.15}, {-65.78, 63.65, -102.12}, {-73.96, 72.73, -152.77}, {-50.30, 82.33, -152.93}, {-57.11, 99.39, -134.51}, {-81.09, 89.31, -134.70} },
		{ {-78.05, -61.7, -134.09}, {-47.83, -59.64, -119.06}, {-57.26, -37.61, -118.80}, {-72.14, -44.72, -103.15}, {-63.65, -65.78, -102.12}, {-72.73, -73.96, -152.77}, {-82.33, -50.30, -152.93}, {-99.39, -57.11, -134.51}, {-89.31, -81.09, -134.70} },
		{ {61.7, -78.05, -134.09}, {59.64, -47.83, -119.06}, {37.61, -57.26, -118.80}, {44.72, -72.14, -103.15}, {65.78, -63.65, -102.12}, {73.96, -72.73, -152.77}, {50.30, -82.33, -152.93}, {57.11, -99.39, -134.51}, {81.09, -89.31, -134.70} }
		};

		//Assuming this is the 2
		double TTigress::GeRed_Position[17][9][3] = { 
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 160.51, 37.12, -27.13 }, { 135.64, 16.35, -36.80 }, { 135.51, 16.71, -12.83 }, { 127.36, 37.91, -13.71 }, { 126.48, 38.50, -36.42 }, { 177.69, 29.76, -40.50 }, { 177.76, 29.53, -14.96 }, { 167.78, 53.55, -14.73 }, { 167.89, 53.35, -40.74 } },
		{ { 87.25, 139.75, -27.13 }, { 84.35, 107.47, -36.80 }, { 84.01, 107.64, -12.83 }, { 63.25, 116.86, -13.71 }, { 62.21, 116.66, -36.42 }, { 104.60, 146.69, -40.50 }, { 104.81, 146.58, -14.96 }, { 80.77, 156.50, -14.73 }, { 80.99, 156.44, -40.74 } },
		{ {-37.12, 160.51, -27.13}, {-16.35, 135.64, -36.80}, {-16.71, 135.51, -12.83}, {-37.91, 127.36, -13.71}, {-38.50, 126.48, -36.42}, {-29.76, 177.69, -40.50}, {-29.53, 177.76, -14.96}, {-53.55, 167.78, -14.73}, {-53.35, 167.89, -40.74} },
		{ {-139.75, 87.25, -27.13}, {-107.47, 84.35, -36.80}, {-107.64, 84.01, -12.83}, {-116.86, 63.25, -13.71}, {-116.66, 62.21, -36.42}, {-146.69, 104.60, -40.50}, {-146.58, 104.81, -14.96}, {-156.50, 80.77, -14.73}, {-156.44, 80.99, -40.74} },
		{ {-160.51, -37.12, -27.13}, {-135.64, -16.35, -36.80}, {-135.51, -16.71, -12.83}, {-127.36, -37.91, -13.71}, {-126.48, -38.50, -36.42}, {-177.69, -29.76, -40.50}, {-177.76, -29.53, -14.96}, {-167.78, -53.55, -14.73}, {-167.89, -53.35, -40.74} },
		{ {-87.25, -139.75, -27.13}, {-84.35, -107.47, -36.80}, {-84.01, -107.64, -12.83}, {-63.25, -116.86, -13.71}, {-62.21, -116.66, -36.42}, {-104.60, -146.69, -40.50}, {-104.81, -146.58, -14.96}, {-80.77, -156.50, -14.73}, {-80.99, -156.44, -40.74} },
		{ {37.12, -160.51, -27.13}, {16.35, -135.64, -36.80}, {16.71, -135.51, -12.83}, {37.91, -127.36, -13.71}, {38.50, -126.48, -36.42}, {29.76, -177.69, -40.50}, {29.53, -177.76, -14.96}, {53.55, -167.78, -14.73}, {53.35, -167.89, -40.74} },
		{ {139.75, -87.25, -27.13}, {107.47, -84.35, -36.80}, {107.64, -84.01, -12.83}, {116.86, -63.25, -13.71}, {116.66, -62.21, -36.42}, {146.69, -104.60, -40.50}, {146.58, -104.81, -14.96}, {156.50, -80.77, -14.73}, {156.44, -80.99, -40.74} },
		{ {98.82, 11.57, -134.09}, {75.99, -8.35, -119.06}, {91.52, -1.51, -102.12}, {82.63, 19.39, -103.15}, {67.08, 13.90, -118.80}, {103.72, -0.87, -152.77}, {120.49, 5.81, -134.70}, {110.66, 29.90, -134.51}, {93.78, 22.65, -152.93} },
		{ {-11.57, 98.82, -134.09}, {8.35, 75.99, -119.06}, {1.51, 91.52, -102.12}, {-19.39, 82.63, -103.15}, {-13.90, 67.08, -118.80}, {0.87, 103.72, -152.77}, {-5.81, 120.49, -134.70}, {-29.90, 110.66, -134.51}, {-22.65, 93.78, -152.93} },
		{ {-98.82, -11.57, -134.09}, {-75.99, 8.35, -119.06}, {-91.52, 1.51, -102.12}, {-82.63, -19.39, -103.15}, {-67.08, -13.90, -118.80}, {-103.72, 0.87, -152.77}, {-120.49, -5.81, -134.70}, {-110.66, -29.90, -134.51}, {-93.78, -22.65, -152.93} },
		{ {11.57, -98.82, -134.09}, {-8.35, -75.99, -119.06}, {-1.51, -91.52, -102.12}, {19.39, -82.63, -103.15}, {13.90, -67.08, -118.80}, {-0.87, -103.72, -152.77}, {5.81, -120.49, -134.70}, {29.90, -110.66, -134.51}, {22.65, -93.78, -152.93} }
		};

		//Assuming this is the 3
		double TTigress::GeWhite_Position[17][9][3] = { 
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
		{ { 160.51, 37.12, 27.13 }, { 135.64, 16.35, 36.80 }, { 126.48, 38.50, 36.42 }, { 127.36, 37.91, 13.71 }, { 135.51, 16.71, 12.83 }, { 177.69, 29.76, 40.50 }, { 167.89, 53.35, 40.74 }, { 167.78, 53.55, 14.73 }, { 177.76, 29.53, 14.96 } },
		{ { 87.25, 139.75, 27.13 }, { 84.35, 107.47, 36.80 }, { 62.21, 116.66, 36.42 }, { 63.25, 116.86, 13.71 }, { 84.01, 107.64, 12.83 }, { 104.60, 146.69, 40.50 }, { 80.99, 156.44, 40.74 }, { 80.77, 156.50, 14.73 }, { 104.81, 146.58, 14.96 } },
		{ {-37.12, 160.51, 27.13}, {-16.35, 135.64, 36.80}, {-38.50, 126.48, 36.42}, {-37.91, 127.36, 13.71}, {-16.71, 135.51, 12.83}, {-29.76, 177.69, 40.50}, {-53.35, 167.89, 40.74}, {-53.55, 167.78, 14.73}, {-29.53, 177.76, 14.96} },
		{ {-139.75, 87.25, 27.13}, {-107.47, 84.35, 36.80}, {-116.66, 62.21, 36.42}, {-116.86, 63.25, 13.71}, {-107.64, 84.01, 12.83}, {-146.69, 104.60, 40.50}, {-156.44, 80.99, 40.74}, {-156.50, 80.77, 14.73}, {-146.58, 104.81, 14.96} },
		{ {-160.51, -37.12, 27.13}, {-135.64, -16.35, 36.80}, {-126.48, -38.50, 36.42}, {-127.36, -37.91, 13.71}, {-135.51, -16.71, 12.83}, {-177.69, -29.76, 40.50}, {-167.89, -53.35, 40.74}, {-167.78, -53.55, 14.73}, {-177.76, -29.53, 14.96} },
		{ {-87.25, -139.75, 27.13}, {-84.35, -107.47, 36.80}, {-62.21, -116.66, 36.42}, {-63.25, -116.86, 13.71}, {-84.01, -107.64, 12.83}, {-104.60, -146.69, 40.50}, {-80.99, -156.44, 40.74}, {-80.77, -156.50, 14.73}, {-104.81, -146.58, 14.96} },
		{ {37.12, -160.51, 27.13}, {16.35, -135.64, 36.80}, {38.50, -126.48, 36.42}, {37.91, -127.36, 13.71}, {16.71, -135.51, 12.83}, {29.76, -177.69, 40.50}, {53.35, -167.89, 40.74}, {53.55, -167.78, 14.73}, {29.53, -177.76, 14.96} },
		{ {139.75, -87.25, 27.13}, {107.47, -84.35, 36.80}, {116.66, -62.21, 36.42}, {116.86, -63.25, 13.71}, {107.64, -84.01, 12.83}, {146.69, -104.60, 40.50}, {156.44, -80.99, 40.74}, {156.50, -80.77, 14.73}, {146.58, -104.81, 14.96} },
		{ {134.26, 26.25, -95.72}, {124.08, 11.56, -67.01}, {114.67, 33.61, -67.30}, {100.55, 26.81, -83.76}, {108.28, 5.43, -83.98}, {156.64, 21.05, -95.49}, {147.01, 44.70, -95.31}, {129.91, 37.87, -113.68}, {140.03, 13.91, -113.54} },
		{ {-26.25, 134.26, -95.72}, {-11.56, 124.08, -67.01}, {-33.61, 114.67, -67.30}, {-26.81, 100.55, -83.76}, {-5.43, 108.28, -83.98}, {-21.05, 156.64, -95.49}, {-44.70, 147.01, -95.31}, {-37.87, 129.91, -113.68}, {-13.91, 140.03, -113.54} },
		{ {-134.26, -26.25, -95.72}, {-124.08, -11.56, -67.01}, {-114.67, -33.61, -67.30}, {-100.55, -26.81, -83.76}, {-108.28, -5.43, -83.98}, {-156.64, -21.05, -95.49}, {-147.01, -44.70, -95.31}, {-129.91, -37.87, -113.68}, {-140.03, -13.91, -113.54} },
		{ {26.25, -134.26, -95.72}, {11.56, -124.08, -67.01}, {33.61, -114.67, -67.30}, {26.81, -100.55, -83.76}, {5.43, -108.28, -83.98}, {21.05, -156.64, -95.49}, {44.70, -147.01, -95.31}, {37.87, -129.91, -113.68}, {13.91, -140.03, -113.54} }
		};









