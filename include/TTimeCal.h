#ifndef TTIMECAL_H__
#define TTIMECAL_H__

#include "TCal.h"

class TTimeCal : public TCal {
 public: 
   TTimeCal(){};
   TTimeCal(const char* name, const char* title) : TCal(name,title) {};
   virtual ~TTimeCal(){}; 

   //pure virtual functions  
   virtual Bool_t IsGroupable() const {return false;}

 public:
   virtual void WriteToChannel() const;
   virtual void ReadFromChannel();
   virtual std::vector<Double_t> GetParameters() const;
   virtual Double_t GetParameter(size_t parameter) const ;

   void AddParameter(Double_t param);
   void SetParameters(std::vector<Double_t> paramVec);
   void SetParameter(Int_t idx, Double_t param);

   virtual void Print(Option_t *opt = "") const;
   virtual void Clear(Option_t *opt = "");

 private:
   std::vector<Double_t> fParameters;

   ClassDef(TTimeCal,1);

};

#endif
