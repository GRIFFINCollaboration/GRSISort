
#ifndef TSILI_H
#define TSILI_H

#include <cstdio>

#include <TGRSIDetector.h>

#ifndef __CINT__
#include "TSiLiData.h"
#else
class TSiLiData;
#endif

#include "TSiLiHit.h"

class TSiLi: public TGRSIDetector  {

  public:
    TSiLi();
    ~TSiLi();

    void BuildHits(TGRSIDetectorData *data, Option_t *opt="");    
      void FillData(TFragment*,TChannel*,MNEMONIC*);

    void Print(Option_t *opt="") const;
    void Clear(Option_t *opt="");
    
    
    int GetMultiplicity()     {  return  sili_hits.size();  }
    TSiLiHit *GetHit(int i)   {  return  &sili_hits.at(i);  }

    TVector3 GetPosition(int segment);

  private:
    #ifndef __CINT__
    TSiLiData *data;    //! 
    #endif
    std::vector<TSiLiHit> sili_hits;


  ClassDef(TSiLi,2);
  
};


#endif
