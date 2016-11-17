#include "TCalPoint.h"

#include <iostream>

/// \cond CLASSIMP
ClassImp(TCalPoint)
/// \endcond

TCalPoint::TCalPoint(){
	Clear();
}

TCalPoint::TCalPoint(const Double_t& centroid, const Double_t &area, const Double_t &dcentroid, const Double_t &darea){
	SetCentroid(centroid,dcentroid);
	SetArea(area,darea);
}

TCalPoint::~TCalPoint(){}

TCalPoint::TCalPoint(const TCalPoint& copy) : TObject(copy){
   copy.Copy(*this);
}

void TCalPoint::Copy(TObject& obj) const {
	TObject::Copy(obj);
	static_cast<TCalPoint&>(obj).fCentroid 		= fCentroid;
	static_cast<TCalPoint&>(obj).fCentroidErr 	= fCentroidErr;
	static_cast<TCalPoint&>(obj).fArea 			= fArea;
	static_cast<TCalPoint&>(obj).fAreaErr 		= fAreaErr;
}

void TCalPoint::SetPoint(const Double_t& centroid, const Double_t &area, const Double_t &dcentroid, const Double_t &darea){
	SetCentroid(centroid,dcentroid);
	SetArea(area,darea);
}

void TCalPoint::SetCentroid(const Double_t& centroid, const Double_t& dcentroid){
	fCentroid 		= centroid;
	fCentroidErr 	= dcentroid;
}

void TCalPoint::SetArea(const Double_t& area, const Double_t& darea){
	fArea 		= area;
	fAreaErr 	= darea;
}

void TCalPoint::Print(Option_t *opt) const {
	std::cout << "Centroid: " << fCentroid << " +/- " << fCentroidErr << std::endl;
	std::cout << "    Area: " << fArea 		<< " +/- " << fAreaErr << std::endl;
}

void TCalPoint::Clear(Option_t *opt) {
	fCentroid 		= 0.0;
	fArea				= 0.0;
	fCentroidErr	= 0.0;
	fAreaErr			= 0.0;
}
