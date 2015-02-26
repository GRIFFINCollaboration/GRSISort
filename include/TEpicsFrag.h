#ifndef TEPICSFRAG_H
#define TEPICSFRAG_H

#include "Globals.h"

#include <vector>
//#include <stdint.h>
#include <time.h>

#include "Rtypes.h"
#include "TObject.h"

//#ifndef __CINT__
//#include "Globals.h"
//#endif

////////////////////////////////////////////////////////////////
//                                                            //
// TEpicsFrag                                                 //
//                                                            //
// This Class should contain all the information found in     //
// NOT typeid 1 midas events.                                 //
//                                                            //
//                                                            //
////////////////////////////////////////////////////////////////

class TEpicsFrag : public TObject	{
  public:
    TEpicsFrag(); 
    ~TEpicsFrag(); 

    time_t   MidasTimeStamp;       //->  Timestamp of the MIDAS event  
    Int_t    MidasId;              //->  MIDAS ID

    std::vector<float>      Data;
    std::vector<std::string> Name;
    std::vector<std::string> Unit;

    int GetSize() { return Data.size(); }
    inline float GetData(const unsigned int &i) { if(i>=Data.size()) return Data.back(); else return Data.at(i); }

    virtual void Clear(Option_t *opt = ""); //!
    virtual void Print(Option_t *opt = ""); //!
    
    ClassDef(TEpicsFrag,1);  // Event Fragments
};
#endif // TEPICSFRAG_H
