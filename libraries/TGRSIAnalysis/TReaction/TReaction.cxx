//g++ -c -fPIC TReaction.cxx -I./ `root-config --cflags`

#include "TReaction.h"
#include "Globals.h"

#include<TStyle.h>

ClassImp(TReaction);

TReaction::TReaction(const char *beam, const char *targ, const char *ejec, const char *reco, double beame, double ex3, bool inverse){
	
	Clear();
	// I THINK INVERSE KINEMATICS NECESSITATES THE BEAM<->TARGET ENTIRELY ?
  fNuc[0] = new TNucleus(beam);
  fNuc[1] = new TNucleus(targ);
  fNuc[2] = new TNucleus(ejec);
  fNuc[3] = new TNucleus(reco);
  
  for(int i=0;i<4;i++)
    fM[i]=fNuc[i]->GetMass();
    
  fTBeam = beame;
  fQVal = (fM[0]+fM[1])-(fM[2]+fM[3])-ex3;  // effective Q value (includes excitation)
  fExc = ex3;
  fInverse = inverse;    

  if(inverse)
  	SetName(Form("%s(%s,%s)%s",beam,targ,ejec,reco));
  else
  	SetName(Form("%s(%s,%s)%s",targ,beam,ejec,reco));  
  
  InitReaction();  
}


void TReaction::InitReaction(){

// An initializing function that sets the energies and momenta of the beam and target in the lab and CM frame,
// as well as a few basic calculations.	
	
	fTLab[0] = fTBeam;																 // target kinetic energy is ebeam
	fELab[0] = fTLab[0] + fM[0]; 											 // total E of beam in lab
  fPLab[0] = sqrt(pow(fTLab[0],2)+2*fTLab[0]*fM[0]); // momentum of beam in lab
  fVLab[0] = fPLab[0]/fELab[0]; 										 // velocity of beam in lab 
  fGLab[0] = 1/sqrt(1-pow(fVLab[0],2));						   // gamma factor of beam
	
	fTLab[1] = 0;							 	 											 // target kinetic energy is always 0
	fELab[1] = fM[1]; 											 					 // total E of target in lab
  fPLab[1] = 0;																			 // momentum of target in lab  
  fVLab[1] = 0; 																		 // velocity of target in lab
  fGLab[1] = 1;						   												 // gamma factor of target

	fS = pow(fM[0],2)+pow(fM[1],2)+2*fELab[0]*fM[1];	 // fELab[1] = fM[1]
	fInvariantMass = sqrt(fS);											   // √s
	
	// CM motion
	fCmE = fInvariantMass;
	fCmTi = fCmE - fM[0] - fM[1];
	fCmTf = fCmTi + fQVal; 
	fCmV = fPLab[0]/(fELab[0]+fM[1]);
	fCmP = fCmV*fCmE;
	fCmG = 1/sqrt(1-pow(fCmV,2));
	
	// take care of particles in CM frame using the reaction excitation energy 
	SetCmFrame(fExc);
	
	// options to make graphs draw nicely
	gStyle->SetTitleYOffset(1.5);
	gStyle->SetTitleXOffset(1.2);	
	gStyle->SetDrawOption("AC");

}

double TReaction::GetTBeam(bool inverse) { 
  if(fInverse || inverse)
    return (fGLab[0]-1)*fM[1];
  else
    return fTLab[0];
}

void TReaction::SetCmFrame(double exc){
	
	// particles in CM frame
	fPCm[0] = sqrt((fS-pow(fM[0]-fM[1],2)) * (fS-pow(fM[0]+fM[1],2)))/(2*sqrt(fS));
	fPCm[1] = fPCm[0]; 
	fPCm[2] = sqrt((fS-pow(exc+fM[2]-fM[3],2)) * (fS-pow(exc+fM[2]+fM[3],2)))/(2*sqrt(fS)); 
	fPCm[3] = fPCm[2];
	
	for(int i=0; i<4; i++){
		fECm[i] = sqrt(pow(fM[i],2)+pow(fPCm[i],2));
		fTCm[i] = fECm[i]-fM[i];
		fVCm[i] = fPCm[i]/fECm[i];
		fGCm[i] = 1/sqrt(1-pow(fVCm[i],2));
			
		// max lab frame theta
		if(i<2)
			fThetaMax[i] = 0;
		else{
			double val = fPCm[i]/(fM[i]*fCmV*fCmG);
			if(val<1)
				fThetaMax[i] = asin(val);			
			else if(val<1.001) // catches elastic channels with small numerical rounding errors
					fThetaMax[i] = PI/2;
			else	
				fThetaMax[i] = PI;
		}	
	}
}

