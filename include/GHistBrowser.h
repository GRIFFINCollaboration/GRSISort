#ifndef GHISTBROWSER_H
#define GHISTBROWSER_H

#include <cstdio>


#include <TGMainFrame.h>

class GHistBrowser : public TGMainFrame {

  public:
    ~GHistBrowser();

  private:
    static GHistBrowser *fGHistBrowser;
    GHistBrowser(const TGWindow *p,UInt_t w,UInt_t h);

    DrawFrame();



  ClassImp(GHistBrowser,0)
};

#endif
