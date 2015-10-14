// TFragment.h TChannel.h TGRSIRunInfo.h TGRSISortInfo.h TGRSIStats.h TFragmentQueue.h TFragmentSelector.h TPPG.h TEpicsFrag.h TScaler.h TScalerQueue.h

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class std::vector<Int_t>+;
//#pragma link C++ class std::vector<Short_t>+;
//#pragma link C++ class std::vector<UShort_t>+;

#pragma link C++ class TFragment+;

#pragma link C++ class TEpicsFrag+;
#pragma link C++ class TChannel-;
#pragma link C++ class TGRSIRunInfo-;
#pragma link C++ class TGRSISortInfo+;
#pragma link C++ class TGRSISortList+;

#pragma link C++ class TGRSIStats+;
#pragma link C++ class TFragmentQueue+;
#pragma link C++ class TFragmentSelector+;
#pragma link C++ class TPPG-;
#pragma link C++ class TPPGData+;
#pragma link C++ class std::map<ULong64_t,TPPGData*>;
#pragma link C++ class TScaler+;
#pragma link C++ class TScalerData+;
#pragma link C++ class TScalerQueue+;
#pragma link C++ class std::map<UInt_t, std::map<ULong64_t, TScalerData*> >;
#pragma link C++ class std::map<ULong64_t, TScalerData*>;

#endif





