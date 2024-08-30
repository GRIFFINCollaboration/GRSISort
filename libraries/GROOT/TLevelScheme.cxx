#include "TLevelScheme.h"

#if __cplusplus >= 201703L

#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <algorithm>

#include "TROOT.h"
#include "TString.h"

#include "Globals.h"
#include "GCanvas.h"
#include "TGRSIUtilities.h"

double TGamma::fTextSize = 0.020;
double TLevel::fTextSize = 0.025;

std::vector<TLevelScheme*> TLevelScheme::fLevelSchemes;

TGamma::TGamma(TLevelScheme* levelScheme, const std::string& label, const double& br, const double& ts)
{
   fLevelScheme        = levelScheme;
   fBranchingRatio     = br;
   fTransitionStrength = ts;
   fLabelText          = label;
   fLabelText.insert(0, 1, ' ');   // prepend a space to get distance from arrow
   SetArrowSize(0.01);
}

TGamma::TGamma(const TGamma& rhs)
   : TArrow(rhs)
{
   if(fDebug || rhs.fDebug) {
      std::cout << __PRETTY_FUNCTION__ << ": copying gamma \"" << rhs.fLabel << "\" to \"" << fLabel << "\" (" << &rhs << " to " << this << ")" << std::endl;
   }

   fDebug                            = rhs.fDebug;
   fEnergy                           = rhs.fEnergy;
   fEnergyUncertainty                = rhs.fEnergyUncertainty;
   fUseTransitionStrength            = rhs.fUseTransitionStrength;
   fScalingGain                      = rhs.fScalingGain;
   fScalingOffset                    = rhs.fScalingOffset;
   fBranchingRatio                   = rhs.fBranchingRatio;
   fBranchingRatioUncertainty        = rhs.fBranchingRatioUncertainty;
   fBranchingRatioPercent            = rhs.fBranchingRatioPercent;
   fBranchingRatioPercentUncertainty = rhs.fBranchingRatioPercentUncertainty;
   fTransitionStrength               = rhs.fTransitionStrength;
   fTransitionStrengthUncertainty    = rhs.fTransitionStrengthUncertainty;
   fLabelText                        = rhs.fLabelText;
   fLabel                            = rhs.fLabel;
   fLevelScheme                      = rhs.fLevelScheme;
   fInitialEnergy                    = rhs.fInitialEnergy;
   fFinalEnergy                      = rhs.fFinalEnergy;
}

TGamma::~TGamma()
{
}

TGamma& TGamma::operator=(const TGamma& rhs)
{
   if(fDebug || rhs.fDebug) {
      std::cout << __PRETTY_FUNCTION__ << ": copying gamma \"" << rhs.fLabel << "\" to \"" << fLabel << "\" (" << &rhs << " to " << this << ")" << std::endl;
   }

   if(this != &rhs) {
      fDebug                            = rhs.fDebug;
      fEnergy                           = rhs.fEnergy;
      fEnergyUncertainty                = rhs.fEnergyUncertainty;
      fUseTransitionStrength            = rhs.fUseTransitionStrength;
      fScalingGain                      = rhs.fScalingGain;
      fScalingOffset                    = rhs.fScalingOffset;
      fBranchingRatio                   = rhs.fBranchingRatio;
      fBranchingRatioUncertainty        = rhs.fBranchingRatioUncertainty;
      fBranchingRatioPercent            = rhs.fBranchingRatioPercent;
      fBranchingRatioPercentUncertainty = rhs.fBranchingRatioPercentUncertainty;
      fTransitionStrength               = rhs.fTransitionStrength;
      fTransitionStrengthUncertainty    = rhs.fTransitionStrengthUncertainty;
      fLabelText                        = rhs.fLabelText;
      fLabel                            = rhs.fLabel;
      fLevelScheme                      = rhs.fLevelScheme;
      fInitialEnergy                    = rhs.fInitialEnergy;
      fFinalEnergy                      = rhs.fFinalEnergy;
   }

   return *this;
}

void TGamma::UpdateWidth()
{
   if(fDebug) std::cout << __PRETTY_FUNCTION__ << std::endl;
   double arrowsize = (fUseTransitionStrength ? fTransitionStrength : fBranchingRatio) * fScalingGain + fScalingOffset;
   SetLineWidth(arrowsize);
}

void TGamma::UpdateLabel()
{
   if(fDebug) std::cout << __PRETTY_FUNCTION__ << std::endl;
   if(fLabel != nullptr) {
      fLabel->SetText(fLabel->GetX(), fLabel->GetY(), fLabelText.c_str());
   }
}

void TGamma::Draw(const double& x1, const double& y1, const double& x2, const double& y2)
{
   double arrowsize = (fUseTransitionStrength ? fTransitionStrength : fBranchingRatio) * fScalingGain + fScalingOffset;

   SetX1(x1);
   SetY1(y1);
   SetX2(x2);
   SetY2(y2);
   SetLineWidth(arrowsize);
   TArrow::Draw("|>");   // |> means using a filled triangle as point at x2,y2
   if(fLabel == nullptr) {
      fLabel = new TLatex((x1 + x2) / 2., (y1 + y2) / 2., fLabelText.c_str());
   } else {
      fLabel->SetText((x1 + x2) / 2., (y1 + y2) / 2., fLabelText.c_str());
   }
   fLabel->SetTextColor(GetLineColor());
   fLabel->SetTextAlign(12);         // left and center adjusted
   fLabel->SetTextFont(42);          // helvetica-medium-r-normal (default is 62 = helvetica-bold-r-normal)
   fLabel->SetTextSize(fTextSize);   // text size in fraction of window width/height in pixel (whichever is smaller)
   fLabel->Draw();
   if(fDebug) Print();
}

void TGamma::Print(Option_t*) const
{
   TArrow::Print();
   std::cout << "Gamma with energy " << fEnergy << " +- " << fEnergyUncertainty << " (from " << fInitialEnergy << " to " << fFinalEnergy << ") " << (fUseTransitionStrength ? "using" : "not using") << " transition strength " << fTransitionStrength << ", branching " << fBranchingRatio << " = " << 100. * fBranchingRatioPercent << "%, scaling gain " << fScalingGain << ", scaling offset " << fScalingOffset << ", arrow size " << GetArrowSize() << ", line color " << GetLineColor() << ", line width " << GetLineWidth() << std::endl;
}

std::map<double, double> TGamma::CoincidentGammas()
{
   /// Returns a map with the energies and relative strength of all feeding and draining gamma rays in coincidence with this gamma ray.
   if(fLevelScheme == nullptr) {
      std::cerr << "Parent level scheme not set, can't find coincident gamma rays" << std::endl;
      return std::map<double, double>();
   }
   if(fDebug) {
      std::cout << "Looking for coincident gammas for gamma of " << fEnergy << " kev from level at " << fInitialEnergy << " keV to level at " << fFinalEnergy << " keV" << std::endl;
   }
   fLevelScheme->ResetGammaMap();   // clear the map of feeding gammas before calling the recursive function using it
   auto result = fLevelScheme->FeedingGammas(fInitialEnergy, fBranchingRatioPercent);
   if(fDebug) {
      std::cout << "Got " << result.size() << " gammas feeding level at " << fInitialEnergy << std::endl;
   }
   if(fFinalEnergy > 0) {
      auto draining = fLevelScheme->DrainingGammas(fFinalEnergy);
      if(fDebug) {
         std::cout << "Plus " << draining.size() << " gammas draining level at " << fFinalEnergy << std::endl;
      }
      for(auto& [energy, factor] : draining) {
         result[energy] += factor;
      }
   }
   if(fDebug) {
      std::cout << "Got " << result.size() << " coincident gammas:";
      for(auto& element : result) {
         std::cout << " " << element.first << " (" << element.second << "%)";
      }
      std::cout << std::endl;
   }

   return result;
}

void TGamma::PrintCoincidentGammas()
{
   auto map = CoincidentGammas();
   std::cout << map.size() << " coincident gammas for gamma " << fEnergy << " +- " << fEnergyUncertainty << " (" << fInitialEnergy << " to " << fFinalEnergy << "):";
   for(auto gamma : map) {
      std::cout << " " << gamma.first << " (" << gamma.second << "%)";
   }
   std::cout << std::endl;
}

std::vector<std::tuple<double, std::vector<double>>> TGamma::ParallelGammas()
{
   /// Returns a map with the relative strength and energies of all gamma rays that connect the same initial and final level.
   /// Meaning these are all gamma rays that are parallel with this one and together add up to the same energy.
   if(fLevelScheme == nullptr) {
      std::cerr << "Parent level scheme not set, can't find parallel gamma rays" << std::endl;
      return std::vector<std::tuple<double, std::vector<double>>>();
   }
   if(fDebug) {
      std::cout << "Looking for parallel gammas for gamma of " << fEnergy << " kev from level at " << fInitialEnergy << " keV to level at " << fFinalEnergy << " keV" << std::endl;
   }
   auto result = fLevelScheme->ParallelGammas(fInitialEnergy, fFinalEnergy);
   auto last   = std::remove_if(result.begin(), result.end(), [](std::tuple<double, std::vector<double>> x) { return std::get<1>(x).size() < 2; });
   if(fDebug) {
      std::cout << "Removing " << std::distance(last, result.end()) << " paths, keeping " << std::distance(result.begin(), last) << std::endl;
   }
   result.erase(last, result.end());
   // if(fDebug) {
   //	std::cout<<"Got "<<result.size()<<" paths from level at "<<fInitialEnergy<<" keV to level at "<<fFinalEnergy<<" keV:"<<std::endl;
   //	for(auto& element : result) {
   //		std::cout<<std::get<0>(element)<<":";
   //		double totalEnergy = 0.;
   //		for(auto& energy : std::get<1>(element)) {
   //			std::cout<<" "<<energy;
   //			totalEnergy += energy;
   //		}
   //		std::cout<<" ("<<totalEnergy<<")"<<std::endl;
   //	}
   // }

   return result;
}

