//  Largely! taken from GRootBrowser in the Root.  Need to change
//  some of the connections in CreateBrowser() function to
//  the GRootCanvas class instead of the the TRootCanvas.
//  pcb.

///////////////////////////////////////////////////////////////////////////
///
/// \class GRootBrowser
///
/// This class creates a ROOT object browser, constitued by three main
/// tabs.
///
/// All tabs can 'swallow' frames, thanks to the new method:
///   ExecPlugin(const char *name = 0, const char *fname = 0,
///              const char *cmd = 0, Int_t pos = kRight,
///              Int_t subpos = -1)
/// allowing to select plugins (can be a macro or a command)
/// to be executed, and where to embed the frame created by
/// the plugin (tab and tab element). Examples:
///
/// create a new browser:
/// TBrowser b;
///
/// create a new TCanvas in a new top right tab element:
/// b.ExecPlugin("Canvas", 0, "new TCanvas()");
///
/// create a new top right tab element embedding the
/// TGMainFrame created by the macro 'myMacro.C':
/// b.ExecPlugin("MyPlugin", "myMacro.C");
///
/// create a new bottom tab element embedding the
/// TGMainFrame created by the macro 'myMacro.C':
/// b.ExecPlugin("MyPlugin", "myMacro.C", 0, GRootBrowser::kBottom);
///
/// this browser implementation can be selected via the env
/// 'Browser.Name' in .rootrc, (GRootBrowser or GRootBrowserLite)
/// the default being GRootBrowserLite (old browser)
/// a list of options (plugins) for the new GRootBrowser is also
/// specified via the env 'Browser.Options' in .rootrc, the default
/// being: FECI
/// Here is the list of available options:
/// F: File browser E: Text Editor H: HTML browser C: Canvas I: I/O
/// redirection P: Proof G: GL viewer
///
///////////////////////////////////////////////////////////////////////////

#include "TROOT.h"
#include "TSystem.h"
#include "TApplication.h"
#include "TBrowser.h"
#include "TGClient.h"
#include "TGFrame.h"
#include "TGTab.h"
#include "TGMenu.h"
#include "TGLayout.h"
#include "TGSplitter.h"
#include "TGStatusBar.h"
#include "Varargs.h"
#include "TInterpreter.h"
#include "TBrowser.h"
#include "TGFileDialog.h"
#include "TObjString.h"
#include "TVirtualPad.h"
#include "TEnv.h"
#include <KeySymbols.h>

#include "RConfigure.h"

#include "GRootBrowser.h"
#include "TGFileBrowser.h"
#include "TGInputDialog.h"
#include "TRootHelpDialog.h"
#include "TVirtualPadEditor.h"
#include "HelpText.h"
#include "Getline.h"

#include "TChannel.h"
#include "TKey.h"

#ifdef WIN32
#include <TWin32SplashThread.h>
#endif

static const char* gOpenFileTypes[] = {"ROOT files", "*.root", "All files", "*", nullptr, nullptr};

static const char* gPluginFileTypes[] = {"ROOT files", "*.C", "All files", "*", nullptr, nullptr};

//_____________________________________________________________________________
//
// GRootBrowser
//
// The main ROOT object browser.
//_____________________________________________________________________________

/// \cond CLASSIMP
ClassImp(GRootBrowser)
/// \endcond

//______________________________________________________________________________
GRootBrowser::GRootBrowser(TBrowser* b, const char* name, UInt_t width, UInt_t height, Option_t* opt,
									Bool_t initshow)
   : TGMainFrame(gClient->GetDefaultRoot(), width, height), TBrowserImp(b)
{
   /// Create browser with a specified width and height.

   fShowCloseTab = kTRUE;
   fActBrowser   = nullptr;
   fIconPic      = nullptr;
   CreateBrowser(name);
   Resize(width, height);
   if(initshow) {
      InitPlugins(opt);
      MapWindow();
   }
   TQObject::Connect("TCanvas", "ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "GRootBrowser", this,
                     "EventInfo(Int_t, Int_t, Int_t, TObject*)");
   gVirtualX->SetInputFocus(GetId());
}

//______________________________________________________________________________
GRootBrowser::GRootBrowser(TBrowser* b, const char* name, Int_t x, Int_t y, UInt_t width, UInt_t height, Option_t* opt,
                           Bool_t initshow)
   : TGMainFrame(gClient->GetDefaultRoot(), width, height), TBrowserImp(b)
{
   /// Create browser with a specified width and height and at position x, y.

   fShowCloseTab = kTRUE;
   fActBrowser   = nullptr;
   fIconPic      = nullptr;
   CreateBrowser(name);
   MoveResize(x, y, width, height);
   SetWMPosition(x, y);
   if(initshow) {
      InitPlugins(opt);
      MapWindow();
   }
   TQObject::Connect("TCanvas", "ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "GRootBrowser", this,
                     "EventInfo(Int_t, Int_t, Int_t, TObject*)");
   gVirtualX->SetInputFocus(GetId());
}

