#include "RConfigure.h"

#include "GRootCanvas.h"

#include "TRootApplication.h"
#include "TRootHelpDialog.h"
#include "TGClient.h"
#include "TGCanvas.h"
#include "TGMenu.h"
#include "TGWidget.h"
#include "TGFileDialog.h"
#include "TGStatusBar.h"
#include "TGTextEditDialogs.h"
#include "TROOT.h"
#include "TClass.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TPadPainter.h"
#include "TBrowser.h"
#include "TClassTree.h"
#include "TMarker.h"
#include "TStyle.h"
#include "TColorWheel.h"
#include "TVirtualX.h"
#include "TApplication.h"
#include "TFile.h"
#include "TInterpreter.h"
#include "TEnv.h"
#include "TMath.h"
#include "Riostream.h"
#include "TGDockableFrame.h"

#include "TG3DLine.h"
#include "TGToolBar.h"
#include "TGToolTip.h"
#include "TVirtualPadEditor.h"
#include "TRootControlBar.h"
#include "TGLabel.h"
#include "TGuiBuilder.h"
#include "TImage.h"
#include "TError.h"
#include "TGDNDManager.h"
#include "TBufferFile.h"
#include "TRootBrowser.h"
#include "TGTab.h"
#include "TGedEditor.h"

#include "TPluginManager.h"
#include "TVirtualGL.h"

#ifdef WIN32
#include "TWin32SplashThread.h"
#endif

#include "HelpText.h"

#include "GCanvas.h"

// Canvas menu command ids
enum ERootCanvasCommands {
   kFileNewCanvas,
   kFileOpen,
   kFileSaveAs,
   kFileSaveAsRoot,
   kFileSaveAsC,
   kFileSaveAsPS,
   kFileSaveAsEPS,
   kFileSaveAsPDF,
   kFileSaveAsGIF,
   kFileSaveAsJPG,
   kFileSaveAsPNG,
   kFilePrint,
   kFileCloseCanvas,
   kFileQuit,

   kEditStyle,
   kEditCut,
   kEditCopy,
   kEditPaste,
   kEditClearPad,
   kEditClearCanvas,
   kEditUndo,
   kEditRedo,

   kViewEditor,
   kViewToolbar,
   kViewEventStatus,
   kViewToolTips,
   kViewColors,
   kViewFonts,
   kViewMarkers,
   kViewIconify,
   kViewX3D,
   kViewOpenGL,

   kOptionAutoResize,
   kOptionResizeCanvas,
   kOptionMoveOpaque,
   kOptionResizeOpaque,
   kOptionInterrupt,
   kOptionRefresh,
   kOptionAutoExec,
   kOptionStatistics,
   kOptionHistTitle,
   kOptionFitParams,
   kOptionCanEdit,

   kInspectRoot,
   kClassesTree,
   kFitPanel,
   kToolsBrowser,
   kToolsBuilder,
   kToolsRecorder,

   kHelpAbout,
   kHelpOnCanvas,
   kHelpOnMenus,
   kHelpOnGraphicsEd,
   kHelpOnBrowser,
   kHelpOnObjects,
   kHelpOnPS,

   kToolModify,
   kToolArc,
   kToolLine,
   kToolArrow,
   kToolDiamond,
   kToolEllipse,
   kToolPad,
   kToolPave,
   kToolPLabel,
   kToolPText,
   kToolPsText,
   kToolGraph,
   kToolCurlyLine,
   kToolCurlyArc,
   kToolLatex,
   kToolMarker,
   kToolCutG

};

#define kArrowKeyPress 25
#define kArrowKeyRelease 26

#define kButton1Ctrl 9
#define kButton1CtrlMotion 10

static std::array<const char*, 6> gOpenTypes = {"ROOT files", "*.root", "All files", "*", nullptr, nullptr};

static std::array<const char*, 32> gSaveAsTypes = {"PDF", "*.pdf", "PostScript", "*.ps", "Encapsulated PostScript",
                                                   "*.eps", "SVG", "*.svg", "TeX", "*.tex",
                                                   "GIF", "*.gif", "ROOT macros", "*.C", "ROOT files",
                                                   "*.root", "XML", "*.xml", "PNG", "*.png",
                                                   "XPM", "*.xpm", "JPEG", "*.jpg", "TIFF",
                                                   "*.tiff", "XCF", "*.xcf", "All files", "*",
                                                   nullptr, nullptr};

static std::array<ToolBarData_t, 11> gToolBarData = {{//{ filename, tooltip, staydown,id, button}
                                                      {"newcanvas.xpm", "New", kFALSE, kFileNewCanvas, nullptr},
                                                      {"open.xpm", "Open", kFALSE, kFileOpen, nullptr},
                                                      {"save.xpm", "Save As", kFALSE, kFileSaveAs, nullptr},
                                                      {"printer.xpm", "Print", kFALSE, kFilePrint, nullptr},
                                                      {"", "", kFALSE, -1, nullptr},
                                                      {"interrupt.xpm", "Interrupt", kFALSE, kOptionInterrupt, nullptr},
                                                      {"refresh2.xpm", "Refresh", kFALSE, kOptionRefresh, nullptr},
                                                      {"", "", kFALSE, -1, nullptr},
                                                      {"inspect.xpm", "Inspect", kFALSE, kInspectRoot, nullptr},
                                                      {"browser.xpm", "Browser", kFALSE, kToolsBrowser, nullptr},
                                                      {nullptr, nullptr, kFALSE, 0, nullptr}}};

static std::array<ToolBarData_t, 18> gToolBarData1 = {{{"pointer.xpm", "Modify", kFALSE, kToolModify, nullptr},
                                                       {"arc.xpm", "Arc", kFALSE, kToolArc, nullptr},
                                                       {"line.xpm", "Line", kFALSE, kToolLine, nullptr},
                                                       {"arrow.xpm", "Arrow", kFALSE, kToolArrow, nullptr},
                                                       {"diamond.xpm", "Diamond", kFALSE, kToolDiamond, nullptr},
                                                       {"ellipse.xpm", "Ellipse", kFALSE, kToolEllipse, nullptr},
                                                       {"pad.xpm", "Pad", kFALSE, kToolPad, nullptr},
                                                       {"pave.xpm", "Pave", kFALSE, kToolPave, nullptr},
                                                       {"pavelabel.xpm", "Pave Label", kFALSE, kToolPLabel, nullptr},
                                                       {"pavetext.xpm", "Pave Text", kFALSE, kToolPText, nullptr},
                                                       {"pavestext.xpm", "Paves Text", kFALSE, kToolPsText, nullptr},
                                                       {"graph.xpm", "Graph", kFALSE, kToolGraph, nullptr},
                                                       {"curlyline.xpm", "Curly Line", kFALSE, kToolCurlyLine, nullptr},
                                                       {"curlyarc.xpm", "Curly Arc", kFALSE, kToolCurlyArc, nullptr},
                                                       {"latex.xpm", "Text/Latex", kFALSE, kToolLatex, nullptr},
                                                       {"marker.xpm", "Marker", kFALSE, kToolMarker, nullptr},
                                                       {"cut.xpm", "Graphical Cut", kFALSE, kToolCutG, nullptr},
                                                       {nullptr, nullptr, kFALSE, 0, nullptr}}};

/////////////////////////////////////////////////////////////////////////
///
/// \class GRootContainer
///
/// Utility class used by GRootCanvas. The GRootContainer is the frame
/// embedded in the TGCanvas widget. The ROOT graphics goes into this
/// frame. This class is used to enable input events on this graphics
/// frame and forward the events to the GRootCanvas handlers.
///
/////////////////////////////////////////////////////////////////////////

class GRootContainer : public TGCompositeFrame {
private:
   GRootCanvas* fCanvas;   // pointer back to canvas imp
public:
   GRootContainer(GRootCanvas* c, Window_t id, const TGWindow* p);

   Bool_t HandleButton(Event_t* event) override;
   Bool_t HandleDoubleClick(Event_t* ev) override { return fCanvas->HandleContainerDoubleClick(ev); }
   Bool_t HandleConfigureNotify(Event_t* ev) override
   {
      TGFrame::HandleConfigureNotify(ev);
      return fCanvas->HandleContainerConfigure(ev);
   }
   Bool_t HandleKey(Event_t* ev) override { return fCanvas->HandleContainerKey(ev); }
   Bool_t HandleMotion(Event_t* ev) override { return fCanvas->HandleContainerMotion(ev); }
   Bool_t HandleExpose(Event_t* ev) override { return fCanvas->HandleContainerExpose(ev); }
   Bool_t HandleCrossing(Event_t* ev) override { return fCanvas->HandleContainerCrossing(ev); }
   void   SavePrimitive(std::ostream& out, Option_t* = "") override;
   void   SetEditable(Bool_t) override {}
};

//______________________________________________________________________________
GRootContainer::GRootContainer(GRootCanvas* canvas, Window_t id, const TGWindow* p) : TGCompositeFrame(gClient, id, p), fCanvas(canvas)
{
   /// Create a canvas container.

   gVirtualX->GrabButton(fId, kAnyButton, kAnyModifier, kButtonPressMask | kButtonReleaseMask | kPointerMotionMask, kNone, kNone);

   AddInput(kKeyPressMask | kKeyReleaseMask | kPointerMotionMask | kExposureMask | kStructureNotifyMask | kLeaveWindowMask);
   fEditDisabled = kEditDisable;
}

