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
   TKinematics(double eBeam, const char* beam, const char* targ, const char* ejec = nullptr, const char* reco = nullptr, const char* name = "");
   TKinematics(TNucleus* projectile, TNucleus* target, double eBeam, const char* name = "");
   TKinematics(TNucleus* projectile, TNucleus* target, TNucleus* recoil, TNucleus* ejectile, double eBeam, const char* name = "");
   TKinematics(TNucleus* projectile, TNucleus* target, TNucleus* recoil, TNucleus* ejectile, double eBeam, double ex3, const char* name = "");
   TKinematics(const char* beam, const char* targ, const char* ejec, const char* reco, double eBeam, double ex3 = 0.0, const char* name = "");

   void Initial();
   void FinalCm();
   void Final(double angle, int part);
   // void SetAngles(double angle, int part);
   void SetAngles(double angle, int part, bool upper = false);
   // get
   TSpline3* Evslab(double thmin, double thmax, double size, int part = 2);
   TSpline3* Evscm(double thmin, double thmax, double size, int part = 2);
   TSpline3* labvscm(double thmin, double thmax, double size, int part = 2) const;
   TSpline3* cmvslab(double thmin, double thmax, double size, int part = 2) const;
   TSpline3* Steffen_labvscminverse(double thmin, double thmax, double size, int part = 2) const;

   TGraph* Evslab_graph(double thmin, double thmax, double size, int part = 2);

   double GetCmEnergy(double eBeam) const;
   double GetCmEnergy() const;
   double GetBeamEnergy(double LabAngle, double LabEnergy) const;
   double GetMaxAngle(double vcm) const;
   double GetMaxAngle(int part) const;
   double NormalkinEnergy() const;
   bool   CheckMaxAngle(double angle, int part) const;
   double GetExcEnergy(TLorentzVector recoil);
   double GetExcEnergy(TVector3 position, double KinE);
   double GetExcEnergy(double theta, double KinE);
   double ELab(double angle_lab, int part);

   double GetQValue() const { return fQValue; }
   double GetElab(int i) const { return fE[i]; }
   double GetM(int i) const { return fM[i]; }
   double GetTlab(int i) const { return fT[i]; }
   double GetEcm(int i) const { return fEcm[i]; }
   double GetTcm(int i) const { return fTcm[i]; }

   double GetThetalab(int i) const
   {
      if(fTheta[i] < 1e-5) {
         return 0;
      }
      return fTheta[i];
   }
   double GetThetacm(int i) const
   {
      if(fThetacm[i] < 1e-5) {
         return 0;
      }
      return fThetacm[i];
   }

   double GetBetacm() const { return fBeta_cm; }
   double GetGammacm() const { return fGamma_cm; }
   double GetBetacm(int i) const { return fBetacm[i]; }
   double GetVcm(int i) const { return fVcm[i]; }
   double GetV(int i) const { return fV[i]; }

   TSpline3* Ruthvscm(double thmin, double thmax, double size) const;
   TSpline3* Ruthvslab(double thmin, double thmax, double size, int part) const;
   double    Angle_lab2cm(double vcm, double angle_lab) const;
   double    Angle_lab2cminverse(double vcm, double angle_lab, bool upper = true) const;
   double    Steffen_cm2labinverse(double theta_cm, int part = 2) const;   // NEW FUNCTIN+
   double    Steffen_lab2cminverse(double theta_lab);                      // assumes part = 2;
   double    Angle_cm2lab(double vcm, double angle_cm) const;
   double    Sigma_cm2lab(double angle_cm, double sigma_cm) const;
   double    Sigma_lab2cm(double angle_cm, double sigma_lab) const;
   void      Transform2cm(double& angle, double& sigma) const;
   void      Transform2cm(double& angle, double& errangle, double& sigma, double& errsigma) const;
   void      AngleErr_lab2cm(double angle, double& err) const;
   void      SigmaErr_lab2cm(double angle, double err, double& sigma, double& errsigma) const;
   double    Rutherford(double angle_cm) const;

private:
   double fTCm_i{0.};
   double fTCm_f{0.};

   // I don't think it makes sense to have these as arrays, we never use these as arrays (apart from setting the masses)
   // and it would make it easier to understand to see fProjectile or fRecoil instead of fParticle[0] or fParticle[2]
   TNucleus* fParticle[4]{nullptr};   // NOLINT(*-avoid-c-arrays)
   double    fM[4]{0.};               // NOLINT(*-avoid-c-arrays)
   double    fEBeam{0.};
   double    fQValue{0.};
   double    fT[4]{0.};       // NOLINT(*-avoid-c-arrays)
   double    fE[4]{0.};       // NOLINT(*-avoid-c-arrays)
   double    fP[4]{0.};       // NOLINT(*-avoid-c-arrays)
   double    fV[4]{0.};       // NOLINT(*-avoid-c-arrays)
   double    fTheta[4]{0.};   // NOLINT(*-avoid-c-arrays)

   double fTcm[4]{0.};       // NOLINT(*-avoid-c-arrays)
   double fEcm[4]{0.};       // NOLINT(*-avoid-c-arrays)
   double fPcm[4]{0.};       // NOLINT(*-avoid-c-arrays)
   double fVcm[4]{0.};       // NOLINT(*-avoid-c-arrays)
   double fBetacm[4]{0.};    // NOLINT(*-avoid-c-arrays)
   double fThetacm[4]{0.};   // NOLINT(*-avoid-c-arrays)

   double fBeta_cm{0.};
   double fGamma_cm{0.};

   TSpline3* fCm2LabSpline{nullptr};

   double Pcm_em(double, double) const;
   double P_tm(double, double) const;
   double E_tm(double, double) const;
   double T_em(double, double) const;
   double betacm_tm(double, double) const;
   double V_pe(double, double) const;
   double E_final(int) const;
   double T_final(int) const;

   /// \cond CLASSIMP
   ClassDefOverride(TKinematics, 1)   // NOLINT
   /// \endcond
};
/*! @} */
#endif
