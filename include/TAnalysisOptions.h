#ifndef TANALYSISOPTIONS_H
#define TANALYSISOPTIONS_H

/** \addtogroup Sorting
 *  @{
 */

#include "TObject.h"
#include "TFile.h"

#include "ArgParser.h"

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
   TAnalysisOptions();

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   bool WriteToFile(TFile* file = nullptr);
   void ReadFromFile(const std::string& file);

   // sorting options
   inline void SetBuildWindow(const long int t_bw) { fBuildWindow = t_bw; }
   inline void SetAddbackWindow(const double t_abw) { fAddbackWindow = t_abw; }
   inline void SetSuppressionWindow(const double t_sup) { fSuppressionWindow = t_sup; }
   inline void SetSuppressionEnergy(const double e_sup) { fSuppressionEnergy = e_sup; }

   inline void SetWaveformFitting(const bool flag) { fWaveformFitting = flag; }
   inline bool                               IsWaveformFitting() { return fWaveformFitting; }

   void SetCorrectCrossTalk(const bool flag, Option_t* opt = "");
   inline bool IsCorrectingCrossTalk() { return fIsCorrectingCrossTalk; }

   inline long int BuildWindow() { return fBuildWindow; }
   inline double   AddbackWindow()
   {
      if(fAddbackWindow < 1) {
         return 15.0;
      }
      return fAddbackWindow;
   }

   inline double   SuppressionWindow()
   {
      return fSuppressionWindow;
   }

   inline double   SuppressionEnergy()
   {
      return fSuppressionEnergy;
   }

   bool StaticWindow() const { return fStaticWindow; }

private:
   // sorting options
   long int
        fBuildWindow;   ///< if building with a window(GRIFFIN) this is the size of the window. (default = 2us (200))
   int  fAddbackWindow; ///< Time used to build Addback-Ge-Events for TIGRESS/GRIFFIN.   (default = 150 ns (150))
   double fSuppressionWindow; ///< Time used to suppress Ge-Events.   (default = 150 ns (150))
   double fSuppressionEnergy; ///< Minimum energy used to suppress Ge-Events.   (default = 0 keV)
   bool fIsCorrectingCrossTalk; ///< True if we are correcting for cross-talk in GRIFFIN at analysis-level
   bool fWaveformFitting;       ///< If true, waveform fitting with SFU algorithm will be performed
   bool fStaticWindow;          ///< Flag to use static window (default moving)

   /// \cond CLASSIMP
   ClassDefOverride(TAnalysisOptions, 3); ///< Class for storing options in GRSISort
                                          /// \endcond
};
/*! @} */
#endif /* TANALYSISOPTIONS_H */
