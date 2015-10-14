#ifndef TCFDCAL_H__
#define TCFDCAL_H__

#include "TCal.h"


class TCFDCal : public TCal {
 public: 
   TCFDCal(){};
   TCFDCal(const char* name, const char* title) : TCal(name,title){};
   virtual ~TCFDCal(){}; 

	//pure virtual functions  
   virtual Bool_t IsGroupable() const {return false;}

 public:
   virtual void WriteToChannel() const;
   virtual void ReadFromChannel();
   virtual std::vector<Double_t> GetParameters() const;
   virtual Double_t GetParameter(UInt_t parameter) const ;

   void AddParameter(Double_t param);
   void SetParameters(std::vector<Double_t> paramvec);
   void SetParameter(Int_t idx, Double_t param);

   //static TGraphErrors MergeGraphs(TCal *cal,...);

   virtual void Print(Option_t *opt = "") const;
   virtual void Clear(Option_t *opt = "");

 private:
   std::vector<Double_t> fparameters;

   ClassDef(TCFDCal,1);

};

#endif