//______________________________________________________________________________
void GRootBrowser::CreateBrowser(const char* name)
{

   /// Create the actual interface.

   fVf = new TGVerticalFrame(this, 100, 100);

   fLH0 = new TGLayoutHints(kLHintsNormal);
   fLH1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
   fLH2 = new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 0, 1, 1);
   fLH3 = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX);
   fLH4 = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);
   fLH5 = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY);
   fLH6 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX);
   fLH7 = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandY);

   // Menubar Frame
   fTopMenuFrame = new TGHorizontalFrame(fVf, 100, 20);

   fPreMenuFrame = new TGHorizontalFrame(fTopMenuFrame, 0, 20, kRaisedFrame);
   fMenuBar      = new TGMenuBar(fPreMenuFrame, 10, 10, kHorizontalFrame);
   fMenuFile     = new TGPopupMenu(gClient->GetDefaultRoot());
   fMenuFile->AddEntry("&Browse...\tCtrl+B", static_cast<int>(ENewBrowserMessages::kBrowse));
   fMenuFile->AddEntry("&Open...\tCtrl+O", static_cast<int>(ENewBrowserMessages::kOpenFile));
   fMenuFile->AddSeparator();

   fMenuHelp = new TGPopupMenu(fClient->GetRoot());
   fMenuHelp->AddEntry("&About ROOT...", static_cast<int>(ENewBrowserMessages::kHelpAbout));
   fMenuHelp->AddSeparator();
   fMenuHelp->AddEntry("Help On Browser...", static_cast<int>(ENewBrowserMessages::kHelpOnBrowser));
   fMenuHelp->AddEntry("Help On Canvas...", static_cast<int>(ENewBrowserMessages::kHelpOnCanvas));
   fMenuHelp->AddEntry("Help On Menus...", static_cast<int>(ENewBrowserMessages::kHelpOnMenus));
   fMenuHelp->AddEntry("Help On Graphics Editor...", static_cast<int>(ENewBrowserMessages::kHelpOnGraphicsEd));
   fMenuHelp->AddEntry("Help On Objects...", static_cast<int>(ENewBrowserMessages::kHelpOnObjects));
   fMenuHelp->AddEntry("Help On PostScript...", static_cast<int>(ENewBrowserMessages::kHelpOnPS));
   fMenuHelp->AddEntry("Help On Remote Session...", static_cast<int>(ENewBrowserMessages::kHelpOnRemote));
   fMenuFile->AddPopup("Browser Help...", fMenuHelp);

   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("&Clone\tCtrl+N", static_cast<int>(ENewBrowserMessages::kClone));
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("New &Editor\tCtrl+E", static_cast<int>(ENewBrowserMessages::kNewEditor));
   fMenuFile->AddEntry("New &Canvas\tCtrl+C", static_cast<int>(ENewBrowserMessages::kNewCanvas));
   fMenuFile->AddEntry("New &HTML\tCtrl+H", static_cast<int>(ENewBrowserMessages::kNewHtml));
   fMenuFile->AddSeparator();
   fMenuExecPlugin = new TGPopupMenu(fClient->GetRoot());
   fMenuExecPlugin->AddEntry("&Macro...", static_cast<int>(ENewBrowserMessages::kExecPluginMacro));
   fMenuExecPlugin->AddEntry("&Command...", static_cast<int>(ENewBrowserMessages::kExecPluginCmd));
   fMenuFile->AddPopup("Execute &Plugin...", fMenuExecPlugin);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("Close &Tab\tCtrl+T", static_cast<int>(ENewBrowserMessages::kCloseTab));
   fMenuFile->AddEntry("Close &Window\tCtrl+W", static_cast<int>(ENewBrowserMessages::kCloseWindow));
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("&Quit Root\tCtrl+Q", static_cast<int>(ENewBrowserMessages::kQuitRoot));
   fMenuBar->AddPopup("&Browser", fMenuFile, fLH1);
   fMenuFile->Connect("Activated(Int_t)", "GRootBrowser", this, "HandleMenu(Int_t)");
   fPreMenuFrame->AddFrame(fMenuBar, fLH2);
   fTopMenuFrame->AddFrame(fPreMenuFrame, fLH0);

   fMenuFrame = new TGHorizontalFrame(fTopMenuFrame, 100, 20, kRaisedFrame);
   fTopMenuFrame->AddFrame(fMenuFrame, fLH5);

   fVf->AddFrame(fTopMenuFrame, fLH3);
   fActMenuBar = fMenuBar;

   // Toolbar Frame
   fToolbarFrame = new TGHorizontalFrame(fVf, 100, 20, kHorizontalFrame | kRaisedFrame);
   fVf->AddFrame(fToolbarFrame, fLH3);

   fHf = new TGHorizontalFrame(fVf, 100, 100);
// Tabs & co...
#if defined(R__HAS_COCOA)
   fV1 = new TGVerticalFrame(fHf, 252, 100, kFixedWidth);
#else
   fV1 = new TGVerticalFrame(fHf, 250, 100, kFixedWidth);
#endif
   fV2 = new TGVerticalFrame(fHf, 600, 100);
   fH1 = new TGHorizontalFrame(fV2, 100, 100);
   fH2 = new TGHorizontalFrame(fV2, 100, 100, kFixedHeight);

   // Left tab
   fTabLeft = new TGTab(fV1, 100, 100);
   // fTabLeft->AddTab("Tab 1");
   fTabLeft->Resize(fTabLeft->GetDefaultSize());
   fV1->AddFrame(fTabLeft, fLH4);

   // Vertical splitter
   fVSplitter = new TGVSplitter(fHf, 4, 4);
   fVSplitter->SetFrame(fV1, kTRUE);
   fHf->AddFrame(fV1, fLH7);
   fHf->AddFrame(fVSplitter, fLH7);

   // Right tab
   fTabRight = new TGTab(fH1, 500, 100);
   // fTabRight->AddTab("Tab 1");
   fTabRight->Resize(fTabRight->GetDefaultSize());
   fH1->AddFrame(fTabRight, fLH5);
   fTabRight->Connect("Selected(Int_t)", "GRootBrowser", this, "DoTab(Int_t)");
   fTabRight->Connect("CloseTab(Int_t)", "GRootBrowser", this, "CloseTab(Int_t)");
   fV2->AddFrame(fH1, fLH4);

   // Horizontal splitter
   fHSplitter = new TGHSplitter(fV2, 4, 4);
   fV2->AddFrame(fHSplitter, fLH3);

   // Bottom tab
   fTabBottom = new TGTab(fH2, 100, 100);
   // fTabBottom->AddTab("Tab 1");
   fH2->AddFrame(fTabBottom, fLH4);
   fV2->AddFrame(fH2, fLH3);

   fHSplitter->SetFrame(fH2, kFALSE);
   fHf->AddFrame(fV2, fLH5);
   fVf->AddFrame(fHf, fLH5);
   AddFrame(fVf, fLH5);

   // status bar
   fStatusBar  = new TGStatusBar(this, 400, 20);
   int parts[] = {33, 10, 10, 47};
   fStatusBar->SetParts(parts, 4);
   AddFrame(fStatusBar, fLH6);

   fNbInitPlugins = 0;
   fEditFrame     = nullptr;
   fEditTab       = nullptr;
   fEditPos       = -1;
   fEditSubPos    = -1;
   fNbTab[0] = fNbTab[1] = fNbTab[2] = 0;
   fCrTab[0] = fCrTab[1] = fCrTab[2] = -1;

   // Set a name to the main frame
   SetWindowName(name);
   SetIconName(name);
   fIconPic = SetIconPixmap("rootdb_s.xpm");
   SetClassHints("ROOT", "Browser");

   if(strcmp(gROOT->GetDefCanvasName(), "c1") == 0) {
      gROOT->SetDefCanvasName("Canvas_1");
   }

   SetWMSizeHints(600, 350, 10000, 10000, 2, 2);
   MapSubwindows();
   Resize(GetDefaultSize());
   AddInput(kKeyPressMask | kKeyReleaseMask);

   fVf->HideFrame(fToolbarFrame);
}

