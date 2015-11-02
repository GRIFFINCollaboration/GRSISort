
#ifndef TSILI_H
#define TSILI_H

#include <cstdio>
#include <iostream>

#include "TDetector.h"

#if !defined (__CINT__) && !defined (__CLING__)
#include "TSiLiData.h"
#else
class TSiLiData;
#endif

#include "TSiLiHit.h"

class TSiLi: public TDetector  {

  public:
    TSiLi();
    ~TSiLi();

    void BuildHits(TDetectorData *data, Option_t *opt="");    
    void FillData(TFragment*,TChannel*,MNEMONIC*);

    void Print(Option_t *opt="") const;
    void Clear(Option_t *opt="");
    
    
    Short_t GetMultiplicity() const         {  return  sili_hits.size();  }
    TSiLiHit *GetSiLiHit(const int& i);

    TVector3 GetPosition(int segment);

  private:
#if !defined (__CINT__) && !defined (__CLING__)
    TSiLiData *data;    //! 
    #endif
    std::vector<TSiLiHit> sili_hits;


  ClassDef(TSiLi,2);
  
};


#endif
