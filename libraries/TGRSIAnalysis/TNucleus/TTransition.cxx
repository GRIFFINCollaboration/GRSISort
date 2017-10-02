#include "TTransition.h"

/// \cond CLASSIMP
ClassImp(TTransition)
/// \endcond

TTransition::TTransition()
{
	Clear();
}

TTransition::~TTransition()
{
	// empty
}

void TTransition::Clear(Option_t*)
{
	fEnergy         = 0;
	fEngUncertainty = 0;
	fIntensity      = 0;
	fIntUncertainty = 0;
}

void TTransition::Print(Option_t*) const
{

	if(!std::isnan(fEngUncertainty)) {
		printf("Energy:    %.02f +/- %.02f", fEnergy, fEngUncertainty);
	} else {
		printf("Energy:    %.02f ", fEnergy);
	}
	if(!std::isnan(fIntensity)) {
		if(!std::isnan(fIntUncertainty)) {
			printf("\tIntensity: %.02f +/- %.02f\n", fIntensity, fIntUncertainty);
		} else {
			printf("\tIntensity: %.02f \n", fEnergy);
		}
	} else {
		printf("\n");
	}

	// printf("**************************\n");
}

std::string TTransition::PrintToString()
{
	std::string toString;
	toString.append(Form("%f\t", fEnergy));
	toString.append(Form("%f\t", fEngUncertainty));
	toString.append(Form("%f\t", fIntensity));
	toString.append(Form("%f\t", fIntUncertainty));

	return toString;
}

int TTransition::CompareIntensity(const TObject* obj) const
{
	if(fIntensity > static_cast<const TTransition*>(obj)->fIntensity) {
		return -1;
	}
	if(fIntensity == static_cast<const TTransition*>(obj)->fIntensity) {
		return 0;
	}
	return 1;

	return -9;
}

int TTransition::Compare(const TObject* obj) const
{

	return CompareIntensity(obj);

	// Compares the intensities of the TTransitions and returns
	if(fEnergy > static_cast<const TTransition*>(obj)->fEnergy) {
		return -1;
	}
	if(fEnergy == static_cast<const TTransition*>(obj)->fEnergy) {
		return 0;
	}
	return 1;

	return -9;
}