//______________________________________________________________________________
GRootBrowser::~GRootBrowser()
{
   /// Clean up all widgets, frames and layouthints that were used
   printf("I AM HERE!\n");
   fflush(stdout);

   if(fIconPic != nullptr) {
      gClient->FreePicture(fIconPic);
   }
   delete fLH0;
   delete fLH1;
   delete fLH2;
   delete fLH3;
   delete fLH4;
   delete fLH5;
   delete fLH6;
   delete fLH7;
   delete fMenuHelp;
   delete fMenuExecPlugin;
   delete fMenuFile;
   delete fMenuBar;
   delete fMenuFrame;
   delete fPreMenuFrame;
   delete fTopMenuFrame;
   delete fToolbarFrame;
   delete fVSplitter;
   delete fHSplitter;
   delete fTabLeft;
   delete fTabRight;
   delete fTabBottom;
   delete fH1;
   delete fH2;
   delete fV1;
   delete fV2;
   delete fHf;
   delete fStatusBar;
   delete fVf;
}

//______________________________________________________________________________
void GRootBrowser::Add(TObject* obj, const char* name, Int_t check)
{
   /// Add items to the actual browser. This function has to be called
   /// by the Browse() member function of objects when they are
   /// called by a browser. If check < 0 (default) no check box is drawn,
   /// if 0 then unchecked checkbox is added, if 1 checked checkbox is added.

   if(obj->InheritsFrom("TObjectSpy")) {
      return;
   }
   if(obj->InheritsFrom("TKey")) {
      if(strcmp((static_cast<TKey*>(obj))->GetClassName(), "TChannel") == 0) {
         (static_cast<TKey*>(obj))->ReadObj();
      }
   }
   if(fActBrowser != nullptr) {
      fActBrowser->Add(obj, name, check);
   }
}

//______________________________________________________________________________
void GRootBrowser::BrowseObj(TObject* obj)
{
   /// Browse object. This, in turn, will trigger the calling of
   /// GRootBrowser::Add() which will fill the IconBox and the tree.
   /// Emits signal "BrowseObj(TObject*)".

   if(fActBrowser != nullptr) {
      fActBrowser->BrowseObj(obj);
   }
   Emit("BrowseObj(TObject*)", (Long_t)obj);
}

//______________________________________________________________________________
void GRootBrowser::CloneBrowser()
{
   /// Clone the browser. A new Browser will be created, with the same
   /// plugins executed in the current one.

   Int_t           loop   = 1;
   GBrowserPlugin* plugin = nullptr;
   auto*           b      = new TBrowser();
   TIter           next(&fPlugins);
   while((plugin = static_cast<GBrowserPlugin*>(next())) != nullptr) {
      if(loop > fNbInitPlugins) {
         b->ExecPlugin(plugin->GetName(), "", plugin->fCommand.Data(), plugin->fTab, plugin->fSubTab);
      }
      ++loop;
   }
}

//______________________________________________________________________________
void GRootBrowser::CloseTab(Int_t id)
{
   /// Remove tab element id from right tab.

   RemoveTab(static_cast<Int_t>(EInsertPosition::kRight), id);
}

//______________________________________________________________________________
void GRootBrowser::CloseTabs()
{
   /// Properly close the mainframes embedded in the different tabs

   TGFrameElement*   el;
   TGCompositeFrame* container;
   Int_t             i;
   Disconnect(fMenuFile, "Activated(Int_t)", this, "HandleMenu(Int_t)");
   Disconnect(fTabRight, "Selected(Int_t)", this, "DoTab(Int_t)");
   if(fPlugins.IsEmpty()) {
      return;
   }
   fActBrowser = nullptr;
   for(i = 0; i < fTabLeft->GetNumberOfTabs(); i++) {
      container = fTabLeft->GetTabContainer(i);
      if(container == nullptr) {
         continue;
      }
      el = static_cast<TGFrameElement*>(container->GetList()->First());
      if((el != nullptr) && (el->fFrame != nullptr)) {
         el->fFrame->SetFrameElement(nullptr);
         if(el->fFrame->InheritsFrom("TVirtualPadEditor")) {
            TVirtualPadEditor::Terminate();
         } else if(el->fFrame->InheritsFrom("TGMainFrame")) {
            (static_cast<TGMainFrame*>(el->fFrame))->CloseWindow();
            gSystem->ProcessEvents();
         } else {
            delete el->fFrame;
         }
         el->fFrame = nullptr;
         if((el->fLayout != nullptr) && (el->fLayout != fgDefaultHints) && (el->fLayout->References() > 0)) {
            el->fLayout->RemoveReference();
            if(el->fLayout->References() == 0u) {
               delete el->fLayout;
            }
         }
         container->GetList()->Remove(el);
         delete el;
      }
   }
   for(i = 0; i < fTabRight->GetNumberOfTabs(); i++) {
      container = fTabRight->GetTabContainer(i);
      if(container == nullptr) {
         continue;
      }
      el = static_cast<TGFrameElement*>(container->GetList()->First());
      if((el != nullptr) && (el->fFrame != nullptr)) {
         el->fFrame->SetFrameElement(nullptr);
         if(el->fFrame->InheritsFrom("TGMainFrame")) {
            Bool_t sleep = (el->fFrame->InheritsFrom("GRootCanvas")) ? kTRUE : kFALSE;
            (static_cast<TGMainFrame*>(el->fFrame))->CloseWindow();
            if(sleep) {
               gSystem->Sleep(150);
            }
            gSystem->ProcessEvents();
         } else {
            delete el->fFrame;
         }
         el->fFrame = nullptr;
         if((el->fLayout != nullptr) && (el->fLayout != fgDefaultHints) && (el->fLayout->References() > 0)) {
            el->fLayout->RemoveReference();
            if(el->fLayout->References() == 0u) {
               delete el->fLayout;
            }
         }
         container->GetList()->Remove(el);
         delete el;
      }
   }
   for(i = 0; i < fTabBottom->GetNumberOfTabs(); i++) {
      container = fTabBottom->GetTabContainer(i);
      if(container == nullptr) {
         continue;
      }
      el = static_cast<TGFrameElement*>(container->GetList()->First());
      if((el != nullptr) && (el->fFrame != nullptr)) {
         el->fFrame->SetFrameElement(nullptr);
         if(el->fFrame->InheritsFrom("TGMainFrame")) {
            (static_cast<TGMainFrame*>(el->fFrame))->CloseWindow();
            gSystem->ProcessEvents();
         } else {
            delete el->fFrame;
         }
         el->fFrame = nullptr;
         if((el->fLayout != nullptr) && (el->fLayout != fgDefaultHints) && (el->fLayout->References() > 0)) {
            el->fLayout->RemoveReference();
            if(el->fLayout->References() == 0u) {
               delete el->fLayout;
            }
         }
         container->GetList()->Remove(el);
         delete el;
      }
   }
   fPlugins.Delete();
   Emit("CloseWindow()");
}

