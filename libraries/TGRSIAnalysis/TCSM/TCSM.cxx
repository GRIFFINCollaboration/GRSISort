#include <TMath.h>
#include "TCSM.h"
#define RECOVERHITS 1

ClassImp(TCSM)

int TCSM::fCfdBuildDiff = 5;

TCSM::TCSM() : data(0)
{
  Class()->IgnoreTObjectStreamer(true);
  //InitializeSRIMInputs();
  AlmostEqualWindow = .2;
}

TCSM::~TCSM()
{
  if(data) delete data;
}

void TCSM::FillData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)
{
  if(!data)
    data = new TCSMData();

  if(mnemonic->collectedcharge.compare(0,1,"N")==0)    //Horizontal Strips. aka "front"
  {
    data->SetHorizontal(frag,channel,mnemonic);
    //cout<<":";
  }
  else if(mnemonic->collectedcharge.compare(0,1,"P")==0)      //Vertical Strips. aka "back"
  {
    data->SetVertical(frag,channel,mnemonic);
    //cout<<".";
  }
}

void	TCSM::BuildHits(TGRSIDetectorData *ddata, Option_t *opt)
{
  TCSMData *cdata = (TCSMData *)ddata;

  if(cdata==0)
    cdata = (this->data);

  if(!cdata)
    return;
  //cout<<endl<<YELLOW<<"****************************************"<<RESET_COLOR<<endl;
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
  std::vector<TCSMHit> D_Hits;
  std::vector<TCSMHit> E_Hits;

  /*This is a quick thing to look at my max angle difference
  std::vector<double> angles;

  angles.push_back(abs(TCSM::GetPosition(1,'D',16,16).Theta()-TCSM::GetPosition(1,'D',16,15).Theta()));
  angles.push_back(abs(TCSM::GetPosition(1,'D',16,16).Theta()-TCSM::GetPosition(1,'D',15,16).Theta()));
  angles.push_back(abs(TCSM::GetPosition(2,'E',16,16).Theta()-TCSM::GetPosition(2,'E',16,15).Theta()));
  angles.push_back(abs(TCSM::GetPosition(2,'E',16,16).Theta()-TCSM::GetPosition(2,'E',15,16).Theta()));
  angles.push_back(abs(TCSM::GetPosition(4,'D',16,16).Theta()-TCSM::GetPosition(4,'D',16,15).Theta()));
  angles.push_back(abs(TCSM::GetPosition(4,'D',16,16).Theta()-TCSM::GetPosition(4,'D',15,16).Theta()));

  sort(angles.begin(),angles.end());

  cout<<angles.at(0)<<" "<<angles.at(0)*180/3.14159<<endl;
  cout<<angles.at(5)<<" "<<angles.at(5)*180/3.14159<<endl;*/
  
  std::vector<int> v1d;
  std::vector<int> v2d;
  std::vector<int> v1e;
  std::vector<int> v2e;
  std::vector<int> v3d;
  std::vector<int> v4d;

  std::vector<int> h1d;
  std::vector<int> h2d;
  std::vector<int> h1e;
  std::vector<int> h2e;
  std::vector<int> h3d;
  std::vector<int> h4d;

  v1d.clear();
  v2d.clear();
  v1e.clear();
  v2e.clear();
  v3d.clear();
  v4d.clear();
  
  h1d.clear();
  h2d.clear();
  h1e.clear();
  h2e.clear();
  h3d.clear();
  h4d.clear();

  for(int hiter=0;hiter<cdata->GetMultiplicityHorizontal();hiter++)
  {
    if(cdata->GetHorizontal_DetectorNbr(hiter)==3)
      h3d.push_back(hiter);
    else if(cdata->GetHorizontal_DetectorNbr(hiter)==4)
      h4d.push_back(hiter);
    else if(cdata->GetHorizontal_DetectorNbr(hiter)==1)
    {
      if(cdata->GetHorizontal_DetectorPos(hiter)=='D')
	h1d.push_back(hiter);
      else if(cdata->GetHorizontal_DetectorPos(hiter)=='E')
	h1e.push_back(hiter);
    }
    else if(cdata->GetHorizontal_DetectorNbr(hiter)==2)
    {
      if(cdata->GetHorizontal_DetectorPos(hiter)=='D')
	h2d.push_back(hiter);
      else if(cdata->GetHorizontal_DetectorPos(hiter)=='E')
	h2e.push_back(hiter);
    }
  }

  for(int viter=0;viter<cdata->GetMultiplicityVertical();viter++)
  {
    if(cdata->GetVertical_DetectorNbr(viter)==3)
      v3d.push_back(viter);
    else if(cdata->GetVertical_DetectorNbr(viter)==4)
      v4d.push_back(viter);
    else if(cdata->GetVertical_DetectorNbr(viter)==1)
    {
      if(cdata->GetVertical_DetectorPos(viter)=='D')
	v1d.push_back(viter);
      else if(cdata->GetVertical_DetectorPos(viter)=='E')
	v1e.push_back(viter);
    }
    else if(cdata->GetVertical_DetectorNbr(viter)==2)
    {
      if(cdata->GetVertical_DetectorPos(viter)=='D')
	v2d.push_back(viter);
      else if(cdata->GetVertical_DetectorPos(viter)=='E')
	v2e.push_back(viter);
    }
  }

  /* I print my vector sizes here.
  cout<<"\t1D: "<<v1d.size()<<" "<<h1d.size()<<endl;
  cout<<"\t1E: "<<v1e.size()<<" "<<h1e.size()<<endl;
  cout<<"\t2D: "<<v2d.size()<<" "<<h2d.size()<<endl;
  cout<<"\t2E: "<<v2e.size()<<" "<<h2e.size()<<endl;
  cout<<"\t3D: "<<v3d.size()<<" "<<h3d.size()<<endl;
  cout<<"\t4D: "<<v4d.size()<<" "<<h4d.size()<<endl;
  */

  BuildVH(v1d,h1d,D_Hits,cdata);
  BuildVH(v1e,h1e,E_Hits,cdata);
  BuildVH(v2d,h2d,D_Hits,cdata);
  BuildVH(v2e,h2e,E_Hits,cdata);
  BuildVH(v3d,h3d,D_Hits,cdata);
  BuildVH(v4d,h4d,D_Hits,cdata);

  BuilddEE(D_Hits,E_Hits,csm_hits);

  /* This prints the built hits
  if(csm_hits.size()>2)
    cout<<DRED;
  else if(csm_hits.size()>1)
    cout<<DGREEN;  
  
  for(int finaliter=0;finaliter<csm_hits.size();finaliter++)
  {
    csm_hits.at(finaliter).Print();
  }
  cout<<RESET_COLOR;
  */
}


