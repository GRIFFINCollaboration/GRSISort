#include "TUserSettings.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <algorithm>
#include <iomanip>

#include "TGRSIUtilities.h"

ClassImp(TUserSettings)

bool TUserSettings::Read(std::string settingsFile)
{
	/// Read user settings from text file.
	/// The file is expected to have the format
	/// <parameter name>: <value>
	/// where <parameter name> is a string w/o whitespace and <value> is the value of the parameter
	/// the type of the value (bool, int, double, or std::string) will be determined automatically
	/// the default type will be std::string
	/// any lines starting with '#' or '//' or without a colon will be ignored
	SetName("UserSettings");
	fSettingsFiles.push_back(settingsFile);

	std::ifstream settings(settingsFile);
	
	if(!settings.is_open()) {
		std::cerr<<"Failed to open user settings file '"<<settingsFile<<"'!"<<std::endl;
		return false;
	}

	std::string line;
	while(std::getline(settings, line)) {
		// skip lines with too few character, starting with '#' or "//", or w/o colon
		if(line.length() < 3) continue; // need at least three characters
		if(line[0] == '#') continue;
		if(line[0] == '/' && line[1] == '/') continue;
		auto colon = line.find(':');
		if(colon == std::string::npos) continue;
		// split line at colon
		auto name = line.substr(0, colon);
		auto value = line.substr(colon+1);
		// remove leading and trailing whitespace
		trim(value);

		// try and parse as integer, if pos is equal to the length of the string we were successful:
		// add it to the map and go to the next line
		size_t pos;
		try {
			auto intVal = std::stoi(value, &pos);
			if(pos == value.length()) {
				fInt[name] = intVal;
				continue;
			}
		} catch(std::invalid_argument& e) {
			// do nothing, we failed to parse the input as integer, just go on to the next type
		}

		// try and parse as double, if pos is equal to the length of the string we were successful:
		// add it to the map and go to the next line
		try {
			auto doubleVal = std::stod(value, &pos);
			if(pos == value.length()) {
				fDouble[name] = doubleVal;
				continue;
			}
		} catch(std::invalid_argument& e) {
			// do nothing, we failed to parse the input as double, just go on to the next type
		}

		// try and parse as bool: convert a copy to lower case and use stringstream with std::boolalpha
		std::string copy = value;
		std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);
		bool boolVal;
		std::stringstream str(copy);
		str>>std::boolalpha>>boolVal;
		if(str.good()) {
			fBool[name] = boolVal;
			continue;
		}

		// trying to parse as integer, double, or bool failed, so we assume it's a string and just add it to the map
		fString[name] = value;
	}

	return true;
}

void TUserSettings::Print(Option_t*) const
{
	std::cout<<"Settings read from";
	for(auto file : fSettingsFiles) std::cout<<" "<<file;
	std::cout<<":"<<std::endl;
	for(auto val : fBool) {
		std::cout<<std::boolalpha<<val.first<<": "<<val.second<<std::endl;
	}
	for(auto val : fInt) {
		std::cout<<val.first<<": "<<val.second<<std::endl;
	}
	for(auto val : fDouble) {
		std::cout<<val.first<<": "<<val.second<<std::endl;
	}
	for(auto val : fString) {
		std::cout<<val.first<<": "<<val.second<<std::endl;
	}
}
