//g++ -c -fPIC TKinematics.cc -I./ `root-config --cflags`

#include "TKinematics.h"
#include "Globals.h"

/// \cond CLASSIMP
ClassImp(TKinematics)
/// \endcond

//////////////////////////////////////////////////////////////////
//
// TKinematics
//
// This class calculates 2 body kinematics from a beam, target,
// recoil, ejectile, and beam energy
//
//////////////////////////////////////////////////////////////////

TKinematics::TKinematics(double beame, const char *beam, const char *targ, const char *ejec, const char *reco, const char *name){
  InitKin();

	TNucleus *b,*t,*e,*r;

	b = new TNucleus(beam);
	t = new TNucleus(targ);
	name = Form("%s(%s,%s)%s",targ,beam,ejec,reco);
	
	if(!ejec  || !reco) {
		//without ejectile or recoil, elastic scattering is assumed
		e = b;
		r = t;
		//TKinematics(b,t,beame,name);
	} else { 
		e = new TNucleus(ejec);
		r = new TNucleus(reco);
	}

  fParticle[0] = b;
  fParticle[1] = t;
  fParticle[2] = e;
  fParticle[3] = r;
  for(int i=0;i<4;i++)
    fM[i] = fParticle[i]->GetMass();
    
  fEBeam = beame;
  fQValue = (fM[0]+fM[1])-(fM[2]+fM[3]);
  Initial();
  FinalCm();
  SetName(name);
  Cm2LabSpline = 0;
}

TKinematics::TKinematics(TNucleus* projectile, TNucleus* target, double ebeam, const char *name){
// By not providing the ejectile (only prociding projectile, target, and beam energy) elestic scattering is assumed
  InitKin();
  fParticle[0] = projectile;
  fParticle[1] = target;
  fParticle[2] = NULL;
  fParticle[3] = NULL;
  fM[0]=fParticle[0]->GetMass();
  fM[1]=fParticle[1]->GetMass();
  fEBeam = ebeam;
  fQValue =0;
  Initial();
  FinalCm();
  SetName(name);
  Cm2LabSpline = 0;
}

TKinematics::TKinematics(TNucleus* projectile, TNucleus* target, TNucleus* recoil, TNucleus* ejectile, double ebeam, const char *name){
// Kinematics using the provided projectile, target, recoil, and ejectile, as well as beam energy
  InitKin();
  fParticle[0] = projectile;
  fParticle[1] = target;
  fParticle[2] = recoil;
  fParticle[3] = ejectile;
  for(int i=0;i<4;i++)
    fM[i]=fParticle[i]->GetMass();
    
  fEBeam = ebeam;
  fQValue = (fM[0]+fM[1])-(fM[2]+fM[3]);
  Initial();
  FinalCm();
  SetName(name);
  Cm2LabSpline = 0;
//  printf("\e[1;31m" "M[0] = %.01f \tM[1] = %.01f \tM[2] = %.01f \tM[3] = %.01f \t\n\n\n " "\e[m",fM[0],fM[1],fM[2],fM[3]);
}

TKinematics::TKinematics(TNucleus* projectile, TNucleus* target, TNucleus* recoil, TNucleus* ejectile, double ebeam, double ex3, const char *name){
// Kinematics using the provided projectile, target, recoil, ejectile, beam energy, and excited state of the recoil
  InitKin();
  fParticle[0] = projectile;
  fParticle[1] = target;
  fParticle[2] = recoil;           // This is oppiste what is right below it. 
  fParticle[3] = ejectile;         // we need to define a convention and stick with it  pcb.
  for(int i=0;i<4;i++)
    fM[i]=fParticle[i]->GetMass();
    
  fEBeam = ebeam;
  fQValue = (fM[0]+fM[1])-(fM[2]+fM[3])-ex3;
  Initial();
  FinalCm();
  SetName(name);
  Cm2LabSpline = 0;
}


TKinematics::TKinematics(const char *beam, const char *targ, const char *ejec, const char *reco, double ebeam, double ex3, const char *name){
  InitKin();
	TNucleus *b,*t,*e,*r;

	b = new TNucleus(beam);
	t = new TNucleus(targ);
	name = Form("%s(%s,%s)%s",targ,beam,ejec,reco);
	
	if(!ejec  || !reco) {
		//without ejectile or recoil, elastic scattering is assumed
		e = b;
		r = t;
		//TKinematics(b,t,beame,name);
	} else { 
		e = new TNucleus(ejec);
		r = new TNucleus(reco);
	}

  fParticle[0] = b;
  fParticle[1] = t;
  fParticle[2] = e;
  fParticle[3] = r;
  for(int i=0;i<4;i++)
    fM[i]=fParticle[i]->GetMass();
    
  fEBeam = ebeam;
  fQValue = (fM[0]+fM[1])-(fM[2]+fM[3])-ex3;
  Initial();
  FinalCm();
  SetName(name);
  Cm2LabSpline = 0;
}