//______________________________________________________________________________
void GRootBrowser::CloseWindow()
{
   /// Called when window is closed via the window manager.

   TQObject::Disconnect("TCanvas", "ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", this,
                        "EventInfo(Int_t, Int_t, Int_t, TObject*)");
   CloseTabs();
   DeleteWindow();
}

//______________________________________________________________________________
void GRootBrowser::DoTab(Int_t id)
{
   /// Handle Tab navigation.

   TGTab* sender = reinterpret_cast<TGTab*>(gTQSender);
   if(((sender) != nullptr) && (sender == fTabRight)) {
      SwitchMenus(sender->GetTabContainer(id));
   }
}

//______________________________________________________________________________
void GRootBrowser::EventInfo(Int_t event, Int_t px, Int_t py, TObject* selected)
{
   /// Display a tooltip with infos about the primitive below the cursor.

   const Int_t kTMAX = 256;
   static char atext[kTMAX];
   if(selected == nullptr || event == kMouseLeave) {
      SetStatusText("", 0);
      SetStatusText("", 1);
      SetStatusText("", 2);
      SetStatusText("", 3);
      return;
   }
   SetStatusText(selected->GetTitle(), 0);
   SetStatusText(selected->GetName(), 1);
   if(event == kKeyPress) {
      snprintf(atext, kTMAX, "%c", static_cast<char>(px));
   } else {
      snprintf(atext, kTMAX, "%d,%d", px, py);
   }
   SetStatusText(atext, 2);
   SetStatusText(selected->GetObjectInfo(px, py), 3);
}

//______________________________________________________________________________
Long_t GRootBrowser::ExecPlugin(const char* name, const char* fname, const char* cmd, Int_t pos, Int_t subpos)
{
   /// Execute a macro and embed the created frame in the tab "pos"
   /// and tab element "subpos".

   Long_t          retval = 0;
   GBrowserPlugin* p;
   TString         command, pname;
   StartEmbedding(pos, subpos);
   if((cmd != nullptr) && (strlen(cmd) != 0u)) {
      command = cmd;
      if(name != nullptr) {
         pname = name;
      } else {
         pname = TString::Format("Plugin %d", fPlugins.GetSize());
      }
      p = new GBrowserPlugin(pname.Data(), command.Data(), pos, subpos);
   } else if((fname != nullptr) && (strlen(fname) != 0u)) {
      pname    = name != nullptr ? name : gSystem->BaseName(fname);
      Ssiz_t t = pname.Last('.');
      if(t > 0) {
         pname.Remove(t);
      }
      command.Form(R"(gROOT->Macro("%s");)", gSystem->UnixPathName(fname));
      p = new GBrowserPlugin(pname.Data(), command.Data(), pos, subpos);
   } else {
      return 0;
   }
   fPlugins.Add(p);
   retval = gROOT->ProcessLine(command.Data());
   if(command.Contains("new TCanvas")) {
      pname = gPad->GetName();
      p->SetName(pname.Data());
   }
   SetTabTitle(pname.Data(), pos, subpos);
   StopEmbedding();
   return retval;
}

//______________________________________________________________________________
Option_t* GRootBrowser::GetDrawOption() const
{
   /// Returns drawing option.

   if(fActBrowser != nullptr) {
      return fActBrowser->GetDrawOption();
   }
   return nullptr;
}

//______________________________________________________________________________
TGTab* GRootBrowser::GetTab(Int_t pos) const
{
   /// Returns the TGTab at position pos.

   switch(static_cast<EInsertPosition>(pos)) {
		case EInsertPosition::kLeft: return fTabLeft;
		case EInsertPosition::kRight: return fTabRight;
		case EInsertPosition::kBottom: return fTabBottom;
   default: return nullptr;
   }
}

