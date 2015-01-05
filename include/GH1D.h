#ifndef TGRUTH1D_H
#define TGRUTH1D_H

//  rootcint -f GH1DDict.cxx -c GH1D.h 
//  g++ -fPIC -c GH1DDict.cxx `root-config --cflags`
//  g++ -fPIC -c GH1D.cxx `root-config --cflags`
//  g++ -shared -Wl,-soname,libGRUTHist.so -olibGRUTHist.so GH1D.o GH1DDict.o `root-config --cflags --glibs` 

#include <map>
#include <string>

#include <TH1D.h>

class GH1D : public TH1D {

   public:
      GH1D(); 
      virtual ~GH1D(); 
      GH1D(const TVectorD& v); 
      //GH1D(const GH1D& h1d); 
      GH1D(const char *name, const char *title,Int_t nbinsx,const Float_t *xbins); 
      GH1D(const char *name, const char *title,Int_t nbinsx,const Double_t *xbins); 
      GH1D(const char *name, const char *title,Int_t nbinsx,Double_t xlow, Double_t xup); 

      //virtual void ExecuteEvent(Int_t,Int_t,Int_t);

      virtual void Draw(Option_t *opt);

      static void CheckMapStats();

   private:

      void InitGH1D();

      static int fUniqueId;
      static std::map <GH1D*,int> fCurrentHistMap;
      static void AddToMap(GH1D*);
      static void RemoveFromMap(GH1D*);


      ClassDef(GH1D,1);
};

#endif 
