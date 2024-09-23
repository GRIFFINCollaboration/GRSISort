#ifndef ROOT_GRootGuiFactory
#define ROOT_GRootGuiFactory

///////////////////////////////////////////////////////////////////////////
///
/// \class GRootGuiFactory
///
/// This class is a factory for ROOT GUI components. It overrides
/// the member functions of the ABS TGuiFactory.  This is necessary
/// in order to override the native root canvas and fully take control
/// of the users interface with objects drawn to the screen   pcb.
///
///////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TGuiFactory
#include "TGuiFactory.h"
#endif

class TApplicationImp;
class TCanvasImp;
class TBrowserImp;
class TContextMenuImp;
class TContextMenu;
class TControlBarImp;
class TControlBar;

class GRootGuiFactory : public TGuiFactory {

public:
   static void Init();

   explicit GRootGuiFactory(const char* name = "Root", const char* title = "GRUT GUI Factory");
   GRootGuiFactory(const GRootGuiFactory&)                = default;
   GRootGuiFactory(GRootGuiFactory&&) noexcept            = default;
   GRootGuiFactory& operator=(const GRootGuiFactory&)     = default;
   GRootGuiFactory& operator=(GRootGuiFactory&&) noexcept = default;
   ~GRootGuiFactory()                                     = default;

   TApplicationImp* CreateApplicationImp(const char* classname, int* argc, char** argv) override;

   TCanvasImp* CreateCanvasImp(TCanvas* c, const char* title, UInt_t width, UInt_t height) override;
   TCanvasImp* CreateCanvasImp(TCanvas* c, const char* title, Int_t x, Int_t y, UInt_t width, UInt_t height) override;

   TBrowserImp* CreateBrowserImp(TBrowser* b, const char* title, UInt_t width, UInt_t height,
                                 Option_t* opt = "") override;
   TBrowserImp* CreateBrowserImp(TBrowser* b, const char* title, Int_t x, Int_t y, UInt_t width, UInt_t height,
                                 Option_t* opt = "") override;

   TContextMenuImp* CreateContextMenuImp(TContextMenu* c, const char* name, const char* title) override;

   TControlBarImp* CreateControlBarImp(TControlBar* c, const char* title) override;
   TControlBarImp* CreateControlBarImp(TControlBar* c, const char* title, Int_t x, Int_t y) override;

   /// \cond CLASSIMP
   ClassDefOverride(GRootGuiFactory, 0)   // NOLINT(readability-else-after-return)
   /// \endcond
};

#endif
