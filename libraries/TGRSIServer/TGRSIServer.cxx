
#include "TGRSIServer.h"


ClassImp(TGRSIServer)

TGRSIServer *TGRSIServer::fGRSIServer = 0;



TGRSIServer *TGRSIServer::instance(int port) {
   if(!fGRSIServer)
      fGRSIServer = new TGRSIServer(port);
   return fGRSIServer;
}

TGRSIServer::TGRSIServer(int port) 
            :TServerSocket::TServerSocket(port,true) {
}

TGRSIServer::~TGRSIServer()   {
}
