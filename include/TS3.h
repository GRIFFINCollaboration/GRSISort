#ifndef TS3_H
#define TS3_H

#include "TDetector.h"
#include <iostream>

#if !defined (__CINT__) && !defined (__CLING__)
#include "TS3Data.h"
#else
class TS3Data;
#endif

#include "TS3Hit.h"


class TS3 : public TDetector {

  public:
    TS3();
    ~TS3();

  public: 
    void Clear(Option_t *opt="");   
    void Print(Option_t *opt="") const;

    virtual void BuildHits(TDetectorData *data=0,Option_t *opt="");
    virtual void FillData(TFragment*,TChannel*,MNEMONIC*);

    TS3Hit *GetS3Hit(const int& i);  
    Short_t GetMultiplicity() const  {  return s3_hits.size();}


    TVector3 GetPosition(int front, int back);


  private:
    TS3Data *data; //!
    std::vector<TS3Hit> s3_hits;

    ///for geometery
    static int ring_number;          //!
    static int sector_number;        //!

    static double offset_phi;        //!
    static double outer_diameter;    //!
    static double inner_diameter;    //!
    static double target_distance;   //!

  ClassDef(TS3,2)
};

#endif
