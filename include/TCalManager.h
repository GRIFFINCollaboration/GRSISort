#ifndef __TCALMANAGER_H__
#define __TCALMANAGER_H__

#include "TCal.h"
#include "TNamed.h"
#include "TClass.h"

class TCalManager : public TNamed {
 public: 
   TCalManager();
   TCalManager(const char* classname);
   ~TCalManager(); 

 public:
   TCal* GetCal(UInt_t channum);
   void AddToManager(TCal* cal, UInt_t channum, Option_t *opt = ""); 
   void AddToManager(TCal* cal, Option_t *opt = "");
   void RemoveCal(UInt_t channum, Option_t *opt="");
   void SetClass(const char* classname);
   void SetClass(TClass *cl);
   const char* GetClass(){ return fClass ? fClass->ClassName() : 0;}

   virtual void Print(Option_t *opt = "") const;
   virtual void Clear(Option_t *opt = "");

   TCal* operator[](UInt_t channum){ return GetCal(channum);}

 private:
   typedef std::map<UInt_t,TCal*> CalMap;
   CalMap fcalmap;
   TClass* fClass;


   ClassDef(TCalManager,1);

};

#endif
