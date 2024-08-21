#ifndef GPOPUP_H
#define GPOPUP_H

#include "TGFrame.h"

class GPopup : public TGTransientFrame {

private:
public:
   GPopup(const TGWindow* p, const TGWindow* main, UInt_t w, UInt_t h, UInt_t options = kVerticalFrame);
   GPopup(const GPopup&)                = delete;
   GPopup(GPopup&&) noexcept            = delete;
   GPopup& operator=(const GPopup&)     = delete;
   GPopup& operator=(GPopup&&) noexcept = delete;
   ~GPopup()                            = default;

   void   CloseWindow() override;
   Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2) override;

   void Print(Option_t* opt = "") const override;

   /// /cond CLASSIMP
   ClassDefOverride(GPopup, 0)   // NOLINT(readability-else-after-return)
                                 /// /endcond
};

#endif
