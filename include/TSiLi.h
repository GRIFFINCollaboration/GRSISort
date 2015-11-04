#ifndef TSILI_H
#define TSILI_H

#include "Globals.h"

#include <vector>
#include <cstdio>

#if !defined (__CINT__) && !defined (__CLING__)
#include "TSiLiData.h"
#else
class TSiLiData;
#endif
#include "TVector3.h" 

#include "TSiLiHit.h" 
#include "TGRSIDetector.h" 
#include "TObject.h"

class TSiLi: public TGRSIDetector  {

  public:
    TSiLi();
    TSiLi(const TSiLi&);
    virtual ~TSiLi();
    
    TGRSIDetectorHit* GetHit(const Int_t& idx =0);
    TSiLiHit* GetSiLiHit(const Int_t& idx = 0);
     

    void BuildHits(TDetectorData *data=0, Option_t *opt="");    
    void FillData(TFragment*,TChannel*,MNEMONIC*);

    TSiLi& operator=(const TSiLi&);  // 

    void Copy(TObject&) const;
    void Clear(Option_t *opt="");   
    void Print(Option_t *opt="") const;
    void PushBackHit(TGRSIDetectorHit* deshit);
    
    
    Short_t GetMultiplicity() const         {  return  sili_hits.size();  }

    static TVector3 GetPosition(int segment);
   
    
  private:
    #ifndef __CINT__
    TSiLiData *silidata;    //! 
    #endif
    std::vector<TSiLiHit> sili_hits;


  ClassDef(TSiLi,3);
};


#endif
