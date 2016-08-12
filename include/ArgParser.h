#ifndef _ARGPARSER_H_
#define _ARGPARSER_H_

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct ParseError : public std::runtime_error{
  ParseError(const char* msg) : std::runtime_error(msg) { }
  ParseError(const std::string& msg) : std::runtime_error(msg) { }
};

//// Because gcc-4.7 does not fully implement C++11
// struct ParseError : public std::runtime_error{
//   using std::runtime_error::runtime_error;
// };


/** Base class used to parse an individual item.
    Most methods are implemented in the templated ArgParseConfig<T>
 */
class ArgParseItem{
public:
  ArgParseItem() : present_(false) { }
  virtual ~ArgParseItem() { }
  virtual bool matches(const std::string& flag) const = 0;
  virtual void parse_item(const std::vector<std::string>& arguments) = 0;
  virtual int num_arguments() const = 0;
  virtual std::string printable(int description_column = -1, int* chars_before_desc=NULL) const = 0;
  virtual bool is_required() const = 0;
  bool is_present() const {return present_;}
  virtual std::string flag_name() const = 0;

  void parse(const std::string& name, const std::vector<std::string>& arguments,
             bool ignore_num_arguments=false){
    if(!ignore_num_arguments){
      if((num_arguments()==-1 && arguments.size()==0) ||
         (num_arguments()!=-1 && arguments.size() != size_t(num_arguments()))){
        std::stringstream ss;
        if(num_arguments()==-1){
          ss << "Flag \"" << name << "\" expected at least one argument";
        } else {
          ss << "Flag \"" << name << "\" expected " << num_arguments()
             << " argument(s) and received " << arguments.size();
        }
        throw ParseError(ss.str());
      }
    }

    present_ = true;
    parse_item(arguments);
  }

private:
  bool present_;
};

template<typename T>
class ArgParseConfig : public ArgParseItem {
public:
  ArgParseConfig(std::string flag_list)
    : desc(""), required_(false) {
    std::stringstream ss(flag_list);
    while(!ss.eof()){
      std::string temp;
      ss >> temp;
      raw_flags.push_back(temp);
      if(temp.length() == 1){
        flags.push_back("-" + temp);
      } else if (temp.length() > 1) {
        flags.push_back("--" + temp);
      }
    }
  }
  virtual ~ArgParseConfig(){ }

  virtual std::string flag_name() const {
    std::string output;
    for(auto flag : flags){
      if(flag.length() > output.length()){
        output = flag;
      }
    }
    return output;
  }

  virtual bool matches(const std::string& flag) const {
    // This is the default option, and something not a flag was passed.
    if(flag.at(0)!='-' && flags.size()==0){
      return true;
    }

    for(auto& f : flags){
      if(f == flag){
        return true;
      }
    }
    return false;
  }

  virtual ArgParseConfig& description(const std::string& d){
    desc = d;
    return *this;
  }

  virtual ArgParseConfig& required(){
    required_ = true;
    return *this;
  }

  virtual bool is_required() const {
    return required_;
  }

  virtual ArgParseConfig& default_value(T value) = 0;

  virtual std::string printable(int description_column = -1, int* chars_before_desc=NULL) const {
    std::stringstream ss;

    ss << "  ";

    bool has_singlechar_flag = false;
    for(auto flag : flags){
      if(flag.length()==2){
        ss << flag << " ";
        has_singlechar_flag = true;
      }
    }
    for(auto flag : flags){
      if(flag.length()!=2){
        if(has_singlechar_flag){
          ss << "[ ";
        }
        ss << flag;
        if(has_singlechar_flag){
          ss << " ]";
        }
      }
    }

    if(num_arguments()!=0){
      ss << " arg ";
    }

    auto chars = ss.tellp();
    if(chars_before_desc){
      *chars_before_desc = chars;
    }

    if(description_column != -1 &&
       chars < description_column){
      for(unsigned int i=0; i<description_column-chars; i++){
        ss << " ";
      }
    }

    ss << desc;

    return ss.str();
  }

protected:
  /// A description for display on the terminal.
  std::string desc;

