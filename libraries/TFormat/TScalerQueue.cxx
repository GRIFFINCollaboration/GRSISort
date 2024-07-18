
#include "TScalerQueue.h"

std::mutex TDeadtimeScalerQueue::All;
std::mutex TDeadtimeScalerQueue::Sorted;

///////////////////////////////////////////////////////////////
///
/// \class TDeadtimeScalerQueue
///
/// This class is where we store scalers. It is thread-safe
/// and returns it's status in order to monitor progress.
///
///////////////////////////////////////////////////////////////

TDeadtimeScalerQueue* TDeadtimeScalerQueue::fDeadtimeScalerQueueClassPointer = nullptr;

TDeadtimeScalerQueue* TDeadtimeScalerQueue::Get()
{
   /// Get a pointer to the global scaler Q.
   if(fDeadtimeScalerQueueClassPointer == nullptr) {
      fDeadtimeScalerQueueClassPointer = new TDeadtimeScalerQueue;
   }
   return fDeadtimeScalerQueueClassPointer;
}

TDeadtimeScalerQueue::TDeadtimeScalerQueue()
   : fScalersInQueue(0), fStop(false), fStopwatch(new TStopwatch())
{
   fDeadtimeScalerQueueClassPointer = this;
   // When the Global Q is created, start a timer to see how long we are using it.
   fStopwatch->Start();

   Clear();
}

void TDeadtimeScalerQueue::Print(Option_t*) const
{
   /// Print the status of the Scaler Queue
   CheckStatus();
}

void TDeadtimeScalerQueue::Clear(Option_t*)
{
   /// Clear the entire Queue, Queue counters, and timer.
   bool locked = false;
   if(!fDeadtimeScalerQueue.empty()) {
      while(!TDeadtimeScalerQueue::All.try_lock()) {
         // do nothing
      }
      locked = true;
   }

   if(fScalersInQueue != 0) {
      std::cout << RED << std::endl
                << "\tWarning, discarding " << fScalersInQueue << " Scalers!" << RESET_COLOR << std::endl;
      while(!fDeadtimeScalerQueue.empty()) {
         fDeadtimeScalerQueue.pop();
      }
      fScalersInQueue = 0;
   }

   fScalersIn  = 0;
   fScalersOut = 0;

   fTotalScalersIn  = 0;
   fTotalScalersOut = 0;

   fStopwatch->Reset();

   if(locked) {
      TDeadtimeScalerQueue::All.unlock();
   }
}

void TDeadtimeScalerQueue::StartStatusUpdate()
{
   /// The status thread runs the status update at various intervals to show the progress of the analysis tree.
   fStatusUpdateOn = true;

   std::thread statusUpdate(&TDeadtimeScalerQueue::StatusUpdate, this);
   statusUpdate.detach();
}

void TDeadtimeScalerQueue::StopStatusUpdate()
{
   /// Stops the status update
   fStatusUpdateOn = false;
}

void TDeadtimeScalerQueue::Add(TScalerData* scalerData)
{
   /// Add a Scaler to the scaler Queue.
   if(scalerData == nullptr) {
      return;
   }

   while(!TDeadtimeScalerQueue::Sorted.try_lock()) {
      // do nothing
   }

   fDeadtimeScalerQueue.push(scalerData);

   fTotalScalersIn++;
   fScalersInQueue++;
   fScalersIn++;

   TDeadtimeScalerQueue::Sorted.unlock();
}

void TDeadtimeScalerQueue::Pop()
{
   // Take a scaler out of the Queue
   while(!TDeadtimeScalerQueue::Sorted.try_lock()) {
      // do nothing
   }
   fDeadtimeScalerQueue.pop();
   fScalersInQueue--;
   fScalersOut++;
   TDeadtimeScalerQueue::Sorted.unlock();
}

TScalerData* TDeadtimeScalerQueue::PopScaler()
{
   // Take a scaler out of the Queue and return a pointer to it.
   while(!TDeadtimeScalerQueue::Sorted.try_lock()) {
      // do nothing
   }
   if(Size() > 0) {
      TScalerData* scaler = fDeadtimeScalerQueue.front();
      if(scaler != nullptr) {
         fDeadtimeScalerQueue.pop();
         fScalersInQueue--;
         fScalersOut++;
         fTotalScalersOut++;
      }
      TDeadtimeScalerQueue::Sorted.unlock();
      return scaler;
   }
   TDeadtimeScalerQueue::Sorted.unlock();
   return nullptr;
}

