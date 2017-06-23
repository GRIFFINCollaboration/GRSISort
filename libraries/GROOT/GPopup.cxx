

#include <sys/stat.h>
#include <cstdio>

#include <TGButton.h>
#include <TGListView.h>

#include <GPopup.h>

ClassImp(GPopup)

   GPopup::GPopup(const TGWindow* p, const TGWindow* main, UInt_t w, UInt_t h, UInt_t options)
   : TGTransientFrame(p, main, w, h, options)
{
   SetCleanup(kDeepCleanup);

   auto* vframe = new TGVerticalFrame(this, 500, 200);

   auto*    listv = new TGListView(vframe, 500, 200);
   auto* listc = new TGLVContainer(listv->GetViewPort(), 500, 200, kHorizontalFrame, fgWhitePixel);
   // listc->Associate(this);
   listv->SetContainer(listc);
   listv->GetViewPort()->SetBackgroundColor(fgWhitePixel);
   listv->SetViewMode(kLVList);
   // listv->SetIncremental(1,19);

   vframe->AddFrame(listv, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

   auto* hframe = new TGHorizontalFrame(vframe, 500, 20);
   auto*      b1     = new TGTextButton(hframe, "&button1");
   auto*      b2     = new TGTextButton(hframe, "&button2");
   b1->Connect("Clicked()", "GPopup", this, "Print()");
   b2->Connect("Clicked()", "GPopup", this, "Print()");

   hframe->AddFrame(b1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   hframe->AddFrame(b2, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

   vframe->AddFrame(hframe, new TGLayoutHints(kLHintsExpandX));

   this->AddFrame(vframe, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

   MapSubwindows();
   Resize(); // resize to default size
   // position relative to the parent's window
   CenterOnParent();
   // SetWindowName("Dialog");
   MapWindow();
   // fClient->WaitFor(this);    // otherwise canvas contextmenu does not work
}

GPopup::~GPopup()
= default;

// void GPopup::AddEntry(const char *name) {
//  std::string sname = name;
//  if(!sname.length())
//    return;
//  TGLVEntry *entry = new TGLVEntry;
//  entry->SetTitle(name);
//}

void GPopup::Print(Option_t*) const
{
   printf("%s was called.\n", __PRETTY_FUNCTION__);
}

void GPopup::CloseWindow()
{
   DeleteWindow();
}

Bool_t GPopup::ProcessMessage(Long_t, Long_t, Long_t)
{
   return true;
}
