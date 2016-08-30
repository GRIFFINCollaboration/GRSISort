#include "StoppableThread.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <TString.h>

#include "TDataLoop.h"
#include "TFragmentChainLoop.h"

std::map<std::string,StoppableThread*> StoppableThread::fthreadmap;
bool StoppableThread::status_thread_on = false;
std::thread StoppableThread::status_thread;


int StoppableThread::GetNThreads() { return fthreadmap.size(); }

StoppableThread::StoppableThread(std::string name)
  : fname(name), running(true), paused(true) {
  //TODO: check if a thread already exists and delete?
  fthreadmap.insert(std::make_pair(fname,this));
  thread = std::thread(&StoppableThread::Loop, this);
  if(!status_thread_on) {
    start_status_thread();
  }
}


bool StoppableThread::AnyThreadRunning() {
  for(auto& elem : fthreadmap){
    if(elem.second->IsRunning()){
      return true;
    }
  }
  return false;
}

std::string StoppableThread::AnyThreadStatus() {
  for(auto& elem : fthreadmap){
    if(elem.second->IsRunning()){
      return elem.second->Status();
    }
  }
  return "";
}

void StoppableThread::PauseAll() {
  for(auto& elem : fthreadmap){
    elem.second->Pause();
  }
}

void StoppableThread::ResumeAll() {
  for(auto& elem : fthreadmap){
    elem.second->Resume();
  }
}

std::string StoppableThread::Status() {
  std::stringstream ss;
  ss << Name()
     << ":\t" << std::setw(8) << GetItemsPushed();
  return ss.str();
}

void StoppableThread::StopAll() {
  for(auto& elem : fthreadmap){
    TDataLoop* data_loop = dynamic_cast<TDataLoop*>(elem.second);
    TFragmentChainLoop* chain_loop = dynamic_cast<TFragmentChainLoop*>(elem.second);
    if(data_loop || chain_loop){
      std::cout << "Stopping thread " << elem.first << std::endl;
      elem.second->Stop();
    }
  }

  for(auto& elem : fthreadmap){
    std::cout << "Joining thread " << elem.first << std::endl;
    StoppableThread* thread = elem.second;
    thread->Join();
  }

  while(fthreadmap.size()){
    StoppableThread* thread = fthreadmap.begin()->second;
    std::cout << "Deleting thread " << fthreadmap.begin()->first << std::endl;
    delete thread;
  }

  status_out();
}

StoppableThread *StoppableThread::Get(std::string name) {
  StoppableThread *mythread = 0;
  if(fthreadmap.count(name)) {
    mythread = fthreadmap.at(name);
  }
  return mythread;
}


StoppableThread::~StoppableThread() {
  if(fthreadmap.count(fname)) {
     fthreadmap.erase(fname);
  }
  if(fthreadmap.size()==0) {
    status_thread_on = false;
    status_thread.join();
  }
}

void StoppableThread::Resume() {
  if(running) {
    std::unique_lock<std::mutex> lock(pause_mutex);
    paused = false;
    paused_wait.notify_one();
  }
}

void StoppableThread::Pause() {
  if(running) {
    paused = true;
  }
}

void StoppableThread::Stop() {
  std::unique_lock<std::mutex> lock(pause_mutex);
  running = false;
  paused = false;
  paused_wait.notify_one();
}

bool StoppableThread::IsRunning() {
  return running;
}

bool StoppableThread::IsPaused() {
  return paused;
}

void StoppableThread::Join() {
  thread.join();
}

void StoppableThread::Loop() {
  while(running){
    {
      std::unique_lock<std::mutex> lock(pause_mutex);
      while(paused && running){
        paused_wait.wait_for(lock, std::chrono::milliseconds(100));
      }
    }
    bool success = Iteration();
    if(!success){
      running = false;
      break;
    }
  }

  OnEnd();
}

void StoppableThread::Print() {
  printf("%i Threads:\n",GetNThreads());
  int counter = 0;
  for(auto it=fthreadmap.begin();it!=fthreadmap.end();it++) {
    printf("  %i\t%s @ 0x%08lx\n",counter,it->first.c_str(),(unsigned long)it->second);
    counter++;
  }
}


void StoppableThread::start_status_thread() {
  if(!status_thread_on){
    status_thread_on = true;
    status_thread = std::thread(StoppableThread::status_out_loop);
  }
}

void StoppableThread::stop_status_thread() {
  if(status_thread_on){
    status_thread_on = false;
  }
}

void StoppableThread::join_status_thread() {
  stop_status_thread();
  status_thread.join();
}

void StoppableThread::status_out_loop() {
  while(status_thread_on) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    status_out();
  }
 std::ofstream outfile(Form("%s/.grsi_thread",getenv("GRSISYS")));
 outfile << "---------------------------------------------------------------\n";
 outfile << "---------------------------------------------------------------\n";
}

void StoppableThread::status_out() {

  std::ofstream outfile(Form("%s/.grsi_thread",getenv("GRSISYS")));
  outfile << "---------------------------------------------------------------\n"; // 64 -.
  for(auto it = fthreadmap.begin();it!=fthreadmap.end();it++) {
    StoppableThread *thread = it->second;
    outfile << "- " << thread->Name() << (thread->IsRunning()?"[Live]":"[Stop]") << std::string(64-8-thread->Name().length(),' ')   << "-\n";
    outfile << "- " << std::string(40,' ') << "items_pushed:  " << thread->GetItemsPushed()  << "\n";
    outfile << "- " << std::string(40,' ') << "items_popped:  " << thread->GetItemsPopped()  << "\n";
    outfile << "- " << std::string(40,' ') << "items_current: " << thread->GetItemsCurrent() << "\n";
    outfile << "- " << std::string(40,' ') << "rate:      " << thread->GetRate()         << "\n";
    outfile << "---------------------------------------------------------------\n"; // 64 -.
  }
  outfile << "---------------------------------------------------------------\n"; // 64 -.


}

std::vector<StoppableThread*> StoppableThread::GetAll() {
  std::vector<StoppableThread*> output;
  for(auto& elem : fthreadmap){
    output.push_back(elem.second);
  }
  return output;
}
