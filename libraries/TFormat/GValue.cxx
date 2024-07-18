#include "GValue.h"
#include "TBuffer.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <fstream>
#include <sstream>

#include "TGRSIUtilities.h"

// std::string GValue::fValueData
// std::map<unsigned int, GValue*> GValue::fValueMap;
GValue*                        GValue::fDefaultValue = new GValue("GValue", sqrt(-1));
std::map<std::string, GValue*> GValue::fValueVector;

GValue::GValue(const char* name, double value, EPriority priority)
   : TNamed(name, name), fValue(value), fPriority(priority)
{
}

GValue::GValue(const char* name) : TNamed(name, name), fValue(0.00), fPriority(EPriority::kDefault)
{
}

GValue::GValue(const GValue& val) : TNamed(val)
{
   val.Copy(*this);
}

void GValue::Copy(TObject& obj) const
{
   TNamed::Copy(obj);
   static_cast<GValue&>(obj).fValue    = fValue;
   static_cast<GValue&>(obj).fPriority = fPriority;
}

double GValue::Value(const std::string& name)
{
   return GValue::Value(name, sqrt(-1));
}

double GValue::Value(const std::string& name, const double& defaultValue)
{
   if(fValueVector.count(name) == 0u) {
      return defaultValue;
   }
   return fValueVector.at(name)->GetValue();
}

void GValue::SetReplaceValue(const std::string& name, double value, EPriority priority)
{
   GValue* gvalue = FindValue(name);
   if(gvalue == nullptr) {
      gvalue = new GValue(name.c_str(), value, priority);
      AddValue(gvalue);
   } else if(priority <= gvalue->fPriority) {
      gvalue->SetValue(value);
      gvalue->fPriority = priority;
   }
}

GValue* GValue::FindValue(const std::string& name)
{
   GValue* value = nullptr;
   if(name.length() == 0u) {
      return GetDefaultValue();
   }
   if(fValueVector.count(name) != 0u) {
      value = fValueVector[name];
   }
   return value;
}

bool GValue::AppendValue(GValue* oldvalue)
{
   if(fPriority <= oldvalue->fPriority) {
      if(strlen(GetName()) != 0u) {
         oldvalue->SetName(GetName());
      }

      if(GetValue() != -1) {
         oldvalue->SetValue(GetValue());
         oldvalue->fPriority = fPriority;
      }

      if(strlen(GetInfo()) != 0u) {
         oldvalue->SetInfo(GetInfo());
      }
      return true;
   }

   return false;
}

bool GValue::ReplaceValue(GValue* oldvalue)
{
   if(fPriority <= oldvalue->fPriority) {
      Copy(*oldvalue);
      return true;
   }
   return false;
}

bool GValue::AddValue(GValue* value, Option_t*)
{
   if(value == nullptr) {
      return false;
   }
   std::string temp_string = value->GetName();
   if(temp_string == "") {
      // default value, get rid of it and ignore;
      delete value;
      return false;
   }

   GValue* oldvalue = GValue::FindValue(temp_string);
   if(oldvalue != nullptr) {
      value->ReplaceValue(oldvalue);
      delete value;
      return true;
   }
   fValueVector[temp_string] = value;
   return true;
}

std::string GValue::PrintToString() const
{

   std::string buffer;
   buffer.append(GetName());
   buffer.append("\t{\n");
   buffer.append(Form("value:\t%f\n", fValue));
   if(!info.empty()) {
      buffer.append("info:\t");
      buffer.append(info);
      buffer.append("\n");
   }
   buffer.append("}\n");
   return buffer;
}

void GValue::Print(Option_t*) const
{
   std::cout << PrintToString() << std::endl;
}

int GValue::WriteValFile(const std::string& filename, Option_t*)
{
   // std::string filebuffer;
   if(filename.length() != 0u) {
      std::ofstream outfile;
      outfile.open(filename.c_str());
      if(!outfile.is_open()) {
         return -1;
      }
      for(auto iter = fValueVector.begin(); iter != fValueVector.end(); iter++) {
         outfile << iter->second->PrintToString() << std::endl
                 << std::endl;
      }
   } else {
      for(auto iter = fValueVector.begin(); iter != fValueVector.end(); iter++) {
         std::cout << iter->second->PrintToString() << std::endl
                   << std::endl;
      }
   }
   return fValueVector.size();
}

std::string GValue::WriteToBuffer(Option_t*)
{
   std::string buffer;
   if(GValue::Size() == 0) {
      return buffer;
   }
   for(auto iter = fValueVector.begin(); iter != fValueVector.end(); iter++) {
      buffer.append(iter->second->PrintToString());
      buffer.append("\n");
   }
   return buffer;
}

