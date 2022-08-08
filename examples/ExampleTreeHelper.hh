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
	ROOT::RDF::RResultPtr<TList> Book(ROOT::RDataFrame* d) override {
      return d->Book<TGriffin, TGriffinBgo, TZeroDegree, TSceptar>(std::move(*this), {"TGriffin", "TGriffinBgo", "TZeroDegree", "TSceptar"});
	}
	void CreateHistograms(unsigned int slot);
	void Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo, TZeroDegree& zds, TSceptar& scep);

private:
	// branches of output trees
	std::vector<double> fSuppressedAddback; ///< vector of suppressed addback energies
	std::vector<double> fBetaGammaTiming; ///< vector of beta-gamma timing
};

extern "C" ExampleTreeHelper* CreateHelper(TList* list) { return new ExampleTreeHelper(list); }

extern "C" void DestroyHelper(TGRSIHelper* helper) { delete helper; }


/*! @} */
#endif