//______________________________________________________________________________
Bool_t GRootBrowser::HandleKey(Event_t* event)
{
   /// Handle keyboard events.

   char   input[10];
   UInt_t keysym;

   if(event->fType == kGKeyPress) {
      gVirtualX->LookupString(event, input, sizeof(input), keysym);

      if((event->fState == 0u) && static_cast<EKeySym>(keysym) == kKey_F5) {
         Refresh(kTRUE);
         return kTRUE;
      }
      switch(static_cast<EKeySym>(keysym)) { // ignore these keys
      case kKey_Shift:
      case kKey_Control:
      case kKey_Meta:
      case kKey_Alt:
      case kKey_CapsLock:
      case kKey_NumLock:
      case kKey_ScrollLock: return kTRUE;
      default: break;
      }
      if((event->fState & kKeyControlMask) != 0u) {     // Cntrl key modifier pressed
         switch(static_cast<EKeySym>(keysym) & ~0x20) { // treat upper and lower the same
				case kKey_B: fMenuFile->Activated(static_cast<int>(ENewBrowserMessages::kBrowse)); return kTRUE;
				case kKey_O: fMenuFile->Activated(static_cast<int>(ENewBrowserMessages::kOpenFile)); return kTRUE;
				case kKey_E: fMenuFile->Activated(static_cast<int>(ENewBrowserMessages::kNewEditor)); return kTRUE;
				case kKey_C: fMenuFile->Activated(static_cast<int>(ENewBrowserMessages::kNewCanvas)); return kTRUE;
				case kKey_H: fMenuFile->Activated(static_cast<int>(ENewBrowserMessages::kNewHtml)); return kTRUE;
				case kKey_N: fMenuFile->Activated(static_cast<int>(ENewBrowserMessages::kClone)); return kTRUE;
				case kKey_T: fMenuFile->Activated(static_cast<int>(ENewBrowserMessages::kCloseTab)); return kTRUE;
				case kKey_W: fMenuFile->Activated(static_cast<int>(ENewBrowserMessages::kCloseWindow)); return kTRUE;
				case kKey_Q: fMenuFile->Activated(static_cast<int>(ENewBrowserMessages::kQuitRoot)); return kTRUE;
         default: break;
         }
      }
   }
   return TGMainFrame::HandleKey(event);
}

//______________________________________________________________________________
void GRootBrowser::HandleMenu(Int_t id)
{
   /// Handle menu entries events.

   TRootHelpDialog* hd;
   TString          cmd;
   static Int_t     eNr    = 1;
   TGPopupMenu*     sender = reinterpret_cast<TGPopupMenu*>(gTQSender);
   if(sender != fMenuFile) {
      return;
   }
   switch(static_cast<ENewBrowserMessages>(id)) {
		case ENewBrowserMessages::kBrowse: new TBrowser(); break;
		case ENewBrowserMessages::kOpenFile: {
      Bool_t         newfile = kFALSE;
      static TString dir(".");
      TGFileInfo     fi;
      fi.fFileTypes = gOpenFileTypes;
      fi.fIniDir    = StrDup(dir);
      new TGFileDialog(gClient->GetDefaultRoot(), this, kFDOpen, &fi);
      dir = fi.fIniDir;
      if(fi.fMultipleSelection && (fi.fFileNamesList != nullptr)) {
         TObjString* el;
         TIter       next(fi.fFileNamesList);
         while((el = static_cast<TObjString*>(next())) != nullptr) {
            gROOT->ProcessLine(Form(R"(new TFile("%s");)", gSystem->UnixPathName(el->GetString())));
         }
         newfile = kTRUE;
      } else if(fi.fFilename != nullptr) {
         gROOT->ProcessLine(Form(R"(new TFile("%s");)", gSystem->UnixPathName(fi.fFilename)));
         newfile = kTRUE;
      }
      if((fActBrowser != nullptr) && newfile) {
         TGFileBrowser* fb = static_cast<TGFileBrowser*>(fActBrowser);
         if(fb != nullptr) {
            fb->Selected(nullptr);
         }
      }
   } break;
   // Handle Help menu items...
		case ENewBrowserMessages::kHelpAbout: {
#ifdef R__UNIX
      TString rootx;
#ifdef ROOTBINDIR
      rootx = ROOTBINDIR;
#else
      rootx = gSystem->Getenv("ROOTSYS");
      if(!rootx.IsNull()) {
         rootx += "/bin";
      }
#endif // ROOTBINDIR
      rootx += "/root -a &";
      gSystem->Exec(rootx);
#else // R__UNIX
#ifdef WIN32
      new TWin32SplashThread(kTRUE);
#else
      char str[32];
      sprintf(str, "About ROOT %s...", gROOT->GetVersion());
      hd = new TRootHelpDialog(this, str, 600, 400);
      hd->SetText(gHelpAbout);
      hd->Popup();
#endif // WIN32
#endif // R__UNIX
   } break;
		case ENewBrowserMessages::kHelpOnCanvas:
      hd = new TRootHelpDialog(this, "Help on Canvas...", 600, 400);
      hd->SetText(gHelpCanvas);
      hd->Popup();
      break;
		case ENewBrowserMessages::kHelpOnMenus:
      hd = new TRootHelpDialog(this, "Help on Menus...", 600, 400);
      hd->SetText(gHelpPullDownMenus);
      hd->Popup();
      break;
		case ENewBrowserMessages::kHelpOnGraphicsEd:
      hd = new TRootHelpDialog(this, "Help on Graphics Editor...", 600, 400);
      hd->SetText(gHelpGraphicsEditor);
      hd->Popup();
      break;
		case ENewBrowserMessages::kHelpOnBrowser:
      hd = new TRootHelpDialog(this, "Help on Browser...", 600, 400);
      hd->SetText(gHelpBrowser);
      hd->Popup();
      break;
		case ENewBrowserMessages::kHelpOnObjects:
      hd = new TRootHelpDialog(this, "Help on Objects...", 600, 400);
      hd->SetText(gHelpObjects);
      hd->Popup();
      break;
		case ENewBrowserMessages::kHelpOnPS:
      hd = new TRootHelpDialog(this, "Help on PostScript...", 600, 400);
      hd->SetText(gHelpPostscript);
      hd->Popup();
      break;
		case ENewBrowserMessages::kHelpOnRemote:
      hd = new TRootHelpDialog(this, "Help on Browser...", 600, 400);
      hd->SetText(gHelpRemote);
      hd->Popup();
      break;
		case ENewBrowserMessages::kClone: CloneBrowser(); break;
		case ENewBrowserMessages::kNewEditor:
      cmd.Form("new TGTextEditor((const char *)0, gClient->GetRoot())");
      ++eNr;
      ExecPlugin(Form("Editor %d", eNr), "", cmd.Data(), 1);
      break;
		case ENewBrowserMessages::kNewCanvas: ExecPlugin("", "", "new TCanvas()", 1); break;
		case ENewBrowserMessages::kNewHtml:
      cmd.Form(R"(new TGHtmlBrowser("%s", gClient->GetRoot()))",
               gEnv->GetValue("Browser.StartUrl", "http://root.cern.ch"));
      ExecPlugin("HTML", "", cmd.Data(), 1);
      break;
		case ENewBrowserMessages::kExecPluginMacro: {
      static TString dir(".");
      TGFileInfo     fi;
      fi.fFileTypes = gPluginFileTypes;
      fi.fIniDir    = StrDup(dir);
      new TGFileDialog(gClient->GetDefaultRoot(), this, kFDOpen, &fi);
      dir = fi.fIniDir;
      if(fi.fFilename != nullptr) {
         ExecPlugin(nullptr, fi.fFilename, nullptr, static_cast<Int_t>(EInsertPosition::kRight));
      }
   } break;
		case ENewBrowserMessages::kExecPluginCmd: {
      char command[1024];
      strlcpy(command, "new TGLSAViewer(gClient->GetRoot(), 0);", sizeof(command));
      new TGInputDialog(gClient->GetRoot(), this, "Enter plugin command line:", command, command);
      if(strcmp(command, "") != 0) {
         ExecPlugin("User", nullptr, command, static_cast<Int_t>(EInsertPosition::kRight));
      }
   } break;
		case ENewBrowserMessages::kCloseTab: CloseTab(fTabRight->GetCurrent()); break;
		case ENewBrowserMessages::kCloseWindow: CloseWindow(); break;
		case ENewBrowserMessages::kQuitRoot:
      CloseWindow();
      gApplication->Terminate(0);
      break;
   default: break;
   }
}

