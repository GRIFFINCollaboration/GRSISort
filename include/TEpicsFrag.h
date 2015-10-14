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
    virtual void Print(Option_t *opt = "") const; //!
    
    ClassDef(TEpicsFrag,1);  // Event Fragments
};



class TSCLRFrag : public TObject	{
  public:
    TSCLRFrag(); 
    ~TSCLRFrag(); 

    time_t   MidasTimeStamp;       //  Timestamp of the MIDAS event  
    Int_t    MidasId;              //  MIDAS ID

    static void SetAddressMap(int*,int);           //!  // done once per run.
    static std::vector<Int_t >      AddressMap; //!
    
    std::vector<UInt_t >     Address;
    std::vector<UInt_t>      Data1;
    std::vector<UInt_t>      Data2;
    std::vector<UInt_t>      Data3;
    std::vector<UInt_t>      Data4;

    int GetSize() { return Data1.size(); }
    inline UInt_t GetData1(const unsigned int &i) { return Data1.at(i); }
    inline UInt_t GetData2(const unsigned int &i) { return Data2.at(i); }
    inline UInt_t GetData3(const unsigned int &i) { return Data3.at(i); }
    inline UInt_t GetData4(const unsigned int &i) { return Data4.at(i); }
    
    virtual void Clear(Option_t *opt = ""); //!
    virtual void Print(Option_t *opt = "") const; //!
    virtual void Copy(const TSCLRFrag&); 

    ClassDef(TSCLRFrag,1);  // Event Fragments
};









#endif // TEPICSFRAG_H