// particle properties in LAB frame
// These guys do the math to get lab frame values using CM angles
double TReaction::GetELabFromThetaCm(double theta_cm, int part){
	if(part==0 || part==1) return fTLab[part];
	
	return fCmG*(fECm[part] - fCmV*fPCm[part]*cos(theta_cm));
}

double TReaction::GetTLabFromThetaCm(double theta_cm, int part){
	if(part==0 || part==1) return fTLab[part];
	
	double ELab = GetELabFromThetaCm(theta_cm,part);		
	return ELab-fM[part]; // T = E - M
}

double TReaction::GetVLabFromThetaCm(double theta_cm, int part){
	if(part==0 || part==1) return fTLab[part];

	double PLab = GetPLabFromThetaCm(theta_cm,part);
	double ELab = GetELabFromThetaCm(theta_cm,part);
	return PLab/ELab; // V = P / E
}

double TReaction::GetPLabFromThetaCm(double theta_cm, int part){
	if(part==0 || part==1) return fTLab[part];

	double Pz = fCmG*(fPCm[part]*cos(theta_cm)-fCmV*fECm[part]);
	double Pperp = fPCm[part]*sin(theta_cm);
	return sqrt(pow(Pperp,2)+pow(Pz,2));
}

double TReaction::GetGLabFromThetaCm(double theta_cm, int part){
	if(part==0 || part==1) return fTLab[part];
	
	double VLab = GetVLabFromThetaCm(theta_cm,part);
	return 1/sqrt(1-pow(VLab,2));
}


double TReaction::GetExcEnergy(double ekin, double theta_lab, int part){
	if(ekin==0.00 && theta_lab==0.00)
		return fExc; 

	double val1 = pow(fM[0]+fM[1],2)+pow(fM[2],2)+2*fM[1]*fTLab[0];
	double val2 = 2*fCmG*sqrt(pow(fM[0]+fM[1],2)+2*fM[1]*fTLab[0]);
	double val3 = fM[2] + ekin - fCmV*sqrt(pow(ekin,2)+2*fM[2]*ekin)*TMath::Cos(theta_lab);

  return sqrt(val1-val2*val3)-fM[3];  
}

void TReaction::AnalysisAngDist(double ekin, double theta_lab, int part, double &exc, double &theta_cm, double &omega_lab2cm){

	exc = GetExcEnergy(ekin,theta_lab,part);
	
	// adjust cm frame to include the excited state
	SetCmFrame(exc);
	
	theta_cm = ConvertThetaLabToCm(theta_lab,part);
	omega_lab2cm = ConvertOmegaLabToCm(theta_lab,part);
	
	// reset the cm frame to normal
	SetCmFrame(fExc); 
}

double TReaction::AnalysisBeta(double ekin, int part){

	return sqrt(pow(ekin,2)+2*fM[part]*ekin)/(ekin+fM[part]);
}

// THIS IS ACTUALLY MOTT SCATTERING
double TReaction::GetRutherfordCm(double theta_cm, int part, bool Units_mb){
  static const double alpha = 0.359994;
	double scale = 1;
	if(Units_mb)
		scale = 0.1;  
		
  double a = pow(alpha*fNuc[0]->GetZ()*fNuc[1]->GetZ()/(fPLab[0]*fVLab[0]),2)*scale;
  double b = 1 - pow(fVLab[0]*sin(theta_cm/2),2);
  return a*b/pow(sin(theta_cm/2),4); // ?
  
}

double TReaction::GetRutherfordLab(double theta_lab, int part, bool Units_mb){
	double theta_cm = ConvertThetaLabToCm(theta_lab,part);
	double jacobian = ConvertOmegaCmToLab(theta_cm,part);

	return GetRutherfordCm(theta_cm,part,Units_mb)*jacobian;
}