  /// The literal flag that is searched for, including leading dashes.
  std::vector<std::string> flags;

  /// The flags without the leading dashes.
  std::vector<std::string> raw_flags;

  /// Whether the flag must be supplied.
  bool required_;
};

template<typename T>
class ArgParseConfigT : public ArgParseConfig<T> {
public:
  ArgParseConfigT(std::string flag, T* output_location)
    : ArgParseConfig<T>(flag), output_location(output_location) { }

  virtual ArgParseConfig<T>& default_value(T value){
    *output_location = value;
    return *this;
  }

  virtual void parse_item(const std::vector<std::string>& arguments){
    std::stringstream ss(arguments[0]);
    ss >> *output_location;
  }

  virtual int num_arguments() const { return 1; }


private:
  T* output_location;
};

template<>
class ArgParseConfigT<bool> : public ArgParseConfig<bool> {
public:
  ArgParseConfigT(std::string flag, bool* output_location)
    : ArgParseConfig<bool>(flag), output_location(output_location),
      stored_default_value(false) {
    *output_location = stored_default_value;
  }

  virtual ArgParseConfig<bool>& default_value(bool value){
    *output_location = value;
    stored_default_value = value;
    return *this;
  }

  virtual void parse_item(const std::vector<std::string>& arguments){
    if(arguments.size() == 0){
      *output_location = !stored_default_value;
    } else {
      std::stringstream ss(arguments[0]);
      ss >> *output_location;
    }
  }

  virtual int num_arguments() const { return 0; }

private:
  bool* output_location;
  bool stored_default_value;
};

template<typename T>
class ArgParseConfigT<std::vector<T> > : public ArgParseConfig<std::vector<T> > {
public:
  ArgParseConfigT(std::string flag, std::vector<T>* output_location)
    : ArgParseConfig<std::vector<T> >(flag), output_location(output_location),
      num_arguments_expected(-1) { }

  virtual void parse_item(const std::vector<std::string>& arguments){
    for(auto arg : arguments){
      std::stringstream ss(arg);
      T val;
      ss >> val;
      output_location->push_back(val);
    }
  }

  virtual int num_arguments() const { return num_arguments_expected; }

  virtual ArgParseConfig<std::vector<T> >& default_value(std::vector<T> value){
    *output_location = value;
    return *this;
  }

private:
  std::vector<T>* output_location;
  int num_arguments_expected;
};

class ArgParser {
public:
  ArgParser() { }

  ~ArgParser() {
    for(auto val : values){
      delete val;
    }
  }

  void parse(int argc, char** argv) {
    bool double_dash_encountered = false;

    int iarg = 1;
    while(iarg < argc){

      std::string arg = argv[iarg++];

      if(arg.at(0) != '-' ||
         double_dash_encountered){
        handle_default_option(argc, argv, iarg);
      } else if(arg.substr(0,2) == "--"){
        handle_long_flag(argc, argv, iarg);
      } else {
        handle_short_flag(argc, argv, iarg);
      }
    }

    for(auto& val : values){
      if(val->is_required() && !val->is_present()){
        std::stringstream ss;
        ss << "Required argument \"" << val->flag_name() << "\" is not present";
        throw ParseError(ss.str());
      }
    }
  }

  void parse_file(std::string& filename) {
    std::ifstream infile(filename);

    std::string line;
    while(std::getline(infile, line)){
      size_t colon = line.find(":");
      bool has_colon = (colon != std::string::npos);

      std::string flag;
      std::string remainder;
      if(has_colon){
        flag = line.substr(0,colon);
        std::stringstream(flag) >> flag; //Strip out whitespace
        if(flag.length()==1){
          flag = "-" + flag;
        } else {
          flag = "--" + flag;
        }
        remainder = line.substr(colon+1, line.length());
      } else {
        flag = "";
        remainder = line;
      }

      std::vector<std::string> args;
      std::stringstream ss(remainder);
      std::string arg;
      while(ss >> arg){
        args.push_back(arg);
      }

      if(has_colon){
        ArgParseItem& item = get_item(flag);
        item.parse(flag, args, true);
      } else {
        for(auto& arg : args){
          ArgParseItem& item = get_item(arg);
          item.parse(arg, std::vector<std::string>{arg});
        }
      }
    }
  }