void TKinematics::InitKin() {

  //  zeros all initial variables.
  //  pcb

  fTCm_i = 0;
  fTCm_f = 0;

  fQValue   = 0;
  fEBeam    = 0;
  fBeta_cm  = 0;
  fGamma_cm = 0;

  for(int i=0;i<4;i++) {
    fParticle[i] = 0;
    fM[i] = 0;
    fT[i] = 0;
    fE[i] = 0;
    fP[i] = 0;
    fV[i] = 0;
    fTheta[i] = 0;
   
    fTcm[i] = 0;
    fEcm[i] = 0;
    fPcm[i] = 0;
    fVcm[i] = 0;
    fBetacm[i]  = 0;
    fThetacm[i] = 0;
  }
}



TSpline3* TKinematics::Evslab(double thmin, double thmax, double size, int part){
  //double* energy = new double[(int)((thmax-thmin)/size)+1];
  //double* angle = new double[(int)((thmax-thmin)/size)+1];

  if(part<2||part>3) {
    printf(ALERTTEXT "WARNING: the function Evslab should use nuclei after the reaction (part 2 or part 3)" RESET_COLOR "\n");
    return 0;
  }

  std::vector<double> energy;
  std::vector<double> angle;

  double deg2rad=PI/180.0;
  double rad2deg=180.0/PI;

  int steps = ((int)(thmax+1) - (int)thmin) / (int)size;   // when is size ever needed to be a double?? pcb.
                                                           // i am under the impression that size should always be 1.0;
                                                           //
  double lastangle = 0.0;;
  //for(int i=0;i<((thmax-thmin)/size);i++){
  for(int i=0;i<steps;i++){
    Final((thmin+i*size)*deg2rad,2);//part);   //2);
    double tmpangle = GetThetalab(part)*(1/deg2rad);
    double tmpeng   = GetTlab(part) * 1000;
    //printf("step[%i] \t tmpangle = %.02f \t tmpeng = %.02f\n",i,tmpangle,tmpeng);
    if(tmpangle<lastangle) {
      printf(ALERTTEXT "WARNING: the abscissa(theta) is no longer increasing; Drawing spline will fail." RESET_COLOR "\n");
      printf(ALERTTEXT "         try Evslab_graph to see what this looks like.                         " RESET_COLOR "\n");
      return 0;
    }
    lastangle = tmpangle;
    if(tmpangle<1 || tmpangle > (GetMaxAngle(fVcm[part])*rad2deg) -1)
      continue;
    if(tmpeng>1e15||tmpeng<0.0)
      continue;

    angle.push_back(GetThetalab(part)* (1/deg2rad));
    energy.push_back(GetTlab(part) * 1000); 

  }


  TGraph graph(angle.size(), angle.data(), energy.data());
  TSpline3* spline = new TSpline3("ETh_lab",&graph);
  return spline;
}



TGraph* TKinematics::Evslab_graph(double thmin, double thmax, double size, int part){
  //double* energy = new double[(int)((thmax-thmin)/size)+1];
  //double* angle = new double[(int)((thmax-thmin)/size)+1];

  if(part<2||part>3) {
    printf(ALERTTEXT "WARNING: the function Evslab_graph should use nuclei after the reaction (part 2 or part 3)" RESET_COLOR "\n");
    return 0;
  }

  std::vector<double> energy;
  std::vector<double> angle;

  double rad2deg=180.0/PI;
  double deg2rad=PI/180.0;

  int steps = ((int)(thmax+1) - (int)thmin) / (int)size;   // when is size ever needed to be a double?? pcb.
                                                           // i am under the impression that size should always be 1.0;
                                                           //
  //for(int i=0;i<((thmax-thmin)/size);i++){
  for(int i=0;i<steps;i++){
    Final((thmin+i*size)*deg2rad,2);//part);   //2);
    double tmpangle = GetThetalab(part)*(1/deg2rad);
    double tmpeng   = GetTlab(part) * 1000;
    //printf("step[%i] \t tmpangle = %.02f \t tmpeng = %.02f\n",i,tmpangle,tmpeng);
    if(tmpangle<1 || tmpangle > (GetMaxAngle(fVcm[part])*rad2deg) -1)
      continue;
    if(tmpeng>1e15||tmpeng<0.0)
      continue;

    angle.push_back(GetThetalab(part)* (1/deg2rad));
    energy.push_back(GetTlab(part) * 1000); 

  }


  TGraph *graph = new TGraph(angle.size(), angle.data(), energy.data());
  //TSpline3* spline = new TSpline3("ETh_lab",&graph);
  return graph;
}










TSpline3* TKinematics::Evscm(double thmin, double thmax, double size, int part){
  double* energy = new double[(int)((thmax-thmin)/size)+1];
  double* angle = new double[(int)((thmax-thmin)/size)+1];
  int number =0;
  double deg2rad=PI/180.;
  for(int i=0;i<((thmax-thmin)/size);i++){
    Final((thmin+i*size)*deg2rad,2);
    angle[i]=GetThetacm(part)*180./PI;
    energy[i]=GetTlab(part); 
    number++;
  }
  TGraph* graph = new TGraph(number, angle, energy);
  TSpline3* spline = new TSpline3("ETh_cm",graph);
  delete graph;
  delete[] angle;
  delete[] energy;
  return spline;
}

