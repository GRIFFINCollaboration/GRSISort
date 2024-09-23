#ifndef TCAL_H
#define TCAL_H

/** \addtogroup Calibration
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TCal
///
/// This is an abstract class that contains the basic info
/// about a calibration. Calibrations here are TGraphErrors
/// that are fit, with the resulting fit function being the
/// calibrating function.
///
/////////////////////////////////////////////////////////////////

#include <map>
#include <vector>
#include <utility>

#include "Varargs.h"
#include "TNamed.h"
#include "TH1.h"
#include "TF1.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TRef.h"

#include "TChannel.h"
#include "TNucleus.h"
#include "TGRSITransition.h"

class TCal : public TGraphErrors {
public:
   TCal();
   TCal(const char* name, const char* title);
   TCal(const TCal&);
   TCal(TCal&&) noexcept            = default;
   TCal& operator=(const TCal&)     = default;
   TCal& operator=(TCal&&) noexcept = default;
   ~TCal()                          = default;

   // pure virtual functions
   virtual Bool_t IsGroupable() const = 0;

   void                          Copy(TObject& obj) const override;
   virtual void                  WriteToChannel() const { Error("WriteToChannel", "Not defined for %s", ClassName()); }
   virtual TF1*                  GetFitFunction() const { return fFitFunc; }
   virtual void                  SetFitFunction(TF1* func) { fFitFunc = func; }
   virtual std::vector<Double_t> GetParameters() const;
   virtual Double_t              GetParameter(size_t parameter) const;

   TChannel* GetChannel() const;
   Bool_t    SetChannel(TChannel* chan);
   Bool_t    SetChannel(UInt_t chanNum);
   void      Print(Option_t* opt = "") const override;
   void      Clear(Option_t* opt = "") override;
   // virtual void Draw(Option_t* chopt = "");

   virtual void WriteToAllChannels(const std::string& mnemonic = "");

   virtual void      SetHist(TH1* hist);
   TH1*              GetHist() const { return fHist; }
   virtual void      SetNucleus(TNucleus* nuc, Option_t* opt = "");
   virtual TNucleus* GetNucleus() const { return fNuc; }

protected:
   void InitTCal();

private:
   TRef      fChan{nullptr};      ///< This points at the TChannel
   TF1*      fFitFunc{nullptr};   ///< Fit function representing calibration
   TH1*      fHist{nullptr};      ///< Histogram that was fit by the TPeak.
   TNucleus* fNuc{nullptr};       ///< Nucleus that we are calibrating against

   /// \cond CLASSIMP
   ClassDefOverride(TCal, 2)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
