
#include "TSiLi.h"
#include <TGRSIRunInfo.h>

/// \cond CLASSIMP
ClassImp(TSiLi)
/// \endcond

TSiLi::TSiLi() {
   Clear();	
}

TSiLi::~TSiLi()  {
}

void TSiLi::Copy(TObject &rhs) const {
  TGRSIDetector::Copy(rhs);
  static_cast<TSiLi&>(rhs).fSiLiHits     = fSiLiHits;
  return;                                      
} 

TSiLi::TSiLi(const TSiLi& rhs) : TGRSIDetector() {
  rhs.Copy(*this);
} 

void TSiLi::Clear(Option_t *opt)  {
  fSiLiHits.clear();
}

TSiLi& TSiLi::operator=(const TSiLi& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TSiLi::Print(Option_t *opt) const  {  
  printf("%lu sili_hits\n",fSiLiHits.size());
}

TGRSIDetectorHit* TSiLi::GetHit(const Int_t& idx){
   return GetSiLiHit(idx);
}

TSiLiHit * TSiLi::GetSiLiHit(const int& i)   {  
   try{
      return &fSiLiHits.at(i);   
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return 0;
}  

void TSiLi::AddFragment(std::shared_ptr<TFragment> frag, TChannel* chan) {
  if(frag == NULL || chan == NULL) {
	 return;
  }

  TSiLiHit hit(*frag);//Waveform fitting happens in ctor now
  fSiLiHits.push_back(std::move(hit));
}

TVector3 TSiLi::GetPosition(int seg)  {
  TVector3 position;
  position.SetXYZ(0,0,-1);
  return position;
}
