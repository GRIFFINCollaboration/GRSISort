#ifndef TANALYSISOPTIONS_H
#define TANALYSISOPTIONS_H

/** \addtogroup Sorting
 *  @{
 */

#include "TObject.h"
#include "TFile.h"

/////////////////////////////////////////////////////////////////
///
/// \class TAnalysisOptions
///
/// This class stores those command line arguments passed to GRSISort, that
/// are pertinent to the analysis. This includes such settings as the addback
/// window width, whether to correct cross talk, etc.
///
/////////////////////////////////////////////////////////////////

class TAnalysisOptions : public TObject {
   friend class TGRSIOptions;

public:
   TAnalysisOptions() = default;

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   bool WriteToFile(const std::string& file);
   bool WriteToFile(TFile* file = nullptr);
   void ReadFromFile(const std::string& file);
   void ReadFromFile(TFile* file = nullptr);

   // sorting options
   void SetBuildWindow(const int64_t& t_bw) { fBuildWindow = t_bw; }
   void SetBuildEventsByTimeStamp(bool& val) { fBuildEventsByTimeStamp = val; }
   void SetAddbackWindow(const double& t_abw) { fAddbackWindow = t_abw; }
   void SetSuppressionWindow(const double& t_sup) { fSuppressionWindow = t_sup; }
   void SetSuppressionEnergy(const double& e_sup) { fSuppressionEnergy = e_sup; }

   void SetWaveformFitting(const bool& flag) { fWaveformFitting = flag; }
   bool IsWaveformFitting() const { return fWaveformFitting; }

   void SetCorrectCrossTalk(const bool& flag, Option_t* opt = "");
   bool IsCorrectingCrossTalk() const { return fIsCorrectingCrossTalk; }

   int64_t BuildWindow() const { return fBuildWindow; }
   bool    BuildEventsByTimeStamp() const { return fBuildEventsByTimeStamp; }
   double  AddbackWindow() const
   {
      if(fAddbackWindow < 1) {
         return 15.0;
      }
      return fAddbackWindow;
   }

   double SuppressionWindow() const { return fSuppressionWindow; }
   double SuppressionEnergy() const { return fSuppressionEnergy; }

   bool StaticWindow() const { return fStaticWindow; }

   Long64_t Merge(TCollection* list) { return Merge(list, ""); }
   Long64_t Merge(TCollection* list, Option_t*);

private:
   using TObject::Compare;
   bool Compare(const TAnalysisOptions* options) const;

   // sorting options
   int64_t fBuildWindow{2000};               ///< if building with a window(GRIFFIN) this is the size of the window. (default = 2us (2000))
   bool    fBuildEventsByTimeStamp{false};   ///< use time stamps instead of time (including CFD) to build events
   double  fAddbackWindow{300.};             ///< Time used to build Addback-Ge-Events for TIGRESS/GRIFFIN.   (default = 300 ns (300))
   double  fSuppressionWindow{300.};         ///< Time used to suppress Ge-Events.   (default = 300 ns (300))
   double  fSuppressionEnergy{0.};           ///< Minimum energy used to suppress Ge-Events.   (default = 0 keV)
   bool    fIsCorrectingCrossTalk{false};    ///< True if we are correcting for cross-talk in GRIFFIN at analysis-level
   bool    fWaveformFitting{false};          ///< If true, waveform fitting with SFU algorithm will be performed
   bool    fStaticWindow{true};              ///< Flag to use static window (default moving)

   /// \cond CLASSIMP
   ClassDefOverride(TAnalysisOptions, 5)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif /* TANALYSISOPTIONS_H */
