#ifndef TEPICSFRAG_H
#define TEPICSFRAG_H

/** \addtogroup Sorting
 *  @{
 */

#include "Globals.h"

#include <vector>
#include <ctime>
#include <map>

#include "Rtypes.h"
#include "TObject.h"
#include "TTree.h"

//#if !defined (__CINT__) && !defined (__CLING__)
//#include "Globals.h"
//#endif

/////////////////////////////////////////////////////////////////
///
/// \class TEpicsFrag
///
/// This Class should contain all the information found in
/// Epics (scaler) events.
///
/////////////////////////////////////////////////////////////////

class TEpicsFrag : public TObject {
public:
   TEpicsFrag();
   ~TEpicsFrag() override;

   time_t fDaqTimeStamp; //->  Timestamp of the daq event
   Int_t  fDaqId;        //->  daq ID

   std::vector<float>       fData; ///<The data in the scaler
   std::vector<std::string> fName; ///<The name of the scaler

   int          GetSize() { return fData.size(); }
   inline float GetData(const unsigned int& i)
   {
      if(i >= fData.size()) {
         return fData.back();
      } else {
         return fData[i];
      }
   }

   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!

   static void AddEpicsVariable(const char* name);
   static void SetEpicsNameList(const std::vector<std::string>& name_vec);
   static std::string GetEpicsVariableName(const int& i);
   static void PrintVariableNames();

   static void BuildScalerMap(TTree* tree);
   static void BuildScalerMap();
   static void PrintScalerMap();

   static TEpicsFrag* GetScalerAtTime(Long64_t time);

private:
   static std::vector<std::string> fNameList; // This stuff should potentially move to a run info of some sort
   static std::map<Long64_t, TEpicsFrag> fScalerMap;
   static Long64_t fSmallestTime;

   /// \cond CLASSIMP
   ClassDefOverride(TEpicsFrag, 2); // Scaler Fragments
   /// \endcond
};
/*! @} */
#endif // TEPICSFRAG_H
