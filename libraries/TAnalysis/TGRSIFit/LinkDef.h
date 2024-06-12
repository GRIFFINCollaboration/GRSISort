// TGRSIFit.h TGRSIFunctions.h TMultiPeak.h TPeak.h TDecay.h TLMFitter.h

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link         C++ nestedclasses;
#pragma link         C++ nestedclass;
#pragma link         C++ nestedtypedef;

#pragma link C++ class TGRSIFit + ;
#pragma link C++ namespace TGRSIFunctions;
#pragma link C++ defined_in namespace TGRSIFunctions;

#pragma link C++ class TPeak + ;
#pragma link C++ class TMultiPeak + ;

#pragma link C++ class TSingleDecay + ;
#pragma link C++ class std::vector < TSingleDecay*> + ;
#pragma link C++ class std::vector < TDecayChain*> + ;
#pragma link C++ class TDecayChain + ;
#pragma link C++ class TDecayFit - ;
#pragma link C++ class TDecay + ;
#pragma link C++ class TVirtualDecay - ;
#pragma link C++ class std::map < Int_t, std::vector < TSingleDecay*>> + ;

#pragma link C++ class TLMFitter + ;

#endif
