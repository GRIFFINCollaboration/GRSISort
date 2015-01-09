#include "TCalManager.h"

ClassImp(TCalManager)

TCalManager::TCalManager(){}

TCalManager::~TCalManager(){}


TCal *TCalManager::GetCalByChanNum(UInt_t channum) {
   TCal *cal  = 0;
   try {
	   cal = fcalmap.at(channum);
   } 
   catch(const std::out_of_range& oor) {
	   return 0;
   }
   return cal;
}

