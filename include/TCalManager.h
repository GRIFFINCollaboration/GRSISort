#ifndef __TCALMANAGER_H__
#define __TCALMANAGER_H__

#include "TCal.h"
#include "TNamed.h"

class TCalManager : public TNamed {
 public: 
   TCalManager();
   TCalManager(const char* name, const char* title) : TNamed(name,title){};
   ~TCalManager(); 

 public:
   TCal* GetCal(UInt_t channum);
   void AddToManager(TCal* cal, UInt_t channum, Option_t *opt = ""); 
   void AddToManager(TCal* cal, Option_t *opt = "");

 private:
   typedef std::map<UInt_t,TCal*> CalMap;
   CalMap fcalmap;

   ClassDef(TCalManager,1);

};

#endif
