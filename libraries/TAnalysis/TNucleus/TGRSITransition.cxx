#include "TGRSITransition.h"

/// \cond CLASSIMP
ClassImp(TGRSITransition)
/// \endcond

TGRSITransition::TGRSITransition()
{
	// Default constructor for TGRSITransition
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TGRSITransition::~TGRSITransition()
{
	// Default Destructor
}

void TGRSITransition::Print(Option_t*) const
{
	// Prints information about the TGRSITransition
	printf("**************************\n");
	printf("TGRSITransition:\n");
	printf("Energy:    %lf\t+/-%lf\n", fEnergy, fEnergyUncertainty);
	printf("Intensity: %lf\t+/-%lf\n", fIntensity, fIntensityUncertainty);
	printf("**************************\n");
}

std::string TGRSITransition::PrintToString()
{
	// Writes transitions in a way that is nicer to ourput.
	std::string buffer;
	buffer.append(Form("%lf\t", fEnergy));
	buffer.append(Form("%lf\t", fEnergyUncertainty));
	buffer.append(Form("%lf\t", fIntensity));
	buffer.append(Form("%lf\t", fIntensityUncertainty));
	return buffer;
}

void TGRSITransition::Clear(Option_t*)
{
	// Clears TGRSITransition
	fEnergy               = 0.;
	fEnergyUncertainty    = 0.;
	fIntensity            = 0.;
	fIntensityUncertainty = 0.;
}

int TGRSITransition::Compare(const TObject* obj) const
{
	// Compares the intensities of the TGRSITransitions and returns
	//-1 if this >  obj
	// 0 if  this == obj
	// 1 if  this <  obj
	if(fIntensity > static_cast<const TGRSITransition*>(obj)->fIntensity) {
		return -1;
	}
	if(fIntensity == static_cast<const TGRSITransition*>(obj)->fIntensity) {
		return 0;
	} //(fIntensity < static_cast<const TGRSITransition*>(obj)->fIntensity)
	return 1;

	printf("%s: Error, intensity neither greater, nor equal, nor smaller than provided intensity!\n",
			__PRETTY_FUNCTION__);
	return -9;
}
