#ifndef TREACTION_H
#define TREACTION_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <iostream>
#include <string>
#include <cmath>

#include "TNamed.h"
#include "TMath.h"
#include "TGraph.h"

#include "TNucleus.h"

#ifndef PI
#define PI (3.14159265358979312e+00)
#endif

#ifndef R2D
#define R2D (5.72957795130823229e+01)
#endif

#ifndef D2R
#define D2R (1.74532925199432955e-02)
#endif

/// *********** *********** *********** *********** *********** *********** *********** //
///
/// \class TReaction
///
/// A simple, intuitive two body reaction class.
/// Lab frame variables are generally calculated by boosting from the CM frame
/// Reactions have the form A(B,C)D where;
/// [0] - B is "beam"
/// [1] - A is "targ"
/// [2] - C is "ejec"
/// [3] - D is "reco"
///
/// Uses planck units (c=1 and is dimensionless)
/// M = mc^2
/// E = full mass energy (inc. kinetic energy)
/// T = kinetic energy
/// V = magnitude of velocity (=beta)
/// P = magnitude of momentum
/// G = gamma factor (=[1-V^2]^-0.5)
///
/// * An excitation energy can be included in the final state heavy recoil nucleus using ex3
///			in the reaction initialisation or by using SetExcEnergy(exc)
/// * ALL ENERGIES ARE IN MEV - MASSES TOO !
/// * currently only works for part==2, I think that part==3 just means theta_cm -> -theta_cm
///
///	BUGS
///	THERE ARE PERSISTANT NUMERICAL ERRORS (~0.1-1%) ...??! WHY?!!?!
/// *********** *********** *********** *********** *********** *********** *********** //

class TReaction : public TNamed {
public:
   TReaction(const char* beam, const char* targ, const char* ejec, const char* reco, double eBeam = 0.0, double ex3 = 0.0, bool inverse = false);

   void InitReaction();

   // returns reaction input parameters
   const char* GetNameFull() const { return Form("%s @ %.3f MeV/u", this->GetName(), fTLab[0] / fNuc[0]->GetA()); }
   TNucleus*   GetNucleus(int part) const { return fNuc[part]; }
   double      GetM(int part) const { return fM[part]; }
   double      GetExc() const { return fExc; }
   double      GetQVal() const { return fQVal; }
   bool        Inverse() const { return fInverse; }
   double      GetTBeam(bool inverse) const;
   double      GetVBeam() const { return fVLab[0]; }

   // CM frame properties
   double GetInvariantMass() const { return fInvariantMass; }
   double GetCmE() const { return fCmE; }
   double GetCmTi() const { return fCmTi; }
   double GetCmTf() const { return fCmTf; }
   double GetCmV() const { return fCmV; }
   double GetCmP() const { return fCmP; }
   double GetCmG() const { return fCmG; }

   // particle properties in CM frame
   double GetECm(int part) const { return fECm[part]; }
   double GetTCm(int part) const { return fTCm[part]; }
   double GetVCm(int part) const { return fVCm[part]; }
   double GetPCm(int part) const { return fPCm[part]; }
   double GetGCm(int part) const { return fGCm[part]; }

   // particle properties in LAB frame (default is beam)
   double GetThetaMax(int part) const { return fThetaMax[part]; }
   // this stuff depends on the lab angle as the cm motion and the particle in the cm are coupled
   double GetELab(double theta_lab = 0.0, int part = 0) const
   {
      return GetELabFromThetaCm(ConvertThetaLabToCm(theta_lab, part), part);
   }
   double GetTLab(double theta_lab = 0.0, int part = 0) const
   {
      return GetTLabFromThetaCm(ConvertThetaLabToCm(theta_lab, part), part);
   }
   double GetVLab(double theta_lab = 0.0, int part = 0) const
   {
      return GetVLabFromThetaCm(ConvertThetaLabToCm(theta_lab, part), part);
   }
   double GetPLab(double theta_lab = 0.0, int part = 0) const
   {
      return GetPLabFromThetaCm(ConvertThetaLabToCm(theta_lab, part), part);
   }
   double GetGLab(double theta_lab = 0.0, int part = 0) const
   {
      return GetGLabFromThetaCm(ConvertThetaLabToCm(theta_lab, part), part);
   }

