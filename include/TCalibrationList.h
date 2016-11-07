#ifndef TCALIBRATIONLIST_H__
#define TCALIBRATIONLIST_H__

/** \addtogroup Calibration
 *  @{
 */

#include "TNamed.h"
#include "TCalibrationPoint.h"
#include "TGraphErrors.h"

#include <map>

class TCalibrationList : public TNamed {
 public: 
   TCalibrationList();
   TCalibrationList(const char*name, const char* title ="");
   virtual ~TCalibrationList(); 

   TCalibrationList(const TCalibrationList &copy);

 public:
	void AddPoint(const TCalibrationPoint& pt);
	Int_t NPoints() const { return fCalList.size(); }
	void FillGraph(TGraph *graph) const;

   virtual void Copy(TObject &obj) const;
   virtual void Clear(Option_t *opt = "");
   virtual void Print(Option_t *opt = "") const;

 private:
	std::map<UInt_t,TCalibrationPoint> fCalList;
   
/// \cond CLASSIMP
   ClassDef(TCalibrationList,1);
/// \endcond

};
/*! @} */
#endif
