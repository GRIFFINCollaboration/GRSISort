#ifndef TCALIBRATIONPOINT_H
#define TCALIBRATIONPOINT_H

/** \addtogroup Calibration
 *  @{
 */

#include "TObject.h"
#include "TPeak.h"

class TCalPoint : public TObject {
public:
   TCalPoint();
   TCalPoint(const Double_t& centroid, const Double_t& area, const Double_t& dcentroid = 0.0, const Double_t& darea = 0.0);
   explicit TCalPoint(const TPeak& peak);
   TCalPoint(const TCalPoint&);
   TCalPoint(TCalPoint&&) noexcept            = default;
   TCalPoint& operator=(const TCalPoint&)     = default;
   TCalPoint& operator=(TCalPoint&&) noexcept = default;
   ~TCalPoint()                               = default;

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
   Double_t fCentroid{0.};
   Double_t fCentroidErr{0.};
   Double_t fArea{0.};
   Double_t fAreaErr{0.};

   /// \cond CLASSIMP
   ClassDefOverride(TCalPoint, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
