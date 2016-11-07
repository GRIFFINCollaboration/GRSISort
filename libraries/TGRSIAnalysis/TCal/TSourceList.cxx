#include "TSourceList.h"
#include "TCalibrationPoint.h"

#include <iostream>

/// \cond CLASSIMP
ClassImp(TSourceList)
/// \endcond

TSourceList::TSourceList():TCalibrationList(){
	Clear();
}

TSourceList::~TSourceList(){}

TSourceList::TSourceList(const char *name, const char* title) : TCalibrationList(name,title) { 
	Clear();
	SetNucleus(TNucleus(name));
}

TSourceList::TSourceList(const TNucleus& nuc, const char *name, const char* title) : TCalibrationList(name,title) { 
	Clear();
	SetNucleus(nuc);
}

TSourceList::TSourceList(const TSourceList& copy) : TCalibrationList(copy){
   copy.Copy(*this);
}

TSourceList::TSourceList(const TNucleus& nuc) { 
	Clear();
	SetNucleus(nuc);
}

void TSourceList::Copy(TObject& obj) const {
	TCalibrationList::Copy(obj);
}

void TSourceList::Print(Option_t *opt) const {
	std::cout << "Nucleus: " << fNucleusName << std::endl;
	TCalibrationList::Print();
}


void TSourceList::Clear(Option_t *opt) {
	TCalibrationList::Clear();
	fNucleusName.clear();
}

Int_t TSourceList::SetNucleus(const TNucleus& nuc){
	fNucleusName = nuc.GetName();
	
	Int_t good_counter = 0;
	const TList * transition_list = nuc.GetTransitionList();
	TIter next(transition_list);
	TObject* transition = 0;
	std::cout << "Adding Transitions..." << std::endl;
	while((transition = next())){
		transition->Print();
		if(AddTransition(dynamic_cast<TTransition*>(transition))) good_counter++;
	}
	return good_counter;
}

bool TSourceList::AddTransition(TTransition *tran){
	if(!tran){
		std::cout << "Trying to add a bad transition" << std::endl;
		return false;
	}

	AddPoint(TCalibrationPoint(tran->GetEnergy(), tran->GetIntensity(), tran->GetEnergyUncertainty(), tran->GetIntensityUncertainty()));

	return true;
}

