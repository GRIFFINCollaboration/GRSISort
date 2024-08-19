#include <sys/stat.h>
#include <cstdio>
#include <iostream>

#include "TGButton.h"
#include "TGListView.h"

#include "GPopup.h"

GPopup::GPopup(const TGWindow* p, const TGWindow* main, UInt_t w, UInt_t h, UInt_t options)
   : TGTransientFrame(p, main, w, h, options)
{
   SetCleanup(kDeepCleanup);

   auto* vframe = new TGVerticalFrame(this, 500, 200);

   auto* listv = new TGListView(vframe, 500, 200);
   auto* listc = new TGLVContainer(listv->GetViewPort(), 500, 200, kHorizontalFrame, fgWhitePixel);
   listv->SetContainer(listc);
   listv->GetViewPort()->SetBackgroundColor(fgWhitePixel);
   listv->SetViewMode(kLVList);

   vframe->AddFrame(listv, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

   auto* hframe = new TGHorizontalFrame(vframe, 500, 20);
   auto* b1     = new TGTextButton(hframe, "&button1");
   auto* b2     = new TGTextButton(hframe, "&button2");
   b1->Connect("Clicked()", "GPopup", this, "Print()");
   b2->Connect("Clicked()", "GPopup", this, "Print()");

   hframe->AddFrame(b1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   hframe->AddFrame(b2, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

   vframe->AddFrame(hframe, new TGLayoutHints(kLHintsExpandX));

   AddFrame(vframe, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

   MapSubwindows();
   Resize();   // resize to default size
   // position relative to the parent's window
   CenterOnParent();
   MapWindow();
}

void GPopup::Print(Option_t*) const
{
   std::cout << __PRETTY_FUNCTION__ << " was called." << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast)
}

void GPopup::CloseWindow()
{
   DeleteWindow();
}

Bool_t GPopup::ProcessMessage(Long_t, Long_t, Long_t)
{
   return true;
}
