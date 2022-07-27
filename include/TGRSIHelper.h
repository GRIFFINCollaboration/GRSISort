#ifndef TGRSIHELPER_H
#define TGRSIHELPER_H
#include "ROOT/RDataFrame.hxx"
#include "TObject.h"
#include "TList.h"
#include "TH1.h"
#include "TH2.h"

#include "TDetector.h"
#include "TPPG.h"
#include "TRunInfo.h"
#include "TGRSIMap.h"

class TGRSIHelper : public TObject {
private:
protected:
   std::vector<std::shared_ptr<TList>> fLists;   //!<! one list per data processing slot to hold all histograms
	std::vector<TGRSIMap<std::string, TH1*>> fH1; //!<! one map per data processing slot for all 1D histograms
   std::vector<TGRSIMap<std::string, TH2*>> fH2; //!<! one map per data processing slot for all 2D histograms
	TPPG*     fPpg{nullptr};     //!<! pointer to the PPG
	TRunInfo* fRunInfo{nullptr}; //!<! pointer to the run info
	std::vector<bool> fWarned{false}; //!<! flag per data processing slot to print a single warning if FillHistograms of this class is called 
	std::string fPrefix{"TGRSIHelper"}; //!<! name of this action (used as prefix)

public:
   /// This type is a requirement for every helper.
   using Result_t = TList;
 
   TGRSIHelper(TPPG* ppg) : fPpg(ppg) {}
   /// This function builds the vectors of TLists and maps for 1D- and 2D-histograms.
	/// It calls the overloaded CreateHistograms functions in which the user can define
	/// their histograms. Then it adds all those histograms to the list of the corresponding slot.
	virtual void Setup() {
		std::cout<<__PRETTY_FUNCTION__<<" calling setup of TGRSIHelper"<<std::endl;
		const auto nSlots = ROOT::IsImplicitMTEnabled() ? ROOT::GetThreadPoolSize() : 1;
		TH1::AddDirectory(false); // turns off warnings about multiple histograms with the same name because ROOT doesn't manage them anymore
		for(auto i : ROOT::TSeqU(nSlots)) {
			fLists.emplace_back(std::make_shared<TList>());
			fH1.emplace_back(TGRSIMap<std::string, TH1*>());
			fH2.emplace_back(TGRSIMap<std::string, TH2*>());
			CreateHistograms(i);
			for(auto it : fH1[i]) {
				fLists[i]->Add(it.second);
			}
			for(auto it : fH2[i]) {
				fLists[i]->Add(it.second);
			}
			fWarned.emplace_back(false);
		}
		TH1::AddDirectory(true); // restores old behaviour
	}
	/// Virtual helper function that the user uses to create their histograms
	virtual void CreateHistograms(unsigned int) {
		std::cout<<this<<" - "<<__PRETTY_FUNCTION__<<", "<<Prefix()<<": This function should not get called, the user's code should replace it. Not creating any histograms!"<<std::endl;
	}
	/// This method will call the Book action on the provided dataframe
	virtual ROOT::RDF::RResultPtr<TList> Book(ROOT::RDataFrame*) {
		std::cout<<this<<" - "<<__PRETTY_FUNCTION__<<", "<<Prefix()<<": This function should not get called, the user's code should replace it. Returning empty list!"<<std::endl; 
		return ROOT::RDF::RResultPtr<TList>();
	}

   TGRSIHelper(TGRSIHelper &&) = default;
   TGRSIHelper(const TGRSIHelper &) = delete;
   std::shared_ptr<TList> GetResultPtr() const { return fLists[0]; }
   void InitTask(TTreeReader *, unsigned int) {}
	void Initialize() {} // required method, gets called once before starting the event loop
   /// This required method is called at the end of the event loop. It is used to merge all the internal TLists which
   /// were used in each of the data processing slots.
   void Finalize() {
		std::cout<<std::endl<<__PRETTY_FUNCTION__<<" calling Finalize of TGRSIHelper - "<<Prefix()<<std::endl;
		auto &res = fLists[0];
		for (auto slot : ROOT::TSeqU(1, fLists.size())) {
			for(const auto&& obj : *res) {
				if(obj->InheritsFrom(TH1::Class())) {
					if(fLists[slot]->FindObject(obj->GetName()) != nullptr) {
						static_cast<TH1*>(obj)->Add(static_cast<TH1*>(fLists[slot]->FindObject(obj->GetName())));
					} else {
						std::cerr<<"Failed to find object '"<<obj->GetName()<<"' in "<<slot<<". list"<<std::endl;
					}
				} else {
					std::cerr<<"Object '"<<obj->GetName()<<"' is not a histogram ("<<obj->ClassName()<<"), don't know what to do with it!"<<std::endl;
				}
			}
		}
	}

	std::string Prefix() const { return fPrefix; }
	void Prefix(const std::string& val) { fPrefix = val; }
	std::string GetActionName() const { return Prefix(); } // apparently a required function (not documented but doesn't compile w/o it)
};

#endif
