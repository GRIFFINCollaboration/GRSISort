#include "TUserSettings.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <algorithm>
#include <iomanip>

#include "TGRSIUtilities.h"

ClassImp(TUserSettings)

bool TUserSettings::ReadSettings(std::string settingsFile)
{
	/// Read user settings from text file.
	/// The file is expected to have the format
	/// parameter name: value
	/// where parameter name is a string w/o whitespace and value is the value of the parameter
	/// the type of the value (bool, int, double, std::string, or vectors of these types) will be determined automatically
	/// vectors are comma separated values of one type
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

		// check if this is a comma separated list of values
		if(value.find(',') != std::string::npos) {
			std::stringstream valueStream(value);
			while(std::getline(valueStream, line, ',')) {
				trim(line);
				ParseValue(name, line, true);
			}
		} else {
			ParseValue(name, value, false);
		}
	}

	return true;
}

void TUserSettings::ParseValue(const std::string& name, const std::string& value, bool vector)
{
	// try and parse as integer, if pos is equal to the length of the string we were successful:
	// add it to the map and go to the next line
	size_t pos;
	try {
		auto intVal = std::stoi(value, &pos);
		if(pos == value.length()) {
			if(!vector) {
				fInt[name] = intVal;
			} else {
				fIntVector[name].push_back(intVal);
			}
			return;
		}
	} catch(std::invalid_argument& e) {
		// do nothing, we failed to parse the input as integer, just go on to the next type
	}

	// try and parse as double, if pos is equal to the length of the string we were successful:
	// add it to the map and go to the next line
	try {
		auto doubleVal = std::stod(value, &pos);
		if(pos == value.length()) {
			if(!vector) {
				fDouble[name] = doubleVal;
			} else {
				fDoubleVector[name].push_back(doubleVal);
			}
			return;
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
		if(!vector) {
			fBool[name] = boolVal;
		} else {
			fBoolVector[name].push_back(boolVal);
		}
		return;
	}

	// trying to parse as integer, double, or bool failed, so we assume it's a string and just add it to the map
	if(!vector) {
		fString[name] = value;
	} else {
		fStringVector[name].push_back(value);
	}
}

bool TUserSettings::GetBool(std::string parameter, bool quiet) const
{
	try { 
		return fBool.at(parameter);
	} catch(std::out_of_range& e) {
		if(!quiet) {
			std::cout<<"Failed to find \""<<parameter<<"\" in boolean map"<<std::endl;
			Print();
		}
		throw e;
	}
}

int TUserSettings::GetInt(std::string parameter, bool quiet) const
{
	try {
		return fInt.at(parameter);
	} catch(std::out_of_range& e) {
		if(!quiet) {
			std::cout<<"Failed to find \""<<parameter<<"\" in integer map"<<std::endl;
			Print();
		}
		throw e;
	}
}

double TUserSettings::GetDouble(std::string parameter, bool quiet) const
{
	try {
		return fDouble.at(parameter);
	} catch(std::out_of_range& e) {
		if(!quiet) {
			std::cout<<"Failed to find \""<<parameter<<"\" in double map"<<std::endl;
			Print();
		}
		throw e;
	}
}

std::string TUserSettings::GetString(std::string parameter, bool quiet) const
{
	try {
		return fString.at(parameter);
	} catch(std::out_of_range& e) {
		if(!quiet) {
			std::cout<<"Failed to find \""<<parameter<<"\" in string map"<<std::endl;
			Print();
		}
		throw e;
	}
}

std::vector<bool> TUserSettings::GetBoolVector(std::string parameter, bool quiet) const
{
	try {
		return fBoolVector.at(parameter);
	} catch(std::out_of_range& e) {
		if(!quiet) {
			std::cout<<"Failed to find \""<<parameter<<"\" in boolean vector map"<<std::endl;
			Print();
		}
		throw e;
	}
}

std::vector<int> TUserSettings::GetIntVector(std::string parameter, bool quiet) const
{
	try {
		return fIntVector.at(parameter);
	} catch(std::out_of_range& e) {
		if(!quiet) {
			std::cout<<"Failed to find \""<<parameter<<"\" in integer vector map"<<std::endl;
			Print();
		}
		throw e;
	}
}

std::vector<double> TUserSettings::GetDoubleVector(std::string parameter, bool quiet) const
{
	try {
		return fDoubleVector.at(parameter);
	} catch(std::out_of_range& e) {
		if(!quiet) {
			std::cout<<"Failed to find \""<<parameter<<"\" in double vector map"<<std::endl;
			Print();
		}
		throw e;
	}
}

std::vector<std::string> TUserSettings::GetStringVector(std::string parameter, bool quiet) const
{
	try {
		return fStringVector.at(parameter);
	} catch(std::out_of_range& e) {
		if(!quiet) {
			std::cout<<"Failed to find \""<<parameter<<"\" in string vector map"<<std::endl;
			Print();
		}
		throw e;
	}
}

void TUserSettings::Print(Option_t*) const
{
	std::cout<<"Settings read from";
	for(auto file : fSettingsFiles) std::cout<<" "<<file;
	std::cout<<":"<<std::endl;
	if(!fBool.empty()) std::cout<<"---------- booleans ----------"<<std::endl;
	for(auto val : fBool) {
		std::cout<<std::boolalpha<<val.first<<": "<<val.second<<std::endl;
	}
	if(!fInt.empty()) std::cout<<"---------- integers ----------"<<std::endl;
	for(auto val : fInt) {
		std::cout<<val.first<<": "<<val.second<<std::endl;
	}
	if(!fDouble.empty()) std::cout<<"---------- doubles ----------"<<std::endl;
	for(auto val : fDouble) {
		std::cout<<val.first<<": "<<val.second<<std::endl;
	}
	if(!fString.empty()) std::cout<<"---------- strings ----------"<<std::endl;
	for(auto val : fString) {
		std::cout<<val.first<<": "<<val.second<<std::endl;
	}
	if(!fBoolVector.empty()) std::cout<<"---------- boolean vectors ----------"<<std::endl;
	for(auto val : fBoolVector) {
		std::cout<<std::boolalpha<<val.first<<": ";
		for(auto item : val.second) {
			std::cout<<item<<" ";
		}
		std::cout<<std::endl;
	}
	if(!fIntVector.empty()) std::cout<<"---------- integer vectors ----------"<<std::endl;
	for(auto val : fIntVector) {
		std::cout<<val.first<<": ";
		for(auto item : val.second) {
			std::cout<<item<<" ";
		}
		std::cout<<std::endl;
	}
	if(!fDoubleVector.empty()) std::cout<<"---------- double vectors ----------"<<std::endl;
	for(auto val : fDoubleVector) {
		std::cout<<val.first<<": ";
		for(auto item : val.second) {
			std::cout<<item<<" ";
		}
		std::cout<<std::endl;
	}
	if(!fStringVector.empty()) std::cout<<"---------- string vectors ----------"<<std::endl;
	for(auto val : fStringVector) {
		std::cout<<val.first<<": ";
		for(auto item : val.second) {
			std::cout<<item<<" ";
		}
		std::cout<<std::endl;
	}
}