void TCSM::Clear(Option_t *option)
{
  //cout << "clearing " << endl;
  if(data) data->Clear();

  csm_hits.clear();
  //cout <<" size: " << csm_hits.size() << endl;
  return;
}

void TCSM::Print(Option_t *option)
{
  printf("not yet written...\n");
  return;
}


TVector3 TCSM::GetPosition(int detector,char pos, int horizontalstrip, int verticalstrip, double X, double Y, double Z)
{
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

  if(detector==3&&pos=='D')
  {
    //Right Side
    verticalstrip=15-verticalstrip;
    x = SideX;
    z = SideZ + (50./32.)*(2*verticalstrip+1);
  }
  else if(detector==4&&pos=='D')
  {
    //Left Side
    x = -SideX;
    z = SideZ + (50./32.)*(2*verticalstrip+1);
  }
  else if(detector==1&&pos=='D')
  {
    //Right dE
    verticalstrip=15-verticalstrip;
    x = dEX - (50./32.)*cos(detTheta)*(2*verticalstrip+1);
    z = dEZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
  }
  else if(detector==2&&pos=='D')
  {
    //Left dE
    x = -dEX + (50./32.)*cos(detTheta)*(2*verticalstrip+1);
    z = dEZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
  }
  else if(detector==1&&pos=='E')
  {
    //Right E
    x = EX - (50./32.)*cos(detTheta)*(2*verticalstrip+1);
    z = EZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
  }
  else if(detector==2&&pos=='E')
  {
    //Left E
    verticalstrip=15-verticalstrip;
    x = -EX + (50./32.)*cos(detTheta)*(2*verticalstrip+1);
    z = EZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
  }
  else
  {
    printf("***Error, unrecognized detector and position combo!***\n");
  }

  y = (50./32.)*(2*horizontalstrip+1) - (50/16.)*8;
  Pos.SetX(x + X);
  Pos.SetY(y + Y);
  Pos.SetZ(z+ Z);
  return(Pos);
}

