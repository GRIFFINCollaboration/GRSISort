#ifndef __GRSIDetectorDATA__
#define __GRSIDetectorDATA__
/*****************************************************************************
 *
 *****************************************************************************/

#include "Globals.h"

#include <cstdlib>

#include <TObject.h>

class TGRSIDetectorData : public TObject  {


   public:
      TGRSIDetectorData();															//!
      virtual ~TGRSIDetectorData(); //!
  
      virtual void Clear(Option_t *opt=""); //!
      virtual void Print(Option_t *opt=""); //!
  

  ClassDef(TGRSIDetectorData,0)  //!  // SharcData structure 
};

#endif
