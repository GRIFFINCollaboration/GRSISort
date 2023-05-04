#ifndef TLEVELSCHEME_H
#define TLEVELSCHEME_H

#include <iostream>
#include <vector>
#include <map>
#include <utility>

#include "TCanvas.h"
#include "TColor.h"
#include "TBox.h"
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
/// vector of bands/groups of levels (0 is the default/unnamed 
/// band/group). Each of the bands contains a vector of levels,
/// and each level has a vector of gamma rays draining it.
/// The gamma rays can have strength assigned to them which then
/// translate to the width of their arrows.
///
/////////////////////////////////////////////////////////////////

class TLevelScheme;

class TGamma : public TArrow {
public:
	TGamma(TLevelScheme* levelScheme = nullptr, const std::string& label = "", const double& br = 100., const double& ts = 1.);
	TGamma(const TGamma& rhs);
	~TGamma();

	// setters
	void UseTransitionStrength(const bool val) { fUseTransitionStrength = val; UpdateWidth(); } // *MENU*
	void Scaling(const double offset, const double gain) { fScalingOffset = offset; fScalingGain = gain; UpdateWidth(); } // *MENU*
	void BranchingRatio(const double val) { if(fDebug) std::cout<<__PRETTY_FUNCTION__<<": "<<std::flush<<val<<std::endl; fBranchingRatio = val; UpdateWidth(); } // *MENU*
	void TransitionStrength(const double val) { fTransitionStrength = val; UpdateWidth(); } // *MENU*
	void LabelText(const char* val) { fLabelText = val; UpdateLabel(); } // *MENU*
	void BranchingRatioUncertainty(const double val) { fBranchingRatioUncertainty = val; }
	void TransitionStrengthUncertainty(const double val) { fTransitionStrengthUncertainty = val; }

	// getters
	bool UseTransitionStrength() const { return fUseTransitionStrength; }
	double ScalingGain() const { return fScalingGain; }
	double ScalingOffset() const { return fScalingOffset; }
	double BranchingRatio() const { return fBranchingRatio; }
	double TransitionStrength() const { return fTransitionStrength; }
	double Width() const { return (fUseTransitionStrength ? fTransitionStrength : fBranchingRatio); }
	std::string LabelText() const { return fLabelText; }
	TLatex* Label() const { return fLabel; }

	using TArrow::Print;
	void Print() const;

	void UpdateWidth();
	void UpdateLabel();

	using TArrow::Draw;
	void Draw(const double& x1, const double& y1, const double& x2, const double& y2);

	void Debug(bool val) { fDebug = val; }

	TGamma& operator=(const TGamma& rhs);

	static void TextSize(double val) { gTextSize = val; }
	static double TextSize() { return gTextSize; }

private:
	bool fDebug{false};
	bool fUseTransitionStrength{false};
	double fScalingGain{1.};
	double fScalingOffset{1.};
	double fBranchingRatio{100.};
	double fBranchingRatioUncertainty{0.};
	double fTransitionStrength{1.};
	double fTransitionStrengthUncertainty{0.};
	std::string fLabelText;
	TLatex* fLabel{nullptr};
	TLevelScheme* fLevelScheme{nullptr};

	static double gTextSize;

   /// \cond CLASSIMP
   ClassDefOverride(TGamma, 1); // Level
   /// \endcond
};

class TLevel : public TPolyLine {
public:
	TLevel(TLevelScheme* levelScheme = nullptr, const double& energy = -1., const std::string& label = "");
	TLevel(const TLevel& rhs);
	~TLevel();

	TGamma* AddGamma(TLevel* level, const std::string label = "", double br = 100., double ts = 1.);
	bool AddGamma(const double energy, const char* label = "", double br = 100., double ts = 1.); // *MENU*

	void Energy(const double val) { fEnergy = val; } // *MENU*
	void EnergyUncertainty(const double val) { fEnergyUncertainty = val; } // *MENU*
	void Label(const char* val) { fLabel = val; } // *MENU*
	void AddFeeding() { ++fNofFeeding; }
	void Offset(const double& val) { fOffset = val; }

	void MoveToBand(const char* val); // *MENU*

	double Energy() const { return fEnergy; }
	double EnergyUncertainty() const { return fEnergyUncertainty; }
	std::string Label() const { return fLabel; }
	double Offset() const { return fOffset; }

	std::pair<double, double> GetMinMaxGamma() const;
	size_t NofDrainingGammas() const { return fGammas.size(); }
	size_t NofFeedingGammas() const { return fNofFeeding; }

	using TPolyLine::Draw;
	void Draw(const double& left, const double& right);
	void DrawLabel(const double& pos);
	void DrawEnergy(const double& pos);

	std::map<TLevel*, TGamma>::iterator begin() { return fGammas.begin(); }
	std::map<TLevel*, TGamma>::iterator end() { return fGammas.end(); }
	std::map<TLevel*, TGamma>::const_iterator begin() const { return fGammas.begin(); }
	std::map<TLevel*, TGamma>::const_iterator end() const { return fGammas.end(); }

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

	void Print() const;

	void Debug(bool val) { fDebug = val; for(auto& [level, gamma] : fGammas) { gamma.Debug(val); } }

	TLevel& operator=(const TLevel& rhs);