void TCSM::BuildVH(vector<int> &vvec,vector<int> &hvec,vector<TCSMHit> &hitvec,TCSMData *cdataVH)
{  
  if(vvec.size()==0 && hvec.size()==0)
    return;

  else if(vvec.size()==1&&hvec.size()==0)
  {
    RecoverHit('V',vvec.at(0),cdataVH,hitvec);
    vvec.clear();
  }

  else if(vvec.size()==0&&hvec.size()==1)
  {
    RecoverHit('H',hvec.at(0),cdataVH,hitvec);
    hvec.clear();
  }
  
  else if(vvec.size()==1&&hvec.size()==1)
  {    
    hitvec.push_back(MakeHit(hvec.at(0),vvec.at(0),cdataVH));
    hvec.clear();
    vvec.clear();
  }

  else if(vvec.size()==1&&hvec.size()==2)
  {
    int ve1 = cdataVH->GetVertical_Energy(vvec.at(0));
    int he1 = cdataVH->GetHorizontal_Energy(hvec.at(0));
    int he2 = cdataVH->GetHorizontal_Energy(hvec.at(1));
    if(AlmostEqual(ve1,he1+he2))
    {
      hitvec.push_back(MakeHit(hvec,vvec,cdataVH));
      hvec.clear();
      vvec.clear();
    }
    else if(AlmostEqual(ve1,he1))
    {
      hitvec.push_back(MakeHit(hvec.at(0),vvec.at(0),cdataVH));
      vvec.clear();
      hvec.erase(hvec.begin());
    }
    else if(AlmostEqual(ve1,he2))
    {
      hitvec.push_back(MakeHit(hvec.at(1),vvec.at(0),cdataVH));
      vvec.clear();
      hvec.pop_back();
    }
  }
  
  else if(vvec.size()==2&&hvec.size()==1)
  {
    int ve1 = cdataVH->GetVertical_Energy(vvec.at(0));
    int ve2 = cdataVH->GetVertical_Energy(vvec.at(1));
    int he1 = cdataVH->GetHorizontal_Energy(hvec.at(0));
    if(AlmostEqual(ve1+ve2,he1))
    {
      hitvec.push_back(MakeHit(hvec,vvec,cdataVH));
      hvec.clear();
      vvec.clear();
    }
    else if(AlmostEqual(ve1,he1))
    {
      hitvec.push_back(MakeHit(hvec.at(0),vvec.at(0),cdataVH));
      vvec.erase(vvec.begin());
      hvec.clear();
    }
    else if(AlmostEqual(ve2,he1))
    {
      hitvec.push_back(MakeHit(hvec.at(0),vvec.at(1),cdataVH));
      vvec.pop_back();
      hvec.clear();
    }
  }

  else if(vvec.size()==2&&hvec.size()==2)
  {    
    int ve1 = cdataVH->GetVertical_Energy(vvec.at(0));
    int ve2 = cdataVH->GetVertical_Energy(vvec.at(1));
    int he1 = cdataVH->GetHorizontal_Energy(hvec.at(0));
    int he2 = cdataVH->GetHorizontal_Energy(hvec.at(1));
    if( (AlmostEqual(ve1,he1) && AlmostEqual(ve2,he2)) || (AlmostEqual(ve1,he2) && AlmostEqual(ve2,he1)) )
    {
      //I can build both 1,1 and 2,2 or 1,2 and 2,1
      if(abs(ve1-he1)+abs(ve2-he2) <= abs(ve1-he2)+abs(ve2-he1))
      {
	//1,1 and 2,2 mimimizes difference
	hitvec.push_back(MakeHit(hvec.at(0),vvec.at(0),cdataVH));
	hitvec.push_back(MakeHit(hvec.at(1),vvec.at(1),cdataVH));
	hvec.clear();
	vvec.clear();
      }
      else if(abs(ve1-he1)+abs(ve2-he2) > abs(ve1-he2)+abs(ve2-he1))
      {
	//1,2 and 2,1 mimimizes difference
	hitvec.push_back(MakeHit(hvec.at(0),vvec.at(1),cdataVH));
	hitvec.push_back(MakeHit(hvec.at(1),vvec.at(0),cdataVH));
	hvec.clear();
	vvec.clear();
      }
    }
    else if( AlmostEqual(ve1,he1) )
    {
      hitvec.push_back(MakeHit(hvec.at(0),vvec.at(0),cdataVH));
      hvec.erase(hvec.begin());
      vvec.erase(vvec.begin());
    }
    else if( AlmostEqual(ve2,he1) )
    {
      hitvec.push_back(MakeHit(hvec.at(1),vvec.at(0),cdataVH));
      hvec.erase(hvec.begin());
      vvec.pop_back();
    }
    else if( AlmostEqual(ve1,he2) )
    {
      hitvec.push_back(MakeHit(hvec.at(0),vvec.at(1),cdataVH));
      hvec.pop_back();
      vvec.erase(vvec.begin());
    }
    else if( AlmostEqual(ve2,he2) )
    {
      hitvec.push_back(MakeHit(hvec.at(1),vvec.at(1),cdataVH));
      hvec.pop_back();
      vvec.pop_back();
    }
  }

  /*
  else if(vvec.size()==3&&hvec.size()==3)
  {
    cdataVH->Print();
    
  }
  else if(vvec.size()==3&&hvec.size()==2)
  {
    cdataVH->Print();
    
  }
  else if(vvec.size()==2&&hvec.size()==3)
  {
    cdataVH->Print();
    
  }
  else if(vvec.size()==2&&hvec.size()==3)
  {
    cdataVH->Print();
    
  }
  else if(vvec.size()==1&&hvec.size()==3)
  {
    cdataVH->Print();
    
  }
  else if(vvec.size()==3&&hvec.size()==1)
  {
    cdataVH->Print();
    
  }
  */
    
  /*else
  {
    vector<bool> vertUsed (vvec.size(),false);
    vector<bool> horUsed (hvec.size(),false);
    for(int vloop = 0; vloop<vvec.size(); vloop++)
    {
      if(vertUsed.at(vloop))
	continue;
      
      double VE = cdataVH->GetVertical_Energy(vvec.at(vloop));
      for(int hloop = 0; hloop<hvec.size(); hloop++)
      {
	if(horUsed.at(hloop))
	  continue;
	
	double HE = cdataVH->GetHorizontal_Energy(hvec.at(hloop));

	if(AlmostEqual(VE,HE))
	{
	  cdataVH->Print();
	  hitvec.push_back(MakeHit(hvec.at(hloop),vvec.at(vloop),cdataVH));
	  hitvec.back().Print();
	  vertUsed.at(vloop) = true;
	  horUsed.at(hloop) = true;
	}
      }
    }
  }*/


  
  //else
    //cdataVH->Print();
  
  /* This prints unused hits in a pretty ugly manner
  if(!vvec.empty() || !hvec.empty())
  {
    //cdataVH->Print();
    for(int iter=0;iter<hvec.size();iter++)
    {
      cout<<DBLUE<<hvec.at(iter)<<RESET_COLOR<<endl;
    }
    for(int iter=0;iter<vvec.size();iter++)
    {
      cout<<DRED<<vvec.at(iter)<<RESET_COLOR<<endl;
    }
  }*/
  
}


