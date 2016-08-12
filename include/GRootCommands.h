#ifndef GROOTCOMMANDS__H
#define GROOTCOMMANDS__H

#include <string>
#include <TChain.h>
class TTree;
class TH1;
class TH2;
class GH2I;
class GH2D;
class GH1D;
class GMarker;
class TF1;
class GPeak;
class GGaus;

#include "TDirectory.h"

extern TChain *gChain;

int  LabelPeaks(TH1*,double,double,Option_t *opt="");
bool ShowPeaks(TH1**,unsigned int, double sigma=2.0, double thresh=0.02);
bool RemovePeaks(TH1**,unsigned int);

GPeak *PhotoPeakFit(TH1*,double,double,Option_t *opt="");
GGaus *GausFit(TH1*,double,double,Option_t *opt="");
TF1   *DoubleGausFit(TH1*,double,double,double,double,Option_t *opt="");

std::string MergeStrings(const std::vector<std::string>& strings, char split='\n');

bool GetProjection(GH2D *hist,double low, double high, double bg_low=0,double bg_high=0);

//bool PeakFit(TH1*,Double_t,Double_t,Option_t *opt="");


//
//  Below are in the interpretur commands. (added to the linkdef!)
//

void Prompt();
void Help();
void Commands();
void Version();
TH1 *GrabHist(int i=0); //return the ith histogram from the current canvas.
TF1 *GrabFit(int i=0); //return the ith fit from the current canvas.

void StartGUI();
bool GUIIsRunning();

//enum EAxis {kXAxis = 1,kYAxis = 2,kXYaxis = 3,kZaxis = 4,kXZaxis = 5,kYZAxis = 6,kXYZAxis=7};
enum EAxis {kXAxis = 1,kYAxis = 2};

TH2 *AddOffset(TH2 *mat,double offset,EAxis axis=kXAxis);

#endif
