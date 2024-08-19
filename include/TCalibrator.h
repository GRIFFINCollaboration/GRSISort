#ifndef TCALIBRATORS_H
#define TCALIBRATORS_H

#include <map>

#include <TNamed.h>
#include <TGraphErrors.h>

class TH1;

class TChannel;
class TNucleus;

class TCalibrator : public TNamed {
public:
   TCalibrator();
   TCalibrator(const TCalibrator&)                = default;
   TCalibrator(TCalibrator&&) noexcept            = default;
   TCalibrator& operator=(const TCalibrator&)     = default;
   TCalibrator& operator=(TCalibrator&&) noexcept = default;
   ~TCalibrator();

   void   Copy(TObject& obj) const override;
   void   Print(Option_t* opt = "") const override;
   void   Clear(Option_t* opt = "") override;
   void   Draw(Option_t* opt = "") override;
   UInt_t Size() const { return fPeaks.size(); }

   int  GetFitOrder() const { return fFitOrder; }
   void SetFitOrder(int order) { fFitOrder = order; }

   TGraph&             MakeCalibrationGraph(double min_figure_of_merit = 0.001);
   TGraphErrors&       MakeEffGraph(double seconds = 3600., double bq = 100000., Option_t* opt = "draw");
   std::vector<double> Calibrate(double min_figure_of_merit = 0.001);

   int AddData(TH1* data, const std::string& source, double sigma = 2.0, double threshold = 0.05, double error = 0.001);

   int AddData(TH1* data, TNucleus* source, double sigma = 2.0, double threshold = 0.05, double error = 0.001);

   void UpdateTChannel(TChannel* channel);

   void   Fit(int order = 1);
   double GetParameter(int i = 0) const;
   double GetEffParameter(int i = 0) const;

   struct Peak {
      double      centroid{0.};
      double      energy{0.};
      double      area{0.};
      double      intensity{0.};
      std::string nucleus;
   };

   void AddPeak(double cent, double eng, std::string nuc, double a = 0.0, double inten = 0.0);
   Peak GetPeak(UInt_t i) const { return fPeaks.at(i); }

   TGraph* FitGraph() { return &fFitGraph; }
   TGraph* EffGraph() { return &fEffGraph; }
   TF1*    LinFit() { return fLinFit; }
   TF1*    EffFit() { return fEffFit; }

   std::string PrintEfficency(const char* filename = "");

#ifndef __CINT__
// struct SingleFit {
//  double max_error;
//  std::string nucleus;
//  std::map<double,double> data2source;
//  TGraph graph;
//};
#endif

private:
#ifndef __CINT__
   // std::map<std::string,SingleFit> all_fits;
   static std::map<double, double> Match(std::vector<double>, std::vector<double>);
#endif
   std::vector<Peak> fPeaks;

   TGraph       fFitGraph;
   TGraphErrors fEffGraph;
   TF1*         fLinFit{nullptr};
   TF1*         fEffFit{nullptr};

   int fFitOrder{0};

   std::array<double, 4> fEffPar{0.};

   static void ResetMap(std::map<double, double>& inmap);
   static void PrintMap(std::map<double, double>& inmap);
   static bool CheckMap(const std::map<double, double>& inmap);

   /// \cond CLASSIMP
   ClassDefOverride(TCalibrator, 1)   // NOLINT(readability-else-after-return)
                                      /// \endcond
};

#endif
