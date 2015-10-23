#include "TMath.h"
#include "TSharc.h"
#include <cstdio>
#include <iostream>
#include "TSharcData.h"
#include "TClass.h"


ClassImp(TSharc)

// various sharc dimensions in mm

//const int TSharc::frontstripslist[16]     = {16,16,16,16,  24,24,24,24,  24,24,24,24,  16,16,16,16};
//const int TSharc::backstripslist[16]      = {24,24,24,24,  48,48,48,48,  48,48,48,48,  24,24,24,24};    

//const double TSharc::frontpitchlist[16]   = {2.0,2.0,2.0,2.0,  3.0,3.0,3.0,3.0,  3.0,3.0,3.0,3.0,  2.0,2.0,2.0,2.0};
//const double TSharc::backpitchlist[16]    = {TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,  1.0,1.0,1.0,1.0,  1.0,1.0,1.0,1.0,  TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48};    // QQQ back pitches are angles
//const double TSharc::stripFpitch          = TSharc::Ydim / TSharc::frontstripslist[5]; // 72.0/24 = 3.0 mm
//const double TSharc::ringpitch            = TSharc::Rdim / TSharc::frontstripslist[1]; // 32.0/16 = 2.0 mm
//const double TSharc::stripBpitch          = TSharc::Zdim / TSharc::backstripslist[5] ; // 48.0/48 = 1.0 mm
//const double TSharc::segmentpitch         = TSharc::Pdim / TSharc::backstripslist[1] ; // 81.6/24 = 3.4 degrees (angular pitch)

//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

double TSharc::X_offset    = +0.00; // 
double TSharc::Y_offset    = +0.00; // 
double TSharc::Z_offset    = +0.00; // 

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

//const int TSharc::frontstripslist[16]     = {16,16,16,16,  24,24,24,24,  24,24,24,24,  16,16,16,16};
//const int TSharc::backstripslist[16]      = {24,24,24,24,  48,48,48,48,  48,48,48,48,  24,24,24,24};    

//const double TSharc::frontpitchlist[16]   = {2.0,2.0,2.0,2.0,  3.0,3.0,3.0,3.0,  3.0,3.0,3.0,3.0,  2.0,2.0,2.0,2.0};
//const double TSharc::backpitchlist[16]    = {TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,  1.0,1.0,1.0,1.0,  1.0,1.0,1.0,1.0,  TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48}; 
// QQQ back pitches are angles
//
double TSharc::stripFpitch          = TSharc::Ydim / 24.0;  //TSharc::frontstripslist[5]; // 72.0/24 = 3.0 mm
double TSharc::ringpitch            = TSharc::Rdim / 16.0;  //TSharc::frontstripslist[1]; // 32.0/16 = 2.0 mm
double TSharc::stripBpitch          = TSharc::Zdim / 48.0;  //TSharc::backstripslist[5] ; // 48.0/48 = 1.0 mm
double TSharc::segmentpitch         = TSharc::Pdim / 24.0;  //TSharc::backstripslist[1] ; // 81.6/24 = 3.4 degrees (angular pitch)

// The dimensions are described for a single detector of each type UQ,UB,DB,DQ, and all other detectors can be calculated by rotating this

TSharc::TSharc() : data(0)  {
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TSharc::~TSharc()  {
  if(data) delete data;
}

TSharc::TSharc(const TSharc& rhs) : TGRSIDetector() {
  Class()->IgnoreTObjectStreamer(kTRUE);
  Clear("ALL");
  ((TSharc&)rhs).Copy(*this);
}


void TSharc::FillData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
   if(!data)
      data = new TSharcData();
   if(mnemonic->arraysubposition.compare(0,1,"E")==0) {//PAD
      data->SetPad(frag,channel,mnemonic);
   } else if(mnemonic->arraysubposition.compare(0,1,"D")==0) {//not a PAD
    if(mnemonic->collectedcharge.compare(0,1,"P")==0) { //front
      data->SetFront(frag,channel,mnemonic);
    } else {  //back
      data->SetBack(frag,channel,mnemonic);
    }
  }
  TSharcData::Set();
}

