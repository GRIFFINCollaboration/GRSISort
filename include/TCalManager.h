#ifndef __TCALMANAGER_H__
#define __TCALMANAGER_H__

#include "TCal.h"

class TCalManager {
 public: 
   TCalManager();
   ~TCalManager(); 

 public:
   TCal* GetCalByChanNum(UInt_t channum);

 private:
   typedef std::map<UInt_t,TCal*> CalMap;
   CalMap fcalmap;

   ClassDef(TCalManager,1);

};

#endif
