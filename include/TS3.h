#ifndef TS3_H
#define TS3_H

#include "Globals.h"

#include <vector>
#include <cstdio>

#include "TS3Hit.h"
#ifndef __CINT__
#include "TS3Data.h"
#else
class TS3Data;
#endif
#include "TVector3.h" 

#include "TGRSIDetector.h" 
#include "TObject.h"


class TS3 : public TGRSIDetector {

  public:
    TS3();
    TS3(const TS3&);
    virtual  ~TS3();
    
    TGRSIDetectorHit* GetHit(const Int_t& idx =0);
    TS3Hit* GetS3Hit(const Int_t& idx = 0);
    
    void Copy(TObject&) const;
    void Clear(Option_t *opt="");   
    void Print(Option_t *opt="") const;
    void PushBackHit(TGRSIDetectorHit* deshit);
     
    void BuildHits(TDetectorData *data=0,Option_t *opt="");
    void FillData(TFragment*,TChannel*,MNEMONIC*);

    TS3& operator=(const TS3&);  // 
     
    Short_t GetMultiplicity()  {  return s3_hits.size();}


    TVector3 GetPosition(int front, int back);


  private:
    TS3Data *s3data; //!
    std::vector<TS3Hit> s3_hits;

    ///for geometery
    static int ring_number;          //!
    static int sector_number;        //!

    static double offset_phi;        //!
    static double outer_diameter;    //!
    static double inner_diameter;    //!
    static double target_distance;   //!


  ClassDef(TS3,3)
};

#endif
