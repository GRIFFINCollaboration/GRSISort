// TFragment.h TBadFragment.h TChannel.h TRunInfo.h TGRSISortInfo.h TPPG.h TEpicsFrag.h TScaler.h TScalerQueue.h TParsingDiagnostics.h TGRSIUtilities.h TMnemonic.h TSortingDiagnostics.h TTransientBits.h TPriorityValue.h TSingleton.h TDetectorInformation.h TParserLibrary.h TDataFrameLibrary.h TUserSettings.h


#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class std::vector<Int_t>+;

#pragma link C++ class std::string+;
#pragma link C++ class TSingleton+;
#pragma link C++ class TRunInfo+;
#pragma link C++ class TSingleton<TRunInfo>-;

#pragma link C++ class TFragment+;
#pragma link C++ class TBadFragment+;

#pragma link C++ class TEpicsFrag+;
#pragma link C++ class TChannel-;
#pragma link C++ class TGRSISortInfo+;
#pragma link C++ class TGRSISortList+;

#pragma link C++ class TPPG-;
#pragma link C++ class TSingleton<TPPG>-;
#pragma link C++ class TPPGData+;
#pragma link C++ class std::map<ULong64_t,TPPGData*>;
#pragma link C++ class TScaler+;
#pragma link C++ class TScalerData+;
#pragma link C++ class std::map<UInt_t, std::map<ULong64_t, TScalerData*> >;
#pragma link C++ class std::map<ULong64_t, TScalerData*>;
#pragma link C++ class TParsingDiagnosticsData+;
#pragma link C++ class TParsingDiagnostics+;
#pragma link C++ class TSingleton<TParsingDiagnostics>-;
#pragma link C++ class TSortingDiagnostics+;
#pragma link C++ class TSingleton<TSortingDiagnostics>-;
#pragma link C++ class TMnemonic+;
#pragma link C++ class TDetectorInformation+;
#pragma link C++ class TParserLibrary+;
#pragma link C++ class TDataFrameLibrary+;
#pragma link C++ class TUserSettings+;

#pragma link C++ class TTransientBits<UChar_t>+;
#pragma link C++ class TTransientBits<UShort_t>+;
#pragma link C++ class TTransientBits<UInt_t>+;
#pragma link C++ class TTransientBits<ULong_t>+;

#pragma link C++ function GetRunNumber;
#pragma link C++ function GetSubRunNumber;

#endif