	static void TextSize(double val) { gTextSize = val; }
	static double TextSize() { return gTextSize; }

private:
	bool fDebug{false};
	double fEnergy{0.}; ///< energy of this level
	double fEnergyUncertainty{0.}; ///< energy uncertainty of this level
	std::string fLabel; ///< label for this level
	std::map<TLevel*, TGamma> fGammas; ///< gamma rays draining this level, each pointing to a level
	size_t fNofFeeding{0}; ///< counter for gammas feeding this level
	TLevelScheme* fLevelScheme{nullptr};

	// graphics elements
	TLatex* fLevelLabel{nullptr};
	TLatex* fEnergyLabel{nullptr};

	double fOffset{0.}; ///< y-offset for labels on right and left side of level

	static double gTextSize;

   /// \cond CLASSIMP
   ClassDefOverride(TLevel, 1); // Level
   /// \endcond
};

class TBand : public TPaveLabel {
public:
	TBand(TLevelScheme* levelScheme = nullptr, const std::string& label = "");
	TBand(const TBand& rhs);
	~TBand() {}

	TLevel* AddLevel(const double energy, const std::string& label); // *MENU*
	TLevel* AddLevel(const double energy, const char* label) { std::string tmp(label); return AddLevel(energy, tmp); } // *MENU*
	void AddLevel(TLevel* level);
	void RemoveLevel(TLevel* level);

	size_t NofLevels() const { return fLevels.size(); }
	TLevel* GetLevel(double energy);
	TLevel* FindLevel(double energy, double energyUncertainty);
	std::pair<double, double> GetMinMaxLevelEnergy() const { return std::make_pair(fLevels.begin()->second.Energy(), fLevels.rbegin()->second.Energy()); }
	std::pair<double, double> GetMinMaxGamma() const;
	double Width(double distance) const; ///< return width of this band using provided distance between gammas

	std::map<double, TLevel>::iterator begin() { return fLevels.begin(); }
	std::map<double, TLevel>::iterator end() { return fLevels.end(); }
	std::map<double, TLevel>::const_iterator begin() const { return fLevels.begin(); }
	std::map<double, TLevel>::const_iterator end() const { return fLevels.end(); }

	void Print() const;

	void Debug(bool val) { fDebug = val; for(auto& [energy, level] : fLevels) { level.Debug(val); } }

	TBand& operator=(const TBand& rhs);

private:
	bool fDebug{false};
	std::map<double, TLevel> fLevels;
	TLevelScheme* fLevelScheme{nullptr};

   /// \cond CLASSIMP
   ClassDefOverride(TBand, 1); // Level
   /// \endcond
};

class TLevelScheme : public TBox {
public:
	enum class EGammaWidth { kNoWidth, kBand, kGlobal };

	TLevelScheme(const std::string& filename = "", bool debug = false);
	TLevelScheme(const char* filename, bool debug = false) : TLevelScheme(std::string(filename), debug) {}
	TLevelScheme(const TLevelScheme& rhs);
	~TLevelScheme();

	TLevel* AddLevel(const double energy, const std::string bandName, const std::string label);
	TLevel* AddLevel(const double energy, const char* bandName, const char* label) { return AddLevel(energy, std::string(bandName), std::string(label)); } // *MENU*
	TLevel* GetLevel(double energy);
	TLevel* FindLevel(double energy, double energyUncertainty);

	void MoveToBand(const char* bandName, TLevel* level);

	void UseGlobalGammaWidth(const int val) { fGammaWidth = static_cast<EGammaWidth>(val); } // *MENU*
	void GammaDistance(const double val) { fGammaDistance = val; } // *MENU*
	void BandGap(const double val) { fBandGap = val; } // *MENU*
	void LeftMargin(const double val) { fLeftMargin = val; } // *MENU*
	void RightMargin(const double val) { fRightMargin = val; } // *MENU*
	void BottomMargin(const double val) { fBottomMargin = val; } // *MENU*
	void TopMargin(const double val) { fTopMargin = val; } // *MENU*

	void Refresh() { Draw(); } // *MENU*
	void Draw(Option_t* option = "") override;

	void Print();

	void Debug(bool val) { fDebug = val; for(auto& band : fBands) { band.Debug(val); } }

private:
	void ParseENSDF(const std::string& filename);
	void DrawAuxillaryLevel(const double& energy, const double& left, const double& right);

	bool fDebug{false};

	std::vector<TBand> fBands;
	std::multimap<double, TLine> fAuxillaryLevels;

	// nuclide information
	std::string fNuclide{""};
	double fQValue{0.};
	double fQValueUncertainty{0.};
	double fNeutronSeparation{0.};
	double fNeutronSeparationUncertainty{0.};

	// graphics settings
	EGammaWidth fGammaWidth{EGammaWidth::kNoWidth};
	double fGammaDistance{50.};
	double fBandGap{200.};
	double fLeftMargin{-1.};
	double fRightMargin{-1.};
	double fBottomMargin{-1.};
	double fTopMargin{-1.};
	double fMinWidth{1.};
	double fMaxWidth{10.};

   /// \cond CLASSIMP
   ClassDefOverride(TLevelScheme, 1); // Level scheme
   /// \endcond
};
/*! @} */

#endif