// Conversion from LAB frame to CM frame
double TReaction::ConvertThetaLabToCm(double theta_lab, int part){

	if(theta_lab>fThetaMax[part])
		theta_lab = fThetaMax[part];

// Uses the particle velocity in the CM frame, which makes it more complex  
  double gtan2 = pow(tan(theta_lab)*fCmG,2);
  double x = fCmV/fVCm[part];
  double expr = sqrt(1+gtan2*(1-pow(x,2)));
	double theta_cm;
	
	// deals with double valued thetas in lab frame
  if(tan(theta_lab)>=0)
    theta_cm = acos((-x*gtan2 + expr)/(1 + gtan2));
  else
    theta_cm = acos((-x*gtan2 - expr)/(1 + gtan2));  
    
  if(fInverse)
  	theta_cm = PI - theta_cm;
  
  if(part==3)
  	theta_cm = -theta_cm;
  		
  return theta_cm;	
}

// dOmegaLab/dOmegaCm[ThetaLab,ThetaCm]
double TReaction::ConvertOmegaLabToCm(double theta_lab, int part){
// the way to test this function is to use the known 4*cos(theta_lab) for elastics
	double theta_cm = ConvertThetaLabToCm(theta_lab,part);
	return 1/ConvertOmegaCmToLab(theta_cm,part);
}

void TReaction::ConvertLabToCm(double theta_lab, double omega_lab, double &theta_cm, double &omega_cm, int part){
	theta_cm = ConvertThetaLabToCm(theta_lab,part);
	omega_cm = ConvertOmegaLabToCm(omega_lab,part);

	return;
}

// Conversion from CM frame to LAB frame 
double TReaction::ConvertThetaCmToLab(double theta_cm, int part){

	if(fInverse)
		theta_cm = PI - theta_cm;
		
	double theta_lab = TMath::ATan2(sin(theta_cm),fCmG*(cos(theta_cm)+fCmV/fVCm[part]));
	
	if(theta_lab>fThetaMax[part])
		return fThetaMax[part];
	else	
		return theta_lab;
}

double TReaction::ConvertOmegaCmToLab(double theta_cm, int part){
	
// the way to test this function is to use the known 4*cos(theta_lab) for elastics
	double x = fCmV/fVCm[part];
	double val1 = pow(pow(sin(theta_cm),2) + pow(fCmG*(x+cos(theta_cm)),2),1.5);
	double val2 = (fCmG*(1+x*cos(theta_cm)));
	
	return val1/val2;
}	

void TReaction::ConvertCmToLab(double theta_cm, double omega_cm, double &theta_lab, double &omega_lab, int part){
	theta_lab = ConvertThetaCmToLab(theta_cm,part);
	omega_lab = ConvertOmegaCmToLab(omega_cm,part);

	return;
}


////////////////////////////////////////////////////////////////////////////////////
// // // // // // // 			 GRAPHS				// // // // // // // // // // // // // // //
////////////////////////////////////////////////////////////////////////////////////

// Kinetic energy (lab frame) versus theta (either frame)
TGraph *TReaction::KinVsTheta(double thmin, double thmax, int part, bool Frame_Lab, bool Units_keV){
	TGraph *g = new TGraph();
	const char *frame = Form("%s",Frame_Lab?"Lab":"Cm");
		
	g->SetName(Form("KinVsTheta%s_%s",frame,GetName()));
	g->SetTitle(Form("Kinematics for %s; Theta_{%s} [deg]; Kinetic energy [%s]",GetName(),frame,Units_keV?"keV":"MeV"));
		
	double theta, T;	

	for(int i=0; i<=180; i++){
		theta = (double)i; // always in CM frame since function is continuous
		
		T = GetTLabFromThetaCm(theta*D2R,part);
		if(Units_keV)
			T *=1e3;	
		
		if(Frame_Lab){ // this is now converted to specified frame (from Frame_Lab)
			theta = ConvertThetaCmToLab(theta*D2R,part)*R2D;
			//if(theta==g->GetX[g->GetN()-1]) // if angle is the same
			//	continue;
		}

		if(theta<thmin || theta>thmax) 
			continue;		// set angular range				
		g->SetPoint(i,theta,T);
	}
	
	return g;
}