void TGamma::PrintParallelGammas()
{
   auto vec = ParallelGammas();
   std::cout << vec.size() << " parallel paths for gamma " << fEnergy << " +- " << fEnergyUncertainty << " (" << fInitialEnergy << " to " << fFinalEnergy << "):" << std::endl;
   for(auto& element : vec) {
      std::cout << std::get<0>(element) << ":";
      double totalEnergy = 0.;
      for(auto& energy : std::get<1>(element)) {
         std::cout << " " << energy;
         totalEnergy += energy;
      }
      std::cout << " (" << totalEnergy << ")" << std::endl;
   }
}

TLevel::TLevel(TLevelScheme* levelScheme, const double& energy, const std::string& label)
{
   if(fDebug && levelScheme == nullptr) {
      std::cout << "Warning, nullptr provided to new band \"" << label << "\" for parent level scheme, some functions might no be available" << std::endl;
   }
   fLevelScheme = levelScheme;
   fEnergy      = energy;
   fLabel       = label;
}

TLevel::TLevel(const TLevel& rhs)
   : TPolyLine(rhs)
{
   if(fDebug || rhs.fDebug) {
      std::cout << __PRETTY_FUNCTION__ << ": copying level \"" << rhs.fLabel << "\" to \"" << fLabel << "\" (" << &rhs << " to " << this << ")" << std::endl;
   }

   fDebug             = rhs.fDebug;
   fEnergy            = rhs.fEnergy;
   fEnergyUncertainty = rhs.fEnergyUncertainty;
   fLabel             = rhs.fLabel;
   fGammas            = rhs.fGammas;
   fNofFeeding        = rhs.fNofFeeding;
   fLevelScheme       = rhs.fLevelScheme;
   fLevelLabel        = rhs.fLevelLabel;
   fEnergyLabel       = rhs.fEnergyLabel;
   fOffset            = rhs.fOffset;
   if(fDebug) {
      std::cout << __PRETTY_FUNCTION__ << ": copied level \"" << rhs.fLabel << "\" to \"" << fLabel << "\" (" << &rhs << " to " << this << ")" << std::endl;
   }
}

TLevel::~TLevel()
{
   if(fDebug) {
      std::cout << "Deleting level \"" << fLabel << "\" at " << fEnergy << " keV (" << this << ")" << std::endl;
   }
}

TLevel& TLevel::operator=(const TLevel& rhs)
{
   if(fDebug || rhs.fDebug) {
      std::cout << __PRETTY_FUNCTION__ << ": copying level \"" << rhs.fLabel << "\" to \"" << fLabel << "\" (" << &rhs << " to " << this << ")" << std::endl;
   }

   if(this != &rhs) {
      TPolyLine::operator=(rhs);
      fDebug       = rhs.fDebug;
      fEnergy      = rhs.fEnergy;
      fLabel       = rhs.fLabel;
      fGammas      = rhs.fGammas;
      fNofFeeding  = rhs.fNofFeeding;
      fLevelScheme = rhs.fLevelScheme;
      fLevelLabel  = rhs.fLevelLabel;
      fEnergyLabel = rhs.fEnergyLabel;
      fOffset      = rhs.fOffset;
      if(fDebug) {
         std::cout << __PRETTY_FUNCTION__ << ": copied level \"" << rhs.fLabel << "\" to \"" << fLabel << "\" (" << &rhs << " to " << this << ")" << std::endl;
      }
   }

   return *this;
}

TGamma* TLevel::AddGamma(const double levelEnergy, const char* label, double br, double ts)
{
   /// MENU function to add gamma from this level to another level at "levelEnergy"
   /// Adds a new gamma ray with specified branching ratio (default 100.), and transition strength (default 1.).
   /// Returns gamma if it doesn't exist yet and was successfully added, null pointer otherwise.
   return AddGamma(levelEnergy, 0., label, br, ts);
}

TGamma* TLevel::AddGamma(const double levelEnergy, const double energyUncertainty, const char* label, double br, double ts)
{
   /// Function to add gamma from this level to another level at "levelEnergy +- energyUncertainty".
   /// Adds a new gamma ray with specified branching ratio (default 100.), and transition strength (default 1.).
   /// Returns gamma if it doesn't exist yet and was successfully added, null pointer otherwise.
   auto* level = fLevelScheme->FindLevel(levelEnergy, energyUncertainty);
   if(level == nullptr) {
      std::cerr << DRED << "Failed to find level at " << levelEnergy << " keV, can't add gamma!" << RESET_COLOR << std::endl;
      return nullptr;
   }

   // now that we found a level, we will use its energy instead of the levelEnergy parameter
   if(fGammas.count(level->Energy()) == 1) {
      if(fDebug) {
         std::cout << "Already found gamma ray from ";
         Print();
         std::cout << " to " << levelEnergy << "/" << level->Energy() << " keV";
         level->Print();
      }
      return nullptr;
   }

   // create the gamma and set its properties
   fGammas.emplace(std::piecewise_construct, std::forward_as_tuple(level->Energy()), std::forward_as_tuple(fLevelScheme, label, br, ts));
   fGammas[level->Energy()].Debug(fDebug);
   fGammas[level->Energy()].Energy(Energy() - levelEnergy);   // here we do use the levelEnergy parameter instead of the energy of the level we found
   fGammas[level->Energy()].EnergyUncertainty(energyUncertainty);
   fGammas[level->Energy()].InitialEnergy(fEnergy);
   fGammas[level->Energy()].FinalEnergy(level->Energy());
   level->AddFeeding();
   // re-calculate the branching ratios in % for all gammas
   double sum = std::accumulate(fGammas.begin(), fGammas.end(), 0., [](double r, std::pair<const double, TGamma>& g) { r += g.second.BranchingRatio(); return r; });
   for(auto& [en, gamma] : fGammas) {
      gamma.BranchingRatioPercent(gamma.BranchingRatio() / sum);
      gamma.BranchingRatioPercentUncertainty(gamma.BranchingRatioUncertainty() / sum);
   }

   if(fDebug) Print();

   fLevelScheme->Refresh();

   return &(fGammas[level->Energy()]);
}

void TLevel::MoveToBand(const char* val)
{
   if(fLevelScheme == nullptr) {
      std::cerr << __PRETTY_FUNCTION__ << ": Parent level scheme pointer not set!" << std::endl;
      return;
   }
   if(fDebug) {
      std::cout << "Trying to move level " << this << " at " << Energy() << " keV to band \"" << val << "\" using " << fLevelScheme << std::endl;
   }
   fLevelScheme->MoveToBand(val, this);
}

std::pair<double, double> TLevel::GetMinMaxGamma() const
{
   if(fGammas.empty()) return std::make_pair(-1., -1.);
   auto result = std::make_pair(fGammas.begin()->second.Width(), fGammas.begin()->second.Width());
   for(const auto& [energy, gamma] : fGammas) {
      if(gamma.Width() < result.first) result.first = gamma.Width();
      if(gamma.Width() > result.second) result.second = gamma.Width();
   }

   return result;
}

void TLevel::Draw(const double& left, const double& right)
{
   std::vector<double> x;
   std::vector<double> y;
   SetLineWidth(2.);
   double tickLength = (fOffset == 0. ? 0. : 10.);   // No need for a tick if we don't have an offset. maybe make this a parameter instead of hard-coded?
   if(fOffset != 0.) {
      // add extra point from label to edge of level
      x.push_back(left);
      y.push_back(fEnergy + fOffset);
      x.push_back(left + tickLength);
      y.push_back(fEnergy + fOffset);
      x.push_back(left + tickLength + std::fabs(fOffset) / 2.);
      y.push_back(fEnergy);
      if(fDebug) std::cout << "Non-zero offset " << fOffset << " => left line " << left << ", " << fEnergy + fOffset << " via " << left + tickLength << ", " << fEnergy + fOffset << " to " << left + tickLength + std::fabs(fOffset) / 2. << ", " << fEnergy << std::endl;
   }
   x.push_back(left + tickLength + std::fabs(fOffset) / 2.);
   y.push_back(fEnergy);
   x.push_back(right - tickLength - std::fabs(fOffset) / 2.);
   y.push_back(fEnergy);
   if(fOffset != 0.) {
      x.push_back(right - tickLength - std::fabs(fOffset) / 2.);
      y.push_back(fEnergy);
      x.push_back(right - tickLength);
      y.push_back(fEnergy + fOffset);
      x.push_back(right);
      y.push_back(fEnergy + fOffset);
      if(fDebug) std::cout << "Non-zero offset " << fOffset << " => right line " << right - tickLength - std::fabs(fOffset) / 2. << ", " << fEnergy << " via " << right - tickLength << ", " << fEnergy + fOffset << " to " << right << ", " << fEnergy + fOffset << std::endl;
   }
   SetPolyLine(x.size(), x.data(), y.data());
   TPolyLine::Draw();
   if(fDebug) std::cout << "Drew TPolyLine using x " << left << "-" << right << " and y " << fEnergy << " with a width of " << GetLineWidth() << " and offset " << fOffset << std::endl;
}

double TLevel::DrawLabel(const double& pos)
{
   if(fLevelLabel == nullptr) {
      fLevelLabel = new TLatex(pos, fEnergy + fOffset, fLabel.c_str());
   } else {
      fLevelLabel->SetText(pos, fEnergy + fOffset, fLabel.c_str());
   }
   fLevelLabel->SetTextColor(GetLineColor());
   fLevelLabel->SetTextAlign(12);         // left adjusted and vertically centered
   fLevelLabel->SetTextFont(42);          // helvetica-medium-r-normal (default is 62 = helvetica-bold-r-normal)
   fLevelLabel->SetTextSize(fTextSize);   // text size in fraction of window width/height in pixel (whichever is smaller)
   fLevelLabel->Draw();
   return fLevelLabel->GetXsize();
}

