
#include "TSiLi.h"

ClassImp(TSiLi)

TSiLi::TSiLi() {
}

TSiLi::~TSiLi()  {
}

void TSiLi::Clear(Option_t *opt)  {
  fSiLiHits.clear();
}

void TSiLi::Print(Option_t *opt) const  {  
  printf("===============\n");
  printf("not yet written\n");
  printf("===============\n");
}

void TSiLi::AddFragment(TFragment* frag, MNEMONIC* mnemonic) {
  if(frag == NULL || mnemonic == NULL) {
	 return;
  }
  TSiLiHit hit;
  hit.SetSegment(mnemonic->segment);
  TVector3 tmppos = GetPosition(mnemonic->segment);
  hit.SetPosition(tmppos);
  hit.SetVariables(*frag);
  hit.SetWavefit(*frag);
  
  fSiLiHits.push_back(hit);
}

TVector3 TSiLi::GetPosition(int seg)  {
  TVector3 position;
  position.SetXYZ(0,0,-1);
  return position;
}

TSiLiHit * TSiLi::GetSiLiHit(const int& i)   {  
   try{
      return &fSiLiHits.at(i);   
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw exit_exception(1);
   }
   return 0;
}  
