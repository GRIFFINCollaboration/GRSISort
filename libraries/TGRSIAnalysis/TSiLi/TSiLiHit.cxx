
#include "TSiLi.h"

ClassImp(TSiLiHit)

TSiLiHit::TSiLiHit()  {  }

TSiLiHit::~TSiLiHit()  {  }


void TSiLiHit::Clear(Option_t *opt)  {
  segment = -1;
  energy  = 0.0;
  cfd     = 0.0;
  charge  = -1;
  time    = -1;
  ts      = -1;

}


void TSiLiHit::Print(Option_t *opt) const {
  printf("===============\n");
  printf("not yet written\n");
  printf("===============\n");
}