//______________________________________________________________________________
Bool_t GRootContainer::HandleButton(Event_t* event)
{
   /// Directly handle scroll mouse buttons (4 and 5), only pass buttons
   /// 1, 2 and 3 on to the TCanvas.

   const auto* viewPort = static_cast<const TGViewPort*>(fParent);
   UInt_t      page     = viewPort->GetHeight() / 4;

   gVirtualX->SetInputFocus(GetMainFrame()->GetId());

   if(event->fCode == kButton4) {
      // scroll up
      auto newpos = fCanvas->fCanvasWindow->GetVsbPosition() - page;
      fCanvas->fCanvasWindow->SetVsbPosition(newpos);
   }
   if(event->fCode == kButton5) {
      // scroll down
      auto newpos = fCanvas->fCanvasWindow->GetVsbPosition() + page;
      fCanvas->fCanvasWindow->SetVsbPosition(newpos);
   }
   return fCanvas->HandleContainerButton(event);
}

//______________________________________________________________________________
GRootCanvas::GRootCanvas(GCanvas* canvas, const char* name, UInt_t width, UInt_t height)
   : TGMainFrame(gClient->GetRoot(), width, height), TCanvasImp(canvas)
{
   /// Create a basic ROOT canvas.
   CreateCanvas(name);

   ShowToolBar(kFALSE);
   ShowEditor(kFALSE);

   Resize(width, height);
}

//______________________________________________________________________________
GRootCanvas::GRootCanvas(GCanvas* canvas, const char* name, Int_t x, Int_t y, UInt_t width, UInt_t height)
   : TGMainFrame(gClient->GetRoot(), width, height), TCanvasImp(canvas)
{
   /// Create a basic ROOT canvas.
   CreateCanvas(name);

   ShowToolBar(kFALSE);
   ShowEditor(kFALSE);

   MoveResize(x, y, width, height);
   SetWMPosition(x, y);
}

//______________________________________________________________________________
void GRootCanvas::CreateCanvas(const char* name)
{
   /// Create the actual canvas.

   fButton   = 0;
   fAutoFit  = kTRUE;   // check also menu entry
   fEditor   = nullptr;
   fEmbedded = kFALSE;

   // Create menus
   fFileSaveMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fFileSaveMenu->AddEntry(Form("%s.&ps", name), kFileSaveAsPS);
   fFileSaveMenu->AddEntry(Form("%s.&eps", name), kFileSaveAsEPS);
   fFileSaveMenu->AddEntry(Form("%s.p&df", name), kFileSaveAsPDF);
   fFileSaveMenu->AddEntry(Form("%s.&gif", name), kFileSaveAsGIF);

   static Int_t img = 0;

   if(img == 0) {
      Int_t sav         = gErrorIgnoreLevel;
      gErrorIgnoreLevel = kFatal;
      img               = TImage::Create() != nullptr ? 1 : -1;
      gErrorIgnoreLevel = sav;
   }
   if(img > 0) {
      fFileSaveMenu->AddEntry(Form("%s.&jpg", name), kFileSaveAsJPG);
      fFileSaveMenu->AddEntry(Form("%s.&png", name), kFileSaveAsPNG);
   }

   fFileSaveMenu->AddEntry(Form("%s.&C", name), kFileSaveAsC);
   fFileSaveMenu->AddEntry(Form("%s.&root", name), kFileSaveAsRoot);

   fFileMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fFileMenu->AddEntry("&New Canvas", kFileNewCanvas);
   fFileMenu->AddEntry("&Open...", kFileOpen);
   fFileMenu->AddEntry("&Close Canvas", kFileCloseCanvas);
   fFileMenu->AddSeparator();
   fFileMenu->AddPopup("&Save", fFileSaveMenu);
   fFileMenu->AddEntry("Save &As...", kFileSaveAs);
   fFileMenu->AddSeparator();
   fFileMenu->AddEntry("&Print...", kFilePrint);
   fFileMenu->AddSeparator();
   fFileMenu->AddEntry("&Quit ROOT", kFileQuit);

   fEditClearMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fEditClearMenu->AddEntry("&Pad", kEditClearPad);
   fEditClearMenu->AddEntry("&Canvas", kEditClearCanvas);

   fEditMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fEditMenu->AddEntry("&Style...", kEditStyle);
   fEditMenu->AddSeparator();
   fEditMenu->AddEntry("Cu&t", kEditCut);
   fEditMenu->AddEntry("&Copy", kEditCopy);
   fEditMenu->AddEntry("&Paste", kEditPaste);
   fEditMenu->AddSeparator();
   fEditMenu->AddPopup("C&lear", fEditClearMenu);
   fEditMenu->AddSeparator();
   fEditMenu->AddEntry("&Undo", kEditUndo);
   fEditMenu->AddEntry("&Redo", kEditRedo);

   fEditMenu->DisableEntry(kEditCut);
   fEditMenu->DisableEntry(kEditCopy);
   fEditMenu->DisableEntry(kEditPaste);
   fEditMenu->DisableEntry(kEditUndo);
   fEditMenu->DisableEntry(kEditRedo);

   fViewWithMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fViewWithMenu->AddEntry("&X3D", kViewX3D);
   fViewWithMenu->AddEntry("&OpenGL", kViewOpenGL);

   fViewMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fViewMenu->AddEntry("&Editor", kViewEditor);
   fViewMenu->AddEntry("&Toolbar", kViewToolbar);
   fViewMenu->AddEntry("Event &Statusbar", kViewEventStatus);
   fViewMenu->AddEntry("T&oolTip Info", kViewToolTips);
   fViewMenu->AddSeparator();
   fViewMenu->AddEntry("&Colors", kViewColors);
   fViewMenu->AddEntry("&Fonts", kViewFonts);
   fViewMenu->AddEntry("&Markers", kViewMarkers);
   fViewMenu->AddSeparator();
   fViewMenu->AddEntry("&Iconify", kViewIconify);
   fViewMenu->AddSeparator();
   fViewMenu->AddPopup("&View With", fViewWithMenu);

   fViewMenu->DisableEntry(kViewFonts);

   fOptionMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fOptionMenu->AddEntry("&Auto Resize Canvas", kOptionAutoResize);
   fOptionMenu->AddEntry("&Resize Canvas", kOptionResizeCanvas);
   fOptionMenu->AddEntry("&Move Opaque", kOptionMoveOpaque);
   fOptionMenu->AddEntry("Resize &Opaque", kOptionResizeOpaque);
   fOptionMenu->AddSeparator();
   fOptionMenu->AddEntry("&Interrupt", kOptionInterrupt);
   fOptionMenu->AddEntry("R&efresh", kOptionRefresh);
   fOptionMenu->AddSeparator();
   fOptionMenu->AddEntry("&Pad Auto Exec", kOptionAutoExec);
   fOptionMenu->AddSeparator();
   fOptionMenu->AddEntry("&Statistics", kOptionStatistics);
   fOptionMenu->AddEntry("Histogram &Title", kOptionHistTitle);
   fOptionMenu->AddEntry("&Fit Parameters", kOptionFitParams);
   fOptionMenu->AddEntry("Can Edit &Histograms", kOptionCanEdit);

   // Opaque options initialized in InitWindow()
   fOptionMenu->CheckEntry(kOptionAutoResize);
   if(gStyle->GetOptStat() != 0) {
      fOptionMenu->CheckEntry(kOptionStatistics);
   }
   if(gStyle->GetOptTitle() != 0) {
      fOptionMenu->CheckEntry(kOptionHistTitle);
   }
   if(gStyle->GetOptFit() != 0) {
      fOptionMenu->CheckEntry(kOptionFitParams);
   }
   if(gROOT->GetEditHistograms()) {
      fOptionMenu->CheckEntry(kOptionCanEdit);
   }

   fToolsMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fToolsMenu->AddEntry("&Inspect ROOT", kInspectRoot);
   fToolsMenu->AddEntry("&Class Tree", kClassesTree);
   fToolsMenu->AddEntry("&Fit Panel", kFitPanel);
   fToolsMenu->AddEntry("&Start Browser", kToolsBrowser);
   fToolsMenu->AddEntry("&Gui Builder", kToolsBuilder);
   fToolsMenu->AddEntry("&Event Recorder", kToolsRecorder);

   fHelpMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fHelpMenu->AddLabel("Basic Help On...");
   fHelpMenu->AddSeparator();
   fHelpMenu->AddEntry("&Canvas", kHelpOnCanvas);
   fHelpMenu->AddEntry("&Menus", kHelpOnMenus);
   fHelpMenu->AddEntry("&Graphics Editor", kHelpOnGraphicsEd);
   fHelpMenu->AddEntry("&Browser", kHelpOnBrowser);
   fHelpMenu->AddEntry("&Objects", kHelpOnObjects);
   fHelpMenu->AddEntry("&PostScript", kHelpOnPS);
   fHelpMenu->AddSeparator();
   fHelpMenu->AddEntry("&About ROOT...", kHelpAbout);

   // This main frame will process the menu commands
   fFileMenu->Associate(this);
   fFileSaveMenu->Associate(this);
   fEditMenu->Associate(this);
   fEditClearMenu->Associate(this);
   fViewMenu->Associate(this);
   fViewWithMenu->Associate(this);
   fOptionMenu->Associate(this);
   fToolsMenu->Associate(this);
   fHelpMenu->Associate(this);

   // Create menubar layout hints
   fMenuBarLayout     = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 1, 1);
   fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
   fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

   // Create menubar
   fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
   fMenuBar->AddPopup("&File", fFileMenu, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Edit", fEditMenu, fMenuBarItemLayout);
   fMenuBar->AddPopup("&View", fViewMenu, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Options", fOptionMenu, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Tools", fToolsMenu, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Help", fHelpMenu, fMenuBarHelpLayout);

   AddFrame(fMenuBar, fMenuBarLayout);

   fHorizontal1       = new TGHorizontal3DLine(this);
   fHorizontal1Layout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
   AddFrame(fHorizontal1, fHorizontal1Layout);

   // Create toolbar dock
   fToolDock = new TGDockableFrame(this);
   fToolDock->SetCleanup();
   fToolDock->EnableHide(kFALSE);
   AddFrame(fToolDock, fDockLayout = new TGLayoutHints(kLHintsExpandX));

   // will alocate it later
   fToolBar         = nullptr;
   fVertical1       = nullptr;
   fVertical2       = nullptr;
   fVertical1Layout = nullptr;
   fVertical2Layout = nullptr;

   fToolBarSep    = new TGHorizontal3DLine(this);
   fToolBarLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
   AddFrame(fToolBarSep, fToolBarLayout);

   fMainFrame       = new TGCompositeFrame(this, GetWidth() + 4, GetHeight() + 4, kHorizontalFrame);
   fMainFrameLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);

   // Create editor frame that will host the pad editor
   fEditorFrame  = new TGCompositeFrame(fMainFrame, 175, fMainFrame->GetHeight() + 4, kFixedWidth);
   fEditorLayout = new TGLayoutHints(kLHintsExpandY | kLHintsLeft);
   fMainFrame->AddFrame(fEditorFrame, fEditorLayout);

   // Create canvas and canvas container that will host the ROOT graphics
   fCanvasWindow = new TGCanvas(fMainFrame, GetWidth() + 4, GetHeight() + 4, kSunkenFrame | kDoubleBorder);

   fCanvasID = -1;

   if(fCanvas->UseGL()) {
      fCanvas->SetSupportGL(kFALSE);
      // first, initialize GL (if not yet)
      if(gGLManager == nullptr) {
         TString osString = "win32";
         if(gVirtualX->InheritsFrom("TGX11")) {
            osString = "x11";
         } else if(gVirtualX->InheritsFrom("TGCocoa")) {
            osString = "osx";
         }
         TPluginHandler* pluginHandler = gROOT->GetPluginManager()->FindHandler("TGLManager", osString);

         if(pluginHandler != nullptr && pluginHandler->LoadPlugin() != -1) {
            if(pluginHandler->ExecPlugin(0) == 0) {
               Error("CreateCanvas", "GL manager plugin failed");
            }
         }
      } else {
         fCanvasID = gGLManager->InitGLWindow(static_cast<ULong_t>(fCanvasWindow->GetViewPort()->GetId()));
         if(fCanvasID != -1) {
            // Create gl context.
            const Int_t glCtx = gGLManager->CreateGLContext(fCanvasID);
            if(glCtx != -1) {
               fCanvas->SetSupportGL(kTRUE);
               fCanvas->SetGLDevice(glCtx);   // Now, fCanvas is responsible for context deletion!
            } else {
               Error("CreateCanvas", "GL context creation failed.");
            }
         } else {
            Error("CreateCanvas", "GL window creation failed\n");
         }
      }
   }

   if(fCanvasID == -1) {
      fCanvasID = gVirtualX->InitWindow(static_cast<ULong_t>(fCanvasWindow->GetViewPort()->GetId()));
   }

   Window_t win     = gVirtualX->GetWindowID(fCanvasID);
   fCanvasContainer = new GRootContainer(this, win, fCanvasWindow->GetViewPort());
   fCanvasWindow->SetContainer(fCanvasContainer);
   fCanvasLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY | kLHintsRight);

   fMainFrame->AddFrame(fCanvasWindow, fCanvasLayout);
   AddFrame(fMainFrame, fMainFrameLayout);

   // create the tooltip with a timeout of 250 ms
   fToolTip = new TGToolTip(fClient->GetDefaultRoot(), fCanvasWindow, "", 250);

   fCanvas->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "GRootCanvas", this,
                    "EventInfo(Int_t, Int_t, Int_t, TObject*)");

   // Create status bar
   std::array<int, 4> parts = {33, 10, 10, 47};
   fStatusBar               = new TGStatusBar(this, 10, 10);
   fStatusBar->SetParts(parts.data(), parts.size());

   fStatusBarLayout = new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 2, 2, 1, 1);

   AddFrame(fStatusBar, fStatusBarLayout);

   // Misc
   SetWindowName(name);
   SetIconName(name);
   fIconPic = SetIconPixmap("macro_s.xpm");
   SetClassHints("ROOT", "Canvas");

   SetEditDisabled(kEditDisable);
   MapSubwindows();

   // by default status bar, tool bar and pad editor are hidden
   HideFrame(fStatusBar);
   HideFrame(fToolDock);
   HideFrame(fToolBarSep);
   HideFrame(fHorizontal1);

   ShowToolBar(kFALSE);
   ShowEditor(kFALSE);

   // we need to use GetDefaultSize() to initialize the layout algorithm...
   Resize(GetDefaultSize());

   gVirtualX->SetDNDAware(fId, fDNDTypeList);
   SetDNDTarget(kTRUE);
}

