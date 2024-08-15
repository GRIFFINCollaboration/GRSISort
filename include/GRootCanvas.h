#ifndef ROOT_GRootCanvas
#define ROOT_GRootCanvas

/** \addtogroup GROOT
 *  @{
 */

///////////////////////////////////////////////////////////////////////////
///
/// \class GRootCanvas
///
/// This class creates a main window with menubar, scrollbars and a
/// drawing area. The widgets used are the new native ROOT GUI widgets.
///
/// A slightly edited version of the the GRootCanvas
/// file in the default root source.  The reason for
/// the swap was take more control over the keyboard
/// interface to the canvas (particularly the arrow
/// keys).
///
///////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TCanvasImp
#include "TCanvasImp.h"
#endif
#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif

class TGCanvas;
class TGMenuBar;
class TGPopupMenu;
class TGLayoutHints;
class TGStatusBar;
class GRootContainer;
class TGToolBar;
class TGHorizontal3DLine;
class TGVertical3DLine;
class TVirtualPadEditor;
class TGDockableFrame;
class TGDNDManager;
class TGToolTip;

class GCanvas;

class GRootCanvas : public TGMainFrame, public TCanvasImp {

   friend class GRootContainer;

private:
   TGCanvas*       fCanvasWindow{nullptr};        ///< canvas widget
   GRootContainer* fCanvasContainer{nullptr};     ///< container in canvas widget
   TGMenuBar*      fMenuBar{nullptr};             ///< menubar
   TGPopupMenu*    fFileMenu{nullptr};            ///< file menu
   TGPopupMenu*    fFileSaveMenu{nullptr};        ///< save cascade submenu
   TGPopupMenu*    fEditMenu{nullptr};            ///< edit menu
   TGPopupMenu*    fEditClearMenu{nullptr};       ///< clear cascade submenu
   TGPopupMenu*    fViewMenu{nullptr};            ///< view menu
   TGPopupMenu*    fViewWithMenu{nullptr};        ///< view with... cascade submenu
   TGPopupMenu*    fOptionMenu{nullptr};          ///< option menu
   TGPopupMenu*    fToolsMenu{nullptr};           ///< tools menu
   TGPopupMenu*    fHelpMenu{nullptr};            ///< help menu
   TGLayoutHints*  fMenuBarLayout{nullptr};       ///< menubar layout hints
   TGLayoutHints*  fMenuBarItemLayout{nullptr};   ///< layout hints for menu in menubar
   TGLayoutHints*  fMenuBarHelpLayout{nullptr};   ///< layout hint for help menu in menubar
   TGLayoutHints*  fCanvasLayout{nullptr};        ///< layout for canvas widget
   TGStatusBar*    fStatusBar{nullptr};           ///< statusbar widget
   TGLayoutHints*  fStatusBarLayout{nullptr};     ///< layout hints for statusbar

   TGCompositeFrame*   fEditorFrame{nullptr};         ///< side frame for current pad editor
   TGLayoutHints*      fEditorLayout{nullptr};        ///< layout for editor frame
   TGCompositeFrame*   fMainFrame{nullptr};           ///< main frame containing canvas and side frame
   TGLayoutHints*      fToolBarLayout{nullptr};       ///< layout for toolbar widget
   TGToolBar*          fToolBar{nullptr};             ///< icon button toolbar
   TGHorizontal3DLine* fToolBarSep{nullptr};          ///< toolbar separator
   TGLayoutHints*      fMainFrameLayout{nullptr};     ///< layout for main frame
   TGVertical3DLine*   fVertical1{nullptr};           ///< toolbar vertical separator
   TGVertical3DLine*   fVertical2{nullptr};           ///< toolbar vertical separator
   TGHorizontal3DLine* fHorizontal1{nullptr};         ///< toolbar sepatator
   TGLayoutHints*      fVertical1Layout{nullptr};     ///< layout hints for separator
   TGLayoutHints*      fVertical2Layout{nullptr};     ///< layout hints for separator
   TGLayoutHints*      fHorizontal1Layout{nullptr};   ///< layout hints for separator
   TGDockableFrame*    fToolDock{nullptr};            ///< dockable frame holding the toolbar
   TGLayoutHints*      fDockLayout{nullptr};          ///< layout hints for dockable frame widget
   const TGPicture*    fIconPic{nullptr};             ///< icon picture
   TGToolTip*          fToolTip{nullptr};             ///< tooltip for object info