double TLevel::DrawEnergy(const double& pos)
{
   if(fEnergyLabel == nullptr) {
      fEnergyLabel = new TLatex(pos, fEnergy + fOffset, Form("%.0f keV", fEnergy));
   } else {
      fEnergyLabel->SetText(pos, fEnergy + fOffset, Form("%.0f keV", fEnergy));
   }
   fEnergyLabel->SetTextColor(GetLineColor());
   fEnergyLabel->SetTextAlign(32);         // right adjusted and vertically centered
   fEnergyLabel->SetTextFont(42);          // helvetica-medium-r-normal (default is 62 = helvetica-bold-r-normal)
   fEnergyLabel->SetTextSize(fTextSize);   // text size in fraction of window width/height in pixel (whichever is smaller)
   fEnergyLabel->Draw();
   return fEnergyLabel->GetXsize();
}

void TLevel::Print(Option_t*) const
{
   std::cout << "Level \"" << fLabel << "\" (" << this << ") at " << fEnergy << " keV has " << fGammas.size() << " draining gammas and " << fNofFeeding << " feeding gammas, debugging" << (fDebug ? "" : " not") << " enabled" << std::endl;
   if(fDebug) {
      for(const auto& [level, gamma] : fGammas) {
         std::cout << "gamma to level " << level << " \"" << gamma.LabelText() << "\"" << std::endl;
      }
   }
}

TBand::TBand(TLevelScheme* levelScheme, const std::string& label)
{
   if(levelScheme == nullptr) {
      std::cout << "Warning, nullptr provided to new band \"" << label << "\" for parent level scheme, some functions might no be available" << std::endl;
   }
   fLevelScheme = levelScheme;
   SetLabel(label.c_str());
   SetTextSize(0);     // default size (?)
   SetTextAlign(22);   // centered in x and y
}

TBand::TBand(const TBand& rhs)
   : TPaveLabel(rhs)
{
   if(fDebug || rhs.fDebug) {
      std::cout << __PRETTY_FUNCTION__ << ": copying band \"" << rhs.GetLabel() << "\" to \"" << GetLabel() << "\" (" << &rhs << " to " << this << "), " << rhs.fLevels.size() << " level(s) to " << fLevels.size() << " level(s)" << std::endl;
   }

   fDebug       = rhs.fDebug;
   fLevels      = rhs.fLevels;
   fLevelScheme = rhs.fLevelScheme;

   if(fDebug || rhs.fDebug) {
      std::cout << __PRETTY_FUNCTION__ << ": copied band \"" << rhs.GetLabel() << "\" to \"" << GetLabel() << "\" (" << &rhs << " to " << this << "), " << rhs.fLevels.size() << " level(s) to " << fLevels.size() << " level(s)" << std::endl;
   }
}

TBand& TBand::operator=(const TBand& rhs)
{
   if(this != &rhs) {
      TPaveLabel::operator=(rhs);
      if(fDebug || rhs.fDebug) {
         std::cout << __PRETTY_FUNCTION__ << ": copying band \"" << rhs.GetLabel() << "\" to \"" << GetLabel() << "\" (" << &rhs << " to " << this << "), " << rhs.fLevels.size() << " level(s) to " << fLevels.size() << " level(s)" << std::endl;
      }
      fDebug       = rhs.fDebug;
      fLevels      = rhs.fLevels;
      fLevelScheme = rhs.fLevelScheme;
      if(fDebug || rhs.fDebug) {
         std::cout << __PRETTY_FUNCTION__ << ": copied band \"" << rhs.GetLabel() << "\" to \"" << GetLabel() << "\" (" << &rhs << " to " << this << "), " << rhs.fLevels.size() << " level(s) to " << fLevels.size() << " level(s)" << std::endl;
      }
   }

   return *this;
}

TLevel* TBand::GetLevel(double energy)
{
   /// Returns level at provided energy, returns null pointer if level at this energy does not exist
   if(fLevels.count(energy) == 0) {
      std::cout << this << ": failed to find level with " << energy << " keV" << std::endl;
      Print();
      return nullptr;
   }
   if(fDebug) {
      std::cout << this << ": found ";
      fLevels.find(energy)->second.Print();
      std::cout << "energy " << energy << " - " << &(fLevels.find(energy)->second) << std::endl;
   }
   return &(fLevels.find(energy)->second);
}

TLevel* TBand::FindLevel(double energy, double energyUncertainty)
{
   /// Returns level at the provided energy +- the uncertainty. If there isn't any level in that range a null pointer is returned.
   /// If there are multiple levels in the range the one with the smallest energy difference is returned.
   auto low  = fLevels.lower_bound(energy - energyUncertainty);
   auto high = fLevels.upper_bound(energy + energyUncertainty);
   if(low == high) {
      std::cout << this << ": failed to find level with " << energy << " +- " << energyUncertainty << " keV" << std::endl;
      Print();
      return nullptr;
   }
   if(std::distance(low, high) == 1) {
      if(fDebug) {
         std::cout << this << ": found ";
         low->second.Print();
         std::cout << "energy " << energy << " +- " << energyUncertainty << " - " << &(low->second) << std::endl;
      }
      return &(low->second);
   }
   // found multiple matching levels, use the one with the smallest energy difference
   double  en    = low->first;
   TLevel* level = &(low->second);
   for(auto& it = ++low; it != high; ++it) {
      if(std::fabs(energy - en) > std::fabs(energy - it->first)) {
         en    = it->first;
         level = &(it->second);
      }
   }
   if(fDebug) {
      std::cout << this << ": found " << std::distance(low, high) << " levels, closest is";
      level->Print();
      std::cout << "energy " << energy << " +- " << energyUncertainty << " - " << level << std::endl;
   }
   return level;
}

std::pair<double, double> TBand::GetMinMaxGamma() const
{
   if(fLevels.empty()) throw std::runtime_error("Trying to get min/max gamma width from empty band");
   auto result = fLevels.begin()->second.GetMinMaxGamma();
   for(const auto& [energy, level] : fLevels) {
      auto [min, max] = level.GetMinMaxGamma();
      if(min < result.first) result.first = min;
      if(max > result.second) result.second = max;
   }

   return result;
}

TLevel* TBand::AddLevel(const double energy, const std::string& label)
{
   /// Add a new level with specified energy and label.
   /// Returns pointer to new level, or, if it already exists, returns pointer to existing level.
   /// Can be called from context menu.
   // emplace returns a pair: the iterator of the newly created/existing element and a boolean true/false
   if(fDebug) {
      std::cout << __PRETTY_FUNCTION__ << " - " << this << ": Trying to add new level \"" << label << "\" at " << energy << " keV to " << std::flush;
      Print();
   }
   auto [newLevel, success] = fLevels.emplace(std::piecewise_construct, std::forward_as_tuple(energy), std::forward_as_tuple(fLevelScheme, energy, label));
   if(!success) {
      std::cerr << DRED << "Failed to add new level \"" << label << "\" at " << energy << " keV to " << RESET_COLOR;
      Print();
   }

   newLevel->second.Debug(fDebug);
   if(fDebug) {
      std::cout << this << ": adding new level \"" << label << "\" at " << energy << " keV " << (success ? "was a success" : "failed") << " now got " << fLevels.size() << "/" << NofLevels() << " levels, debugging" << (fDebug ? "" : " not") << " enabled" << std::endl;
      if(success) {
         std::cout << "New level: ";
         newLevel->second.Print();
         Print();
      }
   }

   fLevelScheme->Refresh();

   return &(newLevel->second);
}

void TBand::AddLevel(TLevel* level)
{
   if(fDebug) {
      std::cout << __PRETTY_FUNCTION__ << " - " << this << " - \"" << GetLabel() << "\": Trying to add new level \"" << level->Label() << "\" at " << level->Energy() << " keV" << std::endl;
   }
   auto [iterator, success] = fLevels.insert(std::pair{level->Energy(), *level});
   if(!success) {
      std::cout << "Failed to insert level at " << level->Energy() << " into ";
      Print();
   } else if(fDebug) {
      std::cout << "Successfully added new level \"" << level->Label() << "\" at " << level->Energy() << " keV" << std::endl;
      Print();
   }
}

void TBand::RemoveLevel(TLevel* level)
{
   /// Removes level that matches the energy of the provided level (if one exists).
   auto nofRemoved = fLevels.erase(level->Energy());
   if(fDebug) {
      std::cout << "\"" << GetLabel() << "\": Removed " << nofRemoved << " levels" << std::endl;
      Print();
   }
}

double TBand::Width(double distance) const
{
   size_t nofGammas = 0.;
   if(fDebug) {
      std::cout << " (" << GetLabel() << " - " << fLevels.size() << ": ";
   }
   for(const auto& level : fLevels) {
      nofGammas += level.second.NofDrainingGammas() + 1;   // plus 1 for the gap between the gammas from each level
      if(fDebug) {
         std::cout << " " << nofGammas << " ";
      }
   }
   if(nofGammas == 0) nofGammas = 1;   // to get a minimum width

   if(fDebug) {
      std::cout << "=> width " << nofGammas * distance << ") ";
   }

   return nofGammas * distance;
}

void TBand::Print(Option_t*) const
{
   std::cout << this << ": band \"" << GetLabel() << "\" " << fLevels.size() << " level(s):";
   for(const auto& level : fLevels) {
      std::cout << " " << level.first;
   }
   std::cout << ", debugging" << (fDebug ? "" : " not") << " enabled" << std::endl;
   if(fDebug) {
      for(const auto& level : fLevels) {
         level.second.Print();
      }
   }
}

