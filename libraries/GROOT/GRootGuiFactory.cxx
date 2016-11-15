// @(#)root/gui:$Id$
// Author: Fons Rademakers   15/01/98

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// GRootGuiFactory                                                      //
//                                                                      //
// This class is a factory for ROOT GUI components. It overrides        //
// the member functions of the ABS TGuiFactory.                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "GRootGuiFactory.h"
#include "TRootApplication.h"

#include "GRootCanvas.h"
//#include "GRootObjectManager.h"

#include "TRootBrowserLite.h"
#include "TRootContextMenu.h"
#include "TRootControlBar.h"
#include "TROOT.h"
#include "TPluginManager.h"
#include "TEnv.h"

#include "GCanvas.h"

ClassImp(GRootGuiFactory)

void GRootGuiFactory::Init(){
  if(gROOT->IsBatch()) return;
  gROOT->LoadClass("TCanvas","Gpad");
  gGuiFactory = new GRootGuiFactory();
}

//______________________________________________________________________________
GRootGuiFactory::GRootGuiFactory(const char *name, const char *title)
   : TGuiFactory(name, title)
{
   // GRootGuiFactory ctor.
}

//______________________________________________________________________________
TApplicationImp *GRootGuiFactory::CreateApplicationImp(const char *classname,
                      Int_t *argc, char **argv)
{
   // Create a ROOT native GUI version of TApplicationImp

   TRootApplication *app = new TRootApplication(classname, argc, argv);
   if (!app->Client()) {
      delete app;
      app = 0;
   }
   return app;
}

//______________________________________________________________________________
TCanvasImp *GRootGuiFactory::CreateCanvasImp(TCanvas *c, const char *title,
                                             UInt_t width, UInt_t height)
{
   // Create a ROOT native GUI version of TCanvasImp
   //GRootObjectManager::Instance()->AddCanvas(c);
   //return new GRootCanvas(c, title, width, height);i
   GRootCanvas *grc = new GRootCanvas((GCanvas*)c, title, width, height);
   //GRootObjectManager::AddCanvas(c);
   //c->Connect("Closed()","GRootObjectManager",this,"RemoveCanvas()");
   //GRootObjectManager::Update();
   return grc;

}

//______________________________________________________________________________
TCanvasImp *GRootGuiFactory::CreateCanvasImp(TCanvas *c, const char *title,
                                  Int_t x, Int_t y, UInt_t width, UInt_t height)
{
   // Create a ROOT native GUI version of TCanvasImp
   //GRootObjectManager::Instance()->AddCanvas(c);
   //return new GRootCanvas(c, title, x, y, width, height);
   GRootCanvas *grc = new GRootCanvas((GCanvas*)c, title, x, y, width, height);
   //GRootObjectManager::AddCanvas(c);
   //c->Connect("Closed()","GRootObjectManager",this,"RemoveCanvas()");
   //GRootObjectManager::Update();
   return grc;
}

//______________________________________________________________________________
TBrowserImp *GRootGuiFactory::CreateBrowserImp(TBrowser *b, const char *title,
                                               UInt_t width, UInt_t height,
                                               Option_t *opt)
{
   // Create a ROOT native GUI version of TBrowserImp

   //TString browserVersion(gEnv->GetValue("Browser.Name", "TRootBrowserLite"));
   TString browserVersion(gEnv->GetValue("Browser.Name", "GRootBrowser"));
   TPluginHandler *ph = gROOT->GetPluginManager()->FindHandler("TBrowserImp",
                                                               browserVersion);
   //gROOT->GetPluginManager()->Print();


   TString browserOptions(gEnv->GetValue("Browser.Options", "FECI"));
   //TString browserOptions(gEnv->GetValue("Browser.Options", "FEI"));
   if (opt && strlen(opt))
      browserOptions = opt;

   //browserOptions = "FCI";

   browserOptions.ToUpper();
   if (browserOptions.Contains("LITE"))
      return new TRootBrowserLite(b, title, width, height);
   if (ph && ph->LoadPlugin() != -1) {
      //printf("i am here now 1.\t %s \n",browserOptions.Data());
      TBrowserImp *imp = (TBrowserImp *)ph->ExecPlugin(5, b, title, width,
         height, browserOptions.Data());
      if (imp) {

        return imp;
      }
   }
   printf(" and never here.\n");
   return new TRootBrowserLite(b, title, width, height);
}

//______________________________________________________________________________
TBrowserImp *GRootGuiFactory::CreateBrowserImp(TBrowser *b, const char *title,
                                               Int_t x, Int_t y, UInt_t width,
                                               UInt_t height, Option_t *opt)
{
   // Create a ROOT native GUI version of TBrowserImp

   TString browserVersion(gEnv->GetValue("Browser.Name", "TRootBrowserLite"));
   TPluginHandler *ph = gROOT->GetPluginManager()->FindHandler("TBrowserImp",
                                                               browserVersion);
   TString browserOptions(gEnv->GetValue("Browser.Options", "FECI"));
   if (opt && strlen(opt))
      browserOptions = opt;
   browserOptions.ToUpper();
   if (browserOptions.Contains("LITE"))
      return new TRootBrowserLite(b, title, width, height);
   if (ph && ph->LoadPlugin() != -1) {
      TBrowserImp *imp = (TBrowserImp *)ph->ExecPlugin(7, b, title, x, y, width,
         height, browserOptions.Data());
      if (imp) return imp;
   }
   return new TRootBrowserLite(b, title, x, y, width, height);
}

//______________________________________________________________________________
TContextMenuImp *GRootGuiFactory::CreateContextMenuImp(TContextMenu *c,
                                             const char *name, const char *)
{
   // Create a ROOT native GUI version of TContextMenuImp

   return new TRootContextMenu(c, name);
}

//______________________________________________________________________________
TControlBarImp *GRootGuiFactory::CreateControlBarImp(TControlBar *c, const char *title)
{
   // Create a ROOT native GUI version of TControlBarImp

   return new TRootControlBar(c, title);
}

//______________________________________________________________________________
TControlBarImp *GRootGuiFactory::CreateControlBarImp(TControlBar *c, const char *title,
                                                     Int_t x, Int_t y)
{
   // Create a ROOT native GUI version of TControlBarImp

   return new TRootControlBar(c, title, x, y);
}