void GValue::Clear()
{
   // loop over all values and delete them
   for(const auto& value : fValueVector) {
      delete value.second;
   }
   // delete map
   fValueVector.clear();
}

int GValue::ReadValFile(const char* filename, Option_t* opt)
{
   std::string infilename = filename;
   if(infilename.length() == 0) {
      return -1;
   }

   std::ifstream infile;
   infile.open(infilename.c_str());
   if(!infile) {
      std::cerr << __PRETTY_FUNCTION__ << ":  could not open infile " << infilename << std::endl;
      return -2;
   }
   infile.seekg(0, std::ios::end);
   size_t length = infile.tellg();
   if(length == 0) {
      std::cerr << __PRETTY_FUNCTION__ << ":  infile " << infilename << " appears to be empty." << std::endl;
      return -2;
   }

   std::string       sbuffer;
   std::vector<char> buffer(length);
   infile.seekg(0, std::ios::beg);
   infile.read(buffer.data(), static_cast<int>(length));
   sbuffer.assign(buffer.data());

   int values_found = ParseInputData(sbuffer, EPriority::kValFile, opt);
   return values_found;
}

// Parses input file. Should be in the form:
// NAME {
//  Name :
//  Value :
//  Info  :
//}
int GValue::ParseInputData(const std::string& input, EPriority priority, Option_t* opt)
{
   std::istringstream infile(input);
   GValue*            value = nullptr;
   std::string        line;
   int                linenumber = 0;
   int                newvalues  = 0;

   bool        brace_open = false;
   std::string name;

   while(!std::getline(infile, line).fail()) {
      linenumber++;
      trim(line);
      size_t comment = line.find("//");
      if(comment != std::string::npos) {
         line = line.substr(0, comment);
      }
      if(line.length() == 0) {
         continue;
      }
      size_t openbrace  = line.find('{');
      size_t closebrace = line.find('}');
      size_t colon      = line.find(':');

      //=============================================//
      if(openbrace == std::string::npos && closebrace == std::string::npos && colon == std::string::npos) {
         continue;
      }
      //=============================================//
      if(openbrace != std::string::npos) {
         brace_open = true;
         name       = line.substr(0, openbrace);
         trim(name);
         value = new GValue(name.c_str());
      }
      //=============================================//
      if(brace_open) {
         if(colon != std::string::npos) {
            std::string type;
            if(openbrace == std::string::npos || openbrace > colon) {
               type = line.substr(0, colon);
            } else {
               type = line.substr(openbrace + 1, colon - (openbrace + 1));
            }
            line = line.substr(colon + 1, line.length());
            trim(line);   // strip beginning whitespace (not needed for value itself, but for the readability of info)
            trim(type);
            std::transform(type.begin(), type.end(), type.begin(), ::toupper);
            if(type == "NAME") {
               value->SetName(line.c_str());
            } else if(type == "VALUE") {
               value->SetValue(std::atof(line.c_str()));
               value->fPriority = priority;
            } else if(type == "INFO") {
               value->SetInfo(line.c_str());
            }
         }
      }
      //=============================================//
      if(closebrace != std::string::npos) {
         brace_open = false;
         if(value != nullptr) {
            // Check whether value is in vector. If it isn't add it.
            GValue* cur_value = FindValue(value->GetName());
            if(cur_value == nullptr) {
               AddValue(value);
               newvalues++;
            } else {
               value->AppendValue(cur_value);
               delete value;
               newvalues++;
            }
         }
         value = nullptr;
         name.clear();
      }
   }
   if(strcmp(opt, "debug") == 0) {
      std::cout << "parsed " << linenumber << " lines" << std::endl;
   }
   return newvalues;
}

void GValue::Streamer(TBuffer& R__b)
{
   SetBit(kCanDelete);
   UInt_t R__s = 0;
   UInt_t R__c = 0;
   if(R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      TNamed::Streamer(R__b);
      if(R__v > 1) {
      }
      {
         TString R__str;
         R__str.Streamer(R__b);
         ParseInputData(R__str.Data(), EPriority::kRootFile);
      }
      R__b.CheckByteCount(R__s, R__c, GValue::IsA());
   } else {
      R__c = R__b.WriteVersion(GValue::IsA(), true);
      TNamed::Streamer(R__b);
      {
         TString R__str = GValue::WriteToBuffer();
         R__str.Streamer(R__b);
      }
      R__b.SetByteCount(R__c, true);
   }
}
