#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Globals.h"

struct ParseError : public std::runtime_error {
   explicit ParseError(const char* msg) : std::runtime_error(msg) {}
   explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

/** Base class used to parse an individual item.
    Most methods are implemented in the templated ArgParseConfig<T>
 */
class ArgParseItem {
public:
   explicit ArgParseItem(bool firstPass) : fFirstPass(firstPass) {}
   ArgParseItem(const ArgParseItem&)                                                   = default;
   ArgParseItem(ArgParseItem&&) noexcept                                               = default;
   ArgParseItem& operator=(const ArgParseItem&)                                        = default;
   ArgParseItem& operator=(ArgParseItem&&) noexcept                                    = default;
   virtual ~ArgParseItem()                                                             = default;
   virtual bool        matches(const std::string& flag) const                          = 0;
   virtual void        parse_item(const std::vector<std::string>& arguments)           = 0;
   virtual int         num_arguments() const                                           = 0;
   virtual std::string printable(int description_column, int* chars_before_desc) const = 0;
   virtual bool        is_required() const                                             = 0;
   bool                is_present() const { return fPresent; }
   virtual std::string flag_name() const = 0;

   void parse(const std::string& name, const std::vector<std::string>& arguments, bool firstPass,
              bool ignore_num_arguments = false)
   {
      if(firstPass != fFirstPass) {
         return;
      }
      if(!ignore_num_arguments) {
         if((num_arguments() == -1 && arguments.empty()) ||
            (num_arguments() != -1 && arguments.size() != static_cast<size_t>(num_arguments()))) {
            std::stringstream str;
            if(num_arguments() == -1) {
               str << R"(Flag ")" << name << R"(" expected at least one argument)";
            } else {
               str << R"(Flag ")" << name << R"(" expected )" << num_arguments() << " argument(s) and received "
                   << arguments.size();
            }
            throw ParseError(str.str());
         }
      }

      fPresent = true;
      parse_item(arguments);
   }

private:
   bool fPresent{false};
   bool fFirstPass;
};

template <typename T>
class ArgParseConfig : public ArgParseItem {
public:
   ArgParseConfig(const std::string& flag_list, bool firstPass) : ArgParseItem(firstPass)
   {
      std::stringstream str(flag_list);
      while(!str.eof()) {
         std::string temp;
         str >> temp;
         fRawFlags.push_back(temp);
         if(temp.length() == 1) {
            fFlags.push_back("-" + temp);
         } else if(temp.length() > 1) {
            fFlags.push_back("--" + temp);
         }
      }
   }
   ArgParseConfig(const ArgParseConfig&)                = default;
   ArgParseConfig(ArgParseConfig&&) noexcept            = default;
   ArgParseConfig& operator=(const ArgParseConfig&)     = default;
   ArgParseConfig& operator=(ArgParseConfig&&) noexcept = default;
   ~ArgParseConfig() override                           = default;

   std::string flag_name() const override
   {
      std::string output;
      for(const auto& flag : fFlags) {
         if(flag.length() > output.length()) {
            output = flag;
         }
      }
      return output;
   }

   bool matches(const std::string& flag) const override
   {
      // This is the default option, and something not a flag was passed.
      if(flag.at(0) != '-' && fFlags.empty()) {
         return true;
      }

      return std::any_of(fFlags.begin(), fFlags.end(), [&flag](auto cfl) { return cfl == flag; });
   }

   virtual ArgParseConfig& description(const std::string& description)
   {
      fDescription = description;
      return *this;
   }

   virtual ArgParseConfig& colour(const std::string& colour)
   {
      fColour = colour;
      return *this;
   }

   virtual ArgParseConfig& required()
   {
      fRequired = true;
      return *this;
   }

   bool is_required() const override { return fRequired; }

   virtual ArgParseConfig& default_value(T value) = 0;

