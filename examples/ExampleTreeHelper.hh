#ifndef ExampleTreeHelper_h
#define ExampleTreeHelper_h

/** \addtogroup Helpers
 * @{
 */

////////////////////////////////////////////////////////////////////////////////
/// \class ExampleTreeHelper
///
/// This selector shows how to create a tree with selected events (beta-tagged
/// with gamma multiplicities of at least three), and selected information 
/// (suppressed addback energies, and beta-gamma timing differences).
/// 
////////////////////////////////////////////////////////////////////////////////

// Header file for the classes stored in the TTree if any.
#include "TGRSIHelper.h"
#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TSceptar.h"
#include "TZeroDegree.h"

// Fixed size dimensions of array or collections stored in the TTree if any.

class ExampleTreeHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<ExampleTreeHelper> {
public :
	ExampleTreeHelper(TList* list) : TGRSIHelper(list) {
		Prefix("ExampleTree");
		Setup();
	}
	//These functions are expected to exist
	ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override {
      return d->Book<TGriffin, TGriffinBgo, TZeroDegree, TSceptar>(std::move(*this), {"TGriffin", "TGriffinBgo", "TZeroDegree", "TSceptar"});
	}
	void CreateHistograms(unsigned int slot);
	void Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo, TZeroDegree& zds, TSceptar& scep);

private:
	// branches of output trees
	// all of these need to be maps for the different slots/workers, we're using maps as they don't need to be resized and once accessed
	// the address of key stays the same (important to be able to create branches)
	std::map<unsigned int, double*> fSuppressedAddback2; ///< vector of suppressed addback energies
	std::map<unsigned int, std::vector<double>> fSuppressedAddback; ///< vector of suppressed addback energies
	std::map<unsigned int, std::vector<double>> fBetaGammaTiming; ///< vector of beta-gamma timing
	std::map<unsigned int, int> fGriffinMultiplicity; ///< multiplicity of suppressed addback energies
};

extern "C" ExampleTreeHelper* CreateHelper(TList* list) { return new ExampleTreeHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }


/*! @} */
#endif
