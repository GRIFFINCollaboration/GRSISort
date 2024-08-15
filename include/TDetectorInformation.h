#ifndef TDETECTORINFORMATION_H
#define TDETECTORINFORMATION_H

#include <iostream>

#include "TObject.h"

#include "TEventBuildingLoop.h"

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TDetectorInformation
/// Class containing setup specific detector information.
///
/////////////////////////////////////////////////////////////////

class TDetectorInformation : public TObject {
public:
   TDetectorInformation()  = default;
	TDetectorInformation(const TDetectorInformation&) = default;
	TDetectorInformation(TDetectorInformation&&) noexcept = default;
	TDetectorInformation& operator=(const TDetectorInformation&) = default;
	TDetectorInformation& operator=(TDetectorInformation&&) noexcept = default;
   ~TDetectorInformation() = default;

   virtual void                           Set(){};                                                                 ///< Set the detector information based on the available TChannels
   virtual TEventBuildingLoop::EBuildMode BuildMode() const { return TEventBuildingLoop::EBuildMode::kDefault; }   ///< Select build mode based on available detectors

   void Print(Option_t* = "") const override { std::cout << "Default detector information, i.e. none" << std::endl; };
   void Clear(Option_t* = "") override{};

   /// \cond CLASSIMP
   ClassDefOverride(TDetectorInformation, 1) // NOLINT
   /// \endcond
};
/*! @} */
#endif