TCSMHit TCSM::MakeHit(int hh, int vv, TCSMData *cdata)
{
  TCSMHit csmhit;
  csmhit.Clear();
  //cout<<"HH: "<<hh<<" VV: "<<vv<<endl;

  if(cdata->GetHorizontal_DetectorNbr(hh)!=cdata->GetVertical_DetectorNbr(vv))
    cerr<<"\tSomething is wrong, Horizontal and Vertical detector numbers don't match."<<endl;
  if(cdata->GetHorizontal_DetectorPos(hh)!=cdata->GetVertical_DetectorPos(vv))
    cerr<<"\tSomething is wrong, Horizontal and Vertical positions don't match."<<endl;

  if(cdata->GetHorizontal_DetectorPos(hh)=='D')
  {
    //cout<<"MakeHit in D"<<endl;
    csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(hh));
    csmhit.SetDHorizontalCharge(cdata->GetHorizontal_Charge(hh));
    csmhit.SetDVerticalCharge(cdata->GetVertical_Charge(vv));
    csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(hh));
    csmhit.SetDVerticalStrip(cdata->GetVertical_StripNbr(vv));
    csmhit.SetDHorizontalCFD(cdata->GetHorizontal_TimeCFD(hh));
    csmhit.SetDVerticalCFD(cdata->GetVertical_TimeCFD(vv));
    csmhit.SetDHorizontalTime(cdata->GetHorizontal_Time(hh));
    csmhit.SetDVerticalTime(cdata->GetVertical_Time(vv));
    csmhit.SetDHorizontalEnergy(cdata->GetHorizontal_Energy(hh));
    csmhit.SetDVerticalEnergy(cdata->GetVertical_Energy(vv));
    csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(hh),
					   cdata->GetHorizontal_DetectorPos(hh),
					   cdata->GetHorizontal_StripNbr(hh),
					   cdata->GetVertical_StripNbr(vv)));
  }
  else if(cdata->GetHorizontal_DetectorPos(hh)=='E')
  {
    //cout<<"MakeHit in E"<<endl;
    csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(hh));
    csmhit.SetEHorizontalCharge(cdata->GetHorizontal_Charge(hh));
    csmhit.SetEVerticalCharge(cdata->GetVertical_Charge(vv));
    csmhit.SetEHorizontalStrip(cdata->GetHorizontal_StripNbr(hh));
    csmhit.SetEVerticalStrip(cdata->GetVertical_StripNbr(vv));
    csmhit.SetEHorizontalCFD(cdata->GetHorizontal_TimeCFD(hh));
    csmhit.SetEVerticalCFD(cdata->GetVertical_TimeCFD(vv));
    csmhit.SetEHorizontalTime(cdata->GetHorizontal_Time(hh));
    csmhit.SetEVerticalTime(cdata->GetVertical_Time(vv));
    csmhit.SetEHorizontalEnergy(cdata->GetHorizontal_Energy(hh));
    csmhit.SetEVerticalEnergy(cdata->GetVertical_Energy(vv));
    csmhit.SetEPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(hh),
					  cdata->GetHorizontal_DetectorPos(hh),
					  cdata->GetHorizontal_StripNbr(hh),
					  cdata->GetVertical_StripNbr(vv)));
  }
  //if(hh!=0||vv!=0)
  //csmhit.Print();
  return(csmhit);
}