   std::string printable(int description_column, int* chars_before_desc) const override
   {
      std::stringstream str;

      str << "  " << fColour;

      bool has_singlechar_flag = false;
      for(const auto& flag : fFlags) {
         if(flag.length() == 2) {
            str << flag << " ";
            has_singlechar_flag = true;
         }
      }
      for(const auto& flag : fFlags) {
         if(flag.length() != 2) {
            if(has_singlechar_flag) {
               str << "[ ";
            }
            str << flag << " ";
            if(has_singlechar_flag) {
               str << "]";
            }
         }
      }

      if(num_arguments() != 0) {
         str << " arg ";
      }

      auto chars = str.tellp();
      chars -= fColour.length();
      if(chars_before_desc != nullptr) {
         *chars_before_desc = static_cast<int>(chars);
      }

      if(description_column != -1 && chars < description_column) {
         for(unsigned int i = 0; i < description_column - chars; i++) {
            str << " ";
         }
      }

      str << fDescription << RESET_COLOR;

      return str.str();
   }

private:
   /// A description for display on the terminal.
   std::string fDescription;

   /// Colour string to be use for display
   std::string fColour;

   /// The literal flag that is searched for, including leading dashes.
   std::vector<std::string> fFlags;

   /// The flags without the leading dashes.
   std::vector<std::string> fRawFlags;

   /// Whether the flag must be supplied.
   bool fRequired{false};
};

template <typename T>
class ArgParseConfigT : public ArgParseConfig<T> {
public:
   ArgParseConfigT(std::string flag, T* output_location, bool firstPass)
      : ArgParseConfig<T>(flag, firstPass), fOutput_location(output_location)
   {
   }

   ArgParseConfig<T>& default_value(T value) override
   {
      *fOutput_location = value;
      return *this;
   }

   void parse_item(const std::vector<std::string>& arguments) override
   {
      std::stringstream str(arguments[0]);
      str >> *fOutput_location;
   }

   int num_arguments() const override { return 1; }

private:
   T* fOutput_location;
};

template <>
class ArgParseConfigT<bool> : public ArgParseConfig<bool> {
public:
   ArgParseConfigT(const std::string& flag, bool* output_location, bool firstPass)
      : ArgParseConfig<bool>(flag, firstPass), fOutput_location(output_location)
   {
      *fOutput_location = fStored_default_value;
   }

   ArgParseConfig<bool>& default_value(bool value) override
   {
      *fOutput_location     = value;
      fStored_default_value = value;
      return *this;
   }

   ArgParseConfig<bool>& takes_argument()
   {
      fNum_arguments_expected = 1;
      return *this;
   }

   void parse_item(const std::vector<std::string>& arguments) override
   {
      if(arguments.empty()) {
         *fOutput_location = !fStored_default_value;
      } else {
         std::stringstream str(arguments[0]);
         str >> std::boolalpha >> *fOutput_location;
      }
   }

   int num_arguments() const override { return fNum_arguments_expected; }

private:
   bool* fOutput_location;
   bool  fStored_default_value{false};
   int   fNum_arguments_expected{0};
};

template <typename T>
class ArgParseConfigT<std::vector<T>> : public ArgParseConfig<std::vector<T>> {
public:
   ArgParseConfigT(std::string flag, std::vector<T>* output_location, bool firstPass)
      : ArgParseConfig<std::vector<T>>(flag, firstPass), fOutput_location(output_location)
   {
   }

   void parse_item(const std::vector<std::string>& arguments) override
   {
      for(const auto& arg : arguments) {
         std::stringstream str(arg);
         T                 val;
         str >> val;
         fOutput_location->push_back(val);
      }
   }

   int num_arguments() const override { return fNum_arguments_expected; }