int TDeadtimeScalerQueue::Size() const
{
   // Returns the number of scalers in the Queue
   return fScalersInQueue;
}

void TDeadtimeScalerQueue::CheckStatus() const
{
   // Checks the status of the Queue. This is called by the Print() function.
   while(!TDeadtimeScalerQueue::All.try_lock()) {
      // do nothing
   }

   std::cout << BLUE << "# Scalers currently in Q     = " << Size() << RESET_COLOR << std::endl;
   std::cout << BLUE << "# Total Scalers put in Q     = " << fTotalScalersIn << RESET_COLOR << std::endl;
   std::cout << DGREEN << "# Total Scalers taken from Q = " << fTotalScalersOut << RESET_COLOR << std::endl;

   TDeadtimeScalerQueue::All.unlock();
}

void TDeadtimeScalerQueue::StatusUpdate()
{
   // Updates the status of the scaler Queue
   float time            = 0;
   float scaler_rate_in  = 0;
   float scaler_rate_out = 0;

   while(fStatusUpdateOn) {
      time            = fStopwatch->RealTime();
      scaler_rate_in  = fScalersIn / time;
      scaler_rate_out = fScalersOut / time;
      while(!TDeadtimeScalerQueue::All.try_lock()) {
         // do nothing
      }

      std::cout << BLUE << "\tscalers rate in  = " << scaler_rate_in << "/sec, nqueue = " << Size() << RESET_COLOR << std::endl;
      std::cout << DGREEN << "\tscalers rate out = " << scaler_rate_out << "/sec" << RESET_COLOR << std::endl;
      TDeadtimeScalerQueue::All.unlock();
      ResetRateCounter();
      fStopwatch->Start();
   }
}

void TDeadtimeScalerQueue::ResetRateCounter()
{
   // Resets the number of scalers in and scalers out counter. This is useful for checking to see if the rate
   // of scaler parsing is changing.
   while(!TDeadtimeScalerQueue::All.try_lock()) {
      // do nothing
   }

   fScalersIn  = 0;
   fScalersOut = 0;
   TDeadtimeScalerQueue::All.unlock();
}

std::mutex TRateScalerQueue::All;
std::mutex TRateScalerQueue::Sorted;

///////////////////////////////////////////////////////////////
///
/// \class TRateScalerQueue
///
/// This class is where we store scalers. It is thread-safe
/// and returns it's status in order to monitor progress.
///
///////////////////////////////////////////////////////////////

TRateScalerQueue* TRateScalerQueue::fRateScalerQueueClassPointer = nullptr;

TRateScalerQueue* TRateScalerQueue::Get()
{
   /// Get a pointer to the global scaler Q.
   if(fRateScalerQueueClassPointer == nullptr) {
      fRateScalerQueueClassPointer = new TRateScalerQueue;
   }
   return fRateScalerQueueClassPointer;
}

TRateScalerQueue::TRateScalerQueue()
   : fScalersInQueue(0), fStop(false), fStopwatch(new TStopwatch())
{
   fRateScalerQueueClassPointer = this;
   // When the Global Q is created, start a timer to see how long we are using it.
   fStopwatch->Start();

   Clear();
}

void TRateScalerQueue::Print(Option_t*) const
{
   /// Print the status of the Scaler Queue
   CheckStatus();
}

void TRateScalerQueue::Clear(Option_t*)
{
   /// Clear the entire Queue, Queue counters, and timer.
   bool locked = false;
   if(!fRateScalerQueue.empty()) {
      while(!TRateScalerQueue::All.try_lock()) {
         // do nothing
      }
      locked = true;
   }

   if(fScalersInQueue != 0) {
      std::cout << RED << "\tWarning, discarding " << fScalersInQueue << " Scalers!" << RESET_COLOR << std::endl;
      while(!fRateScalerQueue.empty()) {
         fRateScalerQueue.pop();
      }
      fScalersInQueue = 0;
   }

   fScalersIn  = 0;
   fScalersOut = 0;

   fTotalScalersIn  = 0;
   fTotalScalersOut = 0;

   fStopwatch->Reset();

   if(locked) {
      TRateScalerQueue::All.unlock();
   }
}