double TKinematics::GetExcEnergy(TLorentzVector recoil){
//Gets the excitation energy of the recoil in the CM frame using a 4-vector
  TLorentzVector ejectile;
  recoil.Boost(0,0,-GetBetacm()); //boost to cm system
  
  ejectile.SetVect( -recoil.Vect() ); //pr = -pe
  ejectile.SetE(GetCmEnergy() - recoil.E()); //Ee=Ecm-Er
  
  ejectile.Boost(0,0,GetBetacm()); //boost to lab system

  double eex = ejectile.M() - fParticle[3]->GetMass();

  return eex;
							    
}

double TKinematics::GetExcEnergy(TVector3 position,double KinE){
//Gets the excitation energy of the recoil in the CM frame using a vector & energy
	TLorentzVector recoil;

//	double TotalEnergy = fParticle[2]->GetMass()+KinE;
	double TotalEnergy = fM[2]+KinE;
		
//	recoil.SetVect(position); 
//	recoil.SetE(TotalEnergy);
	
  //printf( "ejectile_lab = [%.05f,\t %.05f %.05f %.05f]\t\t magnitude = %.05f \n",recoil[3],recoil[0],recoil[1],recoil[2],recoil.Mag()); 
	position.SetMag( sqrt( pow(TotalEnergy,2) - pow(TotalEnergy-KinE,2) ) );
  recoil.SetXYZT(position.X(),position.Y(),position.Z(),TotalEnergy);

  return GetExcEnergy(recoil);
}

double TKinematics::GetExcEnergy(double theta, double KinE){

double val1 = (fM[0]+fM[1])*(fM[0]+fM[1])+fM[2]*fM[2]+2*fM[1]*fT[0];
double val2 = 2*fGamma_cm*sqrt((fM[0]+fM[1])*(fM[0]+fM[1])+2*fM[1]*fT[0]);
double val3 = fM[2] + KinE - fBeta_cm*sqrt(KinE*KinE+2*fM[2]*KinE)*TMath::Cos(theta);

  return sqrt(val1-val2*val3)-fM[3];  
}


double TKinematics::GetBeamEnergy(double LabAngle, double LabEnergy){
// Returns the beam energy given the lab angle and energy of the ejectile
  double ProjectileMass=fM[0];
  double TargetMass=fM[1];

  double ts = pow(TargetMass,2);
  double ps = pow(ProjectileMass,2);
  double cs = pow(cos(LabAngle),2);
  double es = pow(LabEnergy,2);
  double te = TargetMass*LabEnergy;

  return (-8*ProjectileMass - 4*TargetMass + LabEnergy/cs - 2*LabEnergy*tan(LabAngle) 
	  + sqrt(16*ts*cs + LabEnergy*(LabEnergy/cs*pow(cos(LabAngle) - sin(LabAngle),4) + 8*TargetMass*(3 + sin(2*LabAngle))))/cos(LabAngle) 
	  + sqrt((24*ts*LabEnergy*cs + 
		  32*ps*TargetMass*pow(cos(LabAngle),4) + 
		  2*TargetMass*es*pow(cos(LabAngle) - sin(LabAngle),4) + 
		  16*ts*LabEnergy*pow(cos(LabAngle),3)*sin(LabAngle) - 
		  8*ps*LabEnergy*cs*(sin(2*LabAngle) - 1) + 
		  2*ps*cos(3*LabAngle)*sqrt(2*(4*ts + 12*te + es) + 
					    (8*ts - 2*es)*cos(2*LabAngle) + 
					    LabEnergy*(LabEnergy/cs + 8*TargetMass*sin(2*LabAngle) - 4*LabEnergy*tan(LabAngle))) + 
		  2*cos(LabAngle)*(3*ps + te - te*sin(2*LabAngle))*sqrt(2*(4*ts + 12*te + es) + (8*ts - 2*es)*cos(2*LabAngle) + LabEnergy*(LabEnergy/cs + 8*TargetMass*sin(2*LabAngle) - 4*LabEnergy*tan(LabAngle))))/(pow(cos(LabAngle),4)*TargetMass)))/8.;
}