   TVirtualPadEditor* fEditor{nullptr};     ///< pointer to currently loaded pad editor
   Bool_t             fEmbedded{false};   ///< true if embedded in any other frame (e.g. in the browser)
   Int_t              fCanvasID{0};   ///< index in fWindows array of TGX11
   Bool_t             fAutoFit{true};    ///< when true canvas container keeps same size as canvas
   Int_t              fButton{0};     ///< currently pressed button

   GRootCanvas(const GRootCanvas&);                  // Not implemented
   GRootCanvas(GRootCanvas&&) noexcept;              // Not implemented
   GRootCanvas& operator=(const GRootCanvas&);       // Not implemented
   GRootCanvas& operator=(GRootCanvas&&) noexcept;   // Not implemented
   void         CreateCanvas(const char* name);
   void         CreateEditor();

   Bool_t HandleContainerButton(Event_t* event);
   Bool_t HandleContainerDoubleClick(Event_t* event);
   Bool_t HandleContainerConfigure(Event_t* ev);
   Bool_t HandleContainerKey(Event_t* event);
   Bool_t HandleContainerMotion(Event_t* event);
   Bool_t HandleContainerExpose(Event_t* event);
   Bool_t HandleContainerCrossing(Event_t* event);

   Bool_t HandleDNDDrop(TDNDData* data) override;
   Atom_t HandleDNDPosition(Int_t x, Int_t y, Atom_t action, Int_t xroot, Int_t yroot) override;
   Atom_t HandleDNDEnter(Atom_t* typelist) override;
   Bool_t HandleDNDLeave() override;

public:
   explicit GRootCanvas(GCanvas* c = nullptr, const char* name = "ROOT Canvas", UInt_t width = 500, UInt_t height = 300);
   GRootCanvas(GCanvas* c, const char* name, Int_t x, Int_t y, UInt_t width, UInt_t height);
   ~GRootCanvas();

   void   AdjustSize();
   void   Close() override;
   void   ForceUpdate() override { Layout(); }
   void   FitCanvas();
   void   EventInfo(Int_t event, Int_t px, Int_t py, TObject* selected);
   UInt_t GetWindowGeometry(Int_t& x, Int_t& y, UInt_t& w, UInt_t& h) override;
   UInt_t GetCwidth() const;
   UInt_t GetCheight() const;
   void   Iconify() override { IconifyWindow(); }
   Int_t  InitWindow() override;
   void   PrintCanvas();
   void   RaiseWindow() override;
   void   SetWindowPosition(Int_t x, Int_t y) override;
   void   SetWindowSize(UInt_t w, UInt_t h) override;
   void   SetWindowTitle(const char* title) override;
   void   SetCanvasSize(UInt_t w, UInt_t h) override;
   void   SetStatusText(const char* txt = nullptr, Int_t partidx = 0) override;

   void Show() override { MapRaised(); }
   void ShowMenuBar(Bool_t show = kTRUE) override;
   void ShowStatusBar(Bool_t show = kTRUE) override;
   void ShowEditor(Bool_t show = kTRUE) override;
   void ShowToolBar(Bool_t show = kTRUE) override;
   void ShowToolTips(Bool_t show = kTRUE) override;

   Bool_t HasEditor() const override;
   Bool_t HasMenuBar() const override;
   Bool_t HasStatusBar() const override;
   Bool_t HasToolBar() const override;
   Bool_t HasToolTips() const override;

   void Activated(Int_t id);

   TGMenuBar*       GetMenuBar() const { return fMenuBar; }
   TGLayoutHints*   GetMenuBarItemLayout() const { return fMenuBarItemLayout; }
   TGStatusBar*     GetStatusBar() const { return fStatusBar; }
   TGDockableFrame* GetToolDock() const { return fToolDock; }

   // overridden from TGMainFrame
   void   CloseWindow() override;
   Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2) override;
   void   ReallyDelete() override;

   /// \cond CLASSIMP
   ClassDefOverride(GRootCanvas, 0)   // NOLINT
   /// \endcond
};
/*! @} */
#endif
