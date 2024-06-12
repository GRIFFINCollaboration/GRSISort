#ifndef ROOT_GRootBrowser
#define ROOT_GRootBrowser

/** \addtogroup GROOT
 *  @{
 */

#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif

#ifndef ROOT_TBrowserImp
#include "TBrowserImp.h"
#endif

class TGLayoutHints;
class TGTab;
class TGMenuBar;
class TGPopupMenu;
class TGStatusBar;
class TGVSplitter;
class TGHSplitter;

/////////////////////////////////////////////////////////////////
///
/// \class GBrowserPlugin
///
/// Helper class used to manage plugins (command or macro to be executed).
///
///  Largely! taken from GRootBrowserLite in the Root Software package.
///  This class exists because I needed to change
///  some of the connections in CreateBrowser() function to
///  the GRootCanvas class instead of the the TRootCanvas.
///
///  pcb.
///
/////////////////////////////////////////////////////////////////

class GBrowserPlugin : public TNamed {
public:
   Int_t   fTab;       ///< Tab number
   Int_t   fSubTab;    ///< Tab element number
   TString fCommand;   ///< Command to be executed

   GBrowserPlugin(const char* name, const char* cmd = "", Int_t tab = 1, Int_t sub = -1)
      : TNamed(name, cmd), fTab(tab), fSubTab(sub), fCommand(cmd)
   {
   }
   ~GBrowserPlugin() override = default;

   void SetTab(Int_t tab) { fTab = tab; }                 ///< simple setter function for the tab number
   void SetSubTab(Int_t sub) { fSubTab = sub; }           ///< simple setter function for the tab element number
   void SetCommand(const char* cmd) { fCommand = cmd; }   ///< simple setter function for the command to be executed

   /// \cond CLASSIMP
   ClassDefOverride(GBrowserPlugin, 0)   // basic plugin description class
   /// \endcond
};

/////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////

class GRootBrowser : public TGMainFrame, public TBrowserImp {
private:
   GRootBrowser(const GRootBrowser&);              // Not implemented
   GRootBrowser& operator=(const GRootBrowser&);   // Not implemented

protected:
   TGLayoutHints *    fLH0, *fLH1, *fLH2, *fLH3;   ///< Layout hints, part 1
   TGLayoutHints *    fLH4, *fLH5, *fLH6, *fLH7;   ///< Layout hints, part 2
   TGTab*             fTabLeft;                    ///< Left Tab
   TGTab*             fTabRight;                   ///< Right Tab
   TGTab*             fTabBottom;                  ///< Bottom Tab
   TGTab*             fEditTab;                    ///< Tab in "Edit" mode
   Int_t              fEditPos;                    ///< Id of tab in "Edit" mode
   Int_t              fEditSubPos;                 ///< Id of subtab in "Edit" mode
   TGVerticalFrame*   fVf;                         ///< Vertical frame
   TGHorizontalFrame* fHf;                         ///< Horizontal frame
   TGHorizontalFrame* fH1;                         ///< Horizontal frame
   TGHorizontalFrame* fH2;                         ///< Horizontal frame
   TGVerticalFrame*   fV1;                         ///< Vertical frame
   TGVerticalFrame*   fV2;                         ///< Vertical frame
   TGVSplitter*       fVSplitter;                  ///< Vertical splitter
   TGHSplitter*       fHSplitter;                  ///< Horizontal splitter
   TGCompositeFrame*  fEditFrame;                  ///< Frame in "Edit" mode
   TGHorizontalFrame* fTopMenuFrame;               ///< Top menu frame
   TGHorizontalFrame* fPreMenuFrame;               ///< First (owned) menu frame
   TGHorizontalFrame* fMenuFrame;                  ///< Shared menu frame
   TGHorizontalFrame* fToolbarFrame;               ///< Toolbar frame
   TGMenuBar*         fMenuBar;                    ///< Main (owned) menu bar
   TGPopupMenu*       fMenuFile;                   ///< "File" popup menu
   TGPopupMenu*       fMenuExecPlugin;             ///< "Exec Plugin" popup menu
   TGPopupMenu*       fMenuHelp;                   ///< "Browser Help" popup menu
   TGCompositeFrame*  fActMenuBar;                 ///< Actual (active) menu bar
   TBrowserImp*       fActBrowser;                 ///< Actual (active) browser imp
   TList              fBrowsers;                   ///< List of (sub)browsers
   TList              fPlugins;                    ///< List of plugins
   TGStatusBar*       fStatusBar;                  ///< Status bar
   Int_t              fNbInitPlugins;              ///< Number of initial plugins (from .rootrc)
   Int_t              fNbTab[3];                   ///< Number of tab elements (for each Tab)
   Int_t              fCrTab[3];                   ///< Actual (active) tab elements (for each Tab)
   Int_t              fPid;                        ///< Current process id
   Bool_t             fShowCloseTab;               ///< kTRUE to show close icon on tab elements
   const TGPicture*   fIconPic;                    ///< icon picture

public:
   enum class ENewBrowserMessages {
      kBrowse = 11011,
      kOpenFile,
      kClone,
      kHelpAbout,
      kHelpOnBrowser,
      kHelpOnCanvas,
      kHelpOnMenus,
      kHelpOnGraphicsEd,
      kHelpOnObjects,
      kHelpOnPS,
      kHelpOnRemote,
      kNewEditor,
      kNewCanvas,
      kNewHtml,
      kExecPluginMacro,
      kExecPluginCmd,
      kCloseTab,
      kCloseWindow,
      kQuitRoot
   };

