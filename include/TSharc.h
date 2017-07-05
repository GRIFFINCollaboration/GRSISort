#ifndef TSHARC_H
#define TSHARC_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>
#include <map>
#include <set>

#include "TMath.h"
#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TSharcHit.h"

class TSharc : public TGRSIDetector {
public:
   TSharc();
   ~TSharc() override;
   TSharc(const TSharc& rhs);

public:
   TSharcHit* GetSharcHit(const int& i);
   TGRSIDetectorHit* GetHit(const int& idx);
   static TVector3 GetPosition(int detector, int frontstrip, int backstrip, double X = 0.00, double Y = 0.00,
                               double Z = 0.00); //!<!
   static double   GetXOffset() { return fXoffset; }
   static double   GetYOffset() { return fYoffset; }
   static double   GetZOffset() { return fZoffset; }
   static TVector3 GetOffset() { return TVector3(fXoffset, fYoffset, fZoffset); }
   static void SetXYZOffset(const double x, const double y, const double z)
   {
      fXoffset = x;
      fYoffset = y;
      fZoffset = z;
   }

   int     GetSize() const { return fSharcHits.size(); } //!<!
   Short_t GetMultiplicity() const { return fSharcHits.size(); }

   void Copy(TObject&) const override;        //!<!
   void Clear(Option_t* = "") override;       //!<!
   void Print(Option_t* = "") const override; //!<!

   TSharc& operator=(const TSharc& rhs)
   {
      if(this != &rhs) {
         rhs.Copy(*this);
      }
      return *this;
   } //!<!

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
   void BuildHits() override;

private:
   std::vector<TSharcHit> fSharcHits;
   int                    CombineHits(TSharcHit*, TSharcHit*, int, int);       //!<!
   void                   RemoveHits(std::vector<TSharcHit>*, std::set<int>*); //!<!

   // TODO: I think this class could be sped up with a flag method on these transient vectors.
   std::vector<TFragment> fFrontFragments; //!
   std::vector<TFragment> fBackFragments;  //!
   std::vector<TFragment> fPadFragments;   //!

public:
   // static helper functions to make analysis easier.
   // static double GetReconstructedEnergy(TVector3 pos, int det, double edel, double epad=0.0, char ion='p');//!
   // static double GetReconstructedEnergy(int det, int fs, int bs, double edel, double epad=0.0, char ion='p')
   //{ return GetReconstructedEnergy(GetPosition(det,fs,bs),det,edel,epad,ion); } //!

   // takes ekin and returns edel and epad (NB detector thicknesses must be correct!)
   // static std::vector<double> GetMeasuredEnergy(TVector3 pos, int det, double ekin, char ion='p', Option_t *opt="",
   // double edel=-1.0);//!
   // static std::vector<double> GetMeasuredEnergy(int det, int fs, int bs, double ekin, char ion='p', Option_t *opt="",
   // double edel=-1.0)
   //      {return GetMeasuredEnergy(GetPosition(det,fs,bs),det,ekin,ion,opt,edel); } //!

   static double GetDetectorThickness(TSharcHit& hit, double dist = -1.0); //!
   static double GetDeadLayerThickness(TSharcHit& hit);                    //!
   static double GetPadThickness(TSharcHit& hit);                          //!
   static double GetPadDeadLayerThickness(TSharcHit& hit);                 //!

private:
   static double fXoffset; //!<!
   static double fYoffset; //!<!
   static double fZoffset; //!<!

   // various sharc dimensions set in mm, taken from IOP SHARC white paper
   static double fXdim; //! total X dimension of all boxes
   static double fYdim; //! total Y dimension of all boxes
   static double fZdim; //! total Z dimension of all boxes
   static double fRdim; //! Rmax-Rmin for all QQQs
   static double fPdim; //! QQQ quadrant angular range (degrees)
   // BOX dimensions
   static double fXposUB; //!
   static double fYminUB; //!
   static double fZminUB; //!
   static double fXposDB; //!
   static double fYminDB; //!
   static double fZminDB; //!
   // QQQ dimensions
   static double fZposUQ; //!
   static double fRmaxUQ; //!
   static double fRminUQ; //!
   static double fPminUQ; //! degrees
   static double fZposDQ; //!
   static double fRmaxDQ; //!
   static double fRminDQ; //!
   static double fPminDQ; //! degrees
   // segmentation
   // static const int frontstripslist[16]   ;
   // static const int backstripslist[16]    ;
   // pitches
   static double fStripFPitch;  //!
   static double fStripBPitch;  //!
   static double fRingPitch;    //!
   static double fSegmentPitch; //! angular pitch, degrees

   /// \cond CLASSIMP
   ClassDefOverride(TSharc, 7)
   /// \endcond
};
/*! @} */
#endif