// Frame_Lab -> ThetaCm[ThetaLab] 	and 	Frame_Cm -> ThetaLab[ThetaCm]
TGraph *TReaction::ThetaVsTheta(double thmin, double thmax, int part, bool Frame_Lab){
	TGraph *g = new TGraph();
	const char *frame = Form("%s",Frame_Lab?"Lab":"Cm");
	const char *other = Form("%s",!Frame_Lab?"Lab":"Cm");
	
	g->SetName(Form("ThetaVsTheta%s_%s",frame,GetName()));
	g->SetTitle(Form("Angle conversion for %s; Theta_{%s} [deg]; Theta_{%s} [deg]",GetName(),frame,other));
	
	double theta_cm, theta_lab;	
	
	for(int i=0; i<=180; i++){
		theta_cm = (double)i; // always in CM frame
		theta_lab = ConvertThetaCmToLab(theta_cm*D2R,part)*R2D;
		
		if((Frame_Lab && (theta_lab<thmin || theta_lab>thmax)))
			continue;
		else if(!Frame_Lab && (theta_cm<thmin || theta_cm>thmax))
			continue;
		// set angular range	
		
		if(Frame_Lab) // this is now converted to specified frame (from Frame_Lab)
			g->SetPoint(i,theta_lab,theta_cm);
		else
			g->SetPoint(i,theta_cm,theta_lab);
	}
	
	return g;
}

// Frame_Lab -> dOmegaCm/dOmegaLab[ThetaLab] 	and 	Frame_Cm -> dOmegaLab/dOmegaCm[ThetaCm]
TGraph *TReaction::OmegaVsTheta(double thmin, double thmax, int part, bool Frame_Lab){
	TGraph *g = new TGraph();
	const char *frame = Form("%s",Frame_Lab?"Lab":"Cm");
	const char *other = Form("%s",!Frame_Lab?"Lab":"Cm");
	
	g->SetName(Form("%s_OmegaVsTheta%s",GetName(),frame));
	g->SetTitle(Form("Solid angle conversion for %s; Theta_{%s} [deg]; dOmega_{%s} / dOmega_{%s}",GetName(),frame,other,frame));
	
	double theta, Om;	
	for(int i=0; i<=180; i++){

		theta = (double)i; // always in CM frame
		Om = 1/ConvertOmegaCmToLab(theta*D2R,part);
		
		if(Frame_Lab){ // this is now converted to specified frame (from Frame_Lab)
			theta = ConvertThetaCmToLab(theta*D2R,part)*R2D;
			Om = 1/Om;
		} 
		
	//	printf("Theta_%s = %2f deg \t dOmega%s/dOmega%s = %.2e\n",frame,theta,other,frame,Om);
		if(theta<thmin || theta>thmax || Om>1e3 )//|| Om<=0 || isnan(Om) || isinf(Om)) 
			continue;		// set angular range and remove singularities
		g->SetPoint(g->GetN(),theta,Om);
	}
	
	return g;
}