   ArgParseConfig<std::vector<T>>& default_value(std::vector<T> value) override
   {
      *fOutput_location = value;
      return *this;
   }

private:
   std::vector<T>* fOutput_location;
   int             fNum_arguments_expected{-1};
};

//////////////////////////////////////////////////////////////////////////
///
/// \class ArgParser
///
/// This class is used to parse the command line arguments.
///
/// Example usage:
/// ```
/// ArgParser parser;
/// parser.option("some-option s", &myOption, true).description("my cool option").default_value(42);
/// parser.parse(argc, argv, true);
/// ```
/// to read the flag "--some-option" or "-s" into the integer variable myOption.
///
/// The 3rd argument in ArgParser::option and ArgParser::parse that are set
/// to true here are the "firstPass" arguments which simply means we want
/// to read these options right away instead of reading something from an
/// input file and only then parse the command line arguments.
///
//////////////////////////////////////////////////////////////////////////

class ArgParser {
public:
   ArgParser()                     = default;
   ArgParser(const ArgParser&)     = default;
   ArgParser(ArgParser&&) noexcept = default;

   ~ArgParser()
   {
      for(auto* val : values) {
         delete val;
      }
   }

   ArgParser& operator=(const ArgParser&)     = default;
   ArgParser& operator=(ArgParser&&) noexcept = default;

   void parse(int argc, char** argv, bool firstPass)
   {
      /// this version takes argc and argv, parses them, and sets only those
      /// that have the matching firstPass flag set
      /// this allows us to parse command line arguments in two stages, one
      /// to get the normal options and file names (from which the run info
      /// and analysis options are read), and a second stage where only the
      /// run info and analysis option flags are parsed
      bool double_dash_encountered = false;

      int iarg = 1;
      while(iarg < argc) {
         std::string arg = argv[iarg++];

         if(arg.at(0) != '-' || double_dash_encountered) {
            handle_default_option(argc, argv, iarg, firstPass);
         } else if(arg.substr(0, 2) == "--") {
            handle_long_flag(argc, argv, iarg, firstPass);
         } else {
            handle_short_flag(argc, argv, iarg, firstPass);
         }
      }

      for(auto& val : values) {
         if(val->is_required() && !val->is_present()) {
            std::stringstream str;
            str << R"(Required argument ")" << val->flag_name() << R"(" is not present)";
            throw ParseError(str.str());
         }
      }
   }

   void parse_file(std::string& filename)
   {
      std::ifstream infile(filename);

      std::string line;
      while(std::getline(infile, line)) {
         size_t colon     = line.find(':');
         bool   has_colon = (colon != std::string::npos);

         std::string flag;
         std::string remainder;
         if(has_colon) {
            flag = line.substr(0, colon);
            std::stringstream(flag) >> flag;   // Strip out whitespace
            if(flag.length() == 1) {
               flag.insert(0, 1, '-');
            } else {
               flag.insert(0, 2, '-');
            }
            remainder = line.substr(colon + 1, line.length());
         } else {
            flag      = "";
            remainder = line;
         }

         std::vector<std::string> args;
         std::stringstream        str(remainder);
         std::string              tmparg;
         while(str >> tmparg) {
            args.push_back(tmparg);
         }

         if(has_colon) {
            ArgParseItem& item = get_item(flag);
            item.parse(flag, args, true, true);    // parse "firstPass" items only
            item.parse(flag, args, false, true);   // parse "!firstPass" items only
         } else {
            for(auto& arg : args) {
               ArgParseItem& item = get_item(arg);
               item.parse(arg, std::vector<std::string>{arg}, true);    // parse "firstPass" items only
               item.parse(arg, std::vector<std::string>{arg}, false);   // parse "!firstPass" items only
            }
         }
      }
   }

   template <typename T>
   ArgParseConfigT<T>& option(const std::string flag, T* output_location, bool firstPass)
   {
      auto* output = new ArgParseConfigT<T>(flag, output_location, firstPass);
      values.push_back(output);
      return *output;
   }

