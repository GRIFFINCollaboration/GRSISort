#ifndef TGRSISERVER_H
#define TGRSISERVER_H
/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 * 
 * Please indicate changes with your initials.
 * 
 *
*/


/* 
 * A class to allow 'things' to communicate
 * with GRSISort in a somewhat nice convenient and 
 * friendly way.
 *
*/ 

#include <TServerSocket.h>
#include <TMonitor.h>
#include <TSocket.h>
#include <TMessage.h>


class TGRSIServer : public TServerSocket {

   public:
      static TGRSIServer *instance(int port=9099);
      virtual ~TGRSIServer();
      
      void StopServer() { if(fRunning) fRunning = false; }


   private:
      static TGRSIServer *fGRSIServer;
      TGRSIServer(int port);
      bool fRunning;


      TMonitor *fMonitor;

      void AcceptConnectionThread();      
      void MonitorConnectionsThread();    

   ClassDef(TGRSIServer,0)

};

#endif
