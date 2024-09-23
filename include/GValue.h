#ifndef TGRUTVARIABLE_H
#define TGRUTVARIABLE_H

#include <map>
#include <utility>

#include "TList.h"
#include "TNamed.h"

class GValue : public TNamed {
public:
   enum class EPriority { kUser     = 0,
                          kValFile  = 1,
                          kRootFile = 2,
                          kDefault  = 999999 };

   GValue() = default;
   explicit GValue(const char* name);
   GValue(const char* name, double value, EPriority priority = EPriority::kUser);
   GValue(const GValue& val);
   GValue(GValue&& val) noexcept            = default;
   GValue& operator=(const GValue& val)     = default;
   GValue& operator=(GValue&& val) noexcept = default;
   ~GValue()                                = default;

   double      GetValue() const { return fValue; }
   const char* GetInfo() const { return info.c_str(); }

   void SetValue(double value) { fValue = value; }
   void SetInfo(const char* temp) { info.assign(temp); }

   static int ReadValFile(const char* filename = "", Option_t* opt = "replace");
   static int WriteValFile(const std::string& filename = "", Option_t* opt = "");

   static GValue* GetDefaultValue() { return fDefaultValue; }
   // Search fValueVector for GValue with name given by string
   static GValue* FindValue(const std::string& = "");
   static void    SetReplaceValue(const std::string& name, double value, EPriority priority = EPriority::kUser);
   static GValue* Get(const std::string& name = "") { return FindValue(name); }
   static double  Value(const std::string&);                  // get the named value, returns sqrt(-1) = NaN
   static double  Value(const std::string&, const double&);   // try and find the named value, otherwise return the provided default
   static TList*  AllValues()
   {
      auto* output = new TList;
      output->SetOwner(false);
      for(auto& item : fValueVector) {
         output->Add(item.second);
      }
      return output;
   }

   // Add value into static vector fValueVector
   static bool AddValue(GValue*, Option_t* opt = "");

   bool AppendValue(GValue*);
   bool ReplaceValue(GValue*);

   using TNamed::Clear;
   void Print(Option_t* opt = "") const override;
   void Copy(TObject& obj) const override;
   // virtual bool Notify();

   static int         Size() { return static_cast<int>(fValueVector.size()); }
   std::string        PrintToString() const;
   static std::string WriteToBuffer(Option_t* opt = "");
   static void        Clear();

private:
   double                                fValue{0.};
   EPriority                             fPriority{EPriority::kDefault};
   std::string                           info;
   static GValue*                        fDefaultValue;
   static std::map<std::string, GValue*> fValueVector;
   static int                            ParseInputData(const std::string& input, EPriority priority, Option_t* opt = "");

   /// \cond CLASSIMP
   ClassDefOverride(GValue, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

#endif
