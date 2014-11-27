
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

//#pragma link C++ class std::vector<Short_t>+;

//It is important that tigress is built first so the two lines below are 
//handled before griffin is linked!  
//#pragma link C++ class TCrystalHit+;
//#pragma link C++ class std::vector<TCrystalHit>+;
#pragma link C++ class TGriffinHit+;
#pragma link C++ class TGriffin+;

#endif