//______________________________________________________________________________
GRootCanvas::~GRootCanvas()
{
   /// Delete ROOT basic canvas. Order is significant. Delete in reverse
   /// order of creation.

   delete fToolTip;
   if(fIconPic != nullptr) {
      gClient->FreePicture(fIconPic);
   }
   if((fEditor != nullptr) && !fEmbedded) {
      delete fEditor;
   }
   if(fToolBar != nullptr) {
      Disconnect(fToolDock, "Docked()", this, "AdjustSize()");
      Disconnect(fToolDock, "Undocked()", this, "AdjustSize()");
      fToolBar->Cleanup();
      delete fToolBar;
   }

   if(MustCleanup() == 0) {
      delete fStatusBar;
      delete fStatusBarLayout;
      delete fCanvasContainer;
      delete fCanvasWindow;

      delete fEditorFrame;
      delete fEditorLayout;
      delete fMainFrame;
      delete fMainFrameLayout;
      delete fToolBarSep;
#ifndef __APPLE__
      delete fToolDock;
#endif
      delete fToolBarLayout;
      delete fHorizontal1;
      delete fHorizontal1Layout;
      delete fMenuBarLayout;
      delete fMenuBarItemLayout;
      delete fMenuBarHelpLayout;
      delete fCanvasLayout;
      delete fDockLayout;
   }

   delete fFileMenu;
   delete fFileSaveMenu;
   delete fEditMenu;
   delete fEditClearMenu;
   delete fViewMenu;
   delete fViewWithMenu;
   delete fOptionMenu;
   delete fToolsMenu;
   delete fHelpMenu;
}

//______________________________________________________________________________
void GRootCanvas::Close()
{
   /// Called via TCanvasImp interface by TCanvas.

   TVirtualPadEditor* gged = TVirtualPadEditor::GetPadEditor(kFALSE);
   if((gged != nullptr) && gged->GetCanvas() == fCanvas) {
      if(fEmbedded) {
         static_cast<TGedEditor*>(gged)->SetModel(nullptr, nullptr, kButton1Down);
         static_cast<TGedEditor*>(gged)->SetCanvas(nullptr);
      } else {
         gged->Hide();
      }
   }

   gVirtualX->CloseWindow();
}

//______________________________________________________________________________
void GRootCanvas::ReallyDelete()
{
   /// Really delete the canvas and this GUI.

   TVirtualPadEditor* gged = TVirtualPadEditor::GetPadEditor(kFALSE);
   if((gged != nullptr) && gged->GetCanvas() == fCanvas) {
      if(fEmbedded) {
         static_cast<TGedEditor*>(gged)->SetModel(nullptr, nullptr, kButton1Down);
         static_cast<TGedEditor*>(gged)->SetCanvas(nullptr);
      } else {
         gged->Hide();
      }
   }

   fToolTip->Hide();
   Disconnect(fCanvas, "ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", this,
              "EventInfo(Int_t, Int_t, Int_t, TObject*)");

   TVirtualPad* savepad = gPad;
   gPad                 = nullptr;   // hide gPad from CINT
   gInterpreter->DeleteGlobal(fCanvas);
   gPad = savepad;   // restore gPad for ROOT
   delete fCanvas;   // will in turn delete this object
}

//______________________________________________________________________________
void GRootCanvas::CloseWindow()
{
   /// In case window is closed via WM we get here.

   DeleteWindow();
}

//______________________________________________________________________________
UInt_t GRootCanvas::GetCwidth() const
{
   /// Return width of canvas container.

   return fCanvasContainer->GetWidth();
}

//______________________________________________________________________________
UInt_t GRootCanvas::GetCheight() const
{
   /// Return height of canvas container.

   return fCanvasContainer->GetHeight();
}

//______________________________________________________________________________
UInt_t GRootCanvas::GetWindowGeometry(Int_t& x, Int_t& y, UInt_t& w, UInt_t& h)
{
   /// Gets the size and position of the window containing the canvas. This
   /// size includes the menubar and borders.

   gVirtualX->GetWindowSize(fId, x, y, w, h);

   Window_t childdum = 0;
   gVirtualX->TranslateCoordinates(fId, gClient->GetDefaultRoot()->GetId(), 0, 0, x, y, childdum);
   if(!fCanvas->GetShowEditor()) {
      return 0;
   }
   return fEditorFrame->GetWidth();
}

