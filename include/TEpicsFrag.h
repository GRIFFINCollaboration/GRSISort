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
   TEpicsFrag()                                 = default;
   TEpicsFrag(const TEpicsFrag&)                = default;
   TEpicsFrag(TEpicsFrag&&) noexcept            = default;
   TEpicsFrag& operator=(const TEpicsFrag&)     = default;
   TEpicsFrag& operator=(TEpicsFrag&&) noexcept = default;
   ~TEpicsFrag()                                = default;

   size_t       GetSize() const { return fData.size(); }
   inline float GetData(const unsigned int& index) const
   {
      if(index >= fData.size()) {
         return fData.back();
      }
      return fData[index];
   }

   void Clear(Option_t* opt = "") override;         //!<!
   void Print(Option_t* opt = "") const override;   //!<!

   static void        AddEpicsVariable(const char* name);
   static void        SetEpicsNameList(const std::vector<std::string>& names);
   static std::string GetEpicsVariableName(const int& index);
   static void        PrintVariableNames();

   static void BuildScalerMap(TTree* tree);
   static void BuildScalerMap();
   static void PrintScalerMap();

   static TEpicsFrag* GetScalerAtTime(Long64_t time);

   // setters
   void DaqTimeStamp(time_t val) { fDaqTimeStamp = val; }
   void DaqId(Int_t val) { fDaqId = val; }
   void Data(const std::vector<float>& val) { fData = val; }
   void Name(const std::vector<std::string>& val) { fName = val; }

   void AddData(float val) { fData.push_back(val); }
   void AddName(const std::string& val) { fName.push_back(val); }

   // getters
   time_t                   DaqTimeStamp() const { return fDaqTimeStamp; }
   Int_t                    DaqId() const { return fDaqId; }
   std::vector<float>       Data() const { return fData; }
   std::vector<std::string> Name() const { return fName; }

private:
   static std::vector<std::string>       fNameList;   // This stuff should potentially move to a run info of some sort
   static std::map<Long64_t, TEpicsFrag> fScalerMap;
   static Long64_t                       fSmallestTime;

   time_t fDaqTimeStamp{0};   //->  Timestamp of the daq event
   Int_t  fDaqId{-1};         //->  daq ID

   std::vector<float>       fData;   ///< The data in the scaler
   std::vector<std::string> fName;   ///< The name of the scaler

   /// \cond CLASSIMP
   ClassDefOverride(TEpicsFrag, 2)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif   // TEPICSFRAG_H
