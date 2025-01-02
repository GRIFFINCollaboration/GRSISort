// TDetector.h TFragment.h TBadFragment.h TChannel.h TRunInfo.h TGRSISortInfo.h TPPG.h TEpicsFrag.h TScaler.h TScalerQueue.h TParsingDiagnostics.h TGRSIUtilities.h TMnemonic.h TSortingDiagnostics.h TTransientBits.h TPriorityValue.h TSingleton.h TDetectorInformation.h TParserLibrary.h TDataFrameLibrary.h TUserSettings.h GValue.h TGRSIOptions.h TGRSIint.h TAnalysisOptions.h TDataLoop.h StoppableThread.h TFragWriteLoop.h TTerminalLoop.h TEventBuildingLoop.h TDetBuildingLoop.h TAnalysisWriteLoop.h TFragHistLoop.h TCompiledHistograms.h TRuntimeObjects.h TAnalysisHistLoop.h GRootGuiFactory.h GRootFunctions.h GRootCommands.h GRootCanvas.h GRootBrowser.h GCanvas.h GH2Base.h  GH2I.h GH2D.h  GPeak.h GGaus.h GH1D.h GNotifier.h GPopup.h GSnapshot.h TCalibrator.h GHSym.h GCube.h TLevelScheme.h

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link         C++ nestedclasses;

#pragma link C++ class std::vector < Int_t> + ;

#pragma link C++ class std::string + ;
//#pragma link C++ class TSingleton+;
#pragma link C++ class TRunInfo + ;
#pragma link C++ class TSingleton < TRunInfo> - ;

#pragma link C++ class TDetector + ;
#pragma link C++ class TDetectorHit - ;

#pragma link C++ class TFragment + ;
#pragma link C++ class TBadFragment + ;

#pragma link C++ class TEpicsFrag + ;
#pragma link C++ class TChannel - ;
#pragma link C++ class TGRSISortInfo + ;
#pragma link C++ class TGRSISortList + ;

#pragma link C++ class TPPG - ;
#pragma link C++ class TSingleton < TPPG> - ;
#pragma link C++ class TPPGData + ;
#pragma link C++ class std::map < ULong64_t, TPPGData*>;
#pragma link C++ class TScaler + ;
#pragma link C++ class TScalerData + ;
#pragma link C++ class std::map < UInt_t, std::map < ULong64_t, TScalerData*>>;
#pragma link C++ class std::map < ULong64_t, TScalerData*>;
#pragma link C++ class TParsingDiagnosticsData + ;
#pragma link C++ class TParsingDiagnostics + ;
#pragma link C++ class TSingleton < TParsingDiagnostics> - ;
#pragma link C++ class TSortingDiagnostics + ;
#pragma link C++ class TSingleton < TSortingDiagnostics> - ;
#pragma link C++ class TMnemonic + ;
#pragma link C++ class TDetectorInformation + ;
#pragma link C++ class TParserLibrary + ;
#pragma link C++ class TDataFrameLibrary + ;
#pragma link C++ class TUserSettings + ;

#pragma link C++ class TTransientBits < UChar_t> + ;
#pragma link C++ class TTransientBits < UShort_t> + ;
#pragma link C++ class TTransientBits < UInt_t> + ;
#pragma link C++ class TTransientBits < ULong_t> + ;

#pragma link C++ function GetRunNumber;
#pragma link C++ function GetSubRunNumber;

#pragma link C++ class GValue - ;
#pragma link C++ class std::map < std::string, GValue*> + ;
#pragma link C++ class std::pair < std::string, GValue*> + ;

#pragma link C++ class TGRSIOptions + ;
#pragma link C++ class TAnalysisOptions + ;
#pragma link C++ class TGRSIint + ;

#pragma link C++ class StoppableThread + ;
#pragma link C++ class std::vector < StoppableThread*> + ;

#pragma link C++ class TDataLoop + ;
#pragma link C++ class TFragWriteLoop + ;

#pragma link C++ class TEventBuildingLoop + ;
#pragma link C++ class TDetBuildingLoop + ;
#pragma link C++ class TAnalysisWriteLoop + ;

#pragma link C++ class TCompiledHistograms + ;
#pragma link C++ class TRuntimeObjects + ;
#pragma link C++ class TFragHistLoop + ;
#pragma link C++ class TAnalysisHistLoop + ;

#pragma link C++ namespace GRootFunctions;

#pragma link C++ function LabelPeaks;
#pragma link C++ function ShowPeaks;
#pragma link C++ function RemovePeaks;
#pragma link C++ function PhotoPeakFit;
#pragma link C++ function GausFit;

#pragma link C++ function MergeStrings;

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