TLevelScheme::TLevelScheme(const std::string& filename, bool debug)
{
   fDebug = debug;
   SetName("TLevelScheme");
   SetLabel("Level Scheme");

   // open the file and read the level scheme
   // still need to decide what format that should be
   if(!filename.empty()) {
      if(!file_exists(filename.c_str())) {
         std::cout << "file " << filename << " does not exist or we don't have read permissions" << std::endl;
      } else {
         auto lastDot = filename.find_last_of('.');
         if(lastDot == std::string::npos) {
            std::cout << "Failed to find extension => unknown data format" << std::endl;
         } else {
            std::string ext = filename.substr(lastDot + 1);
            if(ext == "ensdf") {
               ParseENSDF(filename);
            } else {
               std::cout << "Unknown extension " << ext << " => unknown data format" << std::endl;
            }
         }
      }
   }

   fLevelSchemes.push_back(this);
}

TLevelScheme::TLevelScheme(const TLevelScheme& rhs)
   : TPaveLabel(rhs)
{
   fDebug                        = rhs.fDebug;
   fBands                        = rhs.fBands;
   fAuxillaryLevels              = rhs.fAuxillaryLevels;
   fQValue                       = rhs.fQValue;
   fQValueUncertainty            = rhs.fQValueUncertainty;
   fNeutronSeparation            = rhs.fNeutronSeparation;
   fNeutronSeparationUncertainty = rhs.fNeutronSeparationUncertainty;
   fGammaWidth                   = rhs.fGammaWidth;
   fGammaDistance                = rhs.fGammaDistance;
   fBandGap                      = rhs.fBandGap;
   fLeftMargin                   = rhs.fLeftMargin;
   fRightMargin                  = rhs.fRightMargin;
   fBottomMargin                 = rhs.fBottomMargin;
   fTopMargin                    = rhs.fTopMargin;
   fMinWidth                     = rhs.fMinWidth;
   fMaxWidth                     = rhs.fMaxWidth;

   fLevelSchemes.push_back(this);
}

TLevelScheme::~TLevelScheme()
{
}

void TLevelScheme::ListLevelSchemes()
{
   for(auto& scheme : fLevelSchemes) {
      std::cout << " \"" << scheme->GetLabel() << "\"";
   }
   std::cout << std::endl;
}

TLevelScheme* TLevelScheme::GetLevelScheme(const char* name)
{
   for(auto& scheme : fLevelSchemes) {
      if(strcmp(name, scheme->GetLabel()) == 0) {
         return scheme;
      }
   }

   std::cout << "Failed to find level scheme \"" << name << "\", " << fLevelSchemes.size() << " level schemes exist";
   ListLevelSchemes();

   return nullptr;
}

TLevel* TLevelScheme::AddLevel(const double energy, const std::string bandName, const std::string label)
{
   /// Add level at specified energy to specified band and give it the provided label.
   /// Can be called from context menu using const char* instead of std::string.
   if(fDebug) Print();

   for(auto& band : fBands) {
      if(bandName == band.GetLabel()) {
         if(fDebug) {
            std::cout << "Found band \"" << band.GetLabel() << "\" (" << &band << ") with " << band.NofLevels() << " levels" << std::endl;
         }
         return band.AddLevel(energy, label);
      }
		if(fDebug) {
         std::cout << "Band \"" << band.GetLabel() << "\" does not match " << bandName << std::endl;
      }
   }
   fBands.push_back(TBand(this, bandName));
   auto& newBand = fBands.back();
   newBand.Debug(fDebug);
   if(fDebug) {
      std::cout << "Created band \"" << newBand.GetLabel() << "\" (" << &newBand << ") with " << newBand.NofLevels() << " level" << std::endl;
      newBand.Print();
      std::cout << std::endl;
   }
   return newBand.AddLevel(energy, label);
}

TLevel* TLevelScheme::GetLevel(double energy)
{
   for(auto& band : fBands) {
      auto* level = band.GetLevel(energy);
      if(level != nullptr) {
         return level;
      }
   }
   // if we reach here we failed to find a level with this energy
   // output bands with min/max levels and return null pointer
   std::cout << "Failed to find level with energy " << energy << " in " << fBands.size() << " bands:" << std::endl;
   for(const auto& band : fBands) {
      band.Print();
   }

   return nullptr;
}

TLevel* TLevelScheme::FindLevel(double energy, double energyUncertainty)
{
   for(auto& band : fBands) {
      auto* level = band.FindLevel(energy, energyUncertainty);
      if(level != nullptr) {
         return level;
      }
   }
   // if we reach here we failed to find a level with this energy
   // output bands with min/max levels and return null pointer
   std::cout << "Failed to find level with energy " << energy << " +- " << energyUncertainty << " in " << fBands.size() << " bands:" << std::endl;
   for(const auto& band : fBands) {
      band.Print();
   }

   return nullptr;
}

TGamma* TLevelScheme::FindGamma(double energy, double energyUncertainty)
{
   /// Finds gamma ray in range [energy - energyUncertainty, energy + energyUncertainty].
   /// If multiple gamma rays are in this range it returns the one closest to the given energy.
   auto list = FindGammas(energy - energyUncertainty, energy + energyUncertainty);

   if(list.empty()) {
      std::cerr << "Failed to find any gamma ray in range [" << energy - energyUncertainty << ", " << energy + energyUncertainty << "]" << std::endl;
      return nullptr;
   }

   auto result = list[0];
   for(size_t i = 1; i < list.size(); ++i) {
      if(std::fabs(list[i]->Energy() - energy) < std::fabs(result->Energy() - energy)) result = list[i];
   }

   return result;
}

std::vector<TGamma*> TLevelScheme::FindGammas(double lowEnergy, double highEnergy)
{
   /// Returns a list of all gamma-rays in the range [lowEnergy, highEnergy]
   std::vector<TGamma*> result;
   // loop over all bands
   for(auto& band : fBands) {
      // loop over all levels in this band
      for(auto& [levelEnergy, level] : band) {
         // loop over all gamma-rays in the level
         for(auto& [finalEnergy, gamma] : level) {
            if(lowEnergy <= gamma.Energy() && gamma.Energy() <= highEnergy) {
               result.push_back(&gamma);
            }
         }
      }
   }

   return result;
}

void TLevelScheme::BuildGammaMap(double levelEnergy)
{
   /// Build a map of all gamma rays that populate levels equal and greater than levelEnergy.
   /// This map is used to determine all gammas feeding a specific level.
   /// Does nothing if the map isn't empty.
   if(!fGammaMap.empty()) return;

   for(auto& band : fBands) {
      for(auto& [energy, level] : band) {
         if(energy <= levelEnergy) {
            // if the level is below this level or this level itself, gamma rays draining it can't feed this level
            continue;
         }
         for(auto& [finalEnergy, gamma] : level) {
            if(finalEnergy >= levelEnergy) {
               fGammaMap.insert({finalEnergy, &gamma});
            }
         }
      }
   }
   if(fDebug) {
      std::cout << "Built map of " << fGammaMap.size() << " gamma rays populating levels above or equal " << levelEnergy << " keV" << std::endl;
   }
}

std::map<double, double> TLevelScheme::FeedingGammas(double levelEnergy, double factor)
{
   /// Returns the energies and relative strengths of all gamma rays feeding the level at <energy>.
   /// Gamma rays may appear multiple times with different strengths, if they are part of multiple cascades.
   /// This search is a bit complicated, we need to find all gammas that feed this level.
   /// Since this requires looping over all gamma rays, and we do this recursize, we will populate a single map with all gammas above this one.
   /// That map has to be deleted before calling this function by calling TLevelScheme::ResetGammaMap!
   std::map<double, double> result;
   BuildGammaMap(levelEnergy);

   auto range = fGammaMap.equal_range(levelEnergy);
   if(fDebug) {
      std::cout << "Got " << std::distance(range.first, range.second) << " gamma rays feeding level at " << levelEnergy << " kev (factor " << factor << ")" << std::endl;
   }
   for(auto& it = range.first; it != range.second; ++it) {
      if(fDebug) {
         std::cout << "Adding gamma " << it->second << ": ";
         it->second->Print();
      }
      result[it->second->Energy()] += factor * it->second->BranchingRatioPercent();
      auto tmp = FeedingGammas(it->second->InitialEnergy(), factor * it->second->BranchingRatioPercent());
      for(auto& [energy, tmpFactor] : tmp) {
         result[energy] += tmpFactor;
      }
   }

   if(fDebug) {
      std::cout << "returning list of " << result.size() << " gammas feeding level at " << levelEnergy << " keV" << std::endl;
   }
   return result;
}

std::map<double, double> TLevelScheme::DrainingGammas(double levelEnergy, double factor)
{
   /// Returns the energies and relative strengths of all gamma rays draining the level at <energy>.
   /// Gamma rays may appear multiple times with different strengths, if they are part of multiple cascades.

   // Since the branching ratio of a gamma ray cascades down to all gamma rays below it, we want to follow each
   // cascade until we reach the ground state. So we get the level whos energy was provided and loop over it's gamma rays
   // and for each gamma ray we get the next level and it's gamma rays
   auto*                    level = GetLevel(levelEnergy);
   std::map<double, double> result;
   if(level == nullptr) {
      std::cerr << "Failed to find level at " << levelEnergy << " keV, returning empty list of gammas draining that level" << std::endl;
      Print();
      return result;
   }

   if(fDebug) {
      std::cout << level << ": looping over " << level->NofDrainingGammas() << " gammas for level at " << levelEnergy << " keV (factor " << factor << ")" << std::endl;
   }

   // loop over all gamma rays
   for(auto& [finalEnergy, gamma] : *level) {
      if(fDebug) {
         std::cout << "Adding gamma: ";
         gamma.Print();
      }
      // add this gamma to the result
      auto branchingRatio = gamma.BranchingRatioPercent();
      result[gamma.Energy()] += factor * branchingRatio;
      // if we aren't at the ground state, add all gammas from the new level as well
      if(gamma.FinalEnergy() > 0.) {
         auto tmp = DrainingGammas(finalEnergy, factor * branchingRatio);
         for(auto& [energy, tmpFactor] : tmp) {
            result[energy] += tmpFactor;
         }
      }
   }

   if(fDebug) {
      std::cout << "returning list of " << result.size() << " gammas draining level at " << levelEnergy << " keV" << std::endl;
   }
   return result;
}

