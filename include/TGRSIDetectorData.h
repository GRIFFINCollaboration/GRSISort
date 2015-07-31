#ifndef TGRSIDetectorDATA_H
#define TGRSIDetectorDATA_H

#ifndef __CINT__

/*****************************************************************************
 *
 *****************************************************************************/

#include "Globals.h"

#include <cstdlib>

#include "TObject.h"

class TGRSIDetectorData {


   public:
      TGRSIDetectorData();															//!
      virtual ~TGRSIDetectorData(); //!
  
      //There should be a commone function that we can use to abstractify
      virtual void Clear(Option_t *opt=""); //!
      virtual void Print(Option_t *opt="") const; //!
  
   //We might want to keep the classDef anyway? Not sure if we can write HTML without it but I'm also not sure if we can do it without inheriting from TObject
  //ClassDef(TGRSIDetectorData,0)  //!  // TGRSIData structure 
};

#endif

#endif
