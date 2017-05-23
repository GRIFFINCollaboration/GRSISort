#ifndef TEPICSFRAG_H
#define TEPICSFRAG_H

/** \addtogroup Sorting
 *  @{
 */

#include "Globals.h"

#include <vector>
#include <time.h>
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
/// NOT typeid 1 midas events. aka Epics (scaler) Events.
///
/////////////////////////////////////////////////////////////////

class TEpicsFrag : public TObject {
public:
   TEpicsFrag();
   virtual ~TEpicsFrag();

   time_t fMidasTimeStamp; //->  Timestamp of the MIDAS event
   Int_t  fMidasId;        //->  MIDAS ID

   std::vector<float>       fData; ///<The data in the scaler
   std::vector<std::string> fName; ///<The name of the scaler

   int          GetSize() { return fData.size(); }
   inline float GetData(const unsigned int& i)
   {
      if (i >= fData.size())
         return fData.back();
      else
         return fData[i];
   }

   virtual void Clear(Option_t* opt = "");       //!<!
   virtual void Print(Option_t* opt = "") const; //!<!

   static void AddEpicsVariable(const char* name);
   static void SetEpicsNameList(const std::vector<std::string>& name_list);
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
   ClassDef(TEpicsFrag, 1); // Scaler Fragments
   /// \endcond
};
/*! @} */
#endif // TEPICSFRAG_H