//______________________________________________________________________________
void GRootCanvas::SetStatusText(const char* txt, Int_t partidx)
{
   /// Set text in status bar.

   fStatusBar->SetText(txt, partidx);
}

//______________________________________________________________________________
Bool_t GRootCanvas::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   /// Handle menu and other command generated by the user.

   switch(GET_MSG(msg)) {

   case kC_COMMAND:

      switch(GET_SUBMSG(msg)) {

      case kCM_BUTTON:
      case kCM_MENU:

         switch(parm1) {
         // Handle toolbar items...
         case kToolModify: gROOT->SetEditorMode(); break;
         case kToolArc: gROOT->SetEditorMode("Arc"); break;
         case kToolLine: gROOT->SetEditorMode("Line"); break;
         case kToolArrow: gROOT->SetEditorMode("Arrow"); break;
         case kToolDiamond: gROOT->SetEditorMode("Diamond"); break;
         case kToolEllipse: gROOT->SetEditorMode("Ellipse"); break;
         case kToolPad: gROOT->SetEditorMode("Pad"); break;
         case kToolPave: gROOT->SetEditorMode("Pave"); break;
         case kToolPLabel: gROOT->SetEditorMode("PaveLabel"); break;
         case kToolPText: gROOT->SetEditorMode("PaveText"); break;
         case kToolPsText: gROOT->SetEditorMode("PavesText"); break;
         case kToolGraph: gROOT->SetEditorMode("PolyLine"); break;
         case kToolCurlyLine: gROOT->SetEditorMode("CurlyLine"); break;
         case kToolCurlyArc: gROOT->SetEditorMode("CurlyArc"); break;
         case kToolLatex: gROOT->SetEditorMode("Text"); break;
         case kToolMarker: gROOT->SetEditorMode("Marker"); break;
         case kToolCutG:
            gROOT->SetEditorMode("CutG");
            break;

         // Handle File menu items...
         case kFileNewCanvas:
            gROOT->MakeDefCanvas();
            // GCanvas::MakeDefCanvas();
            break;
         case kFileOpen: {
            static TString dir(".");
            TGFileInfo     fileInfo;
            fileInfo.fFileTypes = gOpenTypes.data();
            fileInfo.fIniDir    = StrDup(dir);
            new TGFileDialog(fClient->GetDefaultRoot(), this, kFDOpen, &fileInfo);
            if(fileInfo.fFilename == nullptr) {
               return kTRUE;
            }
            dir = fileInfo.fIniDir;
            new TFile(fileInfo.fFilename, "update");
         } break;
         case kFileSaveAs: {
            TString        workdir = gSystem->WorkingDirectory();
            static TString dir(".");
            static Int_t   typeidx = 0;
            static Bool_t  overwr  = kFALSE;
            TGFileInfo     fileInfo;
            TString        defaultType = gEnv->GetValue("Canvas.SaveAsDefaultType", ".pdf");
            if(typeidx == 0) {
               for(int i = 1; gSaveAsTypes[i] != nullptr; i += 2) {
                  TString ftype = gSaveAsTypes[i];
                  if(ftype.EndsWith(defaultType.Data())) {
                     typeidx = i - 1;
                     break;
                  }
               }
            }
            fileInfo.fFileTypes   = gSaveAsTypes.data();
            fileInfo.fIniDir      = StrDup(dir);
            fileInfo.fFileTypeIdx = typeidx;
            fileInfo.fOverwrite   = overwr;
            new TGFileDialog(fClient->GetDefaultRoot(), this, kFDSave, &fileInfo);
            gSystem->ChangeDirectory(workdir.Data());
            if(fileInfo.fFilename == nullptr) {
               return kTRUE;
            }
            Bool_t  appendedType = kFALSE;
            TString fn           = fileInfo.fFilename;
            TString ft           = fileInfo.fFileTypes[fileInfo.fFileTypeIdx + 1];
            dir                  = fileInfo.fIniDir;
            typeidx              = fileInfo.fFileTypeIdx;
            overwr               = fileInfo.fOverwrite;
         again:
            if(fn.EndsWith(".root") || fn.EndsWith(".ps") || fn.EndsWith(".eps") || fn.EndsWith(".pdf") ||
               fn.EndsWith(".svg") || fn.EndsWith(".tex") || fn.EndsWith(".gif") || fn.EndsWith(".xml") ||
               fn.EndsWith(".xpm") || fn.EndsWith(".jpg") || fn.EndsWith(".png") || fn.EndsWith(".xcf") ||
               fn.EndsWith(".tiff")) {
               fCanvas->SaveAs(fn);
            } else if(fn.EndsWith(".C")) {
               fCanvas->SaveSource(fn);
            } else {
               if(!appendedType) {
                  if(ft.Index(".") != kNPOS) {
                     fn += ft(ft.Index("."), ft.Length());
                     appendedType = kTRUE;
                     goto again;   // NOLINT(cppcoreguidelines-avoid-goto)
                  }
               }
               Warning("ProcessMessage", "file %s cannot be saved with this extension", fileInfo.fFilename);
            }
            for(int i = 1; gSaveAsTypes[i] != nullptr; i += 2) {
               TString ftype = gSaveAsTypes[i];
               ftype.ReplaceAll("*.", ".");
               if(fn.EndsWith(ftype.Data())) {
                  typeidx = i - 1;
                  break;
               }
            }
         } break;
         case kFileSaveAsRoot: fCanvas->SaveAs(".root"); break;
         case kFileSaveAsC: fCanvas->SaveSource(); break;
         case kFileSaveAsPS: fCanvas->SaveAs(); break;
         case kFileSaveAsEPS: fCanvas->SaveAs(".eps"); break;
         case kFileSaveAsPDF: fCanvas->SaveAs(".pdf"); break;
         case kFileSaveAsGIF: fCanvas->SaveAs(".gif"); break;
         case kFileSaveAsJPG: fCanvas->SaveAs(".jpg"); break;
         case kFileSaveAsPNG: fCanvas->SaveAs(".png"); break;
         case kFilePrint: PrintCanvas(); break;
         case kFileCloseCanvas: SendCloseMessage(); break;
         case kFileQuit:
            if(!gApplication->ReturnFromRun()) {
               if((TVirtualPadEditor::GetPadEditor(kFALSE) != nullptr)) {
                  TVirtualPadEditor::Terminate();
               }
               SendCloseMessage();
            }
            if(TVirtualPadEditor::GetPadEditor(kFALSE) != nullptr) {
               TVirtualPadEditor::Terminate();
            }
            if(TClass::GetClass("TStyleManager") != nullptr) {
               gROOT->ProcessLine("TStyleManager::Terminate()");
            }
            gApplication->Terminate(0);
            break;

         // Handle Edit menu items...
         case kEditStyle:
            if(TClass::GetClass("TStyleManager") == nullptr) {
               gSystem->Load("libGed");
            }
            gROOT->ProcessLine("TStyleManager::Show()");
            break;
         case kEditCut:
            // still noop
            break;
         case kEditCopy:
            // still noop
            break;
         case kEditPaste:
            // still noop
            break;
         case kEditUndo:
            // noop
            break;
         case kEditRedo:
            // noop
            break;
         case kEditClearPad:
            gPad->Clear();
            gPad->Modified();
            gPad->Update();
            break;
         case kEditClearCanvas:
            fCanvas->Clear();
            fCanvas->Modified();
            fCanvas->Update();
            break;

         // Handle View menu items...
         case kViewEditor: fCanvas->ToggleEditor(); break;
         case kViewToolbar: fCanvas->ToggleToolBar(); break;
         case kViewEventStatus: fCanvas->ToggleEventStatus(); break;
         case kViewToolTips: fCanvas->ToggleToolTips(); break;
         case kViewColors: {
            TVirtualPad* padsav = gPad->GetCanvas();
            // This was the code with the old color table
            //   TCanvas *m = new TCanvas("colors","Color Table");
            //   TPad::DrawColorTable();
            //   m->Update();
            auto* wheel = new TColorWheel();
            wheel->Draw();

            // tp: with Cocoa, window is visible (and repainted)
            // before wheel->Draw() was called and you can see "empty"
            // canvas.
            gPad->Update();
            //
            if(padsav != nullptr) {
               padsav->cd();
            }
         } break;
         case kViewFonts:
            // noop
            break;
         case kViewMarkers: {
            TVirtualPad* padsav = gPad->GetCanvas();
            auto*        m      = new GCanvas("markers", "Marker Types", 600, 200);
            TMarker::DisplayMarkerTypes();
            m->Update();
            if(padsav != nullptr) {
               padsav->cd();
            }
         } break;
         case kViewIconify: Iconify(); break;
         case kViewX3D: gPad->GetViewer3D("x3d"); break;
         case kViewOpenGL:
            gPad->GetViewer3D("ogl");
            break;

         // Handle Option menu items...
         case kOptionAutoExec:
            fCanvas->ToggleAutoExec();
            if(fCanvas->GetAutoExec()) {
               fOptionMenu->CheckEntry(kOptionAutoExec);
            } else {
               fOptionMenu->UnCheckEntry(kOptionAutoExec);
            }
            break;
         case kOptionAutoResize: {
            fAutoFit         = fAutoFit ? kFALSE : kTRUE;
            unsigned int opt = fCanvasContainer->GetOptions();
            if(fAutoFit) {
               opt &= ~kFixedSize;
               fOptionMenu->CheckEntry(kOptionAutoResize);
            } else {
               opt |= kFixedSize;
               fOptionMenu->UnCheckEntry(kOptionAutoResize);
            }
            fCanvasContainer->ChangeOptions(opt);
            // in case of autofit this will generate a configure
            // event for the container and this will force the
            // update of the TCanvas
            // Layout();
         }
            Layout();
            break;
         case kOptionResizeCanvas: FitCanvas(); break;
         case kOptionMoveOpaque:
            if(fCanvas->OpaqueMoving()) {
               fCanvas->MoveOpaque(0);
               fOptionMenu->UnCheckEntry(kOptionMoveOpaque);
            } else {
               fCanvas->MoveOpaque(1);
               fOptionMenu->CheckEntry(kOptionMoveOpaque);
            }
            break;
         case kOptionResizeOpaque:
            if(fCanvas->OpaqueResizing()) {
               fCanvas->ResizeOpaque(0);
               fOptionMenu->UnCheckEntry(kOptionResizeOpaque);
            } else {
               fCanvas->ResizeOpaque(1);
               fOptionMenu->CheckEntry(kOptionResizeOpaque);
            }
            break;
         case kOptionInterrupt: gROOT->SetInterrupt(); break;
         case kOptionRefresh:
            fCanvas->Paint();
            fCanvas->Update();
            break;
         case kOptionStatistics:
            if(gStyle->GetOptStat() != 0) {
               gStyle->SetOptStat(0);
               delete gPad->FindObject("stats");
               fOptionMenu->UnCheckEntry(kOptionStatistics);
            } else {
               gStyle->SetOptStat(1);
               fOptionMenu->CheckEntry(kOptionStatistics);
            }
            gPad->Modified();
            fCanvas->Update();
            break;
         case kOptionHistTitle:
            if(gStyle->GetOptTitle() != 0) {
               gStyle->SetOptTitle(0);
               delete gPad->FindObject("title");
               fOptionMenu->UnCheckEntry(kOptionHistTitle);
            } else {
               gStyle->SetOptTitle(1);
               fOptionMenu->CheckEntry(kOptionHistTitle);
            }
            gPad->Modified();
            fCanvas->Update();
            break;
         case kOptionFitParams:
            if(gStyle->GetOptFit() != 0) {
               gStyle->SetOptFit(0);
               fOptionMenu->UnCheckEntry(kOptionFitParams);
            } else {
               gStyle->SetOptFit(1);
               fOptionMenu->CheckEntry(kOptionFitParams);
            }
            gPad->Modified();
            fCanvas->Update();
            break;
         case kOptionCanEdit:
            if(gROOT->GetEditHistograms()) {
               gROOT->SetEditHistograms(kFALSE);
               fOptionMenu->UnCheckEntry(kOptionCanEdit);
            } else {
               gROOT->SetEditHistograms(kTRUE);
               fOptionMenu->CheckEntry(kOptionCanEdit);
            }
            break;

         // Handle Tools menu items...
         case kInspectRoot:
            fCanvas->cd();
            gROOT->Inspect();
            fCanvas->Update();
            break;
         case kToolsBrowser: new TBrowser("browser"); break;
         case kToolsBuilder: TGuiBuilder::Instance(); break;
         case kToolsRecorder:
            gROOT->ProcessLine("new TGRecorder()");
            break;

         // Handle Tools menu items...
         case kClassesTree: {
            TString cdef;
            auto*   canvasList = static_cast<TList*>(gROOT->GetListOfCanvases());
            if(canvasList->FindObject("ClassTree") != nullptr) {
               cdef = TString::Format("ClassTree_%d", canvasList->GetSize() + 1);
            } else {
               cdef = "ClassTree";
            }
            new TClassTree(cdef.Data(), "TObject");
            fCanvas->Update();
         } break;

         case kFitPanel: {
            // use plugin manager to create instance of TFitEditor
            TPluginHandler* handler = gROOT->GetPluginManager()->FindHandler("TFitEditor");
            if((handler != nullptr) && handler->LoadPlugin() != -1) {
               if(handler->ExecPlugin(2, fCanvas, 0) == 0) {
                  Error("FitPanel", "Unable to crate the FitPanel");
               }
            } else {
               Error("FitPanel", "Unable to find the FitPanel plug-in");
            }
         } break;

         // Handle Help menu items...
         case kHelpAbout: {
#ifdef R__UNIX
            TString rootx;
#ifdef ROOTBINDIR
            rootx = ROOTBINDIR;
#else
            rootx = gSystem->Getenv("ROOTSYS");
            if(!rootx.IsNull()) {
               rootx += "/bin";
            }
#endif   // ROOTBINDIR
            rootx += "/root -a &";
            gSystem->Exec(rootx);
#else   // R__UNIX
#ifdef WIN32
            new TWin32SplashThread(kTRUE);
#else

            std::ostringstream str;
            str << "About ROOT " << gROOT->GetVersion() << "...";
            helperDialog = new GRootHelpDialog(this, str.str(), 600, 400);
            helperDialog->SetText(gHelpAbout);
            helperDialog->Popup();
#endif   // WIN32
#endif   // R__UNIX
         } break;
         case kHelpOnCanvas: {
            auto* helperDialog = new TRootHelpDialog(this, "Help on Canvas...", 600, 400);
            helperDialog->SetText(gHelpCanvas);   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            helperDialog->Popup();
            break;
         }
         case kHelpOnMenus: {
            auto* helperDialog = new TRootHelpDialog(this, "Help on Menus...", 600, 400);
            helperDialog->SetText(gHelpPullDownMenus);   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            helperDialog->Popup();
            break;
         }
         case kHelpOnGraphicsEd: {
            auto* helperDialog = new TRootHelpDialog(this, "Help on Graphics Editor...", 600, 400);
            helperDialog->SetText(gHelpGraphicsEditor);   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            helperDialog->Popup();
            break;
         }
         case kHelpOnBrowser: {
            auto* helperDialog = new TRootHelpDialog(this, "Help on Browser...", 600, 400);
            helperDialog->SetText(gHelpBrowser);   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            helperDialog->Popup();
            break;
         }
         case kHelpOnObjects: {
            auto* helperDialog = new TRootHelpDialog(this, "Help on Objects...", 600, 400);
            helperDialog->SetText(gHelpObjects);   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            helperDialog->Popup();
            break;
         }
         case kHelpOnPS: {
            auto* helperDialog = new TRootHelpDialog(this, "Help on PostScript...", 600, 400);
            helperDialog->SetText(gHelpPostscript);   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            helperDialog->Popup();
            break;
         }
         }
      default: break;
      }
   default: break;
   }
   return kTRUE;
}

