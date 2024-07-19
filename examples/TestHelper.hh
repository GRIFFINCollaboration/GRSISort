// TODO: Replace all EXAMPLEVENT and Test with the name you want to use for this helper action
#ifndef TESTHELPER_HH
#define TESTHELPER_HH

#include "TGRSIHelper.h"

// TODO: edit these include statments to match the detectors you want to use!
#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TZeroDegree.h"
#include "TSceptar.h"

// This is a custom action which respects a well defined interface. It supports parallelism,
// in the sense that it behaves correctly if implicit multi threading is enabled.
// Note the plural: in presence of a MT execution, internally more than a single TList is created.
// The detector types in the specifcation of Book must match those in the call to it as well as those in the Exec function (and be in the same order)!

class TestHelper : public TGRSIHelper, public ROOT::Detail::RDF::RActionImpl<TestHelper> {
public:
   // constructor sets the prefix (which is used for the output file as well)
   // and calls Setup which in turn also calls CreateHistograms
   explicit TestHelper(TList* list)
      : TGRSIHelper(list)
   {
      Prefix("TestHelper");
      Setup();
   }

   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      // TODO: edit the template specification and branch names to match the detectors you want to use!
      // return d->Define("TSceptar", []() { return TSceptar(); }).Book<TGriffin, TGriffinBgo, TZeroDegree, TSceptar>(std::move(*this), {"TGriffin", "TGriffinBgo", "TZeroDegree", "TSceptar"});
      auto colNames = d->GetColumnNames();
      for(auto&& colName : colNames) {
         if(colName.compare("TSceptar") == 0) {
            // found column with name "TSceptar" so we can just use it
            return d->Book<TGriffin, TGriffinBgo, TZeroDegree, TSceptar>(std::move(*this), {"TGriffin", "TGriffinBgo", "TZeroDegree", "TSceptar"});
         }
      }
      // none of the columns was "TSceptar", so we have to create an empty one
      return d->Define("TSceptar", []() { return TSceptar(); }).Book<TGriffin, TGriffinBgo, TZeroDegree, TSceptar>(std::move(*this), {"TGriffin", "TGriffinBgo", "TZeroDegree", "TSceptar"});
   }
   // this function creates and books all histograms
   void CreateHistograms(unsigned int slot) override;
   // this function gets called for every single event and fills the histograms
   // TODO: edit the function arguments to match the detectors you want to use!
   void Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo, TZeroDegree& zds, TSceptar& scep);
   // this function is optional and is called after the output lists off all slots/workers have been merged
   void EndOfSort(std::shared_ptr<std::map<std::string, TList>> list) override;

private:
   // any constants that are set in the CreateHistograms function and used in the Exec function can be stored here
   // or any other settings
   Long64_t fCycleLength{0};
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" TestHelper* CreateHelper(TList* list)
{
   return new TestHelper(list);
}

extern "C" void DestroyHelper(TGRSIHelper* helper)
{
   delete helper;
}

#endif
