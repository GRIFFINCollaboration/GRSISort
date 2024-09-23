
/////////////////////////////////////////////////////////////////////////
///
/// \class GRSIxx
///
/// X11 based routines used to display the splash screen for grsisort.
///
///
/////////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "GVersion.h"

#include "RConfigure.h"

#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <pwd.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "Rtypes.h"

#if defined(R__AIX) || defined(R__SOLARIS)
#include <sys/select.h>
#endif
#include <ctime>
#include <sys/time.h>

#include "TSystem.h"

static Display*     gDisplay       = nullptr;
static Window       gLogoWindow    = 0;
static Pixmap       gLogoPixmap    = 0;
static Pixmap       gCreditsPixmap = 0;
static GC           gGC            = nullptr;
static XFontStruct* gFont          = nullptr;
static bool         gDone          = false;
static bool         gMayPopdown    = false;
static bool         gAbout         = false;
static unsigned int gWidth         = 0;
static unsigned int gHeight        = 0;
static int          gStayUp        = 4000;                     // 4 seconds
static XRectangle   gCreditsRect   = {15, 155, 285, 130};      // clip rect in logo
static unsigned int gCreditsWidth  = gCreditsRect.width / 2;   // credits pixmap size
static unsigned int gCreditsHeight = 0;

static struct timeval gPopupTime;

static std::array<const char*, 2> gConception = {"P. C. Bender", nullptr};

static std::array<const char*, 3> gLeadDevelopers = {"V. Bildstein", "P. C. Bender", nullptr};

static std::array<const char*, 3> gKeyContributors = {"R. Dunlop", "D. Miller", nullptr};

static char** gContributors = nullptr;

static bool StayUp(int milliSec)
{
   /// Returns false if milliSec milliseconds have passed since logo
   /// was popped up, true otherwise.

   struct timeval ctv {};
   struct timeval dtv {};
   struct timeval tv {};
   struct timeval ptv = gPopupTime;

   tv.tv_sec  = milliSec / 1000;
   tv.tv_usec = (milliSec % 1000) * 1000;

   gettimeofday(&ctv, nullptr);
   if((dtv.tv_usec = ctv.tv_usec - ptv.tv_usec) < 0) {
      dtv.tv_usec += 1000000;
      ptv.tv_sec++;
   }
   dtv.tv_sec = ctv.tv_sec - ptv.tv_sec;

   if((ctv.tv_usec = tv.tv_usec - dtv.tv_usec) < 0) {
      ctv.tv_usec += 1000000;
      dtv.tv_sec++;
   }
   ctv.tv_sec = tv.tv_sec - dtv.tv_sec;

   return ctv.tv_sec >= 0;
}

static void Sleep(int milliSec)
{
   /// Sleep for specified amount of milli seconds.

   // get current time
   struct timeval tv {};

   tv.tv_sec  = milliSec / 1000;
   tv.tv_usec = (milliSec % 1000) * 1000;

   select(0, nullptr, nullptr, nullptr, &tv);
}

static Pixmap GetRootLogo()
{
   /// Get logo from xpm file.

   Pixmap  logo    = 0;
   Screen* xscreen = XDefaultScreenOfDisplay(gDisplay);
   if(xscreen == nullptr) {
      return logo;
   }

   int depth = PlanesOfScreen(xscreen);

   XWindowAttributes win_attr;
   XGetWindowAttributes(gDisplay, gLogoWindow, &win_attr);

   XpmAttributes attr;
   attr.valuemask = XpmVisual | XpmColormap | XpmDepth;
   attr.visual    = win_attr.visual;
   attr.colormap  = win_attr.colormap;
   attr.depth     = win_attr.depth;

#ifdef XpmColorKey   // Not available in XPM 3.2 and earlier
   attr.valuemask |= XpmColorKey;
   if(depth > 4) {
      attr.color_key = XPM_COLOR;
   } else if(depth > 2) {
      attr.color_key = XPM_GRAY4;
   } else if(depth > 1) {
      attr.color_key = XPM_GRAY;
   } else if(depth == 1) {
      attr.color_key = XPM_MONO;
   } else {
      attr.valuemask &= ~XpmColorKey;
   }

#endif   // defined(XpmColorKey)
   std::string file;
   file.append(getenv("GRSISYS"));
   file.append("/libraries/TGRSIint/grsisplash_bw.xpm");

   int ret = XpmReadFileToPixmap(gDisplay, gLogoWindow, file.data(), &logo, nullptr, &attr);
   XpmFreeAttributes(&attr);

   if(ret == XpmSuccess || ret == XpmColorError) {
      return logo;
   }

   std::cout << "rootx xpm error: " << XpmGetErrorString(ret) << std::endl;

   if(logo != 0u) {
      XFreePixmap(gDisplay, logo);
   }
   logo = 0;

   return logo;
}

