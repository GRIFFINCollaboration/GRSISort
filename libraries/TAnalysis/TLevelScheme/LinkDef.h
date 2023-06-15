//TLevelScheme.h

#if __cplusplus >= 201703L
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ class TGamma+;
#pragma link C++ class TLevel+;
#pragma link C++ class TBand+;
#pragma link C++ class TLevelScheme+;

#pragma link C++ class std::map<TLevel*, std::tuple<double, double, TColor*, std::string>>+;
#pragma link C++ class std::map<double, TLevel>+;
#pragma link C++ class std::map<std::string, TBand>+;

#endif

#endif
