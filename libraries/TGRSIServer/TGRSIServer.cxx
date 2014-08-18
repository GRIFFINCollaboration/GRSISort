
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
   std::thread acceptThread(&TGRSIServer::AcceptConnectionThread,this);
   std::thread monitorThread(&TGRSIServer::MonitorConnectionsThread,this);
   //acceptThread.detach();
   monitorThread.join();
}

TGRSIServer::~TGRSIServer()   {
}


void TGRSIServer::AcceptConnectionThread() {
   char buffer[8192];
   while(fRunning) {
      TSocket *sock = this->Accept();
      sock->Recv(buffer,8191);
      printf("buffer = \n%s",buffer);

      sock->Send("go");
      fMonitor->Add(sock);
   }
}

void TGRSIServer::MonitorConnectionsThread() {
   while(fRunning) {
      TMessage *mess;
      TSocket *sock = fMonitor->Select();
      sock->Recv(mess);
      printf("recv\n");
      delete mess;
   }
}