void TKinematics::Initial(){
// An initializing function that sets the energies and momenta of the beam and target in the lab and CM frame,
// as well as a few basic calculations.
  fT[0]=fEBeam; // KE of beam in lab
  fT[1]=0; // KE of target in lab
  fE[0]=E_tm(fT[0],fM[0]); // total E of beam in lab
  fE[1]=E_tm(fT[1],fM[1]); // total E of target in lab
  fP[0]=sqrt(fT[0]*fT[0]+2*fT[0]*fM[0]); // momentum of beam in lab
  fP[1]=0;			// momentum of target in lab
  fV[0]=V_pe(fP[0],fE[0]); // velocity of beam in lab 
  fV[1]=V_pe(fP[1],fE[1]); // velocity of target in lab

  fEcm[0]=GetCmEnergy(fEBeam)/2+(fM[0]*fM[0]-fM[1]*fM[1])/(2*GetCmEnergy(fEBeam));
  fEcm[1]=GetCmEnergy(fEBeam)/2-(fM[0]*fM[0]-fM[1]*fM[1])/(2*GetCmEnergy(fEBeam));
  fTcm[0]=fEcm[0]-fM[0];
  fTcm[1]=fEcm[1]-fM[1];
  fPcm[0]=Pcm_em(fEcm[0],fM[0]);
  fPcm[1]=Pcm_em(fEcm[1],fM[1]);
  fVcm[0]=V_pe(fPcm[0],fEcm[0]);
  fVcm[1]=V_pe(fPcm[1],fEcm[1]);

  fBeta_cm = (fP[0]-fP[1])/(fE[0]+fE[1]);
  fBetacm[0] = betacm_tm(fTcm[0],fM[0]);
  fBetacm[1] = -betacm_tm(fTcm[1],fM[1]);
  fGamma_cm = 1/sqrt(1-fBeta_cm*fBeta_cm);
  fTCm_i = GetCmEnergy(fEBeam)-fM[0]-fM[1];
  fTCm_f = fTCm_i + fQValue;
//  printf("\n\nfM[0] = %.05f \tfM[1] = %.05f \tfM[2] = %.05f \tfM[3] = %.05f \n",fM[0],fM[1],fM[2],fM[3]);
 // printf("fT[0] = %.05f	\tfT[1] = %.05f	\tfE[0] = %.05f	\tfE[1] = %.05f	\tfP[0] = %.05f	\tfP[1] = %.05f	\nfV[0] = %.05f	\tfV[1] = %.05f	\tfEcm[0] = %.05f	\tfEcm[1] = %.05f	\tfECM = %.05f	\tfBeta_cm = %.04f	\tfGamma_cm = %.04f \n\n\n",	fT[0],fT[1],fE[0],fE[1],fP[0],fP[1],fV[0],fV[1],fEcm[0],fEcm[1],GetCmEnergy(fEBeam),fBeta_cm,fGamma_cm);

}
void TKinematics::FinalCm(){
// Calculates the recoil and ejectile energies and momenta in the CM frame

//angle of proton in cm system
  if(fParticle[2]==NULL && fParticle[3]==NULL){
    fM[2]=fParticle[1]->GetMass();
    fM[3]=fParticle[0]->GetMass();     
  }
  fTcm[2]=fTCm_f/2*(fTCm_f+2*fM[3])/GetCmEnergy(fEBeam);
  fTcm[3]=fTCm_f/2*(fTCm_f+2*fM[2])/GetCmEnergy(fEBeam);
  fEcm[2]=E_tm(fTcm[2],fM[2]);
  fEcm[3]=E_tm(fTcm[3],fM[3]);
  fPcm[2]=Pcm_em(fEcm[2],fM[2]);
  fPcm[3]=Pcm_em(fEcm[3],fM[3]);
  fVcm[2]=V_pe(fPcm[2],fEcm[2]);
  fVcm[3]=V_pe(fPcm[3],fEcm[3]);
  fBetacm[2]=-betacm_tm(fTcm[2],fM[2]);
  fBetacm[3]=betacm_tm(fTcm[3],fM[3]);
}
void TKinematics::Final(double angle, int part){
// Calculates the recoil and ejectile energies and momenta in the lab frame

//angle of proton in lab system
  if(angle>GetMaxAngle(fVcm[part]))
    SetAngles(0, part);
  else
    SetAngles(angle, part);
  fE[2]=E_final(2);
  fE[3]=E_final(3);
  fT[2]=T_final(2);
  fT[3]=T_final(3);
  //fP[2]=fGamma_cm*(fPcm[2]+fBeta_cm*fEcm[2]);
  //fP[3]=fGamma_cm*(fPcm[3]+fBeta_cm*fEcm[3]);
  fP[2]=P_tm(fT[2],fM[2]);
  //fP[2]=fGamma_cm*fPcm[2]*(cos(fThetacm[2])+fBeta_cm/fVcm[2]);
  fP[3]=P_tm(fT[3],fM[3]);
  //fP[3]=fGamma_cm*fPcm[3]*(cos(fThetacm[3])+fBeta_cm/fVcm[3]);
  fV[2]=V_pe(fP[2],fE[2]);
  fV[3]=V_pe(fP[3],fE[3]);
}

double TKinematics::ELab(double angle_lab, int part){
// Calculates the energy of a particle "part" in the lab given the ejectile lab angle 
  Final(angle_lab, part);
  return GetTlab(part); 
}

