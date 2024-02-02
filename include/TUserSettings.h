#ifndef TUSERSETTINGS_H
#define TUSERSETTINGS_H

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
/// maps for booleans, integers, floats, and strings.
///
/////////////////////////////////////////////////////////////

class TUserSettings : public TNamed {
public:
	TUserSettings() {}
	TUserSettings(std::string settingsFile) { if(!Read(settingsFile)) throw std::runtime_error("Failed to read user settings file!"); }

	~TUserSettings() {}

	bool Read(std::string settingsFile);

	bool empty() { return fBool.empty() && fInt.empty() && fDouble.empty() && fString.empty(); }

	template<typename T>
		T Get(std::string parameter) const
		{
			if(std::is_same<T, bool>::value) return fBool.at(parameter);
			if(std::is_same<T, int>::value) return fInt.at(parameter);
			if(std::is_same<T, double>::value) return fDouble.at(parameter);
			if(std::is_same<T, std::string>::value) return fString.at(parameter);
			throw std::runtime_error("Unknown type, only bool, int, double, or std::string allowed");
		}

	bool GetBool(std::string parameter) const { return fBool.at(parameter); }
	int GetInt(std::string parameter) const { return fInt.at(parameter); }
	double GetDouble(std::string parameter) const { return fDouble.at(parameter); }
	std::string GetString(std::string parameter) const { return fString.at(parameter); }

	void Print(Option_t* opt = "") const override;
	void Clear() { fBool.clear(); fInt.clear(); fDouble.clear(); fString.clear(); SetName(""); }

private:
	std::map<std::string, bool> fBool;
	std::map<std::string, int> fInt;
	std::map<std::string, double> fDouble;
	std::map<std::string, std::string> fString;

	std::vector<std::string> fSettingsFiles;

	/// \cond CLASSIMP
	ClassDefOverride(TUserSettings, 2)
	/// \endcond
};
/*! @} */
#endif