std::vector<std::tuple<double, std::vector<double>>> TLevelScheme::ParallelGammas(double initialEnergy, double finalEnergy, double factor)
{
   /// This (recursive) function returns the combined probability and energies of gamma rays that are connecting the levels at initial and final energy.

   auto*                                                level = GetLevel(initialEnergy);
   std::vector<std::tuple<double, std::vector<double>>> result;
   if(level == nullptr) {
      std::cerr << "Failed to find level at " << initialEnergy << " keV, returning empty list of gammas" << std::endl;
      Print();
      return result;
   }

   if(fDebug) {
      std::cout << level << ": looping over " << level->NofDrainingGammas() << " gammas for level at " << initialEnergy << " keV (factor " << factor << ")" << std::endl;
   }

   result.push_back(std::make_tuple(1., std::vector<double>()));
   // loop over all gamma rays
   for(auto& [levelEnergy, gamma] : *level) {
      // if the level we populate with this gamma is below the final energy, we skip it
      if(levelEnergy < finalEnergy) {
         if(fDebug) {
            std::cout << "skipping gamma ";
            gamma.Print();
         }
         continue;
      }
      // add this gamma ray to the path for now
      std::get<0>(result.back()) *= gamma.BranchingRatioPercent();
      std::get<1>(result.back()).insert(std::get<1>(result.back()).end(), gamma.Energy());
      if(fDebug) {
         std::cout << "Added gamma ";
         gamma.Print();
         std::cout << "New result:";
         for(auto& entry : result) {
            std::cout << " " << std::get<0>(entry) << "-";
            for(auto& e : std::get<1>(entry)) {
               std::cout << e << ",";
            }
         }
         std::cout << std::endl;
      }
      // if the level we populate is above the final energy, we see if we can find a gamma from that energy to the final energy
      if(levelEnergy > finalEnergy) {
         auto tmp = ParallelGammas(levelEnergy, finalEnergy, factor * gamma.BranchingRatioPercent());
         // if we have multiple combinations, we don't just want to add all of them to one single entry, but create separate entries for each
         // so we want to add n-1 copies of the last entry, this does not work if n is zero, so we check for that first
         if(!tmp.empty()) {
            auto it = result.insert(result.end(), tmp.size() - 1, result.back());
            // insert returns position of first inserted element or pos (=.end() in this case) if count is zero, so we need to decrement by one
            --it;
            if(fDebug) {
               std::cout << "Got " << tmp.size() << " paths, so added " << tmp.size() - 1 << " copies of last element:";
               for(auto& entry : result) {
                  std::cout << " " << std::get<0>(entry) << "-";
                  for(auto& e : std::get<1>(entry)) {
                     std::cout << e << ",";
                  }
               }
               std::cout << std::endl;
            }
            for(auto& combo : tmp) {
               // maybe we can reject any that don't reach the final energy already here???
               std::get<0>(*it) *= std::get<0>(combo);
               std::get<1>(*it).insert(std::get<1>(*it).end(), std::get<1>(combo).begin(), std::get<1>(combo).end());
               ++it;
            }
            if(fDebug) {
               std::cout << "Got " << tmp.size() << " more gammas, new result:";
               for(auto& entry : result) {
                  std::cout << " " << std::get<0>(entry) << "-";
                  double sum = 0.;
                  for(auto& e : std::get<1>(entry)) {
                     std::cout << e << ",";
                     sum += e;
                  }
                  std::cout << "total " << sum;
               }
               std::cout << std::endl;
            }
         } else {
            // we failed to find a path from the level this gamma populates to the final level, so we need to remove it from the path
            // that is equivalent to re-setting the factor to one and clearing the vector of gammas
            // std::get<0>(result.back())  = 1.;
            // std::get<1>(result.back()).clear();
            result.pop_back();
            if(fDebug) {
               std::cout << "Failed to find path from " << levelEnergy << " to " << finalEnergy << ", removed last gamma ray";
               for(auto& entry : result) {
                  std::cout << " " << std::get<0>(entry) << "-";
                  for(auto& e : std::get<1>(entry)) {
                     std::cout << e << ",";
                  }
               }
               std::cout << std::endl;
            }
         }
      }
      // if we found a path to the final energy (either via more gamma rays or by this one), we add this one and
      // since we are done with this path, we can add a new one (otherwise we will add to this path)
      result.push_back(std::make_tuple(1., std::vector<double>()));
      if(fDebug) {
         std::cout << "Reached final level, added new (empty) entry to result" << std::endl;
      }
   }   // loop over all gammas

   // delete last entry if it's empty
   if(std::get<1>(result.back()).empty()) {
      result.pop_back();
   }

   return result;
}

void TLevelScheme::MoveToBand(const char* bandName, TLevel* level)
{
   /// Moves provided level to (new) band with provided name.
   // Try and find an existing band with this name to add the level to.
   if(fDebug) {
      std::cout << "TLevelScheme: Trying to move level " << level << " at " << level->Energy() << " keV to band \"" << bandName << "\"" << std::endl;
   }
   size_t i;
   for(i = 0; i < fBands.size(); ++i) {
      if(strcmp(bandName, fBands[i].GetLabel()) == 0) {
         if(fDebug) {
            std::cout << "Found band \"" << fBands[i].GetLabel() << "\" (" << &fBands[i] << ") with " << fBands[i].NofLevels() << " levels" << std::endl;
         }
         fBands[i].AddLevel(level);
         break;
      }
		if(fDebug) {
         std::cout << "Band \"" << fBands[i].GetLabel() << "\" does not match " << bandName << std::endl;
      }
   }
   // if we didn't find a band to add this level to, create a new one and add the level
   if(i == fBands.size()) {
      if(fDebug) {
         std::cout << "Haven't found band \"" << bandName << "\" among existing bands, creating new band" << std::endl;
      }
      fBands.push_back(TBand(this, bandName));
      auto& newBand = fBands.back();
      newBand.Debug(fDebug);
      if(fDebug) {
         std::cout << "Created band \"" << newBand.GetLabel() << "\" (" << &newBand << ") with " << newBand.NofLevels() << " level" << std::endl;
         newBand.Print();
         std::cout << std::endl;
      }
      newBand.AddLevel(level);
   }
   // we added the level, so now we can loop through all bands whose name doesn't match the provided name and delete the level from them.
   for(auto& band : fBands) {
      if(fDebug) {
         std::cout << "Checking band \"" << band.GetLabel() << "\": ";
      }
      if(strcmp(bandName, band.GetLabel()) == 0) {
         if(fDebug) {
            std::cout << "this is the new band, not deleting it here!" << std::endl;
         }
         continue;
      }
      if(fDebug) {
         std::cout << "this is not the new band, removing it here!" << std::endl;
      }
      band.RemoveLevel(level);
   }

   Refresh();
}

void TLevelScheme::Refresh()
{
   // only re-draw if we find a matching canvas
   auto* canvas = static_cast<GCanvas*>(gROOT->GetListOfCanvases()->FindObject("LevelScheme"));
   if(canvas != nullptr) {
      Draw();
   }
}

void TLevelScheme::UnZoom() const
{
   auto* canvas = static_cast<GCanvas*>(gROOT->GetListOfCanvases()->FindObject("LevelScheme"));
   if(canvas != nullptr) {
      canvas->Range(fX1, fY1, fX2, fY2);
      canvas->Modified();
      canvas->Update();
   }
}

