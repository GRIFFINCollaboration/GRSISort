#include "StoppableThread.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <utility>

#include "TString.h"

#include "TDataLoop.h"
#include "TFragmentChainLoop.h"

std::map<std::string, StoppableThread*> StoppableThread::fThreadMap;
bool                                    StoppableThread::fStatusThreadOn = false;
std::thread                             StoppableThread::fStatusThread;

size_t StoppableThread::fColumnWidth = 20;
size_t StoppableThread::fStatusWidth = 80;

int StoppableThread::GetNThreads()
{
   return fThreadMap.size();
}

StoppableThread::StoppableThread(std::string name)
   : fItemsPopped(0), fInputSize(0), fName(std::move(name)), fRunning(true), fPaused(true)
{
   // TODO: check if a thread already exists and delete?
   fThreadMap.insert(std::make_pair(fName, this));
   fThread = std::thread(&StoppableThread::Loop, this);
   if(!fStatusThreadOn) {
      start_status_thread();
   }
}

bool StoppableThread::AnyThreadRunning()
{
   for(auto& elem : fThreadMap) {
      if(elem.second->IsRunning()) {
         return true;
      }
   }
   return false;
}

std::string StoppableThread::AnyThreadStatus()
{
   for(auto& elem : fThreadMap) {
      if(elem.second->IsRunning()) {
         std::stringstream ss;
         ss << elem.first << ":\t " << elem.second->Status();
         return ss.str();
      }
   }
   return "";
}

std::string StoppableThread::AllThreadProgress()
{
   std::stringstream ss;
   for(auto& elem : fThreadMap) {
      if(elem.second->IsRunning()) {
         ss << std::left << std::setw(fColumnWidth - 1) << elem.second->Progress().substr(0, fColumnWidth - 1) << "|";
      } else {
         std::string prog = "not running";
         ss << std::left << std::setw(fColumnWidth - 1) << prog.substr(0, fColumnWidth - 1) << "|";
      }
   }
   return ss.str().substr(0, fStatusWidth);
}

std::string StoppableThread::AllThreadHeader()
{
   std::stringstream ss;
   for(auto& elem : fThreadMap) {
      // left align, fill with spaces
      ss << std::left << std::setw(fColumnWidth - 1) << elem.first.substr(0, fColumnWidth - 1) << "|";
   }
   return ss.str().substr(0, fStatusWidth);
}

std::string StoppableThread::AllThreadStatus()
{
   std::stringstream ss;
   for(auto& elem : fThreadMap) {
      if(elem.second->IsRunning()) {
         ss << std::left << std::setw(fColumnWidth - 1) << elem.second->Status().substr(0, fColumnWidth - 1) << "|";
      } else {
         std::string prog = "not running";
         ss << std::left << std::setw(fColumnWidth - 1) << prog.substr(0, fColumnWidth - 1) << "|";
      }
   }
   return ss.str().substr(0, fStatusWidth);
}

void StoppableThread::PauseAll()
{
   for(auto& elem : fThreadMap) {
      elem.second->Pause();
   }
}

void StoppableThread::ResumeAll()
{
   for(auto& elem : fThreadMap) {
      elem.second->Resume();
   }
}

std::string StoppableThread::Status()
{
   std::stringstream ss;
   ss << std::setw(fColumnWidth / 2 - 1) << fItemsPopped << "/" << std::setw(fColumnWidth / 2 - 1)
      << fItemsPopped + fInputSize;
   return ss.str();
}

std::string StoppableThread::Progress()
{
   std::stringstream ss;
   float             percentDone = (100. * fItemsPopped);
   if(fItemsPopped + fInputSize > 0) {
      percentDone /= (fItemsPopped + fInputSize);
      while(percentDone > 100. / (fColumnWidth - 1)) {
         ss << "*";
         percentDone -= 100. / (fColumnWidth - 1);
      }
   } else {
      ss << "N/A: " << percentDone;
   }
   return ss.str();
}

void StoppableThread::SendStop()
{
   for(auto& elem : fThreadMap) {
      TDataLoop*          data_loop  = dynamic_cast<TDataLoop*>(elem.second);
      TFragmentChainLoop* chain_loop = dynamic_cast<TFragmentChainLoop*>(elem.second);
      if(data_loop != nullptr || chain_loop != nullptr) {
         std::cout << "Stopping thread " << elem.first << std::endl;
         elem.second->Stop();
      }
   }
}