TCSMHit TCSM::MakeHit(vector<int> &hhV,vector<int> &vvV, TCSMData *cdata)
{
  TCSMHit csmhit;
  csmhit.Clear();
  //cout<<"Make Hit Starting."<<endl;
  if(hhV.size()==0 || vvV.size()==0)
    cerr<<"\tSomething is wrong, empty vector in MakeHit"<<endl;

  int DetNumH = cdata->GetHorizontal_DetectorNbr(hhV.at(0));
  char DetPosH = cdata->GetHorizontal_DetectorPos(hhV.at(0));
  int ChargeH = 0;
  int StripH = -1;
  int ConFraH = 0;
  double TimeH = 0;
  double EnergyH = 0;
  int biggestH = 0;

  //cout<<"Make Hit Horizontal Setup Done."<<endl;

  int DetNumV = cdata->GetVertical_DetectorNbr(vvV.at(0));
  char DetPosV = cdata->GetVertical_DetectorPos(vvV.at(0));
  int ChargeV = 0;
  int StripV = -1;
  int ConFraV = 0;
  double TimeV = 0;
  double EnergyV = 0;
  int biggestV = 0;

  //cout<<"Make Hit Vertical Setup Done."<<endl;
  
  for(int iterH=0;iterH<hhV.size();iterH++)
  {
    if(cdata->GetHorizontal_Charge(hhV.at(iterH))>cdata->GetHorizontal_Charge(biggestH))
      biggestH = hhV.at(iterH);

    if(cdata->GetHorizontal_DetectorNbr(hhV.at(iterH))!=DetNumH)
      cerr<<"\tSomething is wrong, Horizontal detector numbers don't match in vector loop."<<endl;
    if(cdata->GetHorizontal_DetectorPos(hhV.at(iterH))!=DetPosH)
      cerr<<"\tSomething is wrong, Horizontal detector positions don't match in vector loop."<<endl;
    
    ChargeH += cdata->GetHorizontal_Charge(hhV.at(iterH));
    EnergyH += cdata->GetHorizontal_Energy(hhV.at(iterH));
  }

  StripH = cdata->GetHorizontal_StripNbr(biggestH);
  ConFraH = cdata->GetHorizontal_TimeCFD(biggestH);
  TimeH = cdata->GetHorizontal_Time(biggestH);

  //cout<<"MakeHit Horizontal Done."<<endl;

  for(int iterV=0;iterV<vvV.size();iterV++)
  {
    if(cdata->GetVertical_Charge(vvV.at(iterV))>cdata->GetVertical_Charge(biggestV))
      biggestV = vvV.at(iterV);
    
    if(cdata->GetVertical_DetectorNbr(vvV.at(iterV))!=DetNumV)
      cerr<<"\tSomething is wrong, Vertical detector numbers don't match in vector loop."<<endl;
    if(cdata->GetVertical_DetectorPos(vvV.at(iterV))!=DetPosV)
      cerr<<"\tSomething is wrong, Vertical detector positions don't match in vector loop."<<endl;
    
    ChargeV += cdata->GetVertical_Charge(vvV.at(iterV));
    EnergyV += cdata->GetVertical_Energy(vvV.at(iterV));
  }
  
  StripV = cdata->GetVertical_StripNbr(biggestV);
  ConFraV = cdata->GetVertical_TimeCFD(biggestV);
  TimeV = cdata->GetVertical_Time(biggestV);

  //cout<<"MakeHIt Vertical Done."<<endl;
  
  if(DetNumH!=DetNumV)
    cerr<<"\tSomething is wrong, Horizontal and Vertical detector numbers don't match in vector."<<endl;
  if(DetPosH!=DetPosV)
    cerr<<"\tSomething is wrong, Horizontal and Vertical positions don't match in vector."<<endl;
  
  if(DetPosH=='D')
  {
    csmhit.SetDetectorNumber(DetNumH);
    csmhit.SetDHorizontalCharge(ChargeH);
    csmhit.SetDVerticalCharge(ChargeV);
    csmhit.SetDHorizontalStrip(StripH);
    csmhit.SetDVerticalStrip(StripV);
    csmhit.SetDHorizontalCFD(ConFraH);
    csmhit.SetDVerticalCFD(ConFraV);
    csmhit.SetDHorizontalTime(TimeH);
    csmhit.SetDVerticalTime(TimeV);
    csmhit.SetDHorizontalEnergy(EnergyH);
    csmhit.SetDVerticalEnergy(EnergyV);
    csmhit.SetDPosition(TCSM::GetPosition(DetNumH,
					  DetPosH,
					  StripH,
					  StripV));
  }
  else if(DetPosH=='E')
  {
    csmhit.SetDetectorNumber(DetNumH);
    csmhit.SetEHorizontalCharge(ChargeH);
    csmhit.SetEVerticalCharge(ChargeV);
    csmhit.SetEHorizontalStrip(StripH);
    csmhit.SetEVerticalStrip(StripV);
    csmhit.SetEHorizontalCFD(ConFraH);
    csmhit.SetEVerticalCFD(ConFraV);
    csmhit.SetEHorizontalTime(TimeH);
    csmhit.SetEVerticalTime(TimeV);
    csmhit.SetEHorizontalEnergy(EnergyH);
    csmhit.SetEVerticalEnergy(EnergyV);
    csmhit.SetEPosition(TCSM::GetPosition(DetNumH,
					  DetPosH,
					  StripH,
					  StripV));
  }

  //csmhit.Print();
  return(csmhit);
}

void TCSM::BuilddEE(vector<TCSMHit> &DHitVec,vector<TCSMHit> &EHitVec,vector<TCSMHit> &BuiltHits)
{
  std::vector<TCSMHit> d1;
  std::vector<TCSMHit> d2;
  std::vector<TCSMHit> e1;
  std::vector<TCSMHit> e2;

  d1.clear();
  d2.clear();
  e1.clear();
  e2.clear();
  
  for(int diter=0;diter<DHitVec.size();diter++)
  {
    if(DHitVec.at(diter).GetDetectorNumber()==3 || DHitVec.at(diter).GetDetectorNumber()==4)//I am in side detectors
    {
      //I will never have a pair in the side detector, so go ahead and send it through.
      BuiltHits.push_back(DHitVec.at(diter));
    }
    else if(DHitVec.at(diter).GetDetectorNumber()==1)
    {
      d1.push_back(DHitVec.at(diter));
    }
    else if(DHitVec.at(diter).GetDetectorNumber()==2)
    {
      d2.push_back(DHitVec.at(diter));
    }
    else
    {
      cerr<<"  Caution, in BuilddEE detector number in D vector is out of bounds."<<endl;
    }
  }

  for(int eiter=0;eiter<EHitVec.size();eiter++)
  {
    if(EHitVec.at(eiter).GetDetectorNumber()==1)
    {
      e1.push_back(EHitVec.at(eiter));
    }
    else if(EHitVec.at(eiter).GetDetectorNumber()==2)
    {
      e2.push_back(EHitVec.at(eiter));
    }
    else
    {
      cerr<<"  Caution, in BuilddEE detector number in E vector is out of bounds."<<endl;
    }
  }

  MakedEE(d1,e1,BuiltHits);
  MakedEE(d2,e2,BuiltHits);
  
}