void TRateScalerQueue::StartStatusUpdate()
{
   /// The status thread runs the status update at various intervals to show the progress of the analysis tree.
   fStatusUpdateOn = true;

   std::thread statusUpdate(&TRateScalerQueue::StatusUpdate, this);
   statusUpdate.detach();
}

void TRateScalerQueue::StopStatusUpdate()
{
   /// Stops the status update
   fStatusUpdateOn = false;
}

void TRateScalerQueue::Add(TScalerData* scalerData)
{
   /// Add a Scaler to the scaler Queue.
   if(scalerData == nullptr) {
      return;
   }

   while(!TRateScalerQueue::Sorted.try_lock()) {
      // do nothing
   }

   fRateScalerQueue.push(scalerData);

   fTotalScalersIn++;
   fScalersInQueue++;
   fScalersIn++;

   TRateScalerQueue::Sorted.unlock();
}

void TRateScalerQueue::Pop()
{
   // Take a scaler out of the Queue
   while(!TRateScalerQueue::Sorted.try_lock()) {
      // do nothing
   }
   fRateScalerQueue.pop();
   fScalersInQueue--;
   fScalersOut++;
   TRateScalerQueue::Sorted.unlock();
}

TScalerData* TRateScalerQueue::PopScaler()
{
   // Take a scaler out of the Queue and return a pointer to it.
   while(!TRateScalerQueue::Sorted.try_lock()) {
      // do nothing
   }
   if(Size() > 0) {
      TScalerData* scaler = fRateScalerQueue.front();
      if(scaler != nullptr) {
         fRateScalerQueue.pop();
         fScalersInQueue--;
         fScalersOut++;
         fTotalScalersOut++;
      }
      TRateScalerQueue::Sorted.unlock();
      return scaler;
   }
   TRateScalerQueue::Sorted.unlock();
   return nullptr;
}

int TRateScalerQueue::Size() const
{
   // Returns the number of scalers in the Queue
   return fScalersInQueue;
}

void TRateScalerQueue::CheckStatus() const
{
   // Checks the status of the Queue. This is called by the Print() function.
   while(!TRateScalerQueue::All.try_lock()) {
      // do nothing
   }

   std::cout << BLUE << "# Scalers currently in Q     = " << Size() << RESET_COLOR << std::endl;
   std::cout << BLUE << "# Total Scalers put in Q     = " << fTotalScalersIn << RESET_COLOR << std::endl;
   std::cout << DGREEN << "# Total Scalers taken from Q = " << fTotalScalersOut << RESET_COLOR << std::endl;

   TRateScalerQueue::All.unlock();
}

void TRateScalerQueue::StatusUpdate()
{
   // Updates the status of the scaler Queue
   float time            = 0;
   float scaler_rate_in  = 0;
   float scaler_rate_out = 0;

   while(fStatusUpdateOn) {
      time            = fStopwatch->RealTime();
      scaler_rate_in  = fScalersIn / time;
      scaler_rate_out = fScalersOut / time;
      while(!TRateScalerQueue::All.try_lock()) {
         // do nothing
      }

      std::cout << BLUE << "\tscalers rate in  = " << scaler_rate_in << "/sec, nqueue = " << Size() << RESET_COLOR << std::endl;
      std::cout << DGREEN << "\tscalers rate out = " << scaler_rate_out << "/sec" << RESET_COLOR << std::endl;
      TRateScalerQueue::All.unlock();
      ResetRateCounter();
      fStopwatch->Start();
   }
}

void TRateScalerQueue::ResetRateCounter()
{
   // Resets the number of scalers in and scalers out counter. This is useful for checking to see if the rate
   // of scaler parsing is changing.
   while(!TRateScalerQueue::All.try_lock()) {
      // do nothing
   }

   fScalersIn  = 0;
   fScalersOut = 0;
   TRateScalerQueue::All.unlock();
}