static void ReadContributors()
{
   /// Read the file $ROOTSYS/README/CREDITS for the names of the
   /// contributors.

   std::array<char, 2048> buf;
#ifdef ROOTDOCDIR
   snprintf(buf.data(), buf.size(), "%s/CREDITS", ROOTDOCDIR);
#else
   snprintf(buf.data(), buf.size(), "%s/README/CREDITS", getenv("ROOTSYS"));
#endif

   gContributors = nullptr;

   FILE* f = fopen(buf.data(), "r");
   if(f == nullptr) {
      return;
   }

   int cnt = 0;
   while(fgets(buf.data(), buf.size(), f) != nullptr) {
      if(strncmp(buf.data(), "N: ", 3) == 0) {
         cnt++;
      }
   }
   gContributors = new char*[cnt + 1];

   cnt = 0;
   rewind(f);
   while(fgets(buf.data(), buf.size(), f) != nullptr) {
      if(strncmp(buf.data(), "N: ", 3) == 0) {
         int len      = strlen(buf.data());
         buf[len - 1] = 0;   // remove \n
         len -= 3;           // remove "N: "
         gContributors[cnt] = new char[len];
         strncpy(gContributors[cnt], &(buf[3]), len);
         cnt++;
      }
   }
   gContributors[cnt] = nullptr;

   fclose(f);
}

static void DrawVersion()
{
   /// Draw version string.

   std::array<char, 80> version;
   snprintf(version.data(), version.size(), "Version %s", GRSI_RELEASE);

   XDrawString(gDisplay, gLogoWindow, gGC, 15, gHeight - 20, version.data(), strlen(version.data()));
}

static void DrawROOTCredit()
{
   /// Draw version string.

   const char* version = "A ROOT based package";

   XDrawString(gDisplay, gLogoWindow, gGC, 15, gHeight - 35, version, strlen(version));
}

static int DrawCreditItem(const char* creditItem, const char** members, int y, bool draw)
{
   /// Draw credit item.

   std::array<char, 1024> credit;
   int                    lineSpacing = gFont->max_bounds.ascent + gFont->max_bounds.descent;

   strlcpy(credit.data(), creditItem, credit.size());
   for(int i = 0; (members != nullptr) && (members[i] != nullptr); i++) {
      if(i != 0) {
         strlcat(credit.data(), ", ", credit.size());
      }
      if(XTextWidth(gFont, credit.data(), strlen(credit.data())) + XTextWidth(gFont, members[i], strlen(members[i])) >
         static_cast<int>(gCreditsWidth)) {
         if(draw) {
            XDrawString(gDisplay, gCreditsPixmap, gGC, 0, y, credit.data(), strlen(credit.data()));
         }
         y += lineSpacing;
         strlcpy(credit.data(), "   ", credit.size());
      }
      strlcat(credit.data(), members[i], credit.size());
   }
   if(draw) {
      XDrawString(gDisplay, gCreditsPixmap, gGC, 0, y, credit.data(), strlen(credit.data()));
   }

   return y;
}

static int DrawCredits(bool draw, bool)
{
   /// Draw credits. If draw is true draw credits,
   /// otherwise just return size of all credit text.

   if(gFont == nullptr) {
      return 150;   // size not important no text will be drawn anyway
   }

   int lineSpacing = gFont->max_bounds.ascent + gFont->max_bounds.descent;
   int y           = lineSpacing;

   y = DrawCreditItem("Conception: ", gConception.data(), y, draw);

   y += 2 * lineSpacing - 1;

   y = DrawCreditItem("Lead Developers: ", gLeadDevelopers.data(), y, draw);

   y += 2 * lineSpacing - 1;   // special layout tweak

   y = DrawCreditItem("Key Contributions: ", gKeyContributors.data(), y, draw);

   y += 2 * lineSpacing - 1;   // special layout tweak

   y += 10;
   return y;
}

void ScrollCredits(int ypos)
{
   XSetClipRectangles(gDisplay, gGC, 0, 0, &gCreditsRect, 1, Unsorted);

   XCopyArea(gDisplay, gCreditsPixmap, gLogoWindow, gGC, 0, ypos, gCreditsWidth, gCreditsHeight, gCreditsRect.x, gCreditsRect.y);

   XSetClipMask(gDisplay, gGC, None);
}

