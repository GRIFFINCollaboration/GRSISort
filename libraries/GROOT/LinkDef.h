// GRootGuiFactory.h GRootFunctions.h GRootCommands.h GRootCanvas.h GRootBrowser.h GCanvas.h GH2Base.h  GH2I.h GH2D.h  GPeak.h GGaus.h GH1D.h GNotifier.h GPopup.h GSnapshot.h TCalibrator.h GHSym.h GCube.h TLevelScheme.h

#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link         C++ nestedclasses;

#pragma link C++ namespace GRootFunctions;

//#pragma link C++ function GRootFunctions::PolyBg;
//#pragma link C++ function GRootFunctions::StepBG;
//#pragma link C++ function GRootFunctions::StepFunction;
//#pragma link C++ function GRootFunctions::PhotoPeak;
//#pragma link C++ function GRootFunctions::PhotoPeakBG;
//#pragma link C++ function GRootFunctions::Gaus;
//#pragma link C++ function GRootFunctions::SkewedGaus;

#pragma link C++ function LabelPeaks;
#pragma link C++ function ShowPeaks;
#pragma link C++ function RemovePeaks;
#pragma link C++ function PhotoPeakFit;
#pragma link C++ function GausFit;

#pragma link C++ function MergeStrings;

//#pragma link C++ function PeakFit;

#pragma link C++ function Help;
#pragma link C++ function Commands;
#pragma link C++ function Version;
#pragma link C++ function GrabHist;
#pragma link C++ function GrabFit;
#pragma link C++ function StartGUI;
#pragma link C++ function GUIIsRunning;
#pragma link C++ function GetProjection;

#pragma link C++ enum EAxis;
#pragma link C++ enum kBackgroundSubtraction;
#pragma link C++ function AddOffset;

#pragma link C++ class GPopup + ;

#pragma link C++ class GMarker + ;
#pragma link C++ class GCanvas + ;
#pragma link C++ class GRootCanvas + ;
#pragma link C++ class GRootBrowser + ;
#pragma link C++ class GBrowserPlugin + ;
#pragma link C++ class GRootGuiFactory + ;

#pragma link C++ class TRegion + ;
#pragma link C++ class GH1D + ;
#pragma link C++ class GH2Base + ;
#pragma link C++ class GH2I + ;
#pragma link C++ class GH2D + ;
#pragma link C++ class GHSym + ;
#pragma link C++ class GHSymF + ;
#pragma link C++ class GHSymD + ;
#pragma link C++ class GCube + ;
#pragma link C++ class GCubeF + ;
#pragma link C++ class GCubeD + ;

#pragma link C++ class GPeak + ;
#pragma link C++ class GGaus + ;
//#pragma link C++ class GEfficiency+;

//#pragma link C++ class TTransition+;
#pragma link C++ class TCalibrator + ;

#pragma link C++ class GNotifier + ;

#pragma link C++ class GSnapshot + ;

#pragma link C++ class TGamma + ;
#pragma link C++ class TLevel + ;
#pragma link C++ class TBand + ;
#pragma link C++ class TLevelScheme + ;

#pragma link C++ class std::map < TLevel*, std::tuple < double, double, TColor*, std::string>> + ;
#pragma link C++ class std::map < double, TLevel> + ;
#pragma link C++ class std::map < std::string, TBand> + ;

#endif