void TLevelScheme::Draw(Option_t*)
{
   if(fDebug) std::cout << __PRETTY_FUNCTION__ << std::endl;
   auto* canvas = static_cast<GCanvas*>(gROOT->GetListOfCanvases()->FindObject("LevelScheme"));
   if(canvas == nullptr) {
      canvas = new GCanvas("LevelScheme", "Level Scheme");
   } else {
      canvas->Clear();
   }

   // find the lowest and highest level by going through each band and getting the lowest and highest levels
   // also get the width by adding up all level widths and get the strongest and weakest gamma ray of each band
   auto                                   minMaxLevel = fBands[0].GetMinMaxLevelEnergy();
   double                                 width       = 0.;
   std::vector<std::pair<double, double>> minMaxGamma;
   for(auto& band : fBands) {
      auto minMax = band.GetMinMaxLevelEnergy();
      if(minMax.first < minMaxLevel.first) minMaxLevel.first = minMax.first;
      if(minMax.second > minMaxLevel.second) minMaxLevel.second = minMax.second;
      if(fDebug) std::cout << "Incrementing width from " << width;
      width += band.Width(fGammaDistance);
      if(fDebug) std::cout << " to " << width << " using " << band.Width(fGammaDistance) << std::endl;
      minMaxGamma.push_back(band.GetMinMaxGamma());
   }
   width += (fBands.size() - 1) * fBandGap;

   // ys calculated from the lowest and highest level plus the bottom and top margins
   fY1           = minMaxLevel.first;
   fY2           = minMaxLevel.second;
   double height = fY2 - fY1;
   // if the margins haven't been set, we add 10% of the height
   if(fBottomMargin < 0) fY1 -= height / 10.;
   else fY1 -= fBottomMargin;
   if(fTopMargin < 0) fY2 += height / 10.;
   else fY2 += fTopMargin;

   // xs are calculated from the width of each band, plus the left and right margin, plus n-1 times the margin between bands
   fX1 = 0.;
   fX2 = width;
   // if the margins haven't been set, we add the band gap
   if(fLeftMargin < 0) fX1 -= fBandGap;
   else fX1 -= fLeftMargin;
   if(fRightMargin < 0) fX2 += fBandGap;
   else fX2 += fRightMargin;

   if(fDebug) std::cout << "got x1 - x2 " << fX1 << " - " << fX2 << ", and y1 - y2 " << fY1 << " - " << fY2 << std::endl;
   canvas->Range(fX1, fY1, fX2, fY2);
   canvas->cd();

   SetX1(fX2 - fBandGap);
   SetX2(fX2);
   if(fTopMargin < 0) SetY1(fX2 - height / 12.);
   else SetY1(fX2 - fTopMargin * 0.75);
   SetY2(fX2);
   SetTextSize(0);     // default size (?)
   SetTextAlign(22);   // centered in x and y
   TPaveLabel::Draw();

   if(fGammaWidth == EGammaWidth::kGlobal) {
      // if we want the gamma width to be on a global scale, i.e. across all bands, we need to find the minimum and maximum values
      // we store those in  the first entry
      for(auto& [min, max] : minMaxGamma) {
         if(min < minMaxGamma[0].first) minMaxGamma[0].first = min;
         if(max > minMaxGamma[0].second) minMaxGamma[0].second = max;
         std::cout << min << " - " << max << ": " << minMaxGamma[0].first << " - " << minMaxGamma[0].second << std::endl;
      }
   }

   // loop over all levels in all bands and draw them
   // for each band we need to determine left and right position, we start at 0
   double left = 0.;
   for(size_t b = 0; b < fBands.size(); ++b) {
      double right = left + fBands[b].Width(fGammaDistance);
      if(fDebug) std::cout << b << ": Using width " << fBands[b].Width(fGammaDistance) << " and left " << left << " we get right " << right << std::endl;
      fBands[b].SetX1(left);
      fBands[b].SetY1(-1.5 * height / 20.);
      fBands[b].SetX2(right);
      fBands[b].SetY2(-height / 40.);
      fBands[b].SetFillColor(10);
      fBands[b].Draw();
      // get the scaling for the gamma's so that the widths are between fMinWidth and fMaxWidth
      // for global scaling we use the first minMax where we stored the global mininum and maximum
      auto& [min, max] = minMaxGamma[0];
      if(fGammaWidth == EGammaWidth::kBand) {
         // if we scale per band, we set the minimum and maximum for each band
         min = minMaxGamma[b].first;
         max = minMaxGamma[b].second;
      }
      double scalingGain   = (fMaxWidth - fMinWidth) / (max - min);
      double scalingOffset = fMinWidth - scalingGain * min;
      if(fGammaWidth == EGammaWidth::kNoWidth) {
         // if we don't use the width at all, we set the scaling by hand so the width is always the minimum width
         scalingGain   = 0.;
         scalingOffset = fMinWidth;
      }

      double labelSize = TLevel::TextSize() * std::min(fX2 - fX1, fY2 - fY1);
      labelSize *= 1.5;   // we want a bit of a gap between labels ...

      // loop to calculate "center" for closely grouped levels
      // to calculate the necessary offset we need to know for each group the center and how many levels are there between this one and the center
      // first let's find the groups of levels, need to first check all groups the current level *could* belong to,
      // as #1 and #2 might not overlap (so be in two different groups) but then #3 overlaps with both #1 and #2 (and would end in the group of #1 if we don't check this)
      // so we create a list of groups this level can belong to, then if it's 0 we create a new group, if it's 1 we add it to it, if it's more than 1 we combine the groups
      std::vector<std::vector<std::pair<double, int>>> groups;
      int                                              index = 0;
      for(auto& [energy, level] : fBands[b]) {
         std::vector<size_t> potentialGroups;
         for(size_t i = 0; i < groups.size(); ++i) {
            for(size_t j = 0; j < groups[i].size(); ++j) {
               // don't compare to the levels of the group as is, but assume that they are going to be shifted by average - energy + label size * (index - (# in group - 1)/2)
               // so the energy we want to compare to is average + label size * (index - (# in group - 1)/2)
               // for that we need the average of the group at this point
               double average = std::accumulate(groups[i].begin(), groups[i].end(), 0., [](double r, std::pair<double, int> p) { r += p.first; return r; }) / groups[i].size();
               // we don't need abs here as we know the current energy and index are larger than the previous ones
               if(energy - (average + labelSize * (j - (groups[i].size() - 1) / 2.)) < labelSize * (index - groups[i][j].second)) {
                  if(fDebug) {
                     std::cout << energy - groups[i][j].first << " (" << energy << " - " << groups[i][j].first << ") < " << labelSize * (index - groups[i][j].second) << " (" << labelSize << "*(" << index << "-" << groups[i][j].second << ")), adding to group " << i << std::endl;
                  }
                  potentialGroups.push_back(i);
                  // break here since we already know this group is a match
                  break;
               }
					if(fDebug) {
                  std::cout << energy - groups[i][j].first << " (" << energy << " - " << groups[i][j].first << ") >= " << labelSize * (index - groups[i][j].second) << " (" << labelSize << "*(" << index << "-" << groups[i][j].second << ")), NOT adding to group " << i << std::endl;
               }
            }
            // we don't want to break again here since we want to check all groups
         }
         if(potentialGroups.empty()) {
            groups.push_back(std::vector(1, std::make_pair(energy, index)));
         } else if(potentialGroups.size() == 1) {
            groups[potentialGroups[0]].push_back(std::make_pair(energy, index));
         } else {
            if(fDebug) {
               std::cout << "combining groups";
               for(size_t i = 0; i < potentialGroups.size(); ++i) {
                  std::cout << " " << potentialGroups[i];
               }
               std::cout << std::endl;
            }
            // multiple potential groups, so we add all of them together, add this level, sort the resulting group and then remove the other groups
            for(size_t i = 1; i < potentialGroups.size(); ++i) {
               groups[potentialGroups[0]].insert(groups[potentialGroups[0]].end(), groups[potentialGroups[i]].begin(), groups[potentialGroups[i]].end());
               groups.erase(groups.begin() + potentialGroups[i]);
            }
            groups[potentialGroups[0]].push_back(std::make_pair(energy, index));
            std::sort(groups[potentialGroups[0]].begin(), groups[potentialGroups[0]].end());
         }
         ++index;
      }
      if(fDebug) {
         std::cout << "got " << groups.size() << " groups:" << std::endl;
         for(auto& group : groups) {
            for(auto& level : group) {
               std::cout << level.second << " " << level.first << ", ";
            }
            std::cout << std::endl;
         }
      }
      // we now have a vector of groups of levels, find the center of each group and how many levels are between it and the current level
      std::vector<std::pair<double, double>> centers;
      for(auto& group : groups) {
         double average = std::accumulate(group.begin(), group.end(), 0., [](double r, std::pair<double, double> p) { r += p.first; return r; }) / group.size();
         for(size_t i = 0; i < group.size(); ++i) {
            centers.push_back(std::make_pair(average, i - (group.size() - 1) / 2.));
         }
      }
      if(fDebug) {
         std::cout << "centers: ";
         for(auto& [center, diff] : centers) {
            std::cout << center << " " << diff << ", ";
         }
         std::cout << std::endl;
      }

      size_t g = 1;   // counter for number of gammas in this band
      // loop over all levels of this band
      index = 0;
      for(auto& [energy, level] : fBands[b]) {
         if(fDebug) {
            std::cout << std::endl;
            std::cout << "starting calculations for drawing level at " << energy << std::endl;
         }
         // check level distance compare to size of labels
         double move = centers[index].first - energy + labelSize * centers[index].second;
         if(fDebug) {
            std::cout << "calculations for drawing level at " << energy << ": move " << move << " (" << centers[index].first << " - " << energy << " + " << labelSize << "*" << centers[index].second << ") => " << energy + move << ", " << left << "-" << right << std::endl;
         }
         level.Offset(move);
         level.Draw(left, right);
         // double labelWidth = level.DrawLabel(right);
         // double energyWidth = level.DrawEnergy(left);
         level.DrawLabel(right);
         level.DrawEnergy(left);
         // TODO: check these widths to see if we need to adjust the margins.
         // Should also adjust gaps between bands to be equal to their sum, but how?
         // If we call Draw recursively if we changed anything it will never stop (as that re-adjusts the text sizes).
         // Maybe only set a flag to re-draw when the change is larger than a minimum value?
         if(fDebug) level.Print();

         // loop over all gammas from this level and draw them
         for(auto& [finalEnergy, gamma] : level) {
            // find the final level, get it's energy and x-position
            size_t b2;
            bool   found = false;
            for(b2 = 0; b2 < fBands.size(); ++b2) {
               for(auto& [energy2, level2] : fBands[b2]) {
                  if(finalEnergy == level2.Energy()) {
                     if(fDebug) {
                        std::cout << "band " << b2 << ": " << finalEnergy << " == " << level2.Energy() << std::endl;
                     }
                     found = true;
                     break;
                  }
						if(fDebug) {
                     std::cout << "band " << b2 << ": " << finalEnergy << " != " << level2.Energy() << std::endl;
                  }
               }
               if(found) { break; }
            }

            if(!found) {
               std::cout << "Warning, failed to find final level at " << finalEnergy << " keV for gamma-ray!" << std::endl;
            }

            // set the scaling for the width
            gamma.Scaling(scalingOffset, scalingGain);

            // y-position is easy, simply grab the energy of the initial and final level
            double gY1 = level.Energy();
            double gY2 = finalEnergy;

            // x-position is more complicated, depends whether it's intra- or inter-band
            // and what other gamma rays are there at this energy range
            double gX1;
            double gX2;
            if(fRadwareStyle) {
               if(b == b2) {   // intra-band: for now just increment the position, later maybe search all previously added intra-band transitions for this band
                  if(fDebug) std::cout << b << " == " << b2 << ": intra band " << g << " at position " << g * fGammaDistance << std::endl;
                  gX1 = left + g * fGammaDistance;
                  gX2 = left + g * fGammaDistance;
                  ++g;
               } else if(b < b2) {   // inter-band to a band on the right
                  if(fDebug) std::cout << b << " < " << b2 << ": inter band to right " << g << " at position " << g * fGammaDistance << std::endl;
                  gX1          = left + g * fGammaDistance;
                  gX2          = left + g * fGammaDistance + (gY1 - gY2) / 10.;
                  double shift = right + fBandGap;
                  // sum the width of bands b+1 to b2-1 and the band gaps between them
                  if(b + 1 < b2) shift = std::accumulate(fBands.begin() + b + 1, fBands.begin() + b2, right + fBandGap, [&](double r, TBand& el) { r += el.Width(fGammaDistance) + fBandGap; return r; });
                  DrawAuxillaryLevel(gY2, right + g * fGammaDistance, shift - fBandGap / 2.);   // for now always a gap of fBandGap/2. for the label
                  ++g;
               } else {   // inter-band to a band on the left
                  if(fDebug) std::cout << b << " > " << b2 << ": inter band to left " << g << " at position " << g * fGammaDistance << std::endl;
                  gX1          = left + g * fGammaDistance;
                  gX2          = left + g * fGammaDistance - (gY1 - gY2) / 10.;
                  double shift = left - fBandGap;
                  // sum the width of bands b2+1 to b-1 and the band gaps between them
                  if(b2 + 1 < b) shift = std::accumulate(fBands.begin() + b2 + 1, fBands.begin() + b, left - fBandGap, [&](double r, TBand& el) { r -= el.Width(fGammaDistance) + fBandGap; return r; });
                  DrawAuxillaryLevel(gY2, left + g * fGammaDistance, shift + fBandGap / 2.);
                  ++g;
               }
            } else {
               if(b == b2) {   // intra-band: for now just increment the position, later maybe search all previously added intra-band transitions for this band
                  if(fDebug) std::cout << b << " == " << b2 << ": intra band " << g << " at position " << g * fGammaDistance << std::endl;
                  gX1 = left + g * fGammaDistance;
                  gX2 = left + g * fGammaDistance;
                  ++g;
               } else if(b + 1 == b2) {   // inter-band from this band to the next band on the right
                  if(fDebug) std::cout << b << "+1 == " << b2 << ": inter band " << g << " to right " << right << "-" << right + fBandGap << std::endl;
                  gX1 = right - fGammaDistance / 2.;
                  gX2 = right + fBandGap + fGammaDistance / 2.;
               } else if(b == b2 + 1) {   // inter-band from this band to the next band on the left
                  if(fDebug) std::cout << b << " == " << b2 << "+1: inter band " << g << " to left " << left << "-" << left - fBandGap << std::endl;
                  gX1 = left + fGammaDistance / 2.;
                  gX2 = left - fBandGap - fGammaDistance / 2.;
               } else if(b < b2) {   // inter-band to a band on the right that is not a direct neighbour
                  if(fDebug) std::cout << b << " < " << b2 << ": inter band far " << g << " right " << right << ", band gap " << fBandGap << std::endl;
                  gX1          = right;
                  gX2          = right + fBandGap / 8.;
                  double shift = right + fBandGap;
                  // sum the width of bands b+1 to b2-1 and the band gaps between them
                  if(b + 1 < b2) shift = std::accumulate(fBands.begin() + b + 1, fBands.begin() + b2, right + fBandGap, [&](double r, TBand& el) { r += el.Width(fGammaDistance) + fBandGap; return r; });
                  DrawAuxillaryLevel(gY2, right, shift - fBandGap / 2.);
               } else {   // inter-band to a band on the left that is not a direct neighbour
                  if(fDebug) std::cout << b << " > " << b2 << ": inter band far " << g << " left " << left << ", band gap " << fBandGap << std::endl;
                  gX1          = left;
                  gX2          = left - fBandGap / 8.;
                  double shift = left - fBandGap;
                  // sum the width of bands b2+1 to b-1 and the band gaps between them
                  if(b2 + 1 < b) shift = std::accumulate(fBands.begin() + b2 + 1, fBands.begin() + b, left - fBandGap, [&](double r, TBand& el) { r -= el.Width(fGammaDistance) + fBandGap; return r; });
                  DrawAuxillaryLevel(gY2, left, shift + fBandGap / 2.);
               }
            }

            gamma.Draw(gX1, gY1, gX2, gY2);
         }   // end of gamma loop
         if(level.begin() != level.end()) {
            if(fDebug) {
               std::cout << "Level has gamma-rays, incrementing g from " << g << std::endl;
               level.Print();
            }
            ++g;   // add an extra space at the end of each level that has gamma rays
         }
         ++index;
      }
      // update left before we go to the next band
      left = right + fBandGap;
   }

   if(fDebug) {
      std::cout << "Canvas " << canvas << " " << canvas->GetName() << " has " << canvas->GetListOfPrimitives()->GetSize() << " primitives:" << std::endl;
      canvas->GetListOfPrimitives()->Print();
      Print();
   }
}