//______________________________________________________________________________
void GRootBrowser::InitPlugins(Option_t* opt)
{
   /// Initialize default plugins. Could be also of the form:
   ///~~~
   /// StartEmbedding(0);
   /// TPluginHandler *ph;
   /// ph = gROOT->GetPluginManager()->FindHandler("TGClassBrowser");
   /// if (ph && ph->LoadPlugin() != -1) {
   ///    ph->ExecPlugin(3, gClient->GetRoot(), 200, 500);
   /// }
   /// StopEmbedding();
   ///~~~

   TString cmd;

   if((opt == nullptr) || (strlen(opt) == 0)) {
      return;
   }
   // --- Left vertical area

   // File Browser plugin
   if(strchr(opt, 'F') != nullptr) {
      cmd.Form("new TGFileBrowser(gClient->GetRoot(), (TBrowser *)0x%lx, 200, 500);", (ULong_t)fBrowser);
      ExecPlugin("Files", nullptr, cmd.Data(), 0);
      ++fNbInitPlugins;
   }

   // --- Right main area

   Int_t i, len = strlen(opt);
   for(i = 0; i < len; ++i) {
      // Editor plugin...
      if(opt[i] == 'E') {
         cmd.Form("new TGTextEditor((const char *)0, gClient->GetRoot());");
         ExecPlugin("Editor 1", nullptr, cmd.Data(), 1);
         ++fNbInitPlugins;
      }

      // HTML plugin...
      if(opt[i] == 'H') {
         if(gSystem->Load("libGuiHtml") >= 0) {
            cmd.Form(R"(new TGHtmlBrowser("%s", gClient->GetRoot());)",
                     gEnv->GetValue("Browser.StartUrl", "http://root.cern.ch/root/html/ClassIndex.html"));
            ExecPlugin("HTML", nullptr, cmd.Data(), 1);
            ++fNbInitPlugins;
         }
      }

      // Canvas plugin...
      if(opt[i] == 'C') {
         printf("I AM NOW HERE!\n");
         fflush(stdout);
         cmd.Form("new TCanvas();");
         ExecPlugin("c1", nullptr, cmd.Data(), 1);
         ++fNbInitPlugins;
      }

      // GLViewer plugin...
      if(opt[i] == 'G') {
         cmd.Form("new TGLSAViewer(gClient->GetRoot(), 0);");
         ExecPlugin("OpenGL", nullptr, cmd.Data(), 1);
         ++fNbInitPlugins;
      }

      // PROOF plugin...
      if(opt[i] == 'P') {
         cmd.Form("new TSessionViewer();");
         ExecPlugin("PROOF", nullptr, cmd.Data(), 1);
         ++fNbInitPlugins;
      }
   }
   // --- Right bottom area

   // Command plugin...
   if(strchr(opt, 'I') != nullptr) {
      cmd.Form("new TGCommandPlugin(gClient->GetRoot(), 700, 300);");
      ExecPlugin("Command", nullptr, cmd.Data(), 2);
      ++fNbInitPlugins;
   }

   // --- Select first tab everywhere
   SetTab(0, 0);
   SetTab(1, 0);
   SetTab(2, 0);
}

//______________________________________________________________________________
void GRootBrowser::ReallyDelete()
{
   /// Really delete the browser and the this GUI.

   gInterpreter->DeleteGlobal(fBrowser);
   delete fBrowser; // will in turn delete this object
}

//______________________________________________________________________________
void GRootBrowser::RecursiveRemove(TObject* obj)
{
   /// Recursively remove object from browser.

   if(fActBrowser != nullptr) {
      fActBrowser->RecursiveRemove(obj);
   }
}

//______________________________________________________________________________
void GRootBrowser::RecursiveReparent(TGPopupMenu* popup)
{
   /// Recursively reparent TGPopupMenu to gClient->GetDefaultRoot().

   TGMenuEntry* entry = nullptr;
   TIter        next(popup->GetListOfEntries());
   while((entry = static_cast<TGMenuEntry*>(next())) != nullptr) {
      if(entry->GetPopup() != nullptr) {
         RecursiveReparent(entry->GetPopup());
      }
   }
   popup->ReparentWindow(gClient->GetDefaultRoot());
}

//______________________________________________________________________________
void GRootBrowser::Refresh(Bool_t force)
{
   /// Refresh the actual browser contents.

   if(fActBrowser != nullptr) {
      fActBrowser->Refresh(force);
   }
}

