#include "TCalibrationPoint.h"

#include <iostream>

/// \cond CLASSIMP
ClassImp(TCalibrationPoint)
/// \endcond

TCalibrationPoint::TCalibrationPoint(){
	Clear();
}

TCalibrationPoint::TCalibrationPoint(const Double_t& centroid, const Double_t &area, const Double_t &dcentroid, const Double_t &darea){
	SetCentroid(centroid,dcentroid);
	SetArea(area,darea);
}

TCalibrationPoint::~TCalibrationPoint(){}

TCalibrationPoint::TCalibrationPoint(const TCalibrationPoint& copy) : TObject(copy){
   copy.Copy(*this);
}

void TCalibrationPoint::Copy(TObject& obj) const {
	TObject::Copy(obj);
	static_cast<TCalibrationPoint&>(obj).fCentroid 		= fCentroid;
	static_cast<TCalibrationPoint&>(obj).fCentroidErr 	= fCentroidErr;
	static_cast<TCalibrationPoint&>(obj).fArea 			= fArea;
	static_cast<TCalibrationPoint&>(obj).fAreaErr 		= fAreaErr;
}

void TCalibrationPoint::SetPoint(const Double_t& centroid, const Double_t &area, const Double_t &dcentroid, const Double_t &darea){
	SetCentroid(centroid,dcentroid);
	SetArea(area,darea);
}

void TCalibrationPoint::SetCentroid(const Double_t& centroid, const Double_t& dcentroid){
	fCentroid 		= centroid;
	fCentroidErr 	= dcentroid;
}

void TCalibrationPoint::SetArea(const Double_t& area, const Double_t& darea){
	fArea 		= area;
	fAreaErr 	= darea;
}

void TCalibrationPoint::Print(Option_t *opt) const {
	std::cout << "Centroid: " << fCentroid << " +/- " << fCentroidErr << std::endl;
	std::cout << "    Area: " << fArea 		<< " +/- " << fAreaErr << std::endl;
}

void TCalibrationPoint::Clear(Option_t *opt) {
	fCentroid 		= 0.0;
	fArea				= 0.0;
	fCentroidErr	= 0.0;
	fAreaErr			= 0.0;
}

