#ifndef GROOTCOMMANDS__H
#define GROOTCOMMANDS__H

#include <string>
class TTree;
class TH1;
class TH2;
class GH2I;
class GH2D;
class GH1D;
class GMarker;
class TF1;
class GPeak;
class TPeak;
class GGaus;
class TFile;

#include "TDirectory.h"

int LabelPeaks(TH1*, double, double, Option_t* opt = "");
bool ShowPeaks(TH1**, unsigned int, double sigma = 2.0, double thresh = 0.02);
bool RemovePeaks(TH1**, unsigned int);

GPeak* PhotoPeakFit(TH1*, double, double, Option_t* opt = "");
TPeak* AltPhotoPeakFit(TH1*, double, double, Option_t* opt = "");
GGaus* GausFit(TH1*, double, double, Option_t* opt = "");
TF1* DoubleGausFit(TH1*, double, double, double, double, Option_t* opt = "");

std::string MergeStrings(const std::vector<std::string>& strings, char split = '\n');

bool GetProjection(GH2D* hist, double low, double high, double bg_low = 0, double bg_high = 0);

// bool PeakFit(TH1*,Double_t,Double_t,Option_t *opt="");

//
//  Below are in the interpretur commands. (added to the linkdef!)
//

void Prompt();
void Help();
void Commands();
void Version();
TH1* GrabHist(int i = 0); // return the ith histogram from the current canvas.
TF1* GrabFit(int i = 0);  // return the ith fit from the current canvas.

void StartGUI();
bool GUIIsRunning();
void AddFileToGUI(TFile* file);

enum class EAxis { kXAxis = 1, kYAxis = 2 };
EAxis operator &(EAxis lhs, EAxis rhs);


TH2* AddOffset(TH2* mat, double offset, EAxis axis = EAxis::kXAxis);

#endif
