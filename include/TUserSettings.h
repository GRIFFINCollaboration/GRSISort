#ifndef TUSERSETTINGS_H
#define TUSERSETTINGS_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>

#include "TNamed.h"

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
/////////////////////////////////////////////////////////////

class TUserSettings : public TNamed {
public:
	TUserSettings() {}
	TUserSettings(std::string settingsFile) { if(!Read(settingsFile)) throw std::runtime_error("Failed to read user settings file!"); }

	~TUserSettings() {}

	bool Read(std::string settingsFile);

	bool empty() { return fBool.empty() && fInt.empty() && fDouble.empty() && fString.empty() && fBoolVector.empty() && fIntVector.empty() && fDoubleVector.empty() && fStringVector.empty(); }

	// getter functions
	template<typename T>
		T Get(std::string parameter) const
		{
			if(std::is_same<T, bool>::value)                     return GetBool(parameter);
			if(std::is_same<T, int>::value)                      return GetInt(parameter);
			if(std::is_same<T, double>::value)                   return GetDouble(parameter);
			if(std::is_same<T, std::string>::value)              return GetString(parameter);
			if(std::is_same<T, std::vector<bool>>::value)        return GetBoolVector(parameter);
			if(std::is_same<T, std::vector<int>>::value)         return GetIntVector(parameter);
			if(std::is_same<T, std::vector<double>>::value)      return GetDoubleVector(parameter);
			if(std::is_same<T, std::vector<std::string>>::value) return GetStringVector(parameter);
			throw std::runtime_error("Unknown type, only bool, int, double, std::string or vectors of those types allowed");
		}

	bool GetBool(std::string parameter, bool quiet = false) const;
	int GetInt(std::string parameter, bool quiet = false) const;
	double GetDouble(std::string parameter, bool quiet = false) const;
	std::string GetString(std::string parameter, bool quiet = false) const;
	std::vector<bool> GetBoolVector(std::string parameter, bool quiet = false) const;
	std::vector<int> GetIntVector(std::string parameter, bool quiet = false) const;
	std::vector<double> GetDoubleVector(std::string parameter, bool quiet = false) const;
	std::vector<std::string> GetStringVector(std::string parameter, bool quiet = false) const;

	// getter functions with default value
	// can't do this for GetBool as the default bool would clash with the signature with the "quiet" bool 
	int GetInt(std::string parameter, int def) const { try { return fInt.at(parameter); } catch(std::out_of_range& e) { return def; } }
	double GetDouble(std::string parameter, double def) const { try { return fDouble.at(parameter); } catch(std::out_of_range& e) { return def; } }
	std::string GetString(std::string parameter, std::string def) const { try { return fString.at(parameter); } catch(std::out_of_range& e) { return def; } }
	
	void Print(Option_t* opt = "") const override;
	void Clear() { fBool.clear(); fInt.clear(); fDouble.clear(); fString.clear(); fBoolVector.clear(); fIntVector.clear(); fDoubleVector.clear(); fStringVector.clear(); SetName(""); }

private:
	void ParseValue(const std::string& name, const std::string& value, bool vector);

	std::map<std::string, bool> fBool;
	std::map<std::string, int> fInt;
	std::map<std::string, double> fDouble;
	std::map<std::string, std::string> fString;
	std::map<std::string, std::vector<bool>> fBoolVector;
	std::map<std::string, std::vector<int>> fIntVector;
	std::map<std::string, std::vector<double>> fDoubleVector;
	std::map<std::string, std::vector<std::string>> fStringVector;

	std::vector<std::string> fSettingsFiles;

	/// \cond CLASSIMP
	ClassDefOverride(TUserSettings, 5)
	/// \endcond
};
/*! @} */
#endif
