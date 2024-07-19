#ifndef GRUTNOTIFIER_H
#define GRUTNOTIFIER_H

#ifndef __CINT__
#include <functional>
#endif

#include "TNamed.h"

#include "GRootCommands.h"

class GNotifier : public TNamed {
public:
   static GNotifier* Get();
   ~GNotifier() override = default;

   void Print(Option_t* = "") const override {}
   void Clear(Option_t* = "") override {}
   bool Notify() override;

#ifdef __CINT__
   static void AddCallback(void (*func)());
#else
   template <typename T>
   static void AddCallback(T callback)
   {
      Get()->fCallbacks.push_back(callback);
   }

   static void AddCallback(void (*func)()) { Get()->fCallbacks.emplace_back(func); }
#endif

private:
   GNotifier() = default;

#ifndef __CINT__
   std::vector<std::function<void()>> fCallbacks;
#endif

   static GNotifier* fGNotifier;

	/// /cond CLASSIMP
   ClassDefOverride(GNotifier, 0) // NOLINT
	/// /endcond
};

#endif