   enum class EInsertPosition { kLeft,
                                kRight,
                                kBottom };

   GRootBrowser(TBrowser* b = nullptr, const char* name = "ROOT Browser", UInt_t width = 800, UInt_t height = 500,
                Option_t* opt = "", Bool_t initshow = kTRUE);
   GRootBrowser(TBrowser* b, const char* name, Int_t x, Int_t y, UInt_t width, UInt_t height, Option_t* opt = "",
                Bool_t initshow = kTRUE);
   ~GRootBrowser() override;

   void InitPlugins(Option_t* opt = "");

   void         CreateBrowser(const char* name);
   void         CloneBrowser();
   void         CloseWindow() override;
   virtual void CloseTab(Int_t id);
   void         CloseTabs() override;
   void         DoTab(Int_t id);
   void         EventInfo(Int_t event, Int_t px, Int_t py, TObject* selected);
   TGFrame*     GetActFrame() const { return static_cast<TGFrame*>(fEditFrame); }
   TGFrame*     GetToolbarFrame() const { return static_cast<TGFrame*>(fToolbarFrame); }
   TGStatusBar* GetStatusBar() const { return fStatusBar; }
   TGTab*       GetTabLeft() const { return fTabLeft; }
   TGTab*       GetTabRight() const { return fTabRight; }
   TGTab*       GetTabBottom() const { return fTabBottom; }
   TGTab*       GetTab(Int_t pos) const;
   void         SetTab(Int_t pos = static_cast<Int_t>(EInsertPosition::kRight), Int_t subpos = -1);
   void         SetTabTitle(const char* title, Int_t pos = static_cast<Int_t>(EInsertPosition::kRight), Int_t subpos = -1);
   void         HandleMenu(Int_t id);
   void         RecursiveReparent(TGPopupMenu* popup);
   void         RemoveTab(Int_t pos, Int_t subpos);
   void         SetActBrowser(TBrowserImp* b) { fActBrowser = b; }
   void         ShowMenu(TGCompositeFrame* menu);
   void         StartEmbedding(Int_t pos = static_cast<Int_t>(EInsertPosition::kRight), Int_t subpos = -1) override;
   void         StopEmbedding(const char* name = nullptr) override { StopEmbedding(name, nullptr); }
   void         StopEmbedding(const char* name, TGLayoutHints* layout);
   void         SwitchMenus(TGCompositeFrame* from);

   void         BrowseObj(TObject* obj) override;              //*SIGNAL*
   void         ExecuteDefaultAction(TObject* obj) override;   //*SIGNAL*
   virtual void DoubleClicked(TObject* obj);                   //*SIGNAL*
   virtual void Checked(TObject* obj, Bool_t checked);         //*SIGNAL*

   void         Add(TObject* obj, const char* name = nullptr, Int_t check = -1) override;
   void         RecursiveRemove(TObject* obj) override;
   void         Refresh(Bool_t force = kFALSE) override;
   void         Show() override { MapRaised(); }
   Option_t*    GetDrawOption() const override;
   TGMainFrame* GetMainFrame() const override { return (TGMainFrame*)this; }

   Long_t ExecPlugin(const char* name = nullptr, const char* fname = nullptr, const char* cmd = nullptr,
                     Int_t pos = static_cast<Int_t>(EInsertPosition::kRight), Int_t subpos = -1) override;
   void   SetStatusText(const char* txt, Int_t col) override;
   Bool_t HandleKey(Event_t* event) override;

   virtual void   ShowCloseTab(Bool_t show) { fShowCloseTab = show; }
   virtual Bool_t IsCloseTabShown() const { return fShowCloseTab; }

   // overridden from TGMainFrame
   void ReallyDelete() override;

   static TBrowserImp* NewBrowser(TBrowser* b = nullptr, const char* title = "ROOT Browser", UInt_t width = 800,
                                  UInt_t height = 500, Option_t* opt = "");
   static TBrowserImp* NewBrowser(TBrowser* b, const char* title, Int_t x, Int_t y, UInt_t width, UInt_t height,
                                  Option_t* opt = "");

   /// \cond CLASSIMP
   ClassDefOverride(GRootBrowser, 0)   // New ROOT Browser
   /// \endcond
};
/*! @} */
#endif
