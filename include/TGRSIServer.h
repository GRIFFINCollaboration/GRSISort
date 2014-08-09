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

class TGRSIServer : public TServerSocket {

   public:
      static TGRSIServer *instance(int port=9099);
      virtual ~TGRSIServer();

   private:
      static TGRSIServer *fGRSIServer;
      TGRSIServer(int port);

   ClassDef(TGRSIServer,0)

};

#endif
