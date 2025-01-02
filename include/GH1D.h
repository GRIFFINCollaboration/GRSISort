#ifndef GH1D_H
#define GH1D_H

#include "TH1.h"
#include "TRef.h"
#include "TPad.h"
#include "GuiTypes.h"
#include "TList.h"
#include "TBox.h"

#include "GH2I.h"

class TF1;
class TRegion;

class GH1D : public TH1D {
public:
   GH1D() : fParent(nullptr), fProjectionAxis(-1) {}
   explicit GH1D(const TVectorD& vec) : TH1D(vec), fParent(nullptr), fProjectionAxis(-1) {}
   GH1D(const char* name, const char* title, Int_t nbinsx, const Float_t* xbins)
      : TH1D(name, title, nbinsx, xbins), fParent(nullptr), fProjectionAxis(-1)
   {
   }
   GH1D(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins)
      : TH1D(name, title, nbinsx, xbins), fParent(nullptr), fProjectionAxis(-1)
   {
   }
   GH1D(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup)
      : TH1D(name, title, nbinsx, xlow, xup), fParent(nullptr), fProjectionAxis(-1)
   {
   }

   GH1D(const TF1& function, Int_t nbinsx, Double_t xlow, Double_t xup);

   explicit GH1D(const TH1& source);
   explicit GH1D(const TH1* source);
   // virtual void SetOption(Option_t* option=" ");

   TObject* GetParent() const { return fParent.GetObject(); }
   void     SetParent(TObject* obj) { fParent = obj; }

   TVirtualPad* GetPad() const { return fPad; }
   void         SetPad(TVirtualPad* pad);

   int  GetProjectionAxis() const { return fProjectionAxis; }
   void SetProjectionAxis(int axis) { fProjectionAxis = axis; }

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;
   void Copy(TObject& obj) const override;
   void Draw(Option_t* opt = "") override;
   TH1* DrawCopy(Option_t* opt = "", const char* name_postfix = "copy") const override;
   TH1* DrawNormalized(Option_t* opt = "", Double_t norm = 1) const override;

   bool WriteDatFile(const char* outFile);

   GH1D* Project(int bins = -1);

   GH1D* GetPrevious(bool DrawEmpty = false) const;
   GH1D* GetNext(bool DrawEmpty = false) const;

   GH1D* Project(double value_low, double value_high) const;
   GH1D* Project_Background(double value_low, double value_high, double bg_value_low, double bg_value_high,
                            EBackgroundSubtraction mode = EBackgroundSubtraction::kRegionBackground) const;

   TVirtualPad* Pad() const { return fPad; }
   TList*       ListOfRegions() { return &fRegions; }

   void HandleMovement(Int_t eventType, Int_t eventX, Int_t eventY, TObject* selected);
   void HandleEvent(Event_t* event, Window_t window);

   void RemoveRegion(TRegion* region);

   void UpdatePad()
   {
      fPad->Modified();
      fPad->Update();
   }
   void UpdateRegions();
   void PrintRegions();
   void DrawRegions(Option_t* opt = "");

   static void VerboseLevel(int level) { fVerboseLevel = level; }
   static int  VerboseLevel() { return fVerboseLevel; }

private:
   void RemoveCurrentRegion();

   TRef         fParent;
   int          fProjectionAxis;
   TVirtualPad* fPad{nullptr};   //!<!
   // variables for regions
   double             fStartX{0.};                                        //!<! initial x-position of new region
   double             fStartY{0.};                                        //!<! initial y-position of new region
   bool               fGate{false};                                       //< flag to indicate that next region will be a gate region
   bool               fBackground{false};                                 //< flag to indicate that next region will be a background region
   bool               fRegion{false};                                     //< flag to indicate that next region will be a default region
   TBox*              fCurrentRegion{nullptr};                            //!<! box for the current region
   std::array<int, 3> fRegionColor{kOrange + 2, kGreen + 2, kCyan + 2};   // could be made static?
   size_t             fNofRegions{0};                                     //!<! counts number of regions in this histogram, only used to set the color of the region
   TList              fRegions;

   static int fVerboseLevel;   //!<! level of verbosity

   /// /cond CLASSIMP
   ClassDefOverride(GH1D, 1)   // NOLINT(readability-else-after-return)
                               /// /endcond
};

enum class ERegionType { kDefault,
                         kGate,
                         kBackground,
                         kRegion };

class TRegion : public TBox {
public:
   TRegion() = default;
   TRegion(TBox* box, ERegionType type, GH1D* parent);
   TRegion(const TRegion&)            = default;
   TRegion(TRegion&&)                 = default;
   TRegion& operator=(const TRegion&) = default;
   TRegion& operator=(TRegion&&)      = default;
   ~TRegion()                         = default;

   bool Update();
   void Hide();
   void Draw(Option_t* opt = "") override;

   void Update(double startX, double stopX);

private:
   GH1D*       fParent{nullptr};
   ERegionType fType{ERegionType::kDefault};
   double      fLowX{0.};
   double      fHighX{0.};

   /// /cond CLASSIMP
   ClassDefOverride(TRegion, 1)   // NOLINT(readability-else-after-return)
                                  /// /endcond
};

#endif /* GH1D_H */