   // this stuff depends on the CM angle as the cm motion and the particle in the cm are coupled
   double GetELabFromThetaCm(double theta_cm = 0.0, int part = 0) const;   // FULL MASS+KINETIC ENERGY
   double GetTLabFromThetaCm(double theta_cm = 0.0, int part = 0) const;   // KINETIC ENERGY
   double GetVLabFromThetaCm(double theta_cm = 0.0, int part = 0) const;
   double GetPLabFromThetaCm(double theta_cm = 0.0, int part = 0) const;
   double GetGLabFromThetaCm(double theta_cm = 0.0, int part = 0) const;

   double GetExcEnergy(double ekin = 0.00, double theta_lab = 0.00, int part = 2) const;
   void   AnalysisAngDist(double ekin, double theta_lab, int part, double& exc, double& theta_cm, double& omega_lab2cm);
   double AnalysisBeta(double ekin, int part) const;

   double GetRutherfordCm(double theta_cm, int part = 2, bool Units_mb = true) const;
   double GetRutherfordLab(double theta_lab, int part = 2, bool Units_mb = true) const;

   // Conversion from LAB frame to CM frame
   double ConvertThetaLabToCm(double theta_lab, int part = 2) const;
   double ConvertOmegaLabToCm(double theta_lab, int part = 2) const;
   void   ConvertLabToCm(double theta_lab, double omega_lab, double& theta_cm, double& omega_cm, int part = 2) const;

   // Conversion from CM frame to LAB frame
   double ConvertThetaCmToLab(double theta_cm, int part = 2) const;
   double ConvertOmegaCmToLab(double theta_cm, int part = 2) const;
   void   ConvertCmToLab(double theta_cm, double omega_cm, double& theta_lab, double& omega_lab, int part = 2) const;

   // Graphs for conversions and kinematic/cross-section curves
   // Frame_Lab -> TLab[ThetaLab] 	and 	Frame_Cm -> TLab[ThetaCm]
   TGraph* KinVsTheta(double thmin = 0.0, double thmax = 180.0, int part = 2, bool Frame_Lab = true, bool Units_keV = true) const;
   // Frame_Lab -> ThetaCm[ThetaLab] 	and 	Frame_Cm -> ThetaLab[ThetaCm]
   TGraph* ThetaVsTheta(double thmin = 0.0, double thmax = 180.0, int part = 2, bool Frame_Lab = true) const;
   // Frame_Lab -> dOmegaCm/dOmegaLab[ThetaLab] 	and 	Frame_Cm -> dOmegaLab/dOmegaCm[ThetaCm]
   TGraph* OmegaVsTheta(double thmin = 0.0, double thmax = 180.0, int part = 2, bool Frame_Lab = true) const;
   // Frame_Lab -> dSigma/dThetaLab[ThetaLab] 	and 	Frame_Cm -> dSigma/dThetaCm[ThetaCm]
   TGraph* RutherfordVsTheta(double thmin = 1.0, double thmax = 179.0, int part = 2, bool Frame_Lab = true, bool Units_mb = true) const;

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   void SetExcEnergy(double exc) { SetCmFrame(exc); }

private:
   void SetCmFrame(double exc);   // enables the reaction to be modified using excitation energy

	// same as in TKinematics, should probably change from arrays to individual members
   // USER INPUTS
   TNucleus* fNuc[4]{nullptr};
   double    fTBeam{0.};
   bool      fInverse{false};
   double    fExc{0.};
   double    fM[4]{0.};

   // CM FRAME MOTION
   double fQVal{0.};   // effective Q value (includes excitation)
   double fS{0.};      // 'S' = M^2
   double fInvariantMass{0.};
   double fCmTi{0.};
   double fCmTf{0.};
   double fCmE{0.};
   double fCmV{0.};
   double fCmP{0.};
   double fCmG{0.};

   // PARTICLES IN CM FRAME
   double fTCm[4]{0.};
   double fECm[4]{0.};
   double fPCm[4]{0.};
   double fVCm[4]{0.};
   double fGCm[4]{0.};

   // PARTICLE IN LAB FRAME
   // Note that in the lab frame only the initial state (beam/targ) is fixed in the reaction
   double fTLab[2]{0.};
   double fELab[2]{0.};
   double fPLab[2]{0.};
   double fVLab[2]{0.};
   double fGLab[2]{0.};
   double fThetaMax[4]{0.};   // only nonzero for ejectile and recoil

   /// \cond CLASSIMP
   ClassDefOverride(TReaction, 1)   // Calculates reaction parameters for scattering experiments
   /// \endcond
};
/*! @} */
#endif
