#ifndef __TKINEMATICS_H
#define __TKINEMATICS_H

#include <iostream>
#include <fstream>
#include <string>

#include <TNamed.h>
#include <TMath.h>
#include <TSpline.h>
#include <TGraph.h>
#include <TLorentzVector.h>

#include "TNucleus.h"

#ifndef PI
#define PI                       (TMath::Pi())
#endif

using namespace std;

class TKinematics : public TNamed {
public:
  TKinematics(TNucleus* projectile, TNucleus* target, double ebeam, const char *name = "");
  TKinematics(TNucleus* projectile, TNucleus* target, TNucleus* recoil, TNucleus* ejectile, double ebeam, const char *name = "");
  TKinematics(TNucleus* projectile, TNucleus* target, TNucleus* recoil, TNucleus* ejectile, double ebeam, double ex3, const char *name = "");

  void Initial();
  void FinalCm();
  void Final(double angle, int part);
  //void SetAngles(double angle, int part);
  void SetAngles(double angle, int part, bool upper=false);
  //get
  TSpline3* Evslab(double thmin, double thmax, double size, int part = 2);
  TSpline3* Evscm(double thmin, double thmax, double size, int part = 2);
  TSpline3* labvscm(double thmin, double thmax, double size, int part = 2);
  TSpline3* cmvslab(double thmin, double thmax, double size, int part = 2);
 
 
  double GetCmEnergy(double ebeam);
  double GetCmEnergy();
  double GetBeamEnergy(double LabAngle, double LabEnergy);
  double GetMaxAngle(double vcm);
  double GetMaxAngle(int part);
  double NormalkinEnergy();
  bool   CheckMaxAngle(double angle, int part);
  double GetExcEnergy(TLorentzVector recoil);
  double GetExcEnergy(TVector3 position, double KinE);
  double ELab(double angle_lab, int part);

  double GetQValue()     { return fQValue;   }

  double GetElab(int i)  {  return fE[i];    }
  double GetM(int i)     {  return fM[i];    }
  double GetTlab(int i)  {  return fT[i];    }
  double GetEcm(int i)   {  return fEcm[i];  }
  double GetTcm(int i)   {  return fTcm[i];  }

  double GetThetalab(int i){
    if(fTheta[i]<1e-5)
      return 0;
    else
      return fTheta[i];
  }
  double GetThetacm(int i){
    if(fThetacm[i]<1e-5)
      return 0;
    else
      return fThetacm[i];
  }

  double GetBetacm()       { return fBeta_cm;   }
  double GetGammacm()      { return fGamma_cm;  }
  double GetBetacm(int i)  { return fBetacm[i]; }
  double GetVcm(int i)     { return fVcm[i];    }
  double GetV(int i)       { return fV[i];      }

  TSpline3* Ruthvscm(double thmin, double thmax, double size); 
  TSpline3* Ruthvslab(double thmin, double thmax, double size, int part); 
  double Angle_lab2cm(double vcm, double angle_lab);
  double Angle_lab2cminverse(double vcm, double angle_lab, bool upper);
  double Angle_cm2lab(double vcm, double angle_cm);
  double Sigma_cm2lab(double angle_cm, double sigma_cm);
  double Sigma_lab2cm(double angle_cm, double sigma_lab);
  void Transform2cm(double &angle, double &sigma);
  void Transform2cm(double &angle, double &errangle, double &sigma, double &errsigma);
  void AngleErr_lab2cm(double angle, double &err);
  void SigmaErr_lab2cm(double angle, double err, double &sigma, double &errsigma);
  //double Sigma_cm2labnew(double vcm, double angle_cm, double sigma_cm);
  double Rutherford(double angle_cm);

  void Print(Option_t *opt="") { }
  void Clear(Option_t *opt="") { }
    
private:

  double fTCm_i;
  double fTCm_f;

  TNucleus* fParticle[4];
  double fM[4];
  double fQValue;
  double fEBeam;
  double fT[4];
  double fE[4];
  double fP[4];
  double fV[4];
  double fTheta[4];

  double fTcm[4];
  double fEcm[4];
  double fPcm[4];
  double fVcm[4];
  double fBetacm[4];
  double fThetacm[4];

  double fBeta_cm;
  double fGamma_cm;

  double Pcm_em(double, double);
  double P_tm(double, double);
  double E_tm(double, double);
  double T_em(double, double);
  double betacm_tm(double, double);
  double V_pe(double, double);
  double E_final(int);
  double T_final(int);
 
  ClassDef(TKinematics,1);
 
};
#endif
