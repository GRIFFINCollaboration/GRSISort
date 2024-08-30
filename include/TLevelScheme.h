#ifndef TLEVELSCHEME_H
#define TLEVELSCHEME_H

#if __cplusplus >= 201703L

#include <iostream>
#include <vector>
#include <map>
#include <utility>

#include "TColor.h"
#include "TPolyLine.h"
#include "TArrow.h"
#include "TPaveLabel.h"
#include "TLatex.h"

/** \addtogroup GUI
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TLevelScheme
///
/// This class implements a level scheme. The level scheme has a
/// vector of bands/groups of levels. Each of the bands contains
/// a map of levels, and each level has a map of gamma rays draining it.
/// The gamma rays can have strength assigned to them which then
/// can translate to the width of their arrows.
///
/////////////////////////////////////////////////////////////////

class TLevelScheme;

class TGamma : public TArrow {
public:
   TGamma(TLevelScheme* levelScheme = nullptr, const std::string& label = "", const double& br = 100., const double& ts = 1.);
   TGamma(const TGamma& rhs);
   ~TGamma();

   // setters
   void Energy(const double val) { fEnergy = val; }
   void EnergyUncertainty(const double val) { fEnergyUncertainty = val; }
   void UseTransitionStrength(const bool val)
   {
      fUseTransitionStrength = val;
      UpdateWidth();
   }   // *MENU*
   void Scaling(const double offset, const double gain)
   {
      fScalingOffset = offset;
      fScalingGain   = gain;
      UpdateWidth();
   }   // *MENU*
   void BranchingRatio(const double val)
   {
      fBranchingRatio = val;
      UpdateWidth();
   }   // *MENU*
   void BranchingRatioUncertainty(const double val) { fBranchingRatioUncertainty = val; }
   void BranchingRatioPercent(const double val) { fBranchingRatioPercent = val; }
   void BranchingRatioPercentUncertainty(const double val) { fBranchingRatioPercentUncertainty = val; }
   void TransitionStrength(const double val)
   {
      fTransitionStrength = val;
      UpdateWidth();
   }   // *MENU*
   void TransitionStrengthUncertainty(const double val) { fTransitionStrengthUncertainty = val; }
   void LabelText(const char* val)
   {
      fLabelText = val;
      UpdateLabel();
   }   // *MENU*
   void InitialEnergy(const double val) { fInitialEnergy = val; }
   void FinalEnergy(const double val) { fFinalEnergy = val; }

   // getters
   double      Energy() const { return fEnergy; }
   double      EnergyUncertainty() const { return fEnergyUncertainty; }
   bool        UseTransitionStrength() const { return fUseTransitionStrength; }
   double      ScalingGain() const { return fScalingGain; }
   double      ScalingOffset() const { return fScalingOffset; }
   double      BranchingRatio() const { return fBranchingRatio; }
   double      BranchingRatioUncertainty() const { return fBranchingRatioUncertainty; }
   double      BranchingRatioPercent() const { return fBranchingRatioPercent; }
   double      BranchingRatioPercentUncertainty() const { return fBranchingRatioPercentUncertainty; }
   double      TransitionStrength() const { return fTransitionStrength; }
   double      TransitionStrengthUncertainty() const { return fTransitionStrengthUncertainty; }
   double      InitialEnergy() const { return fInitialEnergy; }
   double      FinalEnergy() const { return fFinalEnergy; }
   double      Width() const { return (fUseTransitionStrength ? fTransitionStrength : fBranchingRatio); }
   std::string LabelText() const { return fLabelText; }
   TLatex*     Label() const { return fLabel; }

   std::map<double, double> CoincidentGammas();
   void                     PrintCoincidentGammas();   // *MENU*

   std::vector<std::tuple<double, std::vector<double>>> ParallelGammas();
   void                                                 PrintParallelGammas();   // *MENU*

   void Print(Option_t* option = "") const override;

   void UpdateWidth();
   void UpdateLabel();

   using TArrow::Draw;
   void Draw(const double& x1, const double& y1, const double& x2, const double& y2);

   void Debug(bool val) { fDebug = val; }

   TGamma& operator=(const TGamma& rhs);

   static void   TextSize(double val) { fTextSize = val; }
   static double TextSize() { return fTextSize; }

private:
   bool          fDebug{false};
   bool          fUseTransitionStrength{false};
   double        fEnergy{0.};   ///< Energy of this gamma ray
   double        fEnergyUncertainty{0.};
   double        fScalingGain{1.};
   double        fScalingOffset{1.};
   double        fBranchingRatio{100.};
   double        fBranchingRatioUncertainty{0.};
   double        fBranchingRatioPercent{100.};
   double        fBranchingRatioPercentUncertainty{0.};
   double        fTransitionStrength{1.};
   double        fTransitionStrengthUncertainty{0.};
   std::string   fLabelText;
   TLatex*       fLabel{nullptr};
   TLevelScheme* fLevelScheme{nullptr};
   double        fInitialEnergy{0.};   ///< Energy of initial level that emits this gamma ray
   double        fFinalEnergy{0.};     ///< Energy of final level that is populated by this gamma ray

   static double fTextSize;

   /// \cond CLASSIMP
   ClassDefOverride(TGamma, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

class TLevel : public TPolyLine {
public:
   TLevel(TLevelScheme* levelScheme = nullptr, const double& energy = -1., const std::string& label = "");
   TLevel(const TLevel& rhs);
   ~TLevel();

   TGamma* AddGamma(double levelEnergy, const char* label = "", double br = 100., double ts = 1.);   // *MENU*
   TGamma* AddGamma(double levelEnergy, double energyUncertainty, const char* label = "", double br = 100., double ts = 1.);

   void Energy(const double val) { fEnergy = val; }                         // *MENU*
   void EnergyUncertainty(const double val) { fEnergyUncertainty = val; }   // *MENU*
   void Label(const char* val) { fLabel = val; }                            // *MENU*
   void AddFeeding() { ++fNofFeeding; }
   void Offset(const double& val) { fOffset = val; }

   void MoveToBand(const char* val);   // *MENU*

   double      Energy() const { return fEnergy; }
   double      EnergyUncertainty() const { return fEnergyUncertainty; }
   std::string Label() const { return fLabel; }
   double      Offset() const { return fOffset; }

   std::pair<double, double> GetMinMaxGamma() const;
   size_t                    NofDrainingGammas() const { return fGammas.size(); }
   size_t                    NofFeedingGammas() const { return fNofFeeding; }

   using TPolyLine::Draw;
   void   Draw(const double& left, const double& right);
   double DrawLabel(const double& pos);
   double DrawEnergy(const double& pos);

   std::map<double, TGamma>::iterator       begin() { return fGammas.begin(); }
   std::map<double, TGamma>::iterator       end() { return fGammas.end(); }
   std::map<double, TGamma>::const_iterator begin() const { return fGammas.begin(); }
   std::map<double, TGamma>::const_iterator end() const { return fGammas.end(); }

   // comparison operators (level-level, level-double, and double-level)
   friend bool operator<(const TLevel& lhs, const TLevel& rhs) { return lhs.fEnergy < rhs.fEnergy; }
   friend bool operator>(const TLevel& lhs, const TLevel& rhs) { return rhs < lhs; }
   friend bool operator<=(const TLevel& lhs, const TLevel& rhs) { return !(rhs < lhs); }
   friend bool operator>=(const TLevel& lhs, const TLevel& rhs) { return !(lhs < rhs); }

   friend bool operator<(const TLevel& lhs, const double& rhs) { return lhs.fEnergy < rhs; }
   friend bool operator>(const TLevel& lhs, const double& rhs) { return rhs < lhs; }
   friend bool operator<=(const TLevel& lhs, const double& rhs) { return !(rhs < lhs); }
   friend bool operator>=(const TLevel& lhs, const double& rhs) { return !(lhs < rhs); }

   friend bool operator<(const double& lhs, const TLevel& rhs) { return lhs < rhs.fEnergy; }
   friend bool operator>(const double& lhs, const TLevel& rhs) { return rhs < lhs; }
   friend bool operator<=(const double& lhs, const TLevel& rhs) { return !(rhs < lhs); }
   friend bool operator>=(const double& lhs, const TLevel& rhs) { return !(lhs < rhs); }

   void Print(Option_t* option = "") const override;

   void Debug(bool val)
   {
      fDebug = val;
      for(auto& [level, gamma] : fGammas) { gamma.Debug(val); }
   }

   TLevel& operator=(const TLevel& rhs);

   static void   TextSize(double val) { fTextSize = val; }
   static double TextSize() { return fTextSize; }

private:
   bool                     fDebug{false};
   double                   fEnergy{0.};              ///< energy of this level
   double                   fEnergyUncertainty{0.};   ///< energy uncertainty of this level
   std::string              fLabel;                   ///< label for this level
   std::map<double, TGamma> fGammas;                  ///< gamma rays draining this level, each pointing to a level
   size_t                   fNofFeeding{0};           ///< counter for gammas feeding this level
   TLevelScheme*            fLevelScheme{nullptr};

   // graphics elements
   TLatex* fLevelLabel{nullptr};
   TLatex* fEnergyLabel{nullptr};

   double fOffset{0.};   ///< y-offset for labels on right and left side of level

   static double fTextSize;

   /// \cond CLASSIMP
   ClassDefOverride(TLevel, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

class TBand : public TPaveLabel {
public:
   TBand(TLevelScheme* levelScheme = nullptr, const std::string& label = "");
   TBand(const TBand& rhs);
   ~TBand() {}

   TLevel* AddLevel(double energy, const std::string& label);   // *MENU*
   TLevel* AddLevel(const double energy, const char* label)
   {
      std::string tmp(label);
      return AddLevel(energy, tmp);
   }   // *MENU*
   void AddLevel(TLevel* level);
   void RemoveLevel(TLevel* level);

   size_t                    NofLevels() const { return fLevels.size(); }
   TLevel*                   GetLevel(double energy);
   TLevel*                   FindLevel(double energy, double energyUncertainty);
   std::pair<double, double> GetMinMaxLevelEnergy() const { return std::make_pair(fLevels.begin()->second.Energy(), fLevels.rbegin()->second.Energy()); }
   std::pair<double, double> GetMinMaxGamma() const;
   double                    Width(double distance) const;   ///< return width of this band using provided distance between gammas

   std::map<double, TLevel>::iterator       begin() { return fLevels.begin(); }
   std::map<double, TLevel>::iterator       end() { return fLevels.end(); }
   std::map<double, TLevel>::const_iterator begin() const { return fLevels.begin(); }
   std::map<double, TLevel>::const_iterator end() const { return fLevels.end(); }

   void Print(Option_t* option = "") const override;

   void Debug(bool val)
   {
      fDebug = val;
      for(auto& [energy, level] : fLevels) { level.Debug(val); }
   }

   TBand& operator=(const TBand& rhs);

private:
   bool                     fDebug{false};
   std::map<double, TLevel> fLevels;
   TLevelScheme*            fLevelScheme{nullptr};

   /// \cond CLASSIMP
   ClassDefOverride(TBand, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

class TLevelScheme : public TPaveLabel {
public:
   enum class EGammaWidth { kNoWidth,
                            kBand,
                            kGlobal };

   TLevelScheme(const std::string& filename = "", bool debug = false);
   TLevelScheme(const char* filename, bool debug = false) : TLevelScheme(std::string(filename), debug) {}
   TLevelScheme(const TLevelScheme& rhs);
   ~TLevelScheme();

   static void          ListLevelSchemes();
   static TLevelScheme* GetLevelScheme(const char* name);

   TLevel* AddLevel(double energy, const std::string& bandName, const std::string& label);
   TLevel* AddLevel(const double energy, const char* bandName, const char* label) { return AddLevel(energy, std::string(bandName), std::string(label)); }   // *MENU*
   TLevel* GetLevel(double energy);
   TLevel* FindLevel(double energy, double energyUncertainty);

   TGamma*              FindGamma(double energy, double energyUncertainty = 0.);
   std::vector<TGamma*> FindGammas(double lowEnergy, double highEnergy);

   std::map<double, double>                             FeedingGammas(double levelEnergy, double factor = 1.);
   std::map<double, double>                             DrainingGammas(double levelEnergy, double factor = 1.);
   void                                                 ResetGammaMap() { fGammaMap.clear(); }
   std::vector<std::tuple<double, std::vector<double>>> ParallelGammas(double initialEnergy, double finalEnergy, double factor = 1.);

   void MoveToBand(const char* bandName, TLevel* level);

   void UseGlobalGammaWidth(const int val)
   {
      fGammaWidth = static_cast<EGammaWidth>(val);
      Refresh();
   }   // *MENU*
   void RadwareStyle(const bool val)
   {
      fRadwareStyle = val;
      Refresh();
   }   // *MENU*
   void GammaDistance(const double val)
   {
      fGammaDistance = val;
      Refresh();
   }   // *MENU*
   void BandGap(const double val)
   {
      fBandGap = val;
      Refresh();
   }   // *MENU*
   void LeftMargin(const double val)
   {
      fLeftMargin = val;
      Refresh();
   }   // *MENU*
   void RightMargin(const double val)
   {
      fRightMargin = val;
      Refresh();
   }   // *MENU*
   void BottomMargin(const double val)
   {
      fBottomMargin = val;
      Refresh();
   }   // *MENU*
   void TopMargin(const double val)
   {
      fTopMargin = val;
      Refresh();
   }   // *MENU*

   void Refresh();   // *MENU*
   void UnZoom() const;
   void Draw(Option_t* option = "") override;

   void Print(Option_t* option = "") const override;

   void Debug(bool val)
   {
      fDebug = val;
      for(auto& band : fBands) { band.Debug(val); }
   }

private:
   void ParseENSDF(const std::string& filename);
   void DrawAuxillaryLevel(const double& energy, const double& left, const double& right);
   void BuildGammaMap(double levelEnergy);

   bool                              fDebug{false};
   static std::vector<TLevelScheme*> fLevelSchemes;

   std::vector<TBand>             fBands;
   std::multimap<double, TLine>   fAuxillaryLevels;
   std::multimap<double, TGamma*> fGammaMap;

   // nuclide information
   double fQValue{0.};
   double fQValueUncertainty{0.};
   double fNeutronSeparation{0.};
   double fNeutronSeparationUncertainty{0.};

   // graphics settings
   EGammaWidth fGammaWidth{EGammaWidth::kNoWidth};
   bool        fRadwareStyle{true};
   double      fGammaDistance{50.};
   double      fBandGap{200.};
   double      fLeftMargin{-1.};
   double      fRightMargin{-1.};
   double      fBottomMargin{-1.};
   double      fTopMargin{-1.};
   double      fMinWidth{1.};
   double      fMaxWidth{10.};

   // original canvas range
   double fX1{0.};
   double fY1{0.};
   double fX2{0.};
   double fY2{0.};

   /// \cond CLASSIMP
   ClassDefOverride(TLevelScheme, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */

#endif
#endif