//______________________________________________________________________________
Int_t GRootCanvas::InitWindow()
{
   /// Called by TCanvas ctor to get window indetifier.

   if(fCanvas->OpaqueMoving()) {
      fOptionMenu->CheckEntry(kOptionMoveOpaque);
   }
   if(fCanvas->OpaqueResizing()) {
      fOptionMenu->CheckEntry(kOptionResizeOpaque);
   }

   return fCanvasID;
}

//______________________________________________________________________________
void GRootCanvas::SetCanvasSize(UInt_t w, UInt_t h)
{
   /// Set size of canvas container. Units in pixels.

   // turn off autofit, we want to stay at the given size
   fAutoFit = kFALSE;
   fOptionMenu->UnCheckEntry(kOptionAutoResize);
   unsigned int opt = fCanvasContainer->GetOptions();
   opt |= kFixedSize;   // turn on fixed size mode
   fCanvasContainer->ChangeOptions(opt);
   fCanvasContainer->SetWidth(w);
   fCanvasContainer->SetHeight(h);
   Layout();   // force layout (will update container to given size)
   fCanvas->Resize();
   fCanvas->Update();
}

//______________________________________________________________________________
void GRootCanvas::SetWindowPosition(Int_t x, Int_t y)
{
   /// Set canvas position (units in pixels).

   Move(x, y);
}

//______________________________________________________________________________
void GRootCanvas::SetWindowSize(UInt_t w, UInt_t h)
{
   /// Set size of canvas (units in pixels).

   Resize(w, h);

   // Make sure the change of size is really done.
   if(gThreadXAR == nullptr) {
      gSystem->ProcessEvents();
      gSystem->Sleep(10);
      gSystem->ProcessEvents();
   }
}

//______________________________________________________________________________
void GRootCanvas::RaiseWindow()
{
   /// Put canvas window on top of the window stack.

   gVirtualX->RaiseWindow(GetId());
}

//______________________________________________________________________________
void GRootCanvas::SetWindowTitle(const char* title)
{
   /// Change title on window.

   SetWindowName(title);
   SetIconName(title);
   fToolDock->SetWindowName(Form("ToolBar: %s", title));
}

//______________________________________________________________________________
void GRootCanvas::FitCanvas()
{
   /// Fit canvas container to current window size.

   if(!fAutoFit) {
      unsigned int opt  = fCanvasContainer->GetOptions();
      unsigned int oopt = opt;
      opt &= ~kFixedSize;   // turn off fixed size mode
      fCanvasContainer->ChangeOptions(opt);
      Layout();   // force layout
      fCanvas->Resize();
      fCanvas->Update();
      fCanvasContainer->ChangeOptions(oopt);
   }
}