//______________________________________________________________________________
void GRootBrowser::RemoveTab(Int_t pos, Int_t subpos)
{
   /// Remove tab element "subpos" from tab "pos".

   TGTab* edit = nullptr;
   switch(static_cast<EInsertPosition>(pos)) {
		case EInsertPosition::kLeft: // left
      edit = fTabLeft;
      break;
		case EInsertPosition::kRight: // right
      edit = fTabRight;
      fMenuFrame->HideFrame(fActMenuBar);
      fMenuFrame->GetList()->Remove(fActMenuBar);
      fActMenuBar = nullptr;
      break;
		case EInsertPosition::kBottom: // bottom
      edit = fTabBottom;
      break;
   }
   if((edit == nullptr) || (edit->GetTabTab(subpos) == nullptr)) {
      return;
   }
   const char* tabName = edit->GetTabTab(subpos)->GetString();
   TObject*    obj     = nullptr;
   if((obj = fPlugins.FindObject(tabName)) != nullptr) {
      fPlugins.Remove(obj);
   }
   TGFrameElement* el = nullptr;
   if(edit->GetTabContainer(subpos) != nullptr) {
      el = static_cast<TGFrameElement*>(edit->GetTabContainer(subpos)->GetList()->First());
   }
   if((el != nullptr) && (el->fFrame != nullptr)) {
      el->fFrame->Disconnect("ProcessedConfigure(Event_t*)");
      el->fFrame->SetFrameElement(nullptr);
      if(el->fFrame->InheritsFrom("TGMainFrame")) {
         Bool_t sleep = (el->fFrame->InheritsFrom("GRootCanvas")) ? kTRUE : kFALSE;
         (static_cast<TGMainFrame*>(el->fFrame))->CloseWindow();
         if(sleep) {
            gSystem->Sleep(150);
         }
         gSystem->ProcessEvents();
      } else {
         delete el->fFrame;
      }
      el->fFrame = nullptr;
      if((el->fLayout != nullptr) && (el->fLayout != fgDefaultHints) && (el->fLayout->References() > 0)) {
         el->fLayout->RemoveReference();
         if(el->fLayout->References() == 0u) {
            delete el->fLayout;
         }
      }
      edit->GetTabContainer(subpos)->GetList()->Remove(el);
      delete el;
   }
   fNbTab[pos]--;
   edit->RemoveTab(subpos);
   SwitchMenus(edit->GetTabContainer(edit->GetCurrent()));
}

//______________________________________________________________________________
void GRootBrowser::SetTab(Int_t pos, Int_t subpos)
{
   /// Switch to Tab "subpos" in TGTab "pos".

   TGTab* tab = GetTab(pos);
   if(subpos == -1) {
      subpos = fCrTab[pos];
   }

   if((tab != nullptr) && tab->SetTab(subpos, kFALSE)) { // Block signal emit
      if(static_cast<EInsertPosition>(pos) == EInsertPosition::kRight) {
         SwitchMenus(tab->GetTabContainer(subpos));
      }
      tab->Layout();
   }
}

//______________________________________________________________________________
void GRootBrowser::SetTabTitle(const char* title, Int_t pos, Int_t subpos)
{
   /// Set text "title" of Tab "subpos" in TGTab "pos".

   GBrowserPlugin* p    = nullptr;
   TGTab*          edit = GetTab(pos);
   if(edit == nullptr) {
      return;
   }
   if(subpos == -1) {
      subpos = fCrTab[pos];
   }

   TGTabElement* el = edit->GetTabTab(subpos);
   if(el != nullptr) {
      el->SetText(new TGString(title));
      edit->Layout();
      if((p = static_cast<GBrowserPlugin*>(fPlugins.FindObject(title))) != nullptr) {
         p->SetName(title);
      }
   }
}

//______________________________________________________________________________
void GRootBrowser::SetStatusText(const char* txt, Int_t col)
{
   /// Set text in culumn col in status bar.

   fStatusBar->SetText(txt, col);
}