void TCSM::MakedEE(vector<TCSMHit> &DHitVec,vector<TCSMHit> &EHitVec,vector<TCSMHit> &BuiltHits)
{

  if(DHitVec.size()==0 && EHitVec.size()==0)
    return;
  else if(DHitVec.size()==1 && EHitVec.size()==0)
    BuiltHits.push_back(DHitVec.at(0));
  else if(DHitVec.size()==0 && EHitVec.size()==1)
    BuiltHits.push_back(EHitVec.at(0));
  else if(DHitVec.size()==1 && EHitVec.size()==1)
    BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(0)));
  else if(DHitVec.size()==2 && EHitVec.size()==0)
  {
    BuiltHits.push_back(DHitVec.at(0));
    BuiltHits.push_back(DHitVec.at(1));
  }
  else if(DHitVec.size()==0 && EHitVec.size()==2)
  {
    BuiltHits.push_back(EHitVec.at(0));
    BuiltHits.push_back(EHitVec.at(1));
  }
  else if(DHitVec.size()==2 && EHitVec.size()==1)
  {
    double dt1 = DHitVec.at(0).GetDPosition().Theta();
    double dt2 = DHitVec.at(1).GetDPosition().Theta();
    double et = EHitVec.at(0).GetEPosition().Theta();

    if( abs(dt1-et) <= abs(dt2-et) )
    {
      //cout<<DRED;
      BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(0)));
      //BuiltHits.back().Print();
      BuiltHits.push_back(DHitVec.at(1));
      //BuiltHits.back().Print();
      //cout<<RESET_COLOR;
    }
    else
    {
      //cout<<DBLUE;
      BuiltHits.push_back(CombineHits(DHitVec.at(1),EHitVec.at(0)));
      //BuiltHits.back().Print();
      BuiltHits.push_back(DHitVec.at(0));
      //BuiltHits.back().Print();
      //cout<<RESET_COLOR;
    }
  }
  else if(DHitVec.size()==1 && EHitVec.size()==2)
  {
    double dt = DHitVec.at(0).GetDPosition().Theta();
    double et1 = EHitVec.at(0).GetEPosition().Theta();
    double et2 = EHitVec.at(0).GetEPosition().Theta();
    
    if( abs(dt-et1) <= abs(dt-et2) )
    {
      //cout<<DRED;
      BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(0)));
      //BuiltHits.back().Print();
      BuiltHits.push_back(EHitVec.at(1));
      //BuiltHits.back().Print();
      //cout<<RESET_COLOR;
    }
    else
    {
      //cout<<DBLUE;
      BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(1)));
      //BuiltHits.back().Print();
      BuiltHits.push_back(EHitVec.at(0));
      //BuiltHits.back().Print();
      //cout<<RESET_COLOR;
    }
  }
  else if(DHitVec.size()==2 && EHitVec.size()==2)
  {
    double dt1 = DHitVec.at(0).GetDPosition().Theta();
    double dt2 = DHitVec.at(1).GetDPosition().Theta();
    double et1 = EHitVec.at(0).GetEPosition().Theta();
    double et2 = EHitVec.at(1).GetEPosition().Theta();

    if( abs(dt1-et1)+abs(dt2-et2) <= abs(dt1-et2)+abs(dt2-et1) )
    {
      //cout<<DRED;
      BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(0)));
      //BuiltHits.back().Print();
      BuiltHits.push_back(CombineHits(DHitVec.at(1),EHitVec.at(1)));
      //BuiltHits.back().Print();
      //cout<<RESET_COLOR;
    }
    else
    {
      //cout<<DBLUE;
      BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(1)));
      //BuiltHits.back().Print();
      BuiltHits.push_back(CombineHits(DHitVec.at(1),EHitVec.at(0)));
      //BuiltHits.back().Print();
      //cout<<RESET_COLOR;
    }
  }
  else
  {
    cout<<"D Size: "<<DHitVec.size()<<" E Size: "<<EHitVec.size()<<endl;
  }
}

