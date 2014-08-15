
#include "TGRSIServer.h"

#include<thread>




ClassImp(TGRSIServer)

TGRSIServer *TGRSIServer::fGRSIServer = 0;



TGRSIServer *TGRSIServer::instance(int port) {
   if(!fGRSIServer)
      fGRSIServer = new TGRSIServer(port);
   return fGRSIServer;
}

TGRSIServer::TGRSIServer(int port) 
            :TServerSocket::TServerSocket(port,true) {
   fMonitor = new TMonitor;
   fRunning = true;
}

TGRSIServer::~TGRSIServer()   {
}


void TGRSIServer::AcceptConnectionThread() {
   while(fRunning) {
      TSocket *sock = this->Accept();
      sock->Send("go");
      fMonitor->Add(sock);
   }
}

void TGRSIServer::MonitorConnectionsThread() {
   while(fRunning) {
      TMessage *mess;
      TSocket *sock = fMonitor->Select();
      sock->Recv(mess);
      delete mess;
   }
}