void TLevelScheme::DrawAuxillaryLevel(const double& energy, const double& left, const double& right)
{
   if(fDebug) {
      std::cout << "Drawing auxillary level at " << energy << " keV from " << left << " to " << right << std::endl;
   }
   // maybe change this to be a short solid line connected by a dotted line to the original level?
   if(fAuxillaryLevels.count(energy) > 0) {
      // we have multiple auxillary levels => try and find one with matching left and right
      auto range = fAuxillaryLevels.equal_range(energy);
      for(auto it = range.first; it != range.second; ++it) {
         if(it->second.GetX1() == left && it->second.GetX2() == right) return;
      }
   }
   // haven't found a matching level, so we add a new one
   auto it = fAuxillaryLevels.emplace(std::piecewise_construct, std::forward_as_tuple(energy), std::forward_as_tuple(left, energy, right, energy));
   it->second.SetLineStyle(2);
   it->second.Draw();
}

void TLevelScheme::Print(Option_t*) const
{
   std::cout << this << ": got " << fBands.size() << " bands: ";
   for(size_t b = 0; b < fBands.size(); ++b) {
      std::cout << " " << b << "-" << &fBands[b] << "-" << fBands[b].GetLabel();
   }
   std::cout << ", debugging" << (fDebug ? "" : " not") << " enabled" << std::endl;
   if(fDebug) {
      for(const auto& band : fBands) {
         band.Print();
      }
   }
}

