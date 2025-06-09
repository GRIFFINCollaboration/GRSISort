#ifndef TUSERSETTINGS_H
#define TUSERSETTINGS_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>

#include "TNamed.h"
#include "TCollection.h"

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TUserSettings
///
/// The TUserSettings class defines user settings that can be
/// read from text or root files. It stores them in separate
/// maps for booleans, integers, floats, and strings as well
/// as vectors of those types.
///
/// For helpers the reading of the user settings is done in
/// the TGRSIHelper class and made available as the member
/// fUserSettings. Code to read the user setting "MyDouble"
/// into the member variable `fMyMemberVariable` in the helper's
/// constructor could be:
/// ```
///		if(fUserSettings != nullptr) {
///			fMyMemberVariable = fUserSettings->GetDouble("MyDouble", 1.);
///		}
/// ```
/// See the AngularCorrelationHelper's header file for more.
///
/// In general the user settings can be accessed via the
/// static function `TUserSettings* TGRSIOptions::UserSettings()`.
///
/// If TGRSIOptions are not available, the user settings can
/// be read using the constructor:
/// ```
/// userSettings = new TUserSettings(filename);
/// ```
/// or using the ReadSettings function:
/// ```
/// userSettings->ReadSettings(filename);
/// ```
/// where userSettings is of type TUserSettings* and filename is
/// either a std::string or char*.
///
/// The settings file is expected to have the format
/// parameter name: value
/// where parameter name is a string w/o whitespace and value is the value of the parameter
/// the type of the value (bool, int, double, std::string, or vectors of these types) will be determined automatically.
/// Vectors are comma separated values of one type.
/// The default type is std::string.
/// Any lines starting with '#' or '//' or without a colon will be ignored.
/// See "examples/AngularCorrelationSettings.par" for an example of a settings file.
///
/////////////////////////////////////////////////////////////

class TUserSettings : public TNamed {
public:
   TUserSettings() = default;
   explicit TUserSettings(const std::string& settingsFile)
   {
      if(!ReadSettings(settingsFile)) { throw std::runtime_error("Failed to read user settings file!"); }
   }

   TUserSettings(const TUserSettings&)                = default;
   TUserSettings(TUserSettings&&) noexcept            = default;
   TUserSettings& operator=(const TUserSettings&)     = default;
   TUserSettings& operator=(TUserSettings&&) noexcept = default;
   ~TUserSettings()                                   = default;

   bool ReadSettings(const std::string& settingsFile);

   bool empty() { return fBool.empty() && fInt.empty() && fDouble.empty() && fString.empty() && fBoolVector.empty() && fIntVector.empty() && fDoubleVector.empty() && fStringVector.empty(); }

   // getter functions
   template <typename T>
   T Get(const std::string& parameter) const
   {
      if(std::is_same<T, bool>::value) { return GetBool(parameter); }
      if(std::is_same<T, int>::value) { return GetInt(parameter); }
      if(std::is_same<T, double>::value) { return GetDouble(parameter); }
      if(std::is_same<T, std::string>::value) { return GetString(parameter); }
      if(std::is_same<T, std::vector<bool>>::value) { return GetBoolVector(parameter); }
      if(std::is_same<T, std::vector<int>>::value) { return GetIntVector(parameter); }
      if(std::is_same<T, std::vector<double>>::value) { return GetDoubleVector(parameter); }
      if(std::is_same<T, std::vector<std::string>>::value) { return GetStringVector(parameter); }
      throw std::runtime_error("Unknown type, only bool, int, double, std::string or vectors of those types allowed");
   }

   bool                     GetBool(const std::string& parameter, bool quiet = false) const;
   int                      GetInt(const std::string& parameter, bool quiet = false) const;
   double                   GetDouble(const std::string& parameter, bool quiet = false) const;
   std::string              GetString(const std::string& parameter, bool quiet = false) const;
   std::vector<bool>        GetBoolVector(const std::string& parameter, bool quiet = false) const;
   std::vector<int>         GetIntVector(const std::string& parameter, bool quiet = false) const;
   std::vector<double>      GetDoubleVector(const std::string& parameter, bool quiet = false) const;
   std::vector<std::string> GetStringVector(const std::string& parameter, bool quiet = false) const;

   // getter functions with default value
   // can't do this for GetBool as the default bool would clash with the signature with the "quiet" bool
   int GetInt(const std::string& parameter, int def) const
   {
      try {
         return fInt.at(parameter);
      } catch(std::out_of_range& e) {
         return def;
      }
   }
   double GetDouble(const std::string& parameter, double def) const
   {
      try {
         return fDouble.at(parameter);
      } catch(std::out_of_range& e) {
         return def;
      }
   }
   std::string GetString(const std::string& parameter, std::string def) const
   {
      try {
         return fString.at(parameter);
      } catch(std::out_of_range& e) {
         return def;
      }
   }

   // setter functions
   void SetBool(const std::string& parameter, bool value) { fBool[parameter] = value; }
   void SetInt(const std::string& parameter, int value) { fInt[parameter] = value; }
   void SetDouble(const std::string& parameter, double value) { fDouble[parameter] = value; }
   void SetString(const std::string& parameter, const std::string& value) { fString[parameter] = value; }
   void SetBoolVector(const std::string& parameter, const std::vector<bool>& value) { fBoolVector[parameter] = value; }
   void SetIntVector(const std::string& parameter, const std::vector<int>& value) { fIntVector[parameter] = value; }
   void SetDoubleVector(const std::string& parameter, const std::vector<double>& value) { fDoubleVector[parameter] = value; }
   void SetStringVector(const std::string& parameter, const std::vector<std::string>& value) { fStringVector[parameter] = value; }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* = "") override
   {
      fBool.clear();
      fInt.clear();
      fDouble.clear();
      fString.clear();
      fBoolVector.clear();
      fIntVector.clear();
      fDoubleVector.clear();
      fStringVector.clear();
      SetName("");
   }

   Long64_t Merge(TCollection* list, Option_t* = "");

private:
   using TObject::Compare;
   bool Compare(const TUserSettings* settings) const;

   void ParseValue(const std::string& name, const std::string& value, bool vector);

   std::map<std::string, bool>                     fBool;
   std::map<std::string, int>                      fInt;
   std::map<std::string, double>                   fDouble;
   std::map<std::string, std::string>              fString;
   std::map<std::string, std::vector<bool>>        fBoolVector;
   std::map<std::string, std::vector<int>>         fIntVector;
   std::map<std::string, std::vector<double>>      fDoubleVector;
   std::map<std::string, std::vector<std::string>> fStringVector;

   std::vector<std::string> fSettingsFiles;

   /// \cond CLASSIMP
   ClassDefOverride(TUserSettings, 5)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