void  TSharc::BuildHits(TDetectorData *ddata,Option_t *opt)  {
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

  if((sdata->GetSizeFront()!=1) || (sdata->GetSizeBack()!=1) )   ///need to remove this soon and replaces with time gates in the loops below
    return;                                                      /// !!! pcb. and add better building condition.
    

  //printf("Building sharc hits.\n");
  for(size_t i=0;i<sdata->GetSizeFront();i++)  {  
    for(size_t j=0;j<sdata->GetSizeBack();j++)  {  
      if(sdata->GetFront_DetectorNbr(i) != sdata->GetBack_DetectorNbr(j)) {
        continue;
      }
      if(std::abs(sdata->GetFront_Charge(i) - sdata->GetBack_Charge(j)) > 6000)//naive charge cut keeps >99.9% of data.
        continue;

      TSharcHit hit; 

      hit.SetDetectorNumber(sdata->GetFront_DetectorNbr(i));        
      hit.SetFrontStrip(sdata->GetFront_StripNbr(i));
      hit.SetBackStrip(sdata->GetBack_StripNbr(j));

      hit.SetFront(sdata->GetFront_Fragment(i));
      hit.SetBack(sdata->GetBack_Fragment(j));

      this->sharc_hits.push_back(hit);
    }
  }
  for(size_t k=0;k<sdata->GetSizePad();k++)  {  
    for(size_t l=0;l<sharc_hits.size();l++)  {
      if(sdata->GetPad_DetectorNbr(k) != sharc_hits.at(l).GetDetectorNumber())
        continue;
      sharc_hits.at(l).SetPad(sdata->GetPad_Fragment(k)); 
    }
  }

}

void TSharc::RemoveHits(std::vector<TSharcHit> *hits,std::set<int> *to_remove)  {

  std::set<int>::reverse_iterator iter;
  for(iter= to_remove->rbegin(); iter != to_remove->rend(); iter++)  {
    if(*iter == -1)
      continue;
    hits->erase(hits->begin()+*iter);

  }
}

void TSharc::Clear(Option_t *option)  {
  TGRSIDetector::Clear(option);
  if(data) data->Clear();
    sharc_hits.clear();
  
  if(!strcmp(option,"ALL")) { 
    X_offset = 0.00;
    Y_offset = 0.00;
    Z_offset = 0.00;
  }
  return;
}

void TSharc::Print(Option_t *option) const  {
  printf("not yet written...\n");
  return;
}

void TSharc::Copy(TObject &rhs) const {
  //if(!rhs.InheritsFrom("TSharc"))
  //  return;
  TGRSIDetector::Copy((TObject&)rhs);

  ((TSharc&)rhs).sharc_hits = ((TSharc&)*this).sharc_hits;
  ((TSharc&)rhs).data     = 0;                        //((TSharc&)rhs).data;    
  ((TSharc&)rhs).X_offset = ((TSharc&)*this).X_offset;
  ((TSharc&)rhs).Y_offset = ((TSharc&)*this).Y_offset;
  ((TSharc&)rhs).Z_offset = ((TSharc&)*this).Z_offset;
  return;                                      
}                                       


TVector3 TSharc::GetPosition(int detector, int frontstrip, int backstrip, double X, double Y, double Z)  {
  int FrontDet = detector;
  int FrontStr = frontstrip;
  //int BackDet  = detector;
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
    nrots = FrontDet-8;                                             // edited to make box 5 on the ceiling.  assuming rotaing ccw around the +z axis!!
    x = XposUB;                                             
    y = YminUB + (FrontStr+0.5)*stripFpitch;           // [(-36.0) - (+36.0)] 
    z = ZminUB - (BackStr+0.5)*stripBpitch;            // [(-5.0) - (-53.0)]
    position.SetXYZ(x,y,z);
  }
  else if(FrontDet>=13){ // backward (upstream) QQQ
    nrots = FrontDet-13;
    double z = ZposUQ;
    double rho = RminUQ + (FrontStr+0.5)*ringpitch;    // [(+9.0) - (+41.0)] 
    double phi = (PminUQ + (BackStr+0.5)*segmentpitch)*TMath::Pi()/180.0;  // [(+2.0) - (+83.6)] 
    position.SetXYZ(rho*TMath::Sin(phi),rho*TMath::Cos(phi),z);   
  }
  else if(FrontDet<=4){ // forward (downstream) QQQ
    nrots = FrontDet-1;
    double z = ZposDQ;
    double rho = RminDQ + (FrontStr+0.5)*ringpitch;    // [(+9.0) - (+41.0)] 
    double phi = (PminDQ + (BackStr+0.5)*segmentpitch)*TMath::Pi()/180.0;  // [(+6.4) - (+88.0)] 
    position.SetXYZ(rho*TMath::Sin(phi),rho*TMath::Cos(phi),z);    
  }  

  position.RotateZ(TMath::Pi()*nrots/2);
  return (position + position_offset);
}

TGRSIDetectorHit* TSharc::GetHit(const Int_t& idx) {
   return GetSharcHit(idx);
}

TSharcHit* TSharc::GetSharcHit(const int& i) {
   try{
      return &sharc_hits.at(i);   
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw exit_exception(1);
   }
   return 0;
}


