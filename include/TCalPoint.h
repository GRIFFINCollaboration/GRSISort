#ifndef TCALIBRATIONPOINT_H__
#define TCALIBRATIONPOINT_H__

/** \addtogroup Calibration
 *  @{
 */

#include "TObject.h"

class TCalPoint : public TObject {
 public: 
   TCalPoint();
   TCalPoint(const Double_t &centroid, const Double_t &area, const Double_t &dcentroid=0.0, const Double_t &dArea=0.0);
   virtual ~TCalPoint(); 

   TCalPoint(const TCalPoint &copy);

 public:
   void Copy(TObject &obj) const;
   void SetPoint(const Double_t &centroid, const Double_t &area, const Double_t &dcentroid=0.0, const Double_t &dArea=0.0);
	void SetCentroid(const Double_t &centroid, const Double_t &dcentroid =0.0); 
	void SetArea(const Double_t &area, const Double_t &darea =0.0); 

   void Clear(Option_t *opt = "");
   void Print(Option_t *opt = "") const;

	Double_t Centroid() const 		{ return fCentroid; 		}
	Double_t Area() const 			{ return fArea; 			}
	Double_t CentroidErr() const 	{ return fCentroidErr; 	}
	Double_t AreaErr() const 		{ return fAreaErr; 		}

 private:
	Double_t fCentroid;
	Double_t fCentroidErr;
	Double_t fArea;
	Double_t fAreaErr;
   
/// \cond CLASSIMP
   ClassDef(TCalPoint,1);
/// \endcond

};
/*! @} */
#endif