//______________________________________________________________________________
void GRootCanvas::PrintCanvas()
{
   /// Print the canvas.

   Int_t          ret      = 0;
   Bool_t         pname    = kTRUE;
   char*          printer  = nullptr;
   char*          printCmd = nullptr;
   static TString sprinter;
   static TString sprintCmd;

   if(sprinter == "") {
      printer = StrDup(gEnv->GetValue("Print.Printer", ""));
   } else {
      printer = StrDup(sprinter);
   }
   if(sprintCmd == "") {
#ifndef WIN32
      printCmd = StrDup(gEnv->GetValue("Print.Command", ""));
#else
      printCmd = StrDup(gEnv->GetValue("Print.Command", "start AcroRd32.exe /p"));
#endif
   } else {
      printCmd = StrDup(sprintCmd);
   }

   new TGPrintDialog(fClient->GetDefaultRoot(), this, 400, 150, &printer, &printCmd, &ret);
   if(ret != 0) {
      sprinter  = printer;
      sprintCmd = printCmd;

      if(sprinter == "") {
         pname = kFALSE;
      }

      TString fn = "rootprint";
      FILE*   f  = gSystem->TempFileName(fn, gEnv->GetValue("Print.Directory", gSystem->TempDirectory()));
      if(f != nullptr) {
         fclose(f);
      }
      fn += TString::Format(".%s", gEnv->GetValue("Print.FileType", "pdf"));
      fCanvas->Print(fn);

      TString cmd = sprintCmd;
      if(cmd.Contains("%p")) {
         cmd.ReplaceAll("%p", sprinter);
      } else if(pname) {
         cmd += " ";
         cmd += sprinter;
         cmd += " ";
      }

      if(cmd.Contains("%f")) {
         cmd.ReplaceAll("%f", fn);
      } else {
         cmd += " ";
         cmd += fn;
         cmd += " ";
      }

      gSystem->Exec(cmd);
#ifndef WIN32
      gSystem->Unlink(fn);
#endif
   }
   delete[] printer;
   delete[] printCmd;
}

//______________________________________________________________________________
void GRootCanvas::EventInfo(Int_t event, Int_t px, Int_t py, TObject* selected)
{
   /// Display a tooltip with infos about the primitive below the cursor.

   fToolTip->Hide();
   if(!fCanvas->GetShowToolTips() || selected == nullptr || event != kMouseMotion || fButton != 0) {
      return;
   }
   TString tipInfo;
   TString objInfo = selected->GetObjectInfo(px, py);
   if(objInfo.BeginsWith("-")) {
      // if the string begins with '-', display only the object info
      objInfo.Remove(TString::kLeading, '-');
      tipInfo = objInfo;
   } else {
      const char* title = selected->GetTitle();
      tipInfo += TString::Format("%s::%s", selected->ClassName(), selected->GetName());
      if((title != nullptr) && (strlen(title) != 0u)) {
         tipInfo += TString::Format("\n%s", selected->GetTitle());
      }
      tipInfo += TString::Format("\n%d, %d", px, py);
      if(!objInfo.IsNull()) {
         tipInfo += TString::Format("\n%s", objInfo.Data());
      }
   }
   fToolTip->SetText(tipInfo.Data());
   fToolTip->SetPosition(px + 15, py + 15);
   fToolTip->Reset();
}

//______________________________________________________________________________
void GRootCanvas::ShowMenuBar(Bool_t show)
{
   /// Show or hide menubar.

   if(show) {
      ShowFrame(fMenuBar);
   } else {
      HideFrame(fMenuBar);
   }
}

//______________________________________________________________________________
void GRootCanvas::ShowStatusBar(Bool_t show)
{
   /// Show or hide statusbar.

   UInt_t dh = fClient->GetDisplayHeight();
   UInt_t ch = fCanvas->GetWindowHeight();

   UInt_t h  = GetHeight();
   UInt_t sh = fStatusBar->GetHeight() + 2;

   if(show) {
      ShowFrame(fStatusBar);
      fViewMenu->CheckEntry(kViewEventStatus);
      if(dh - ch >= sh) {
         h = h + sh;
      } else {
         h = ch;
      }
   } else {
      HideFrame(fStatusBar);
      fViewMenu->UnCheckEntry(kViewEventStatus);
      if(dh - ch < sh) {
         h = ch;
      } else {
         h = h - sh;
      }
   }
   Resize(GetWidth(), h);
}

//______________________________________________________________________________
void GRootCanvas::ShowEditor(Bool_t show)
{
   /// Show or hide side frame.

   TVirtualPad* savedPad = nullptr;
   savedPad              = gPad;
   gPad                  = Canvas();

   UInt_t w = GetWidth();
   UInt_t e = fEditorFrame->GetWidth();
   UInt_t h = GetHeight();
   UInt_t s = fHorizontal1->GetHeight();

   if((fParent != nullptr) && fParent != fClient->GetDefaultRoot()) {
      const auto* main = static_cast<const TGMainFrame*>(fParent->GetMainFrame());   // fParent is of type TGWindow, so GetMainFrame returns "const TGWindow*"
      fMainFrame->HideFrame(fEditorFrame);
      if((main != nullptr) && main->InheritsFrom("TRootBrowser")) {
         auto* browser = const_cast<TRootBrowser*>(static_cast<const TRootBrowser*>(main));   // NOLINT(cppcoreguidelines-pro-type-const-cast)
         if(!fEmbedded) {
            browser->GetTabRight()->Connect("Selected(Int_t)", "GRootCanvas", this, "Activated(Int_t)");
         }
         fEmbedded = kTRUE;
         if(show && ((fEditor == nullptr) || !static_cast<TGedEditor*>(fEditor)->IsMapped())) {
            if(browser->GetTabLeft()->GetTabTab("Pad Editor") == nullptr) {
               browser->StartEmbedding(TRootBrowser::kLeft);
               if(fEditor == nullptr) {
                  fEditor = TVirtualPadEditor::GetPadEditor(kTRUE);
               } else {
                  static_cast<TGedEditor*>(fEditor)->ReparentWindow(fClient->GetRoot());
                  static_cast<TGedEditor*>(fEditor)->MapWindow();
               }
               browser->StopEmbedding("Pad Editor");
               fEditor->SetGlobal(kFALSE);
               gROOT->GetListOfCleanups()->Remove(static_cast<TGedEditor*>(fEditor));
               if(fEditor != nullptr) {
                  static_cast<TGedEditor*>(fEditor)->SetCanvas(fCanvas);
                  static_cast<TGedEditor*>(fEditor)->SetModel(fCanvas, fCanvas, kButton1Down);
               }
            }
            fEditor = TVirtualPadEditor::GetPadEditor(kFALSE);
         }
         if(show) {
            browser->GetTabLeft()->SetTab("Pad Editor");
         }
      }
   } else {
      if(show) {
         if(fEditor == nullptr) {
            CreateEditor();
         }
         TVirtualPadEditor* gged = TVirtualPadEditor::GetPadEditor(kFALSE);
         if((gged != nullptr) && gged->GetCanvas() == fCanvas) {
            gged->Hide();
         }
         if(!fViewMenu->IsEntryChecked(kViewToolbar) || fToolDock->IsUndocked()) {
            ShowFrame(fHorizontal1);
            h = h + s;
         }
         fMainFrame->ShowFrame(fEditorFrame);
         fEditor->Show();
         fViewMenu->CheckEntry(kViewEditor);
         w = w + e;
      } else {
         if(!fViewMenu->IsEntryChecked(kViewToolbar) || fToolDock->IsUndocked()) {
            HideFrame(fHorizontal1);
            h = h - s;
         }
         if(fEditor != nullptr) {
            fEditor->Hide();
         }
         fMainFrame->HideFrame(fEditorFrame);
         fViewMenu->UnCheckEntry(kViewEditor);
         w = w - e;
      }
      Resize(w, h);
   }
   if(savedPad != nullptr) {
      gPad = savedPad;
   }
}

//______________________________________________________________________________
void GRootCanvas::CreateEditor()
{
   /// Create embedded editor.

   fEditorFrame->SetEditDisabled(kEditEnable);
   fEditorFrame->SetEditable();
   gPad = Canvas();
   // next two lines are related to the old editor
   Int_t show = gEnv->GetValue("Canvas.ShowEditor", 0);
   gEnv->SetValue("Canvas.ShowEditor", "true");
   fEditor = TVirtualPadEditor::LoadEditor();
   if(fEditor != nullptr) {
      fEditor->SetGlobal(kFALSE);
   }
   fEditorFrame->SetEditable(kEditDisable != 0u);
   fEditorFrame->SetEditable(kFALSE);

   // next line is related to the old editor
   if(show == 0) {
      gEnv->SetValue("Canvas.ShowEditor", "false");
   }
}