void TKinematics::SetAngles(double angle, int part, bool upper){
// Set the angle for a particle "part"
  int given;
  int other;
  if(part==2){
    given =2;
    other =3;
  }
  else if(part==3){
    given =3;
    other =2;
  }
  else{
    std::cout << " error in TKinematics::SetAngles("<<angle<<", "<<part<<") "<<std::endl;
    std::cout << " part must be 2 or 3 " << std::endl;
    exit(4);
  } 
  fTheta[given]=angle;
  fThetacm[given]=Angle_lab2cm(fVcm[given],fTheta[given]);
  if(given==3&&(fParticle[0]->GetMass()>fParticle[1]->GetMass())){
    fThetacm[given]=Angle_lab2cminverse(fVcm[given],fTheta[given],upper);
  }
  fThetacm[other]=PI-fThetacm[given];
  if(fTheta[given]==0)
    fTheta[other]=PI/given;
  else
    fTheta[other]=Angle_cm2lab(fVcm[other],fThetacm[other]);

//  fTheta[3]=angle;
//  fThetacm[3]=Angle_lab2cm(fVcm[3],fTheta[3]);
//  fThetacm[2]=PI-fThetacm[3];
//  if(fTheta[3]==0)
//    fTheta[2]=PI/2;
//  else
//    fTheta[2]=Angle_cm2lab(fVcm[2],fThetacm[2]);
}


double TKinematics::GetCmEnergy(double ebeam){
// Returns the total energy of the CM system, given the beam energy
  double ecm;
  ecm = sqrt(fM[0]*fM[0]+fM[1]*fM[1]+2.*fM[1]*(fM[0]+ebeam));
  return ecm;
}
double TKinematics::GetCmEnergy(){
// Returns the total energy of the CM system
  return GetCmEnergy(fEBeam);
}
double TKinematics::NormalkinEnergy(){
// Returns the total kinetic energy of the system, assuming normal kinematics
  double ENorm = (GetCmEnergy(fEBeam)*GetCmEnergy(fEBeam)-fM[0]*fM[0]-fM[1]*fM[1])/(2*fM[0])-fM[1];
  return ENorm;
}

double TKinematics::GetMaxAngle(double vcm){
// Returns the maximum angle of the ejectile in the CM frame
  double x;
  x = fBeta_cm/vcm;
  if(x*x<1)
    return PI;
  else
    return atan2(sqrt(1/(x*x-1)),fGamma_cm);
}
double TKinematics::GetMaxAngle(int part){
// Returns the maximum angle of a given particle "part"
  return GetMaxAngle(fVcm[part]);
}
bool TKinematics::CheckMaxAngle(double angle, int part){
// A check to ensure the angle for a given particle is allowed (i.e. less than the max angle)
  return angle <= GetMaxAngle(fVcm[part]);
}
double TKinematics::Angle_lab2cm(double vcm, double angle_lab){
// Converts the lab angle to the CM angle given the velocity in the CM frame
  double tan_lab, gtan,x;
  tan_lab = tan(angle_lab);
  gtan = tan_lab*tan_lab*fGamma_cm*fGamma_cm;
  x = fBeta_cm/vcm;

  if(tan_lab>=0){
    return acos( (-x*gtan+sqrt( 1+gtan*(1-x*x) ))/(1+gtan) );
  }
  else{
    return acos( (-x*gtan-sqrt( 1+gtan*(1-x*x) ))/(1+gtan) );
  }
}
double TKinematics::Angle_lab2cminverse(double vcm, double angle_lab, bool upper){
// Converts the lab angle to the CM angle given the velocity in the CM frame under inverse kinematics
  double tan_lab, gtan,x;
  tan_lab = tan(angle_lab);
  gtan = tan_lab*tan_lab*fGamma_cm*fGamma_cm;
  x = fBeta_cm/vcm;

  if(upper){
    return acos( (-x*gtan+sqrt( 1+gtan*(1-x*x) ))/(1+gtan) );
  }
  else{
    return acos( (-x*gtan-sqrt( 1+gtan*(1-x*x) ))/(1+gtan) );
  }
}

double TKinematics::Steffen_cm2labinverse(double theta_cm, int part){
  //Final(theta_lab,part);
  //FinalCm();  
  double v_in_cm = fVcm[part];
  double beta_of_cm = fBeta_cm;
  double gamma_of_cm = fGamma_cm;

  double theta_lab = PI-TMath::ATan2(sin(theta_cm),gamma_of_cm*(cos(theta_cm)-beta_of_cm/v_in_cm));

  return theta_lab;
}

double TKinematics::Steffen_lab2cminverse(double theta_lab){ // assumes part = 2;

  if(!Cm2LabSpline)
    Cm2LabSpline = Steffen_labvscminverse(0.01,179.9,1.0,2);

  return Cm2LabSpline->Eval(theta_lab);
}