void TCSM::OldBuilddEE(vector<TCSMHit> &DHitVec,vector<TCSMHit> &EHitVec,vector<TCSMHit> &BuiltHits)
{
  bool printbit =0;
  //cout<<"DHitVec size: "<<DHitVec.size()<<" EHitVec size: "<<EHitVec.size()<<endl;
  if(DHitVec.size()==0&&EHitVec.size()==0)//Why am I even here?!
    return;
  
  /*else if(DHitVec.size()>2 && EHitVec.size()>=2)
  {
    printbit =1;
    cout<<YELLOW<<"******************************************"<<RESET_COLOR<<endl;

    for(int i =0; i< DHitVec.size();i++)
    {
      cout<<DRED;
      DHitVec.at(i).Print();
      cout<<RESET_COLOR;
    }
    for(int i =0; i< EHitVec.size();i++)
    {
      cout<<DBLUE;
      EHitVec.at(i).Print();
      cout<<RESET_COLOR;
    }
  }*/

  vector<bool> EUsed (EHitVec.size(),false);
  vector<bool> DUsed (DHitVec.size(),false);
  
  for(int diter=0;diter<DHitVec.size();diter++)
  {
    //cout<<"diter: "<<diter<<endl;
    if(DUsed.at(diter))
      continue;
    
    for(int eiter=0;eiter<EHitVec.size();eiter++)
    {
      //cout<<"eiter: "<<eiter<<endl;
      if(EUsed.at(eiter))
	continue;

      if(DHitVec.at(diter).GetDetectorNumber()==EHitVec.at(eiter).GetDetectorNumber())//Hits are in the same stack
      {
	if( AlmostEqual(DHitVec.at(diter).GetDPosition().Theta(),EHitVec.at(eiter).GetEPosition().Theta()))//Same-ish Theta
	  //&& AlmostEqual(DHitVec.at(diter).GetDPosition().Phi(),EHitVec.at(eiter).GetEPosition().Phi()) )//Same-ish Phi
	{
	  BuiltHits.push_back(CombineHits(DHitVec.at(diter),EHitVec.at(eiter)));
	  /*cout<<DRED;
	  BuiltHits.back().Print();
	  cout<<RESET_COLOR;*/
	  DUsed.at(diter) = true;
	  EUsed.at(eiter) = true;
	  break;
	}
      }
    }
  }

  //This loop adds uncorrelated events in the telescope together.  This may be bad, but let's see.
  for(int i=0;i<DHitVec.size();i++)
  {
    if(!DUsed.at(i))
    {
      for(int j=0;j<EHitVec.size();j++)
      {
	if(!EUsed.at(j))
	{
	  if(EHitVec.at(j).GetDetectorNumber()==DHitVec.at(i).GetDetectorNumber())
	  {
	    BuiltHits.push_back(CombineHits(DHitVec.at(i),EHitVec.at(j)));
	    /*cout<<DRED;
	    BuiltHits.back().Print();
	    cout<<RESET_COLOR;*/
	    DUsed.at(i) = true;
	    EUsed.at(j) = true;
	    break;
	  }
	}
      }
    }
  }


  //Send through the stragglers.  This is very permissive, but we trust BuildVH to take care of the riff-raff
  for(int i=0;i<DHitVec.size();i++)
  {
    //cout<<"*************************"<<endl;
  
//     if(EHitVec.size()>0)
//     {
//       cout<<DGREEN;
//       DHitVec.at(i).Print();
//       cout<<RESET_COLOR;
//     }
    if(!DUsed.at(i))
    {
      BuiltHits.push_back(DHitVec.at(i));
//       if(printbit)
//       {
//       cout<<DGREEN;
//       BuiltHits.back().Print();
//       cout<<RESET_COLOR;
//       }
    }
  }
  for(int j=0;j<EHitVec.size();j++)
  {
//     if(DHitVec.size()>0)
//     {
//       cout<<DGREEN;
//       EHitVec.at(j).Print();
//       cout<<RESET_COLOR;
//     }
    if(!EUsed.at(j))
    {
      BuiltHits.push_back(EHitVec.at(j));
//       if(printbit)
//       {
// 	cout<<DGREEN;
// 	BuiltHits.back().Print();
// 	cout<<RESET_COLOR;
//       }
    }
  }

  if(printbit)
  {
    for(int k =0; k<BuiltHits.size();k++)
    {
      cout<<DGREEN;
      BuiltHits.at(k).Print();
      cout<<RESET_COLOR<<endl;
    }
  }
}

