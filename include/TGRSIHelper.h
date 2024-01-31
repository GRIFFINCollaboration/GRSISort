#ifndef TGRSIHELPER_H
#define TGRSIHELPER_H
#include "ROOT/RDataFrame.hxx"
#include "TObject.h"
#include "TList.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TTree.h"
#include "TCutG.h"
#include "TBufferFile.h"

#include "TGRSIOptions.h"
#include "TAnalysisOptions.h"
#include "GHSym.h"
#include "GCube.h"
#include "GValue.h"
#include "TPPG.h"
#include "TRunInfo.h"
#include "TGRSIMap.h"
#include "TChannel.h"

// 1 GB size limit for objects in ROOT
#define SIZE_LIMIT 1073741822

class TGRSIHelper : public TObject {
protected:
   std::vector<std::shared_ptr<std::map<std::string, TList>>> fLists;   //!<! one map of lists and directories per data processing slot to hold all output objects
	std::vector<TGRSIMap<std::string, TH1*>> fH1; //!<! one map per data processing slot for 1D histograms
   std::vector<TGRSIMap<std::string, TH2*>> fH2; //!<! one map per data processing slot for 2D histograms
   std::vector<TGRSIMap<std::string, TH3*>> fH3; //!<! one map per data processing slot for 3D histograms
   std::vector<TGRSIMap<std::string, GHSym*>> fSym; //!<! one map per data processing slot for GRSISort's symmectric 2D histograms
   std::vector<TGRSIMap<std::string, GCube*>> fCube; //!<! one map per data processing slot for GRSISort's 3D histograms
   std::vector<TGRSIMap<std::string, TTree*>> fTree; //!<! one map per data processing slot for trees
   std::vector<TGRSIMap<std::string, TObject*>> fObject; //!<! one map per data processing slot for any TObjects
	TPPG*     fPpg{nullptr};     //!<! pointer to the PPG
	TRunInfo* fRunInfo{nullptr}; //!<! pointer to the run info
	std::map<std::string, TCutG*>      fCuts; //!<! map of cuts
	std::string fPrefix{"TGRSIHelper"}; //!<! name of this action (used as prefix)

private:
	void CheckSizes(unsigned int slot, const char* usage);

public:
   /// This type is a requirement for every helper.
   using Result_t = std::map<std::string, TList>;
 
   TGRSIHelper(TList* input);

	/// This function builds the vectors of TLists and maps for 1D- and 2D-histograms.
	/// It calls the overloaded CreateHistograms functions in which the user can define
	/// their histograms. Then it adds all those histograms to the list of the corresponding slot.
	virtual void Setup();
	/// Virtual helper function that the user uses to create their histograms
	virtual void CreateHistograms(unsigned int) {
		std::cout<<this<<" - "<<__PRETTY_FUNCTION__<<", "<<Prefix()<<": This function should not get called, the user's code should replace it. Not creating any histograms!"<<std::endl;
	}
	/// This method will call the Book action on the provided dataframe
	virtual ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame*) {
		std::cout<<this<<" - "<<__PRETTY_FUNCTION__<<", "<<Prefix()<<": This function should not get called, the user's code should replace it. Returning empty list!"<<std::endl; 
		return ROOT::RDF::RResultPtr<std::map<std::string, TList>>();
	}

	TGRSIHelper(TGRSIHelper &&) = default;
	TGRSIHelper(const TGRSIHelper &) = delete;
	std::shared_ptr<std::map<std::string, TList>> GetResultPtr() const { return fLists[0]; }
	void InitTask(TTreeReader *, unsigned int) {}
	void Initialize() {} // required method, gets called once before starting the event loop
	/// This required method is called at the end of the event loop. It is used to merge all the internal TLists which
	/// were used in each of the data processing slots.
	void Finalize();

	/// This method gets called at the end of Finalize()
	virtual void EndOfSort(std::shared_ptr<std::map<std::string, TList>>) {}

	std::string Prefix() const { return fPrefix; }
	void Prefix(const std::string& val) { fPrefix = val; }
	std::string GetActionName() const { return Prefix(); } // apparently a required function (not documented but doesn't compile w/o it)
};

#endif