void PopupLogo(bool about)
{
   /// Popup logo, waiting till ROOT is ready to run.
   gDisplay = XOpenDisplay("");
   if(gDisplay == nullptr) {
      return;
   }

   gAbout = about;

   int screen = DefaultScreen(gDisplay);

   Pixel back = WhitePixel(gDisplay, screen);
   Pixel fore = BlackPixel(gDisplay, screen);

   gLogoWindow = XCreateSimpleWindow(gDisplay, DefaultRootWindow(gDisplay), -100, -100, 50, 50, 0, fore, back);

   gLogoPixmap = GetRootLogo();

   if(gLogoPixmap == 0u) {
      XCloseDisplay(gDisplay);
      gDisplay = nullptr;
      return;
   }

   Window       root  = 0;
   int          x     = 0;
   int          y     = 0;
   unsigned int bw    = 0;
   unsigned int depth = 0;
   XGetGeometry(gDisplay, gLogoPixmap, &root, &x, &y, &gWidth, &gHeight, &bw, &depth);

   Screen* xscreen = XDefaultScreenOfDisplay(gDisplay);

   if(xscreen == nullptr) {
      XCloseDisplay(gDisplay);
      gDisplay = nullptr;
      return;
   }
   x = (WidthOfScreen(xscreen) - gWidth) / 2;
   y = (HeightOfScreen(xscreen) - gHeight) / 2;

   XMoveResizeWindow(gDisplay, gLogoWindow, x, y, gWidth, gHeight);
   XSync(gDisplay, False);   // make sure move & resize is done before mapping

   XSetWindowAttributes xswa;
   uint64_t             valmask = CWBackPixmap | CWOverrideRedirect;
   xswa.background_pixmap       = gLogoPixmap;
   xswa.override_redirect       = True;
   XChangeWindowAttributes(gDisplay, gLogoWindow, valmask, &xswa);

   gGC   = XCreateGC(gDisplay, gLogoWindow, 0, nullptr);
   gFont = XLoadQueryFont(gDisplay, "-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1");
   if(gFont == nullptr) {
      std::cout << "Couldn't find font \"-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1\"," << std::endl
                << "trying \"fixed\". Please fix your system so helvetica can be found, " << std::endl
                << "this font typically is in the rpm (or pkg equivalent) package " << std::endl
                << "XFree86-[75,100]dpi-fonts or fonts-xorg-[75,100]dpi." << std::endl;
      gFont = XLoadQueryFont(gDisplay, "fixed");
      if(gFont == nullptr) {
         std::cout << "Also couln't find font \"fixed\", your system is terminally misconfigured." << std::endl;
      }
   }
   if(gFont != nullptr) {
      XSetFont(gDisplay, gGC, gFont->fid);
   }
   XSetForeground(gDisplay, gGC, fore);
   XSetBackground(gDisplay, gGC, back);

   if(about) {
      ReadContributors();
   }

   gCreditsHeight = DrawCredits(false, about) + gCreditsRect.height + 50;
   gCreditsPixmap = XCreatePixmap(gDisplay, gLogoWindow, gCreditsWidth, gCreditsHeight, depth);
   XSetForeground(gDisplay, gGC, back);
   XFillRectangle(gDisplay, gCreditsPixmap, gGC, 0, 0, gCreditsWidth, gCreditsHeight);
   XSetForeground(gDisplay, gGC, fore);
   DrawCredits(true, about);

   XSelectInput(gDisplay, gLogoWindow, ButtonPressMask | ExposureMask);

   XMapRaised(gDisplay, gLogoWindow);

   gettimeofday(&gPopupTime, nullptr);
}

void WaitLogo()
{
   /// Main event loop waiting till time arrives to pop down logo
   /// or when forced by button press event.

   if(gDisplay == nullptr) {
      return;
   }

   int  ypos       = 0;
   bool stopScroll = false;

   ScrollCredits(ypos);
   DrawVersion();
   DrawROOTCredit();
   XFlush(gDisplay);

   while(!gDone) {
      gSystem->Sleep(3500);
      break;

      XEvent event;
      if(XCheckMaskEvent(gDisplay, ButtonPressMask | ExposureMask, &event) != 0) {
         switch(event.type) {
         case Expose:
            if(event.xexpose.count == 0) {
               ScrollCredits(ypos);
               DrawVersion();
            }
            break;
         case ButtonPress:
            if(gAbout && event.xbutton.button == 3) {
               stopScroll = !stopScroll;
            } else {
               gDone = true;
            }
            break;
         default: break;
         }
      }

      Sleep(100);

      if(!gAbout && !StayUp(gStayUp) && gMayPopdown) {
         gDone = true;
      }

      if(gAbout && !stopScroll) {
         if(ypos == 0) {
            Sleep(2000);
         }
         ypos++;
         if(ypos > static_cast<int>(gCreditsHeight - gCreditsRect.height - 50)) {
            ypos = -static_cast<int>(gCreditsRect.height);
         }
         ScrollCredits(ypos);
         XFlush(gDisplay);
      }
   }

   if(gLogoWindow != 0u) {
      XUnmapWindow(gDisplay, gLogoWindow);
      XDestroyWindow(gDisplay, gLogoWindow);
      gLogoWindow = 0;
   }
   if(gLogoPixmap != 0u) {
      XFreePixmap(gDisplay, gLogoPixmap);
      gLogoPixmap = 0;
   }
   if(gCreditsPixmap != 0u) {
      XFreePixmap(gDisplay, gCreditsPixmap);
      gCreditsPixmap = 0;
   }
   if(gFont != nullptr) {
      XFreeFont(gDisplay, gFont);
      gFont = nullptr;
   }
   if(gGC != nullptr) {
      XFreeGC(gDisplay, gGC);
      gGC = nullptr;
   }
   if(gDisplay != nullptr) {
      XSync(gDisplay, False);
      XCloseDisplay(gDisplay);
      gDisplay = nullptr;
   }
}

void PopdownLogo()
{
   /// ROOT is ready to run, may pop down the logo if stay up time expires.

   gMayPopdown = true;
}

void CloseDisplay()
{
   /// Close connection to X server (called by child).

   if(gDisplay != nullptr) {
      close(ConnectionNumber(gDisplay));
   }
}
