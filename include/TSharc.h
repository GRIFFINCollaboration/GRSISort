#ifndef TSHARC_H
#define TSHARC_H

#include "Globals.h"

#include <vector>
#include <cstdio>
#include <map>
#include <set>

#include "TMath.h"
#include "TVector3.h"

#include "TGRSIDetector.h"

#include "TSharcHit.h"
class TSharcData;

class TSharc : public TGRSIDetector  {
  public:
    TSharc();
    virtual ~TSharc();
    TSharc(const TSharc& rhs);

  public: 
    TSharcHit        *GetSharcHit(const int& i);  
    TGRSIDetectorHit *GetHit(const int& i);
    static TVector3 GetPosition(int detector, int frontstrip, int backstrip, double X=0.00, double Y=0.00, double Z=0.00);  //! 
    static double GetXOffset()  { return X_offset; }
    static double GetYOffset()  { return Y_offset; }
    static double GetZOffset()  { return Z_offset; }
    static TVector3 GetOffset() { return TVector3(X_offset,Y_offset,Z_offset); } 
    static void   SetXYZOffset(const double x,const double y,const double z) { X_offset =x; Y_offset=y; Z_offset=z; }


    int GetSize() const { return sharc_hits.size();} //!
    Short_t GetMultiplicity() const { return sharc_hits.size(); } 

    virtual void Copy(TObject&) const;         //!
    virtual void Clear(Option_t * = "");       //!
    virtual void Print(Option_t * = "") const; //!
    
    TSharc& operator=(const TSharc& rhs)  { if(this!=&rhs) ((TSharc&)rhs).Copy(*this); return *this; }//!

    //TSharcData *GetData() { return &data; }  //!
    void FillData(TFragment*,TChannel*,MNEMONIC*);           //! Collects the fragments to make front/back/pad coinc.
    void BuildHits(TDetectorData *sd=0,Option_t * = ""); //! Builds the fragments into sharchits.

  protected:
    void PushBackHit(TGRSIDetectorHit* sharchit) { sharc_hits.push_back(*((TSharcHit*)sharchit)); };
  
  private:
    std::vector <TSharcHit> sharc_hits;
    int  CombineHits(TSharcHit*,TSharcHit*,int,int);        //!
    void RemoveHits(std::vector<TSharcHit>*,std::set<int>*);  //!

  private: 
    TSharcData *data;    //!

    static double X_offset;  //!
    static double Y_offset;  //!
    static double Z_offset;  //!

    // various sharc dimensions set in mm, taken from IOP SHARC white paper
    static double Xdim; // total X dimension of all boxes
    static double Ydim; // total Y dimension of all boxes
    static double Zdim; // total Z dimension of all boxes
    static double Rdim; // Rmax-Rmin for all QQQs 
    static double Pdim; // QQQ quadrant angular range (degrees)
    // BOX dimensions
    static double XposUB;
    static double YminUB; 
    static double ZminUB; 
    static double XposDB; 
    static double YminDB; 
    static double ZminDB; 
    // QQQ dimensions
    static double ZposUQ;    
    static double RminUQ;
    static double PminUQ; // degrees
    static double ZposDQ;    
    static double RminDQ;
    static double PminDQ; // degrees
    // segmentation
    //static const int frontstripslist[16]   ;
    //static const int backstripslist[16]    ;        
    //pitches
    static  double stripFpitch;
    static  double stripBpitch;
    static  double ringpitch;
    static  double segmentpitch; // angular pitch, degrees

   ClassDef(TSharc,6)  
};


#endif
