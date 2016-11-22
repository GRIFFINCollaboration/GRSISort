
#ifndef __GPOPUP_H__
#define __GPOPUP_H__


#include "TGFrame.h"


class GPopup : public TGTransientFrame {

  private:
  
  public:
    GPopup(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
                 UInt_t options = kVerticalFrame);
    virtual ~GPopup();
  
    virtual void CloseWindow();
    virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

    virtual void Print(Option_t *opt="") const;

  ClassDef(GPopup,0) 
};

#endif