void TLevelScheme::ParseENSDF(const std::string& filename)
{
   /// This function parses the given file assuming it's ENSDF formatted.
   // TODO: check if file exists
   std::ifstream input(filename);
   if(!input.is_open()) {
      std::cerr << DRED << "Failed to open \"" << filename << "\"" << RESET_COLOR << std::endl;
      return;
   }

   std::string       line;
   std::stringstream str;
   TLevel*           currentLevel = nullptr;

   // general identifier format of line (first 8 characters)
   // 1-3 mass
   // 4-5 element symbol (except for reference record where it's blank)
   // 6 blank/'1' for primary, any ascii character (but not '1) for continuation
   // 7 blank or c, C, d, D, t, or T for comment
   // 8 R - reference, X - cross reference, H - history, Q - q-value, P - parent, N - normalization, L - level, B - beta, E - EC, A - alpha, D - delayed particle, G - gamma

   // first line should be identification record (1-5 nuclide, 10-39 data set ident., 40-65 refs., 66-74 publ. inf., 75-80 date
   std::getline(input, line);
   SetLabel(line.substr(0, 5).c_str());
   // trimWS(fNuclide); // trim whitespace
   std::cout << "Reading level scheme for " << GetLabel() << std::endl;
   // check that data set ident is "ADOPTED LEVELS, GAMMAS" or at least "ADOPTED LEVELS"
   if(line.substr(9, 14) == "ADOPTED LEVELS") {
      std::cout << "Data set is not \"ADOPTED LEVELS\" or \"ADOPTED LEVELS, GAMMAS\", but \"" << line.substr(9, 14) << "\", don't know how to read that (" << line << ")" << std::endl;
      return;
   }
   // if the identification record is continued, 6 will not be blank
   do {
      std::getline(input, line);
   } while(line[5] != ' ');

   // now we have the first line that is not part of the identification record, so we check it's format and continue reading lines as long as we can
   do {
      switch(line[7]) {
      case 'H':   // history record (1-5 nuclide, 6 blank or anything but '1' for cont., 7 blank, 8 'H', 9 blank, 10-80 history)
         // ignored
         break;
      case 'Q':   // q-value record (1-5 nuclide, 6-9 "  Q ", 10-19 beta- q-value, 20-21 uncert., 22-29 S_n, 30-31 uncert., 32-39 S_p, 40-41 uncert., 42-49 alpha q-value, 50-51 uncert., 56-80 refs.)
         if(line.substr(5, 4) == "  Q ") {
            // we only read the beta- q-value and the neutron separation energy
            str.clear();
            str.str(line.substr(9, 10));
            str >> fQValue;
            str.clear();
            str.str(line.substr(19, 2));
            str >> fQValueUncertainty;
            if(fDebug) std::cout << "reading S_n old \"" << str.str() << "\" @ " << str.tellg() << (str.fail() ? " failed" : " good");
            str.clear();
            str.str(line.substr(21, 8));   // TODO: doesn't work?
            if(fDebug) std::cout << ", new \"" << str.str() << "\" @ " << str.tellg() << (str.fail() ? " failed" : " good");
            str >> fNeutronSeparation;
            if(fDebug) std::cout << " => S_n " << fNeutronSeparation << std::endl;
            str.clear();
            str.str(line.substr(29, 2));
            str >> fNeutronSeparationUncertainty;
            if(fDebug) {
               std::cout << "Found q-value line: " << fQValue << " +- " << fQValueUncertainty << ", S_n " << fNeutronSeparation << " +- " << fNeutronSeparationUncertainty << std::endl;
               std::cout << "\"" << line.substr(9, 10) << "\", \"" << line.substr(19, 2) << "\", \"" << line.substr(21, 8) << "\", \"" << line.substr(29, 2) << "\"" << std::endl;
            }
         } else if(fDebug) {   // if 7 is not blank this is a comment for the q-value and we ignore it
            std::cout << "Ignoring q-value comment \"" << line << "\"" << std::endl;
         }
         break;
      case 'X':   // cross-reference record (1-5 nuclide, 6-7 blank, 8 'X', 9 identifier, 10-39 DSID used, 40-80 blank)
         // ignored
         if(fDebug) std::cout << "Ignoring cross-reference \"" << line << "\"" << std::endl;
         break;
      case 'P':   // parent record (1-5 nuclide, 6-7 blank, 8 'P', 9 blank or integer, ...), seems to be for decay data sets only?
         if(fDebug) std::cout << "Ignoring parent \"" << line << "\"" << std::endl;
         break;
      case 'N':   // this can be one of two records, depending on whether 7 is blank or 'P'
         // normalization record (1-5 nuclide, 6-7 blank, 8 'N', 9 blank or integer, ...), seems to be for decay data sets only?
         // production normalization record (1-5 nuclide, 6-9 " PN ", ...) ignored
         if(fDebug) std::cout << "Ignoring normalization \"" << line << "\"" << std::endl;
         break;
      case 'L':   // level record (1-5 nuclide, 6 blank or not '1' for cont., 7-9 " L ", 10-19 energy keV, 20-21 uncert., 22-39 spin and parity, 40-49 half life with units, 50-55 uncert.
                  // 56-74 angular momentum transfer in reaction, 75-76 uncert., 77 comment flag, 78-79 metastable as "M ", or "M1", "M2", etc., 80 '?' denotes uncertain level and 'S' denotes neutron, proton, alpha sep. en.)
         if(line[5] == ' ' && line[6] == ' ') {
            // read energy and energy uncertainty
            double energy            = 0.;
            double energyUncertainty = 0.;
            if(fDebug) std::cout << "reading level energy old \"" << str.str() << "\"";
            str.clear();
            str.str(line.substr(9, 10));
            if(fDebug) std::cout << ", new \"" << str.str() << "\"";
            str >> energy;
            if(fDebug) std::cout << " => energy " << energy << std::endl;
            str.clear();
            str.str(line.substr(19, 2));
            str >> energyUncertainty;
            // read string for spin and parity, half-life, and half-life uncertainty seperately
            // reading a string from stringstream means we discard all leading whitespace and stop the moment we encounter more whitespace
            // i.e. we read only one word
            std::string spinParity = line.substr(21, 18);
            trimWS(spinParity);
            std::string halfLife = line.substr(39, 10);   // include the unit!
            trimWS(halfLife);
            std::string halfLifeUncertainty = line.substr(49, 6);
            trimWS(halfLifeUncertainty);
            // combine all non-zero strings to get the label
            std::string label;
            if(!spinParity.empty()) {
               label += spinParity;
            }
            if(!halfLife.empty()) {
               label += halfLife;
               label += halfLifeUncertainty;
            }
            if(fDebug) {
               std::cout << "Adding new level " << energy << " +- " << energyUncertainty << ", " << spinParity << ", half-life " << halfLife << " +- " << halfLifeUncertainty << std::endl;
               std::cout << "\"" << line.substr(9, 10) << "\", \"" << line.substr(19, 2) << "\", \"" << line.substr(21, 18) << "\", \"" << line.substr(39, 10) << "\", \"" << line.substr(49, 6) << "\"" << std::endl;
            }
            // create new level and set it's energy uncertainty
            currentLevel = AddLevel(energy, GetLabel(), label);
            currentLevel->EnergyUncertainty(energyUncertainty);
         } else if(fDebug) std::cout << "Ignoring level \"" << line << "\"" << std::endl;
         break;
      case 'G':   // gamma record (1-5 nuclide, 6 blank or not '1' for cont., 7-9 " G ", 10-19 energy in keV, 20-21 uncert. 22-29 relative photon intens., 30-31 uncert., 32-41 multipol.
         // 42-49 mixing ratio, 50-55 uncert., 56-62 conversion coeff., 63-64 uncert., 65-74 relative total intens., 75-76 uncert., 77 comment flag, 78 'C' confirmed coincidence, '?' questionable coincidence,
         // 79 blank, 80 '?' questionable placement, 'S' expected but unobserved)
         if(line[5] == ' ' && line[6] == ' ') {
            if(currentLevel == nullptr) {
               std::cout << "Found unassigned gamma, ignoring it (" << line << ")!" << std::endl;
               break;
            }
            // read energy and uncertainty
            double energy            = 0.;
            double energyUncertainty = 0.;
            str.clear();
            str.str(line.substr(9, 10));
            str >> energy;
            str.clear();
            str.str(line.substr(19, 2));
            str >> energyUncertainty;
            // read relative photon intensity and uncertainty
            double photonIntensity            = 0.;
            double photonIntensityUncertainty = 0.;
            str.clear();
            str.str(line.substr(21, 8));
            str >> photonIntensity;
            str.clear();
            str.str(line.substr(29, 2));
            str >> photonIntensityUncertainty;
            // read multipolarity
            std::string multipolarity = line.substr(31, 10);
            trimWS(multipolarity);
            // read mixing ratio and uncertainty
            double mixingRatio            = 0.;
            double mixingRatioUncertainty = 0.;
            str.clear();
            str.str(line.substr(41, 8));
            str >> mixingRatio;
            str.clear();
            str.str(line.substr(49, 6));
            str >> mixingRatioUncertainty;
            // read conversion coeff. and uncertainty
            double conversionCoeff            = 0.;
            double conversionCoeffUncertainty = 0.;
            str.clear();
            str.str(line.substr(55, 7));
            str >> conversionCoeff;
            str.clear();
            str.str(line.substr(62, 2));
            str >> conversionCoeffUncertainty;
            // read relative total intensity and uncertainty
            double totalIntensity            = 0.;
            double totalIntensityUncertainty = 0.;
            str.clear();
            str.str(line.substr(64, 10));
            str >> totalIntensity;
            str.clear();
            str.str(line.substr(74, 2));
            str >> totalIntensityUncertainty;
            // we already checked that the current level is not a null pointer so we can add the gamma here
            if(fDebug) {
               std::cout << "Adding gamma with energy " << energy << " +- " << energyUncertainty << ", " << photonIntensity << " +- " << photonIntensityUncertainty << ", " << multipolarity << ", " << mixingRatio << " +- " << mixingRatioUncertainty << ", " << conversionCoeff << " +- " << conversionCoeffUncertainty << ", " << totalIntensity << " +- " << totalIntensityUncertainty << ", final level energy " << currentLevel->Energy() - energy << std::endl;
               std::cout << "\"" << line.substr(9, 10) << "\", \"" << line.substr(19, 2) << "\", \"" << line.substr(21, 8) << "\", \"" << line.substr(29, 2) << "\", \"" << line.substr(31, 10) << "\", \"" << line.substr(41, 8) << "\", \"" << line.substr(49, 6) << "\", \"" << line.substr(55, 7) << "\", \"" << line.substr(62, 2) << "\", \"" << line.substr(64, 10) << "\", \"" << line.substr(74, 2) << "\"" << std::endl;
            }
            auto* gamma = currentLevel->AddGamma(currentLevel->Energy() - energy, energyUncertainty, multipolarity.c_str(), photonIntensity, totalIntensity);
            if(gamma != nullptr) {
               gamma->BranchingRatioUncertainty(photonIntensityUncertainty);
               gamma->TransitionStrengthUncertainty(totalIntensityUncertainty);
               // currently ignoring mixing ratio and conversion coefficents
            }
         } else if(fDebug) std::cout << "Ignoring gamma \"" << line << "\"" << std::endl;
         break;
      case 'B':   // beta record (1-5 nuclide, 6 blank or not '1' for cont., 7-9 " B ", ...), seems to be for decay data sets only?
         if(fDebug) std::cout << "Ignoring beta \"" << line << "\"" << std::endl;
         break;
      case 'E':   // EC record (1-5 nuclide, 6 blank or not '1' for cont., 7-9 " E ", ...), seems to be for decay data sets only?
         if(fDebug) std::cout << "Ignoring EC \"" << line << "\"" << std::endl;
         break;
      case 'A':   // alpha record (1-5 nuclide, 6 blank or not '1' for cont., 7-9 " A ", ...), seems to be for decay data sets only?
         if(fDebug) std::cout << "Ignoring alpha \"" << line << "\"" << std::endl;
         break;
      case 'D':   // delayed particle record (1-5 nuclide, 6 blank or not '1' for cont., 7-8 " D", 9 particle N, P, or A, ...), seems to be for decay data sets only?
         if(fDebug) std::cout << "Ignoring delayed \"" << line << "\"" << std::endl;
         break;
      case 'R':   // reference record (1-3 mass, 4-7 blank, 8 'R', 9 blank, ...) not present?
         if(fDebug) std::cout << "Ignoring reference \"" << line << "\"" << std::endl;
         break;
      case ' ':   // comment (if line[6] is 'c' or maybe 'C', 'd', 'D', 't', or 'T')
         if(fDebug) std::cout << "Ignoring comment \"" << line << "\"" << std::endl;
         break;
      default:
         // probably a comment record (1-5 nuclide, 6 blank or character not '1' for cont., 7 'c', 'D', 'T', or 't', plus other stuff we ignore)
         if(fDebug) std::cout << "Skipping unknown character " << line[7] << " from line \"" << line << "\"" << std::endl;
         break;
      };
   } while(std::getline(input, line) && !line.empty());

   if(fDebug) std::cout << "Done reading \"" << filename << "\"" << std::endl;

   input.close();
}
#endif