//______________________________________________________________________________
void GRootCanvas::ShowToolBar(Bool_t show)
{
   /// Show or hide toolbar.

   if(show && (fToolBar == nullptr)) {

      fToolBar = new TGToolBar(fToolDock, 60, 20, kHorizontalFrame);
      fToolDock->AddFrame(fToolBar, fHorizontal1Layout);

      Int_t spacing = 6;
      for(Int_t i = 0; gToolBarData[i].fPixmap != nullptr; i++) {
         if(strlen(gToolBarData[i].fPixmap) == 0) {
            spacing = 6;
            continue;
         }
         fToolBar->AddButton(this, &gToolBarData[i], spacing);
         spacing = 0;
      }
      fVertical1       = new TGVertical3DLine(fToolBar);
      fVertical2       = new TGVertical3DLine(fToolBar);
      fVertical1Layout = new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 4, 2, 0, 0);
      fVertical2Layout = new TGLayoutHints(kLHintsLeft | kLHintsExpandY);
      fToolBar->AddFrame(fVertical1, fVertical1Layout);
      fToolBar->AddFrame(fVertical2, fVertical2Layout);

      spacing = 6;
      for(Int_t i = 0; gToolBarData1[i].fPixmap != nullptr; i++) {
         if(strlen(gToolBarData1[i].fPixmap) == 0) {
            spacing = 6;
            continue;
         }
         fToolBar->AddButton(this, &gToolBarData1[i], spacing);
         spacing = 0;
      }
      fToolDock->MapSubwindows();
      fToolDock->Layout();
      fToolDock->SetWindowName(Form("ToolBar: %s", GetWindowName()));
      fToolDock->Connect("Docked()", "GRootCanvas", this, "AdjustSize()");
      fToolDock->Connect("Undocked()", "GRootCanvas", this, "AdjustSize()");
   }

   if(fToolBar == nullptr) {
      return;
   }

   UInt_t h  = GetHeight();
   UInt_t sh = fToolBarSep->GetHeight();
   UInt_t dh = fToolBar->GetHeight();

   if(show) {
      ShowFrame(fToolDock);
      if(!fViewMenu->IsEntryChecked(kViewEditor)) {
         ShowFrame(fHorizontal1);
         h = h + sh;
      }
      ShowFrame(fToolBarSep);
      fViewMenu->CheckEntry(kViewToolbar);
      h = h + dh + sh;
   } else {
      if(fToolDock->IsUndocked()) {
         fToolDock->DockContainer();
         h = h + 2 * sh;
      } else {
         h = h - dh;
      }

      HideFrame(fToolDock);
      if(!fViewMenu->IsEntryChecked(kViewEditor)) {
         HideFrame(fHorizontal1);
         h = h - sh;
      }
      HideFrame(fToolBarSep);
      h = h - sh;
      fViewMenu->UnCheckEntry(kViewToolbar);
   }
   Resize(GetWidth(), h);
}

//______________________________________________________________________________
void GRootCanvas::ShowToolTips(Bool_t show)
{
   /// Enable or disable tooltip info.

   if(show) {
      fViewMenu->CheckEntry(kViewToolTips);
   } else {
      fViewMenu->UnCheckEntry(kViewToolTips);
   }
}

//______________________________________________________________________________
Bool_t GRootCanvas::HasEditor() const
{
   /// Returns kTRUE if the editor is shown.

   return ((fEditor) != nullptr) && fViewMenu->IsEntryChecked(kViewEditor);
}

//______________________________________________________________________________
Bool_t GRootCanvas::HasMenuBar() const
{
   /// Returns kTRUE if the menu bar is shown.

   return ((fMenuBar) != nullptr) && fMenuBar->IsMapped();
}

//______________________________________________________________________________
Bool_t GRootCanvas::HasStatusBar() const
{
   /// Returns kTRUE if the status bar is shown.

   return ((fStatusBar) != nullptr) && fStatusBar->IsMapped();
}

//______________________________________________________________________________
Bool_t GRootCanvas::HasToolBar() const
{
   /// Returns kTRUE if the tool bar is shown.

   return ((fToolBar) != nullptr) && fToolBar->IsMapped();
}

//______________________________________________________________________________
Bool_t GRootCanvas::HasToolTips() const
{
   /// Returns kTRUE if the tooltips are enabled.

   return ((fCanvas) != nullptr) && fCanvas->GetShowToolTips();
}

//______________________________________________________________________________
void GRootCanvas::AdjustSize()
{
   /// Keep the same canvas size while docking/undocking toolbar.

   UInt_t h  = GetHeight();
   UInt_t dh = fToolBar->GetHeight();
   UInt_t sh = fHorizontal1->GetHeight();

   if(fToolDock->IsUndocked()) {
      if(!fViewMenu->IsEntryChecked(kViewEditor)) {
         HideFrame(fHorizontal1);
         h = h - sh;
      }
      HideFrame(fToolBarSep);
      h = h - dh - sh;
   } else {
      if(!fViewMenu->IsEntryChecked(kViewEditor)) {
         ShowFrame(fHorizontal1);
         h = h + sh;
      }
      ShowFrame(fToolBarSep);
      h = h + dh + sh;
   }
   Resize(GetWidth(), h);
}

//______________________________________________________________________________
Bool_t GRootCanvas::HandleContainerButton(Event_t* event)
{
   /// Handle mouse button events in the canvas container.

   UInt_t button = event->fCode;
   Int_t  x      = event->fX;
   Int_t  y      = event->fY;

   if(event->fType == kButtonPress) {
      if((fToolTip != nullptr) && fCanvas->GetShowToolTips()) {
         fToolTip->Hide();
         gVirtualX->UpdateWindow(0);
         gSystem->ProcessEvents();
      }
      fButton = button;
      if(button == kButton1) {
         if((event->fState & kKeyShiftMask) != 0u) {
            (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton1Shift, x, y);
         } else if((event->fState & kKeyControlMask) != 0u) {
            (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton1Ctrl, x, y);
         } else {
            (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton1Down, x, y);
         }
      }
      if(button == kButton2) {
         (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton2Down, x, y);
      }
      if(button == kButton3) {
         (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton3Down, x, y);
         fButton = 0;   // button up is consumed by TContextMenu
      }

   } else if(event->fType == kButtonRelease) {
      if(button == kButton4) {
         (static_cast<GCanvas*>(fCanvas))->HandleInput(kWheelUp, x, y);
      }
      if(button == kButton5) {
         (static_cast<GCanvas*>(fCanvas))->HandleInput(kWheelDown, x, y);
      }
      if(button == kButton1) {
         (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton1Up, x, y);
      }
      if(button == kButton2) {
         (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton2Up, x, y);
      }
      if(button == kButton3) {
         (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton3Up, x, y);
      }

      fButton = 0;
   }

   return kTRUE;
}

//______________________________________________________________________________
Bool_t GRootCanvas::HandleContainerDoubleClick(Event_t* event)
{
   /// Handle mouse button double click events in the canvas container.

   UInt_t button = event->fCode;
   Int_t  x      = event->fX;
   Int_t  y      = event->fY;

   if(button == kButton1) {
      (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton1Double, x, y);
   }
   if(button == kButton2) {
      (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton2Double, x, y);
   }
   if(button == kButton3) {
      (static_cast<GCanvas*>(fCanvas))->HandleInput(kButton3Double, x, y);
   }

   return kTRUE;
}

//______________________________________________________________________________
Bool_t GRootCanvas::HandleContainerConfigure(Event_t*)
{
   /// Handle configure (i.e. resize) event.
   if(fAutoFit) {
      fCanvas->Resize();
      fCanvas->Update();
   }

   if(fCanvas->HasFixedAspectRatio()) {
      // get menu height
      static UInt_t dh = 0;
      if(dh == 0) {
         dh = GetHeight() - fCanvasContainer->GetHeight();
      }
      UInt_t h = TMath::Nint(fCanvasContainer->GetWidth() / fCanvas->GetAspectRatio()) + dh;
      SetWindowSize(GetWidth(), h);
   }
   return kTRUE;
}