void TKinematics::AngleErr_lab2cm(double angle, double &err){
// Calculates the uncertainty associated with converting the angle from the lab to CM frame
  double angle_lab = angle;
  angle = Angle_lab2cm(fVcm[2], angle_lab);
  err =  fabs(Angle_lab2cm(fVcm[2], angle_lab+err)-Angle_lab2cm(fVcm[2], angle_lab-err))/2.;
  /*
  double tang, tang2, gtang, g2,x;
  tang = tan(angle_lab);
  gtang = tang*tang*fGamma_cm*fGamma_cm;
  tang2 = tang*tang;
  g2= fGamma_cm*fGamma_cm;
  x = fBeta_cm/fVcm[2];
  
  if(tang>=0){
    err *= fabs((-2*x*tang*g2*(1*tang2)+tang*g2*(1-x*x)*(1+tang2)/sqrt(1+gtang*(1-x*x)))/(1-gtang) + 2*(-x*gtang+sqrt(1+gtang*(1-x*x))*(1+tang2)*tang*g2)/(1-gtang)/(1-gtang) )/sqrt(1-((-x*gtang+sqrt( 1+gtang*(1-x*x) ))/(1+gtang))*((-x*gtang+sqrt( 1+gtang*(1-x*x) ))/(1+gtang)));
  }
  else{
    err *= fabs((-2*x*tang*g2*(1*tang2)-tang*g2*(1-x*x)*(1+tang2)/sqrt(1+gtang*(1-x*x)))/(1-gtang) + 2*(-x*gtang-sqrt(1+gtang*(1-x*x))*(1+tang2)*tang*g2)/(1-gtang)/(1-gtang) )/sqrt(1-((-x*gtang-sqrt( 1+gtang*(1-x*x) ))/(1+gtang))*((-x*gtang-sqrt( 1+gtang*(1-x*x) ))/(1+gtang)));
    if(err>1){
      cout << "part1 " << (-2*x*tang*g2*(1*tang2)-tang*g2*(1-x*x)*(1+tang2)/sqrt(1+gtang*(1-x*x)))/(1-gtang) << endl;
      cout << "part2 " << 2*(-x*gtang-sqrt(1+gtang*(1-x*x))*(1+tang2)*tang*g2)/(1-gtang)/(1-gtang) << endl;
    }
  }
  */
}
double TKinematics::Angle_cm2lab(double vcm, double angle_cm){
// Converts the CM angle to the lab angle given the velocity in the CM frame
  double x;
  x = fBeta_cm/vcm;
  return atan2(sin(angle_cm),fGamma_cm*(cos(angle_cm)+x));
  /*
  cout << " old " << atan2(sin(angle_cm),fGamma_cm*(cos(angle_cm)+x)) << " x " << x << endl;
  double gam2 = fM[0]*fM[2]/fM[1]/fM[3]*fTCm_i/fTCm_f;
  gam2 = sqrt(gam2);
  double gam3 = fM[0]*fM[3]/fM[1]/fM[2]*fTCm_i/fTCm_f;
  gam3 = sqrt(gam3);
  double y2=1.+gam2*gam2+2.*gam2*cos(angle_cm);
  double y3=1.+gam3*gam3+2.*gam3*cos(PI-angle_cm);
  cout << "y2 " << y2 << " y3 " << y3 << "\n";
  if(asin(sin(PI-angle_cm)/sqrt(y2)) < PI-acos(-gam2))
    cout << " new " << PI - asin(sin(PI-angle_cm)/sqrt(y2)) << " = asin("<<sin(PI-angle_cm)<<"/"<<sqrt(y2)<<")" << endl;
  else 
    cout << " new " << asin(sin(PI-angle_cm)/sqrt(y2)) << " = asin("<<sin(PI-angle_cm)<<"/"<<sqrt(y2)<<")" << endl;
  */

}
//x = sqrt(fM[0]*fM[3]/fM[1]/fM[2]*fTCm_i/fTCm_f);
//cout << "thorsten\t" << asin(sin(angle_cm)/sqrt(1+x*x+2*x*cos(angle_cm)))*180./PI << endl;
//cout << "ich\t" << atan2(sin(angle_cm),fGamma_cm*(cos(angle_cm)+x))*180./PI << endl;  
TSpline3* TKinematics::labvscm(double thmin, double thmax, double size, int part){
  double* cm = new double[(int)((thmax-thmin)/size)+1];
  double* lab = new double[(int)((thmax-thmin)/size)+1];
  int nr =0;
  for(int i=0;i<((thmax-thmin)/size);i++){
    cm[nr] = i;
    lab[nr] = Angle_cm2lab(fVcm[part],cm[nr]*PI/180.)*180./PI;
    if(lab[nr]>0.01 && lab[nr]<179.99)
      nr++;
  }
  TGraph* graph = new TGraph(nr, cm, lab);
  TSpline3* spline = new TSpline3("Th_cmvslab",graph);
  delete graph;
  delete[] lab;
  delete[] cm;
  return spline;
}

TSpline3* TKinematics::cmvslab(double thmin, double thmax, double size, int part){
  double* cm = new double[(int)((thmax-thmin)/size)+1];
  double* lab = new double[(int)((thmax-thmin)/size)+1];
  int nr =0;
  for(int i=0;i<((thmax-thmin)/size);i++){
    cm[nr] = i;
    lab[nr] = Angle_cm2lab(fVcm[part],cm[nr]*PI/180.)*180./PI;
    if(lab[nr]>0.01 && lab[nr]<179.99)
      nr++;
  }
  TGraph* graph = new TGraph(nr, lab, cm);
  TSpline3* spline = new TSpline3("Th_cmvslab",graph);
  delete graph;
  delete[] lab;
  delete[] cm;
  return spline;
}