// Frame_Lab -> dSigma/dOmegaLab[ThetaLab] 	and 	Frame_Cm -> dSigma/dOmegaCm[ThetaCm]
TGraph *TReaction::RutherfordVsTheta(double thmin, double thmax, int part, bool Frame_Lab, bool Units_mb){
	TGraph *g = new TGraph();
	const char *frame = Form("%s",Frame_Lab?"Lab":"Cm");
	
	g->SetName(Form("%s_RutherfordVsTheta%s",GetName(),frame));
	g->SetTitle(Form("Rutherford cross section for %s; Theta_{%s} [deg]; dSigma / dOmega_{%s} [%s]",GetName(),frame,frame,Units_mb?"mb":"fm^2"));
	
	double theta, R;	
	
	for(int i=1; i<=180; i++){
		theta = (double)i; // always in CM frame
			
		R = GetRutherfordCm(theta*D2R,part,Units_mb);//*ConvertOmega?
		
		if(Frame_Lab){ // this is now converted to specified frame (from Frame_Lab)
			R*=ConvertOmegaCmToLab(theta*D2R,part);
			theta = ConvertThetaCmToLab(theta*D2R,part)*R2D;
		}
		
		if(theta<thmin || theta>thmax) 
			continue;		// set angular range			
		g->SetPoint(g->GetN(),theta,R);
	}
	
	return g;
}

	
void TReaction::Print(Option_t *opt) { 
 
 	std::string pstring;
 	pstring.assign(opt);
 	
 	printf("\n\n * * * * * * * * * * * * * * * * * * * * * * * * *");
 	printf("\n\n\tTReaction  ' %s ' :\n\n",GetName());
 	
 	printf(" -> Beam '%s' kinetic energy = %.4e [MeV]\n",fNuc[0]->GetName(),fTBeam);
 	printf(" -> Reaction Q value (total)   = %.4e [MeV]\n",fQVal);
 	printf(" -> Reaction kinematics type   = '%s' \n",fInverse?"INVERSE":"NORMAL");
	if(fInverse){
		printf("\n Inverse beam '%s' [lab frame] :- \n",fNuc[1]->GetName());
		printf("\t Kinetic energy = %.4e [MeV]\n\t Velocity       = %.4e [/c] \n",(fGLab[0]-1)*fM[1],fVLab[0]);
	}
	
	printf("\n Center of mass motion :- \n");
	printf("\t CmE  = %.4e [MeV]\n\t CmTi = %.4e [MeV]\n\t CmTf = %.4e [MeV]\n\t CmV  = %.4e [/c]\n\t CmP  = %.4e [MeV/c]\n\t CmG  = %.4e \n\n",fCmE,fCmTi,fCmTf,fCmV,fCmP,fCmG);		

 	if(pstring.find("all")!=std::string::npos){
		for(int i=0; i<4; i++){	
			printf("\n Particle %i : '%s' : \t A = %i, Z = %i, Mass = %.4e [MeV]\n",i,fNuc[i]->GetName(),fNuc[i]->GetA(),fNuc[i]->GetZ(),fM[i]);
			
			if(i<2){
				printf("\t ECm = %.4e [MeV]\t\t ELab = %.4e [MeV]\n",fECm[i],fELab[i]);
				printf("\t TCm = %.4e [MeV]\t\t TLab = %.4e [MeV]\n",fTCm[i],fTLab[i]);
				printf("\t VCm = %.4e [/c]\t\t VLab = %.4e [/c]\n",fVCm[i],fVLab[i]);
				printf("\t PCm = %.4e [MeV/c]\t PLab = %.4e [MeV/c]\n",fPCm[i],fPLab[i]);
				printf("\t GCm = %.4e \t\t GLab = %.4e \n",fGCm[i],fGLab[i]);
			} else {
				printf("\t ECm = %.4e [MeV]\t\t ELab = N/A       \n",fECm[i]);
				printf("\t TCm = %.4e [MeV]\t\t TLab = N/A       \n",fTCm[i]);
				printf("\t VCm = %.4e [/c]\t\t VLab = N/A       \n",fVCm[i]);
				printf("\t PCm = %.4e [MeV/c]\t PLab = N/A       \n",fPCm[i]);
				printf("\t GCm = %.4e \t\t GLab = N/A       \n",fGCm[i]);
				printf("\t\t ThetaLab_max = %.2f [deg]\n",fThetaMax[i]*R2D);
			}					
		} 	
 	}	
 	printf("\n\n * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");

	return;
}

void TReaction::Clear(Option_t *opt) {
	
	fQVal = 0;
	fS = 0;
	fInvariantMass = 0;
	fTBeam = 0;
	fInverse = false;
	
	fCmTi = 0;
	fCmTf = 0;	
	fCmE = 0;
	fCmV = 0;
	fCmP = 0;
	fCmG = 0;			
	
	for(int i=0; i<4; i++){
	
		fNuc[i] = 0;
		fM[i] = 0;
		
		fTCm[i] = 0;
		fECm[i] = 0;
		fVCm[i] = 0;
		fPCm[i] = 0;
		fGCm[i] = 0;
                	
	        if(i<2) {
		  fTLab[i] = 0;
		  fELab[i] = 0;
		  fVLab[i] = 0;
		  fPLab[i] = 0;
		  fGLab[i] = 0;		
		}
	}
	return;
}
	

