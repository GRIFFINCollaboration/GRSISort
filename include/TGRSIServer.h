#ifndef TGRSISERVER_H
#define TGRSISERVER_H

/** \addtogroup Sorting
 *  @{
 */

/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 *
 * Please indicate changes with your initials.
 *
 *
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TGRSIServer
///
/// A class to allow 'things' to communicate
/// with GRSISort in a somewhat nice convenient and
/// friendly way.
///
////////////////////////////////////////////////////////////////////////////////

#include "TServerSocket.h"
#include "TMonitor.h"
#include "TSocket.h"
#include "TMessage.h"

class TGRSIServer : public TServerSocket {
public:
   static TGRSIServer* instance(int port = 9099);
   TGRSIServer(const TGRSIServer&)                = default;
   TGRSIServer(TGRSIServer&&) noexcept            = default;
   TGRSIServer& operator=(const TGRSIServer&)     = default;
   TGRSIServer& operator=(TGRSIServer&&) noexcept = default;
   ~TGRSIServer();

   void StopServer()
   {
      if(fRunning) { fRunning = false; }
   }

private:
   static TGRSIServer* fGRSIServer;
   explicit TGRSIServer(int port);
   bool fRunning;

   TMonitor* fMonitor;

   void AcceptConnectionThread();
   void MonitorConnectionsThread();

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIServer, 0)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