TSpline3* TKinematics::Steffen_labvscminverse(double thmin, double thmax, double size, int part){
  double* cm = new double[(int)((thmax-thmin)/size)+1];
  double* lab = new double[(int)((thmax-thmin)/size)+1];
  int nr =0;
  for(int i=((thmax-thmin)/size);i>0;i--){
    cm[nr] = i;
    lab[nr] = Steffen_cm2labinverse(cm[nr]*PI/180.,part)*180./PI;
    if(lab[nr]>0.01 && lab[nr]<179.99)
      nr++;
  }
  TGraph* graph = new TGraph(nr, lab, cm);
  TSpline3* spline = new TSpline3("Th_cmvslabinverse",graph);
  delete graph;
  delete[] lab;
  delete[] cm;
  return spline;
}

double TKinematics::Sigma_cm2lab(double angle_cm, double sigma_cm){
  double gam2 = fM[0]*fM[2]/fM[1]/fM[3]*fTCm_i/fTCm_f;
  gam2 = sqrt(gam2);
  double wurzel=1.+gam2*gam2+2.*gam2*cos(PI-angle_cm);
  wurzel = sqrt(wurzel);
  return sigma_cm*(1/fGamma_cm*wurzel*wurzel*wurzel/(1+gam2*cos(PI-angle_cm)));
}
/*
double TKinematics::Sigma_cm2lab(double angle_cm, double sigma_cm){
  //old
  double x;
  x = fBeta_cm/fVcm[2];
  double wurzel;
  wurzel = sqrt(sin(PI-angle_cm)*sin(PI-angle_cm) + fGamma_cm*fGamma_cm*(cos(PI-angle_cm)+x)*(cos(PI-angle_cm)+x));
  return sigma_cm*(1/fGamma_cm*wurzel*wurzel*wurzel/(1+x*cos(PI-angle_cm)));  
}
*/


double TKinematics::Sigma_lab2cm(double angle_cm, double sigma_lab){
  double gam2 = fM[0]*fM[2]/fM[1]/fM[3]*fTCm_i/fTCm_f;
  gam2 = sqrt(gam2);
  //test
  //double x;
  //x = fBeta_cm/fVcm[2];
  //cout << "x " << x << " gam2 "<< gam2 << " cm "<< angle_cm << " pi - cm "<< PI-angle_cm << endl;
  //test
  
  //double angle_cm = Angle_lab2cm(fVcm[2], angle_lab);
  //orig
  double wurzel=1.+gam2*gam2+2.*gam2*cos(PI-angle_cm);
  wurzel = sqrt(wurzel);
  return sigma_lab/(1/fGamma_cm*wurzel*wurzel*wurzel/(1+gam2*cos(PI-angle_cm)));  
  //test no pi-
  //double wurzel=1.+gam2*gam2+2.*gam2*cos(angle_cm);
  //wurzel = sqrt(wurzel);
  //return sigma_lab/(1/fGamma_cm*wurzel*wurzel*wurzel/(1+gam2*cos(angle_cm)));  
}


void TKinematics::SigmaErr_lab2cm(double angle, double err, double &sigma, double &errsigma){
  double g = fM[0]*fM[2]/fM[1]/fM[3]*fTCm_i/fTCm_f;
  g = sqrt(g);
  double w=1.+g*g+2.*g*cos(PI-angle);
  w = sqrt(w);
  errsigma = fGamma_cm/pow(w,1.5) * sqrt( pow(sigma*g*sin(PI-angle)*(-2+g*g-g*cos(PI-angle))/w * err,2) + pow((1+g*cos(PI-angle))*errsigma,2  ) ); 
  //sigma/=(1/fGamma_cm*w*w*w/(1+g*cos(PI-angle)));
}


void TKinematics::Transform2cm(double &angle, double &sigma){
  //double angle_lab = angle;
  angle = PI-Angle_lab2cm(fVcm[2], angle);
  sigma = Sigma_lab2cm(angle, sigma);
  return;
}


void TKinematics::Transform2cm(double &angle, double &errangle, double &sigma, double &errsigma){
  AngleErr_lab2cm(angle, errangle);
  Transform2cm(angle,sigma);
  SigmaErr_lab2cm(angle, errangle, sigma, errsigma);
  return;
}


double TKinematics::Rutherford(double angle_cm){
// Returns the Rutherford scattering impact parameter, b, given the angle of the ejectile in the CM frame
  double a = 0.5*1.43997649*fParticle[0]->GetZ()*fParticle[1]->GetZ()/fTCm_i;
  double b = sin(angle_cm/2.)*sin(angle_cm/2.);
  b=b*b;
  return a*a/b*0.0025;//1b=0.01fm
}


