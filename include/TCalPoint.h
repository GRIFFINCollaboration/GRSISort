#ifndef TCALIBRATIONPOINT_H__
#define TCALIBRATIONPOINT_H__

/** \addtogroup Calibration
 *  @{
 */

#include "TObject.h"
#include "TPeak.h"

class TCalPoint : public TObject {
public:
   TCalPoint();
   TCalPoint(const Double_t& centroid, const Double_t& area, const Double_t& dcentroid = 0.0,
             const Double_t& darea = 0.0);
   TCalPoint(const TPeak& peak);
   ~TCalPoint() override;

   TCalPoint(const TCalPoint& copy);

public:
   void Copy(TObject& obj) const override;
   void SetPoint(const Double_t& centroid, const Double_t& area, const Double_t& dcentroid = 0.0,
                 const Double_t& darea = 0.0);
   void SetPoint(const TPeak* peak);
   void SetCentroid(const Double_t& centroid, const Double_t& dcentroid = 0.0);
   void SetArea(const Double_t& area, const Double_t& darea = 0.0);

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   Double_t Centroid() const { return fCentroid; }
   Double_t Area() const { return fArea; }
   Double_t CentroidErr() const { return fCentroidErr; }
   Double_t AreaErr() const { return fAreaErr; }

private:
   Double_t fCentroid{};
   Double_t fCentroidErr{};
   Double_t fArea{};
   Double_t fAreaErr{};

   /// \cond CLASSIMP
   ClassDefOverride(TCalPoint, 1);
   /// \endcond
};
/*! @} */
#endif