void TCSM::RecoverHit(char orientation, int location, TCSMData *cdata, vector<TCSMHit> &hits)
{
  if(!RECOVERHITS)
    return;
  
  TCSMHit csmhit;
  csmhit.Clear();

  int detno=-1;
  char pos='X';
  //cout<<DGREEN<<"*****************************************"<<RESET_COLOR<<endl;
  
  if(orientation=='V')
  {
    //cout<<"pos should be "<<char(cdata->GetVertical_DetectorPos(location))<<endl;
    pos=char(cdata->GetVertical_DetectorPos(location));
    //cout<<"pos is "<<pos<<endl;
    detno=cdata->GetVertical_DetectorNbr(location);
  }
  else if(orientation=='H')
  {
    //cout<<"pos should be "<<char(cdata->GetHorizontal_DetectorPos(location))<<endl;
    pos=char(cdata->GetHorizontal_DetectorPos(location));
    //cout<<"pos is "<<pos<<endl;
    detno=cdata->GetHorizontal_DetectorNbr(location);
  }
  
  if(detno==1)
    return;
  else if(detno==2)
  {
    if(pos=='D' && orientation=='V')//Recover 2DN09, channel 1040
    {
      csmhit.SetDetectorNumber(cdata->GetVertical_DetectorNbr(location));
      csmhit.SetDHorizontalCharge(cdata->GetVertical_Charge(location));
      csmhit.SetDVerticalCharge(cdata->GetVertical_Charge(location));
      csmhit.SetDHorizontalStrip(9);
      csmhit.SetDVerticalStrip(cdata->GetVertical_StripNbr(location));
      csmhit.SetDHorizontalCFD(cdata->GetVertical_TimeCFD(location));
      csmhit.SetDVerticalCFD(cdata->GetVertical_TimeCFD(location));
      csmhit.SetDHorizontalTime(cdata->GetVertical_Time(location));
      csmhit.SetDVerticalTime(cdata->GetVertical_Time(location));
      csmhit.SetDHorizontalEnergy(cdata->GetVertical_Energy(location));
      csmhit.SetDVerticalEnergy(cdata->GetVertical_Energy(location));
      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetVertical_DetectorNbr(location),
					    cdata->GetVertical_DetectorPos(location),
					    9,
					    cdata->GetVertical_StripNbr(location)));
    }
  }
  else if(detno==3)
  {
    if(pos=='E')
    {
      cerr<<"3E in RecoverHit"<<endl;
      //cdata->Print();
      //cout<<"Loc: "<<location<<endl;
      //cout<<"Vars: "<<detno<<" "<<pos<<endl;
      //if(orientation=='V') cout<<"V "<<cdata->GetVertical_DetectorNbr(location)<<" "<<char(cdata->GetVertical_DetectorPos(location))<<endl;
      //else if(orientation=='H') cout<<"H "<<cdata->GetHorizontal_DetectorNbr(location)<<" "<<char(cdata->GetHorizontal_DetectorPos(location))<<endl;
      
      return;
    }
    else if(orientation=='H')//Recover 3DP11, channel 1145
    {
      csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(location));
      csmhit.SetDHorizontalCharge(cdata->GetHorizontal_Charge(location));
      csmhit.SetDVerticalCharge(cdata->GetHorizontal_Charge(location));
      csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(location));
      csmhit.SetDVerticalStrip(11);
      csmhit.SetDHorizontalCFD(cdata->GetHorizontal_TimeCFD(location));
      csmhit.SetDVerticalCFD(cdata->GetHorizontal_TimeCFD(location));
      csmhit.SetDHorizontalTime(cdata->GetHorizontal_Time(location));
      csmhit.SetDVerticalTime(cdata->GetHorizontal_Time(location));
      csmhit.SetDHorizontalEnergy(cdata->GetHorizontal_Energy(location));
      csmhit.SetDVerticalEnergy(cdata->GetHorizontal_Energy(location));
      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(location),
					    cdata->GetHorizontal_DetectorPos(location),
					    cdata->GetHorizontal_StripNbr(location),
					    11));
    }
  }
  else if(detno==4)
  {
    if(pos=='E')
    {
      cerr<<"4E in RecoverHit"<<endl;
      return;
    }
    else if(orientation=='H')//Recover 4DP15, channel 1181
    {
      csmhit.SetDetectorNumber(cdata->GetHorizontal_DetectorNbr(location));
      csmhit.SetDHorizontalCharge(cdata->GetHorizontal_Charge(location));
      csmhit.SetDVerticalCharge(cdata->GetHorizontal_Charge(location));
      csmhit.SetDHorizontalStrip(cdata->GetHorizontal_StripNbr(location));
      csmhit.SetDVerticalStrip(15);
      csmhit.SetDHorizontalCFD(cdata->GetHorizontal_TimeCFD(location));
      csmhit.SetDVerticalCFD(cdata->GetHorizontal_TimeCFD(location));
      csmhit.SetDHorizontalTime(cdata->GetHorizontal_Time(location));
      csmhit.SetDVerticalTime(cdata->GetHorizontal_Time(location));
      csmhit.SetDHorizontalEnergy(cdata->GetHorizontal_Energy(location));
      csmhit.SetDVerticalEnergy(cdata->GetHorizontal_Energy(location));
      csmhit.SetDPosition(TCSM::GetPosition(cdata->GetHorizontal_DetectorNbr(location),
					    cdata->GetHorizontal_DetectorPos(location),
					    cdata->GetHorizontal_StripNbr(location),
					    15));
    }
  }
  else
  {
    cerr<<"Something is wrong.  The detector number in recover hit is out of bounds."<<endl;
    return;
  }

  if(!csmhit.IsEmpty())
  {
    //cdata->Print();
    //cout<<DRED;
    //csmhit.Print();
    //cout<<RESET_COLOR;
    hits.push_back(csmhit);
  }
}

TCSMHit TCSM::CombineHits(TCSMHit d_hit,TCSMHit e_hit)
{
  //cout<<"I'm combining hits!!"<<endl;
  //d_hit.Print();
  //e_hit.Print();
  
  if(d_hit.GetDetectorNumber()!=e_hit.GetDetectorNumber())
    cerr<<"Something is wrong.  In combine hits, the detector numbers don't match"<<endl;

  d_hit.SetEHorizontalStrip(e_hit.GetEHorizontalStrip());
  d_hit.SetEVerticalStrip(e_hit.GetEVerticalStrip());
  
  d_hit.SetEHorizontalCharge(e_hit.GetEHorizontalCharge());
  d_hit.SetEVerticalCharge(e_hit.GetEVerticalCharge());

  d_hit.SetEHorizontalEnergy(e_hit.GetEHorizontalEnergy());
  d_hit.SetEVerticalEnergy(e_hit.GetEVerticalEnergy());

  d_hit.SetEHorizontalCFD(e_hit.GetEHorizontalCFD());
  d_hit.SetEVerticalCFD(e_hit.GetEVerticalCFD());

  d_hit.SetEHorizontalTime(e_hit.GetEHorizontalTime());
  d_hit.SetEVerticalTime(e_hit.GetEVerticalTime());

  d_hit.SetEPosition(e_hit.GetEPosition());
  
  //d_hit.Print();
  return(d_hit);
  
}

bool TCSM::AlmostEqual(int val1, int val2)
{
  //cout<<"int AlmostEqual Called."<<endl;
  double diff = double(abs(val1 - val2));
  double ave = (val1+val2)/2.;
  double frac = diff/ave;
  //cout<<"Val1: "<<val1<<" Val2: "<<val2<<" Diff: "<<diff<<" Ave: "<<ave<<" Frac: "<<frac<<" Return: "<< (frac<AlmostEqualWindow) <<endl;
  return frac < AlmostEqualWindow;
}

bool TCSM::AlmostEqual(double val1, double val2)
{
  double frac = fabs(val1 - val2)/((val1+val2)/2.);
  return frac < AlmostEqualWindow;
}