   template <typename T>
   ArgParseConfigT<std::vector<T>>& default_option(std::vector<T>* output_location, bool firstPass)
   {
      return option("", output_location, firstPass);
   }

   void print(std::ostream& out) const
   {
      out << "Options:\n";

      int max_length = -1;
      for(auto* item : values) {
         int length = 0;
         item->printable(-1, &length);
         max_length = std::max(length, max_length);
      }

      for(auto it = values.begin(); it != values.end(); it++) {
         ArgParseItem* item = *it;
         out << item->printable(max_length, nullptr);
         if(it != values.end() - 1) {
            out << "\n";
         }
      }
   }

private:
   void handle_long_flag(int argc, char** argv, int& iarg, bool firstPass)
   {
      std::string              arg = argv[iarg - 1];
      std::vector<std::string> flag_args;
      std::string              flag;
      size_t                   equals_index = arg.find('=');
      if(equals_index == std::string::npos) {
         // flag followed by list of flag_args
         flag = arg;
      } else {
         // = inside flag
         flag = arg.substr(0, equals_index);
      }

      ArgParseItem& item = get_item(flag);

      if(equals_index == std::string::npos) {
         flag_args = argument_list(argc, argv, iarg, item.num_arguments());
      } else {
         flag_args.push_back(arg.substr(equals_index + 1));
      }
      item.parse(flag, flag_args, firstPass);
   }

   void handle_short_flag(int argc, char** argv, int& iarg, bool firstPass)
   {
      std::string   arg  = argv[iarg - 1];
      std::string   flag = arg.substr(0, 2);
      ArgParseItem& item = get_item(flag);
      if(item.num_arguments() == 0) {
         // Each character is a boolean flag
         for(unsigned int ichar = 1; ichar < arg.length(); ichar++) {
            std::string              tmpflag = "-" + arg.substr(ichar, 1);
            std::vector<std::string> flag_args;
            get_item(tmpflag).parse(tmpflag, flag_args, firstPass);
         }
      } else {
         if(arg.length() == 2) {
            // Next arguments passed to the program get passed to the flag.
            std::vector<std::string> flag_args = argument_list(argc, argv, iarg, item.num_arguments());
            item.parse(flag, flag_args, firstPass);
         } else {
            // Everything past the first character is argument to the flag.
            std::vector<std::string> flag_args{arg.substr(2)};
            item.parse(flag, flag_args, firstPass);
         }
      }
   }

   void handle_default_option(int /*argc*/, char** argv, int& iarg, bool firstPass)
   {
      std::string              arg = argv[iarg - 1];
      std::vector<std::string> flag_args{arg};

      ArgParseItem& item = get_item(arg);
      item.parse(arg, flag_args, firstPass);
   }

   //! Reads arguments into a list until finding one that begins with '-'
   static std::vector<std::string> argument_list(int argc, char** argv, int& iarg, int max_args)
   {
      std::vector<std::string> output;
      bool                     read_extra = false;
      while(iarg < argc && (max_args == -1 || output.size() < static_cast<size_t>(max_args))) {
         std::string next_arg = argv[iarg++];
         if(!next_arg.empty() && next_arg.at(0) == '-') {
            read_extra = true;
            break;
         }
         output.push_back(next_arg);
      }
      if(read_extra) {
         iarg--;
      }
      return output;
   }

   ArgParseItem& get_item(const std::string& flag)
   {
      for(auto* val : values) {
         if(val->matches(flag)) {
            return *val;
         }
      }

      std::stringstream str;
      if(flag.at(0) == '-') {
         str << R"(Unknown option: ")" << flag << R"(")";
      } else {
         str << R"(Was passed ")" << flag << R"(" as a non-option argument, when no non-option arguments are allowed)";
      }
      throw ParseError(str.str());
   }

   std::vector<ArgParseItem*> values;
};

std::ostream& operator<<(std::ostream& out, const ArgParser& val);

#endif /* _ARGPARSER_H_ */