  template<typename T>
  ArgParseConfigT<T>& option(const std::string flag, T* output_location){
    ArgParseConfigT<T>* output = new ArgParseConfigT<T>(flag, output_location);
    values.push_back(output);
    return *output;
  }

  template<typename T>
  ArgParseConfigT<std::vector<T> >& default_option(std::vector<T>* output_location){
    return option("", output_location);
  }

  void print(std::ostream& out) const {
    out << "Options:\n";

    int max_length = -1;
    for(auto item : values){
      int length;
      item->printable(-1, &length);
      max_length = std::max(length, max_length);
    }

    for(auto it = values.begin(); it!=values.end(); it++){
      ArgParseItem* item = *it;
      out << item->printable(max_length);
      if(it!=values.end()-1){
        out << "\n";
      }
    }
  }

private:
  void handle_long_flag(int argc, char** argv, int& iarg){
    std::string arg = argv[iarg-1];
    std::vector<std::string> flag_args;
    std::string flag;
    size_t equals_index = arg.find("=");
    if(equals_index == std::string::npos){
      // flag followed by list of flag_args
      flag = arg;
    } else {
      // = inside flag
      flag = arg.substr(0, equals_index);
    }

    ArgParseItem& item = get_item(flag);

    if(equals_index == std::string::npos){
      flag_args = argument_list(argc, argv, iarg, item.num_arguments());
    } else {
      flag_args.push_back(arg.substr(equals_index+1));
    }

    item.parse(flag, flag_args);
  }

  void handle_short_flag(int argc, char** argv, int& iarg){
    std::string arg = argv[iarg-1];
    std::string flag = arg.substr(0,2);
    ArgParseItem& item = get_item(flag);
    if(item.num_arguments() == 0){
      // Each character is a boolean flag
      for(unsigned int ichar=1; ichar<arg.length(); ichar++){
        std::string flag = "-" + arg.substr(ichar,1);
        std::vector<std::string> flag_args;
        get_item(flag).parse(flag, flag_args);
      }
    } else {
      if(arg.length() == 2){
        // Next arguments passed to the program get passed to the flag.
        std::vector<std::string> flag_args = argument_list(argc, argv, iarg, item.num_arguments());
        item.parse(flag, flag_args);
      } else {
        // Everything past the first character is argument to the flag.
        std::vector<std::string> flag_args{arg.substr(2)};
        item.parse(flag, flag_args);
      }
    }
  }

  void handle_default_option(int /*argc*/, char** argv, int& iarg){
    std::string arg = argv[iarg-1];
    std::vector<std::string> flag_args{arg};

    ArgParseItem& item = get_item(arg);
    item.parse(arg, flag_args);
  }

  //! Reads arguments into a list until finding one that begins with '-'
  std::vector<std::string> argument_list(int argc, char** argv, int& iarg, int max_args){
    std::vector<std::string> output;
    bool read_extra = false;
    while(iarg<argc &&
          (max_args==-1 || output.size() < size_t(max_args))){
      std::string next_arg = argv[iarg++];
      if(next_arg.length() && next_arg.at(0) == '-'){
        read_extra = true;
        break;
      } else {
        output.push_back(next_arg);
      }
    }
    if(read_extra){
      iarg--;
    }
    return output;
  }

  ArgParseItem& get_item(const std::string& flag){
    for(auto val : values){
      if(val->matches(flag)){
        return *val;
      }
    }

    std::stringstream ss;
    if(flag.at(0)=='-'){
      ss << "Unknown option: \"" << flag << "\"";
    } else {
      ss << "Was passed \"" << flag << "\" as a non-option argument, when no non-option arguments are allowed";
    }
    throw ParseError(ss.str());
  }

  std::vector<ArgParseItem*> values;
};

std::ostream& operator<<(std::ostream& out, const ArgParser& val){
  val.print(out);
  return out;
}

#endif /* _ARGPARSER_H_ */
