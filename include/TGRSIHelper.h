#ifndef TGRSIHELPER_H
#define TGRSIHELPER_H
#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 14, 0)
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
#include "TUserSettings.h"

////////////////////////////////////////////////////////////////////////////////
///
/// \class TGRSIHelper
///
/// Base class for all helpers used in grsiframe.
/// It provides some general members that are set from the input list, like
/// TPPG, run info, and user settings. It also loads settings from the input
/// list into general GRSISort variables, like the analysis options, g-value
/// files, cut files, or calibration files.
///
////////////////////////////////////////////////////////////////////////////////

class TGRSIHelper : public TObject {
public:
   std::string& Prefix() { return fPrefix; }

protected:
   TPPG*          Ppg() { return fPpg; }
   TRunInfo*      RunInfo() { return fRunInfo; }
   TUserSettings* UserSettings() { return fUserSettings; }

   std::vector<std::shared_ptr<std::map<std::string, TList>>> fLists;                   // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map of lists and directories per data processing slot to hold all output objects
   std::vector<TGRSIMap<std::string, TH1*>>                   fH1;                      // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for 1D histograms
   std::vector<TGRSIMap<std::string, TH2*>>                   fH2;                      // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for 2D histograms
   std::vector<TGRSIMap<std::string, TH3*>>                   fH3;                      // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for 3D histograms
   std::vector<TGRSIMap<std::string, GHSym*>>                 fSym;                     // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for GRSISort's symmectric 2D histograms
   std::vector<TGRSIMap<std::string, GCube*>>                 fCube;                    // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for GRSISort's 3D histograms
   std::vector<TGRSIMap<std::string, TTree*>>                 fTree;                    // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for trees
   std::vector<TGRSIMap<std::string, TObject*>>               fObject;                  // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for any TObjects
   std::map<std::string, TCutG*>                              fCuts;                    // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! map of cuts
   TPPG*                                                      fPpg{nullptr};            // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! pointer to the PPG
   TRunInfo*                                                  fRunInfo{nullptr};        // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! pointer to the run info
   TUserSettings*                                             fUserSettings{nullptr};   // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! pointer to the user settings
   std::string                                                fPrefix{"TGRSIHelper"};   // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! name of this action (used as prefix)

private:
   static constexpr int fSizeLimit = 1073741822;   //!<! 1 GiB size limit for objects in ROOT
   void                 CheckSizes(unsigned int slot, const char* usage);

public:
   /// This type is a requirement for every helper.
   using Result_t = std::map<std::string, TList>;

   explicit TGRSIHelper(TList* input);

   /// This function builds the vectors of TLists and maps for 1D- and 2D-histograms.
   /// It calls the overloaded CreateHistograms functions in which the user can define
   /// their histograms. Then it adds all those histograms to the list of the corresponding slot.
   virtual void Setup();
   /// Virtual helper function that the user uses to create their histograms
   virtual void CreateHistograms(unsigned int)
   {
      std::cout << this << " - " << __PRETTY_FUNCTION__ << ", " << Prefix() << ": This function should not get called, the user's code should replace it. Not creating any histograms!" << std::endl;   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   /// This method will call the Book action on the provided dataframe
   virtual ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame*)
   {
      std::cout << this << " - " << __PRETTY_FUNCTION__ << ", " << Prefix() << ": This function should not get called, the user's code should replace it. Returning empty list!" << std::endl;   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      return {};
   }

   TGRSIHelper(const TGRSIHelper&)            = delete;
   TGRSIHelper(TGRSIHelper&&)                 = default;
   TGRSIHelper& operator=(const TGRSIHelper&) = delete;
   TGRSIHelper& operator=(TGRSIHelper&&)      = default;
   ~TGRSIHelper()                             = default;
   std::shared_ptr<std::map<std::string, TList>> GetResultPtr() const { return fLists[0]; }
   void                                          InitTask(TTreeReader*, unsigned int) {}
   void                                          Initialize() {}   // required method, gets called once before starting the event loop
   /// This required method is called at the end of the event loop. It is used to merge all the internal TLists which
   /// were used in each of the data processing slots.
   void Finalize();

   /// This method gets called at the end of Finalize()
   virtual void EndOfSort(std::shared_ptr<std::map<std::string, TList>>&) {}

   std::string Prefix() const { return fPrefix; }
   void        Prefix(const std::string& val) { fPrefix = val; }
   std::string GetActionName() const { return Prefix(); }   // apparently a required function (not documented but doesn't compile w/o it)
};

#endif
#endif