void StoppableThread::StopAll()
{
   SendStop();

   for(auto& elem : fThreadMap) {
      std::cout << "Joining thread " << elem.first << std::endl;
      StoppableThread* thread = elem.second;
      thread->Join();
   }

   while(!fThreadMap.empty()) {
      StoppableThread* thread = fThreadMap.begin()->second;
      std::cout << "Deleting thread " << fThreadMap.begin()->first << std::endl;
      delete thread;
   }

   status_out();
}

void StoppableThread::ClearAllQueues()
{
   for(auto& elem : fThreadMap) {
      elem.second->ClearQueue();
   }
}

StoppableThread* StoppableThread::Get(const std::string& name)
{
   StoppableThread* mythread = nullptr;
   if(fThreadMap.count(name) != 0u) {
      mythread = fThreadMap.at(name);
   }
   return mythread;
}

StoppableThread::~StoppableThread()
{
   if(fThreadMap.count(fName) != 0u) {
      fThreadMap.erase(fName);
   }
   if(fThreadMap.empty()) {
      fStatusThreadOn = false;
      fStatusThread.join();
   }
}

void StoppableThread::Resume()
{
   if(fRunning) {
      std::unique_lock<std::mutex> lock(fPauseMutex);
      fPaused = false;
      fPausedWait.notify_one();
   }
}

void StoppableThread::Pause()
{
   if(fRunning) {
      fPaused = true;
   }
}

void StoppableThread::Stop()
{
   std::unique_lock<std::mutex> lock(fPauseMutex);
   fRunning = false;
   std::cout << std::endl;
   fPaused = false;
   std::cout << EndStatus();
   fPausedWait.notify_one();
}

bool StoppableThread::IsRunning()
{
   return fRunning;
}

bool StoppableThread::IsPaused()
{
   return fPaused;
}

void StoppableThread::Join()
{
   if(fThread.joinable()) {
      std::cout << EndStatus();
      fThread.join();
   }
}

void StoppableThread::Loop()
{
   while(fRunning) {
      std::unique_lock<std::mutex> lock(fPauseMutex);
      while(fPaused && fRunning) {
         fPausedWait.wait_for(lock, std::chrono::milliseconds(100));
      }
      bool success = Iteration();
      if(!success) {
         fRunning = false;
         std::cout << std::endl;
         break;
      }
   }
   OnEnd();
}

void StoppableThread::Print()
{
   std::cout << "column width " << fColumnWidth << ", status width " << fStatusWidth << std::endl;
   std::cout << GetNThreads() << " Threads:" << std::endl;
   int counter = 0;
   for(auto& it : fThreadMap) {
      std::cout << "  " << counter << "\t" << it.first << " @ " << hex(it.second, 8) << std::endl;
      counter++;
   }
}

void StoppableThread::start_status_thread()
{
   if(!fStatusThreadOn) {
      fStatusThreadOn = true;
      fStatusThread   = std::thread(StoppableThread::status_out_loop);
   }
}

void StoppableThread::stop_status_thread()
{
   if(fStatusThreadOn) {
      fStatusThreadOn = false;
   }
}

void StoppableThread::join_status_thread()
{
   stop_status_thread();
   fStatusThread.join();
}

void StoppableThread::status_out_loop()
{
   while(fStatusThreadOn) {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      status_out();
   }
   std::ofstream outfile(Form("%s/.grsi_thread", getenv("GRSISYS")));
   outfile << "---------------------------------------------------------------\n";
   outfile << "---------------------------------------------------------------\n";
}

void StoppableThread::status_out()
{

   std::ofstream outfile(Form("%s/.grsi_thread", getenv("GRSISYS")));
   outfile << "---------------------------------------------------------------\n";   // 64 -.
   for(auto& it : fThreadMap) {
      StoppableThread* thread = it.second;
      outfile << "- " << thread->Name() << (thread->IsRunning() ? "[Live]" : "[Stop]")
              << std::string(64 - 8 - thread->Name().length(), ' ') << "-\n";
      outfile << "- " << std::string(40, ' ') << "items_pushed:  " << thread->GetItemsPushed() << "\n";
      outfile << "- " << std::string(40, ' ') << "items_popped:  " << thread->GetItemsPopped() << "\n";
      outfile << "- " << std::string(40, ' ') << "items_current: " << thread->GetItemsCurrent() << "\n";
      outfile << "- " << std::string(40, ' ') << "rate:      " << thread->GetRate() << "\n";
      outfile << "---------------------------------------------------------------\n";   // 64 -.
   }
   outfile << "---------------------------------------------------------------\n";   // 64 -.
}

std::vector<StoppableThread*> StoppableThread::GetAll()
{
   std::vector<StoppableThread*> output;
   for(auto& elem : fThreadMap) {
      output.push_back(elem.second);
   }
   return output;
}