//______________________________________________________________________________
void GRootBrowser::ShowMenu(TGCompositeFrame* menu)
{
   /// Show the selected frame's menu and hide previous one.

   TGFrameElement* el = nullptr;
   // temporary solution until I find a proper way to handle
   // these bloody menus...
   fBindList->Delete();
   TIter nextm(fMenuBar->GetList());
   while((el = static_cast<TGFrameElement*>(nextm())) != nullptr) {
      TGMenuTitle* t    = static_cast<TGMenuTitle*>(el->fFrame);
      Int_t        code = t->GetHotKeyCode();
      BindKey(fMenuBar, code, kKeyMod1Mask);
      BindKey(fMenuBar, code, kKeyMod1Mask | kKeyShiftMask);
      BindKey(fMenuBar, code, kKeyMod1Mask | kKeyLockMask);
      BindKey(fMenuBar, code, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);
      BindKey(fMenuBar, code, kKeyMod1Mask | kKeyMod2Mask);
      BindKey(fMenuBar, code, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
      BindKey(fMenuBar, code, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
      BindKey(fMenuBar, code, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
   }
   fMenuFrame->HideFrame(fActMenuBar);
   fMenuFrame->ShowFrame(menu);
   menu->Layout();
   fMenuFrame->Layout();
   fActMenuBar = menu;
}

//______________________________________________________________________________
void GRootBrowser::StartEmbedding(Int_t pos, Int_t subpos)
{
   /// Start embedding external frame in the tab "pos" and tab element "subpos".

   fEditTab = GetTab(pos);
   if(fEditTab == nullptr) {
      return;
   }
   fEditPos    = pos;
   fEditSubPos = subpos;

   if(fEditFrame == nullptr) {
      if(subpos == -1) {
         fCrTab[pos] = fNbTab[pos]++;
         fEditFrame  = fEditTab->AddTab(Form("Tab %d", fNbTab[pos]));
         fEditSubPos = fEditTab->GetNumberOfTabs() - 1;
         fEditFrame->MapWindow();
         TGTabElement* tabel = fEditTab->GetTabTab(fEditSubPos);
         if(tabel != nullptr) {
            tabel->MapWindow();
            if(fShowCloseTab && (pos == 1)) {
               tabel->ShowClose();
            }
         }
         fEditTab->SetTab(fEditTab->GetNumberOfTabs() - 1);
         fEditTab->Layout();
      } else {
         fCrTab[pos] = subpos;
         fEditFrame  = fEditTab->GetTabContainer(subpos);
         fEditTab->SetTab(subpos);
      }
      if(fEditFrame != nullptr) {
         fEditFrame->SetEditable();
      }
   }
}

//______________________________________________________________________________
void GRootBrowser::StopEmbedding(const char* name, TGLayoutHints* layout)
{
   /// Stop embedding external frame in the current editable frame.

   if(fEditFrame != nullptr) {
      fEditFrame->SetEditable(kFALSE);
      TGFrameElement* el = static_cast<TGFrameElement*>(fEditFrame->GetList()->First());
      if((el != nullptr) && (el->fFrame != nullptr)) {
         // let be notified when the inside frame gets resized, and tell its
         // container to recompute its layout
         el->fFrame->Connect("ProcessedConfigure(Event_t*)", "TGCompositeFrame", fEditFrame, "Layout()");
      }
      if(layout != nullptr) {
         el = static_cast<TGFrameElement*>(fEditFrame->GetList()->Last());
         // !!!! MT what to do with the old layout? Leak it for now ...
         if(el != nullptr) {
            el->fLayout = layout;
         }
      }
      fEditFrame->Layout();
      if(fEditTab == fTabRight) {
         SwitchMenus(fEditFrame);
      }
   }
   if((name != nullptr) && (strlen(name) != 0u)) {
      SetTabTitle(name, fEditPos, fEditSubPos);
   }
   fEditTab->Selected(fEditSubPos);
   fEditFrame = fEditTab = nullptr;
   fEditPos = fEditSubPos = -1;
}

//______________________________________________________________________________
void GRootBrowser::SwitchMenus(TGCompositeFrame* from)
{
   /// Move the menu from original frame to our TGMenuFrame, or display the
   /// menu associated to the current tab.

   if(from == nullptr) {
      return;
   }
   TGFrameElement* fe = static_cast<TGFrameElement*>(from->GetList()->First());
   if(fe == nullptr) {
      if(fActMenuBar != fMenuBar) {
         ShowMenu(fMenuBar);
      }
      return;
   }
   TGCompositeFrame* embed = static_cast<TGCompositeFrame*>(fe->fFrame);
   TGFrameElement*   el    = nullptr;
   if((embed != nullptr) && (embed->GetList() != nullptr)) {
      TIter next(embed->GetList());
      while((el = static_cast<TGFrameElement*>(next())) != nullptr) {
         if(el->fFrame->InheritsFrom("TGMenuBar")) {
            TGMenuBar* menu = static_cast<TGMenuBar*>(el->fFrame);
            if(fActMenuBar == menu) {
               return;
            }
            TGFrameElement* nw;
            TIter           nel(fMenuFrame->GetList());
            while((nw = static_cast<TGFrameElement*>(nel())) != nullptr) {
               if(nw->fFrame == menu) {
                  ShowMenu(menu);
                  return;
               }
            }
            const_cast<TGCompositeFrame*>(static_cast<const TGCompositeFrame*>(menu->GetParent()))->HideFrame(menu);
            const_cast<TGCompositeFrame*>(static_cast<const TGCompositeFrame*>(menu->GetParent()))
               ->SetCleanup(kNoCleanup);
            menu->ReparentWindow(fMenuFrame);
            fMenuFrame->AddFrame(menu, fLH2);
            TGFrameElement* mel;
            TIter           mnext(menu->GetList());
            while((mel = static_cast<TGFrameElement*>(mnext())) != nullptr) {
               TGMenuTitle* t     = static_cast<TGMenuTitle*>(mel->fFrame);
               TGPopupMenu* popup = menu->GetPopup(t->GetName());
               if(popup != nullptr) {
                  RecursiveReparent(popup);
                  if(popup->GetEntry("Close Canvas") != nullptr) {
                     TGMenuEntry* exit = popup->GetEntry("Close Canvas");
                     popup->HideEntry(exit->GetEntryId());
                  }
                  if(popup->GetEntry("Close Viewer") != nullptr) {
                     TGMenuEntry* exit = popup->GetEntry("Close Viewer");
                     popup->HideEntry(exit->GetEntryId());
                  }
                  if(popup->GetEntry("Quit ROOT") != nullptr) {
                     TGMenuEntry* exit = popup->GetEntry("Quit ROOT");
                     popup->HideEntry(exit->GetEntryId());
                  }
                  if(popup->GetEntry("Exit") != nullptr) {
                     TGMenuEntry* exit = popup->GetEntry("Exit");
                     popup->HideEntry(exit->GetEntryId());
                  }
               }
            }
            ShowMenu(menu);
            return;
         }
      }
   }
   if(fActMenuBar != fMenuBar) {
      ShowMenu(fMenuBar);
   }
}

//______________________________________________________________________________
void GRootBrowser::DoubleClicked(TObject* obj)
{
   /// Emits signal when double clicking on icon.
   Emit("DoubleClicked(TObject*)", (Long_t)obj);
}

//______________________________________________________________________________
void GRootBrowser::Checked(TObject* obj, Bool_t checked)
{
   /// Emits signal when double clicking on icon.

   Long_t args[2];

   args[0] = (Long_t)obj;
   args[1] = static_cast<Long_t>(checked);

   Emit("Checked(TObject*,Bool_t)", args);
}

//______________________________________________________________________________
void GRootBrowser::ExecuteDefaultAction(TObject* obj)
{
   /// Emits signal "ExecuteDefaultAction(TObject*)".
   Emit("ExecuteDefaultAction(TObject*)", (Long_t)obj);
}

//______________________________________________________________________________
TBrowserImp* GRootBrowser::NewBrowser(TBrowser* b, const char* title, UInt_t width, UInt_t height, Option_t* opt)
{
   /// static contructor returning TBrowserImp,
   /// as needed by the plugin mechanism.

   auto* browser = new GRootBrowser(b, title, width, height, opt);
   return (TBrowserImp*)browser;
}

//______________________________________________________________________________
TBrowserImp* GRootBrowser::NewBrowser(TBrowser* b, const char* title, Int_t x, Int_t y, UInt_t width, UInt_t height,
                                      Option_t* opt)
{
   /// static contructor returning TBrowserImp,
   /// as needed by the plugin mechanism.

   auto* browser = new GRootBrowser(b, title, x, y, width, height, opt);
   return (TBrowserImp*)browser;
}
