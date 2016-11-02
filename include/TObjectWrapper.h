// Author: Ryan Dunlop    10/16

/** \addtogroup GRSIProof
 *  @{
 */

#ifndef TOBJECTWRAPPER_H
#define TOBJECTWRAPPER_H

#include "Globals.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <time.h>
#include <fstream>
#include <streambuf>


/////////////////////////////////////////////////////////////////
///
/// \class TObjectWrapper
///
/// This Class allows proof to work with GRSI
/// 
///
/////////////////////////////////////////////////////////////////


template<typename T>
class TObjectWrapper : public TObject {
   T* pT;

public:
   TObjectWrapper(T* ptr) : pT(ptr) {}
   T* operator->() { return pT; }
   ClassDef(TObjectWrapper,1);
};


#endif // TOBJECTWRAPPER_H
