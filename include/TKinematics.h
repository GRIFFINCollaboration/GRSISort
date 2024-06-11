#ifndef TKINEMATICS_H
#define TKINEMATICS_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <iostream>
#include <fstream>
#include <string>

#include "TNamed.h"
#include "TMath.h"
#include "TSpline.h"
#include "TGraph.h"
#include "TLorentzVector.h"

#include "TNucleus.h"

#ifndef PI
#define PI (TMath::Pi())
#endif

/////////////////////////////////////////////////////////////////
///
/// \class TKinematics
///
/// This class calculates 2 body kinematics from a beam, target,
/// recoil, ejectile, and beam energy.
///
/////////////////////////////////////////////////////////////////

class TKinematics : public TNamed {
public:
   TKinematics(double beame, const char* beam, const char* targ, const char* ejec = nullptr, const char* reco = nullptr,
               const char* name = "");
   TKinematics(const char* beam, const char* targ, const char* ejec, const char* reco, double ebeam, double ex3 = 0.0,
               const char* name = "");
   TKinematics(TNucleus* projectile, TNucleus* target, double ebeam, const char* name = "");
   TKinematics(TNucleus* projectile, TNucleus* target, TNucleus* recoil, TNucleus* ejectile, double ebeam,
               const char* name = "");
   TKinematics(TNucleus* projectile, TNucleus* target, TNucleus* recoil, TNucleus* ejectile, double ebeam, double ex3,
               const char* name = "");

   void Initial();
   void FinalCm();
   void Final(double angle, int part);
   // void SetAngles(double angle, int part);
   void SetAngles(double angle, int part, bool upper = false);
   // get
   TSpline3* Evslab(double thmin, double thmax, double size, int part = 2);
   TSpline3* Evscm(double thmin, double thmax, double size, int part = 2);
   TSpline3* labvscm(double thmin, double thmax, double size, int part = 2);
   TSpline3* cmvslab(double thmin, double thmax, double size, int part = 2);
   TSpline3* Steffen_labvscminverse(double thmin, double thmax, double size, int part = 2);

   TGraph* Evslab_graph(double thmin, double thmax, double size, int part = 2);

   double GetCmEnergy(double ebeam);
   double GetCmEnergy();
   double GetBeamEnergy(double LabAngle, double LabEnergy);
   double GetMaxAngle(double vcm);
   double GetMaxAngle(int part);
   double NormalkinEnergy();
   bool   CheckMaxAngle(double angle, int part);
   double GetExcEnergy(TLorentzVector recoil);
   double GetExcEnergy(TVector3 position, double KinE);
   double GetExcEnergy(double theta, double KinE);
   double ELab(double angle_lab, int part);

   double GetQValue() { return fQValue; }

   double GetElab(int i) { return fE[i]; }
   double GetM(int i) { return fM[i]; }
   double GetTlab(int i) { return fT[i]; }
   double GetEcm(int i) { return fEcm[i]; }
   double GetTcm(int i) { return fTcm[i]; }

   double GetThetalab(int i)
   {
      if(fTheta[i] < 1e-5) {
         return 0;
      } else {
         return fTheta[i];
      }
   }
   double GetThetacm(int i)
   {
      if(fThetacm[i] < 1e-5) {
         return 0;
      } else {
         return fThetacm[i];
      }
   }

   double GetBetacm() { return fBeta_cm; }
   double GetGammacm() { return fGamma_cm; }
   double GetBetacm(int i) { return fBetacm[i]; }
   double GetVcm(int i) { return fVcm[i]; }
   double GetV(int i) { return fV[i]; }

   TSpline3* Ruthvscm(double thmin, double thmax, double size);
   TSpline3* Ruthvslab(double thmin, double thmax, double size, int part);
   double    Angle_lab2cm(double vcm, double angle_lab);
   double    Angle_lab2cminverse(double vcm, double angle_lab, bool upper = true);
   double    Steffen_cm2labinverse(double theta_cm, int part = 2);   // NEW FUNCTIN+
   double    Steffen_lab2cminverse(double theta_lab);                // assumes part = 2;
   double    Angle_cm2lab(double vcm, double angle_cm);
   double    Sigma_cm2lab(double angle_cm, double sigma_cm);
   double    Sigma_lab2cm(double angle_cm, double sigma_lab);
   void      Transform2cm(double& angle, double& sigma);
   void      Transform2cm(double& angle, double& errangle, double& sigma, double& errsigma);
   void      AngleErr_lab2cm(double angle, double& err);
   void      SigmaErr_lab2cm(double angle, double err, double& sigma, double& errsigma);
   // double Sigma_cm2labnew(double vcm, double angle_cm, double sigma_cm);
   double Rutherford(double angle_cm);

   // void Print(Option_t* opt="") { }
   // void Clear(Option_t* opt="") { }

private:
   double fTCm_i{0.};
   double fTCm_f{0.};

   TNucleus* fParticle[4]{nullptr};
   double    fM[4]{0.};
   double    fQValue;
   double    fEBeam;
   double    fT[4]{0.};
   double    fE[4]{0.};
   double    fP[4]{0.};
   double    fV[4]{0.};
   double    fTheta[4]{0.};

   double fTcm[4]{0.};
   double fEcm[4]{0.};
   double fPcm[4]{0.};
   double fVcm[4]{0.};
   double fBetacm[4]{0.};
   double fThetacm[4]{0.};

   double fBeta_cm{0.};
   double fGamma_cm{0.};

   double Pcm_em(double, double);
   double P_tm(double, double);
   double E_tm(double, double);
   double T_em(double, double);
   double betacm_tm(double, double);
   double V_pe(double, double);
   double E_final(int);
   double T_final(int);

   TSpline3* Cm2LabSpline;
   void      InitKin();

   /// \cond CLASSIMP
   ClassDefOverride(TKinematics,
                    1);   // Calculates kinematics parameters (both normal and inverse) for scattering experiments
   /// \endcond
};
/*! @} */
#endif
