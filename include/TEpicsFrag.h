#ifndef TEPICSFRAG_H
#define TEPICSFRAG_H

#include "Globals.h"

#include <vector>
#include <time.h>

#include "Rtypes.h"
#include "TObject.h"

//#if !defined (__CINT__) && !defined (__CLING__)
//#include "Globals.h"
//#endif

////////////////////////////////////////////////////////////////
//                                                            //
// TEpicsFrag                                                 //
//                                                            //
// This Class should contain all the information found in     //
// NOT typeid 1 midas events. aka Epics (scaler) Events.      //
//                                                            //
//                                                            //
////////////////////////////////////////////////////////////////

class TEpicsFrag : public TObject	{
  public:
    TEpicsFrag(); 
    virtual ~TEpicsFrag(); 

    time_t   fMidasTimeStamp;       //->  Timestamp of the MIDAS event  
    Int_t    fMidasId;              //->  MIDAS ID

    std::vector<float>       fData;  //The data in the scaler
    std::vector<std::string> fName; //The name of hte scaler
    std::vector<std::string> fUnit; //The Scaler unit

    int GetSize() { return fData.size(); }
    inline float GetData(const unsigned int &i) { 
			 if(i >= fData.size())
				 return fData.back();
			 else return fData[i];
	 }

    virtual void Clear(Option_t *opt = ""); //!<!
    virtual void Print(Option_t *opt = "") const; //!<!
    
/// \cond CLASSIMP
    ClassDef(TEpicsFrag,1);  // Scaler Fragments
/// \endcond
};
#endif // TEPICSFRAG_H