TSpline3* TKinematics::Ruthvscm(double thmin, double thmax, double size){
  double* cross = new double[(int)((thmax-thmin)/size)+1];
  double* angle = new double[(int)((thmax-thmin)/size)+1];
  int number =0;
  for(int i=0;i<((thmax-thmin)/size);i++){
    angle[i]=thmin+i*size;
    if(angle[i]>179.99||angle[i]<0.01)
      break;
    cross[i]=Rutherford(angle[i]*PI/180.);
    number++;

    //cout << angle[i] << "   " << Rutherford(GetThetacm(2))<<endl;
    //cout << setprecision(4) << GetThetacm(3)/deg2rad << "\t" << setprecision(4) << GetThetacm(2)/deg2rad << "\t" << setprecision(4) << GetThetalab(3)/deg2rad << "\t" << setprecision(4) << GetThetalab(2)/deg2rad << "\t" << setprecision(4) << Rutherford(GetThetacm(3)) << "\t" << setprecision(4) << Rutherford(GetThetacm(2)) << endl;  
    //cout << (thmin+i*size)*PI/180. << "  max angle: " << GetMaxAngle(fVcm[2]) << endl;
  }
  TGraph* graph = new TGraph(number, angle, cross);
  TSpline3* spline = new TSpline3("sigmaTh_cm",graph);
  delete graph;
  delete[] angle;
  delete[] cross;
  return spline;
}


TSpline3* TKinematics::Ruthvslab(double thmin, double thmax, double size, int part){
  double* cross = new double[(int)((thmax-thmin)/size)+1];
  double* angle = new double[(int)((thmax-thmin)/size)+1];
  int number =0;
  for(int i=0;i<((thmax-thmin)/size);i++){
    if(part==3||part==2)
      angle[i]=thmin+i*size; //angle[i] is in cm system
    else{
      std::cout << "error " << std::endl;
      exit(1);
    }
    if(angle[i]>179.99||angle[i]<0.01)
      break;
    cross[i]=Rutherford(angle[i]*PI/180.);
    number++;
    //cout << " angle cm " << angle[i]; 
    //cout << " cs cm " << cross[i];
    
    cross[i]=Sigma_cm2lab(angle[i]*PI/180., Rutherford(angle[i]*PI/180.));
    if(part==2){
      angle[i]=180-angle[i];
    }
    angle[i]=Angle_cm2lab(fVcm[part],angle[i]*PI/180.)*180./PI;
    //cout << fVcm[part] << "fVcm[part]" << endl;
    //cout << "\t\tangle lab " << angle[i]; 
    //cout << " cs lab " << cross[i] << endl;; 

  /* for(int i=0;i<(thmax-thmin)/size;i++){
    Final((thmin+i*size)*PI/180.);
    angle[i]=(thmin+i*size);
    if(CheckMaxAngle(angle[i]*PI/180., part)){
      cout << " angle lab " << angle[i] << " GetThetacm(2) " << GetThetacm(2)*180./PI << " GetThetacm(3) " << GetThetacm(3)*180./PI << endl;
      if(angle[i]>179.999||angle[i]<0.001)
	continue;
      if( fabs(GetMaxAngle(fVcm[part])*180./PI-angle[i]) <size)
	continue;
      
      cross[i]= Sigma_cm2lab(fVcm[part], GetThetacm(2), Rutherford(GetThetacm(2)));
      cout << " cs2 cm " <<  Rutherford(GetThetacm(2));
      cout << " cs3 cm " <<  Rutherford(GetThetacm(3));
      cout << " cs lab " << cross[i] << endl;
      if(cross[i]<1e-9)
	continue;
      if(cross[i]>1e+9)
	continue;
      number++;
    }
    else
      continue;
*/
  }
  TGraph* graph = new TGraph(number, angle, cross);
  TSpline3* spline = new TSpline3("sigmaTh_lab",graph);
  delete graph;
  delete[] angle;
  delete[] cross;
  return spline;
}

double TKinematics::Pcm_em(double e, double m){
  return sqrt(e*e-m*m);
}
double TKinematics::P_tm(double t, double m){
  return sqrt(t*t+2.*t*m);
}
double TKinematics::E_tm(double t, double m){
  return t+m;
}
double TKinematics::T_em(double e, double m){
  return e-m;
}
double TKinematics::betacm_tm(double t, double m){
  return sqrt(t*t+2*t*m)/(t+m);
}
double TKinematics::V_pe(double p, double e){
  return p/e;
}
double TKinematics::E_final(int i){
  return fGamma_cm*(fEcm[i]+fBeta_cm*fPcm[i]);
}
double TKinematics::T_final(int i){
  //cout <<(fGamma_cm-1)*fM[i]<<" + "<<fGamma_cm*fTcm[i]<<" + "<<fGamma_cm*fBeta_cm*fPcm[i]*cos(fThetacm[i])<<endl;
  //cout <<fGamma_cm<<"*"<<fBeta_cm<<"*"<<fPcm[i]<<"*"<<cos(fThetacm[i])
  return (fGamma_cm-1)*fM[i]+fGamma_cm*fTcm[i]+fGamma_cm*fBeta_cm*fPcm[i]*cos(fThetacm[i]); 
}