//______________________________________________________________________________
Bool_t GRootCanvas::HandleContainerKey(Event_t* event)
{
   /// Handle keyboard events in the canvas container.

   ///
   ///   This is the function I wanted to edit so bad that
   ///   I went through all the trouble to copy this thing.
   ///   This will allow us to take complete control of the
   ///   canvas interface make a more root-ish/radware-isf
   ///   work enviorment.      pcb.
   ///

   static EGEventType previous_event  = kOtherEvent;
   static UInt_t      previous_keysym = 0;

   UInt_t              keysym = 0;
   std::array<char, 2> str;
   gVirtualX->LookupString(event, str.data(), str.size(), keysym);

   if(event->fType == kGKeyPress) {
      fButton = event->fCode;

      if(str[0] == kESC) {   // ESC sets the escape flag
         gROOT->SetEscape();
         static_cast<GCanvas*>(fCanvas)->HandleInput(kButton1Up, 0, 0);
         static_cast<GCanvas*>(fCanvas)->HandleInput(kMouseMotion, 0, 0);
         gPad->Modified();
         return kTRUE;
      }
      if(str[0] == 3) {   // ctrl-c sets the interrupt flag
         gROOT->SetInterrupt();
      }

      // handle arrow keys
      if(keysym > 0x1011 && keysym < 0x1016) {
         Window_t dum1 = 0;
         Window_t dum2 = 0;
         Window_t wid  = 0;
         UInt_t   mask = 0;
         Int_t    mx   = 0;
         Int_t    my   = 0;
         Int_t    tx   = 0;
         Int_t    ty   = 0;
         wid           = gVirtualX->GetDefaultRootWindow();
         gVirtualX->QueryPointer(wid, dum1, dum2, mx, my, mx, my, mask);
         gVirtualX->TranslateCoordinates(gClient->GetDefaultRoot()->GetId(), fCanvasContainer->GetId(), mx, my, tx, ty, dum1);

         //((GCanvas*)fCanvas)->HandleInput(kArrowKeyPress, tx, ty);
         static_cast<GCanvas*>(fCanvas)->HandleInput(static_cast<EEventType>(kArrowKeyPress), tx, keysym);
         // handle case where we got consecutive same keypressed events coming
         // from auto-repeat on Windows (as it fires only successive keydown events)
         if((previous_keysym == keysym) && (previous_event == kGKeyPress)) {
            switch(keysym) {
            case 0x1012:   // left
               //                  gVirtualX->Warp(--mx, my, wid); --tx;
               break;
            case 0x1013:   // up
               //                  gVirtualX->Warp(mx, --my, wid); --ty;
               break;
            case 0x1014:   // right
               //                  gVirtualX->Warp(++mx, my, wid); ++tx;
               break;
            case 0x1015:   // down
               //                  gVirtualX->Warp(mx, ++my, wid); ++ty;
               break;
            default: break;
            }
            static_cast<GCanvas*>(fCanvas)->HandleInput(static_cast<EEventType>(kArrowKeyRelease), tx, ty);
         }
         previous_keysym = keysym;
      } else {
         static_cast<GCanvas*>(fCanvas)->HandleInput(kKeyPress, str[0], keysym);
      }
   } else if(event->fType == kKeyRelease) {
      if(keysym > 0x1011 && keysym < 0x1016) {
         Window_t dum1 = 0;
         Window_t dum2 = 0;
         Window_t wid  = 0;
         UInt_t   mask = 0;
         Int_t    mx   = 0;
         Int_t    my   = 0;
         Int_t    tx   = 0;
         Int_t    ty   = 0;
         wid           = gVirtualX->GetDefaultRootWindow();
         gVirtualX->QueryPointer(wid, dum1, dum2, mx, my, mx, my, mask);

         auto* gCanvas = static_cast<GCanvas*>(gPad->GetCanvas());
         gCanvas->HandleArrowKeyPress(event, &keysym);
         gVirtualX->TranslateCoordinates(gClient->GetDefaultRoot()->GetId(), fCanvasContainer->GetId(), mx, my, tx, ty,
                                         dum1);
         static_cast<GCanvas*>(fCanvas)->HandleInput(static_cast<EEventType>(kArrowKeyRelease), tx, ty);
         previous_keysym = keysym;
      } else {
         auto* gCanvas = static_cast<GCanvas*>(gPad->GetCanvas());
         gCanvas->HandleKeyboardPress(event, &keysym);
      }

      fButton = 0;
   }
   previous_event = event->fType;
   return kTRUE;
}

//______________________________________________________________________________
Bool_t GRootCanvas::HandleContainerMotion(Event_t* event)
{
   /// Handle mouse motion event in the canvas container.

   Int_t x = event->fX;
   Int_t y = event->fY;

   if(fButton == 0) {
      static_cast<GCanvas*>(fCanvas)->HandleInput(kMouseMotion, x, y);
   }
   if(fButton == kButton1) {
      if((event->fState & kKeyShiftMask) != 0u) {
         static_cast<GCanvas*>(fCanvas)->HandleInput(static_cast<EEventType>(8), x, y);
      } else {
         static_cast<GCanvas*>(fCanvas)->HandleInput(kButton1Motion, x, y);
      }
   }
   if(fButton == kButton2) {
      static_cast<GCanvas*>(fCanvas)->HandleInput(kButton2Motion, x, y);
   }

   return kTRUE;
}

//______________________________________________________________________________
Bool_t GRootCanvas::HandleContainerExpose(Event_t* event)
{
   /// Handle expose events.

   if(event->fCount == 0) {
      fCanvas->Flush();
   }

   return kTRUE;
}

//______________________________________________________________________________
Bool_t GRootCanvas::HandleContainerCrossing(Event_t* event)
{
   /// Handle enter/leave events. Only leave is activated at the moment.

   Int_t x = event->fX;
   Int_t y = event->fY;

   // pointer grabs create also an enter and leave event but with fCode
   // either kNotifyGrab or kNotifyUngrab, don't propagate these events
   if(event->fType == kLeaveNotify && event->fCode == kNotifyNormal) {
      (static_cast<GCanvas*>(fCanvas))->HandleInput(kMouseLeave, x, y);
   }

   return kTRUE;
}

//______________________________________________________________________________
Bool_t GRootCanvas::HandleDNDDrop(TDNDData* data)
{
   /// Handle drop events.

   static Atom_t rootObj = gVirtualX->InternAtom("application/root", kFALSE);
   static Atom_t uriObj  = gVirtualX->InternAtom("text/uri-list", kFALSE);

   if(data->fDataType == rootObj) {
      TBufferFile buf(TBuffer::kRead, data->fDataLength, data->fData);
      buf.SetReadMode();
      auto* obj = reinterpret_cast<TObject*>(buf.ReadObjectAny(TObject::Class()));
      if(obj == nullptr) {
         return kTRUE;
      }
      gPad->Clear();
      if(obj->InheritsFrom("TKey")) {
         auto* object = reinterpret_cast<TObject*>(gROOT->ProcessLine(Form("((TKey *)0x%lx)->ReadObj();", reinterpret_cast<ULong_t>(obj))));   // NOLINT(performance-no-int-to-ptr)
         if(object == nullptr) {
            return kTRUE;
         }
         if(object->InheritsFrom("TGraph")) {
            object->Draw("ALP");
         } else if(object->InheritsFrom("TImage")) {
            object->Draw("x");
         } else if(object->IsA()->GetMethodAllAny("Draw") != nullptr) {
            object->Draw();
         }
      } else if(obj->InheritsFrom("TGraph")) {
         obj->Draw("ALP");
      } else if(obj->IsA()->GetMethodAllAny("Draw") != nullptr) {
         obj->Draw();
      }
      gPad->Modified();
      gPad->Update();
      return kTRUE;
   }
   if(data->fDataType == uriObj) {
      TString sfname(reinterpret_cast<char*>(data->fData));
      if(sfname.Length() > 7) {
         sfname.ReplaceAll("\r\n", "");
         TUrl uri(sfname.Data());
         if(sfname.EndsWith(".bmp") || sfname.EndsWith(".gif") || sfname.EndsWith(".jpg") || sfname.EndsWith(".png") ||
            sfname.EndsWith(".ps") || sfname.EndsWith(".eps") || sfname.EndsWith(".pdf") || sfname.EndsWith(".tiff") ||
            sfname.EndsWith(".xpm")) {
            TImage* img = TImage::Open(uri.GetFile());
            if(img != nullptr) {
               img->Draw("x");
               img->SetEditable(kTRUE);
            }
         }
         gPad->Modified();
         gPad->Update();
      }
   }
   return kFALSE;
}

//______________________________________________________________________________
Atom_t GRootCanvas::HandleDNDPosition(Int_t x, Int_t y, Atom_t action, Int_t /*xroot*/, Int_t /*yroot*/)
{
   /// Handle dragging position events.

   TPad* pad = fCanvas->Pick(x, y, nullptr);
   if(pad != nullptr) {
      pad->cd();
      gROOT->SetSelectedPad(pad);
      // make sure the pad is highlighted (on Windows)
      pad->Update();
   }
   return action;
}

//______________________________________________________________________________
Atom_t GRootCanvas::HandleDNDEnter(Atom_t* typelist)
{
   /// Handle drag enter events.

   static Atom_t rootObj = gVirtualX->InternAtom("application/root", kFALSE);
   static Atom_t uriObj  = gVirtualX->InternAtom("text/uri-list", kFALSE);
   Atom_t        ret     = kNone;
   for(int i = 0; typelist[i] != kNone; ++i) {
      if(typelist[i] == rootObj) {
         ret = rootObj;
      }
      if(typelist[i] == uriObj) {
         ret = uriObj;
      }
   }
   return ret;
}

//______________________________________________________________________________
Bool_t GRootCanvas::HandleDNDLeave()
{
   /// Handle drag leave events.

   return kTRUE;
}

//______________________________________________________________________________
void GRootCanvas::Activated(Int_t id)
{
   /// Slot handling tab switching in the browser, to properly set the canvas
   /// and the model to the editor.

   if(fEmbedded) {
      auto* sender = reinterpret_cast<TGTab*>(gTQSender);
      if(sender != nullptr) {
         TGCompositeFrame* cont = sender->GetTabContainer(id);
         if(cont == fParent) {
            if(fEditor == nullptr) {
               fEditor = TVirtualPadEditor::GetPadEditor(kFALSE);
            }
            if(fEditor != nullptr && static_cast<TGedEditor*>(fEditor)->IsMapped()) {
               static_cast<TGedEditor*>(fEditor)->SetCanvas(fCanvas);
               static_cast<TGedEditor*>(fEditor)->SetModel(fCanvas, fCanvas, kButton1Down);
            }
         }
      }
   }
}

//______________________________________________________________________________
void GRootContainer::SavePrimitive(std::ostream& out, Option_t* /*= ""*/)
{
   /// Save a canvas container as a C++ statement(s) on output stream out.

   out << std::endl
       << "   // canvas container" << std::endl;
   out << "   Int_t canvasID = gVirtualX->InitWindow((ULong_t)" << GetParent()->GetParent()->GetName() << "->GetId());" << std::endl;
   out << "   Window_t winC = gVirtualX->GetWindowID(canvasID);" << std::endl;
   out << "   TGCompositeFrame *";
   out << GetName() << " = new TGCompositeFrame(gClient, winC, " << GetParent()->GetName() << ");" << std::endl;
}
