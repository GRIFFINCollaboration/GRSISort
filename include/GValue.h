#ifndef TGRUTVARIABLE_H
#define TGRUTVARIABLE_H

#include <map>

#include "TList.h"
#include "TNamed.h"

class GValue : public TNamed {
public:
  enum EPriority {
    kUser = 0,
    kValFile = 1,
    kRootFile = 2,
    kUnset = 999999
  };
  
  GValue();
  GValue(const char *name);
  GValue(const char *name,double value, GValue::EPriority priority=kUser);
  GValue(const GValue &val);

  double GetValue() { return fValue; }
  const char *GetInfo()   const { return info.c_str(); }

  void SetValue(double value) { fValue = value; }
  void SetInfo(const char *temp) { info.assign(temp); }


  static int ReadValFile(const char *filename="",Option_t *opt="replace");
  static int WriteValFile(std::string filename="",Option_t *opt="");

  static GValue* GetDefaultValue() { return fDefaultValue; }
  //Search fValueVector for GValue with name given by string
  static GValue* FindValue(std::string="");
  static void SetReplaceValue(std::string name, double value,
			      GValue::EPriority priority = kUser);
  static GValue* Get(std::string name="") { return FindValue(name); }
  static double Value(std::string);
  static TList* AllValues() {
    TList* output = new TList;
    output->SetOwner(false);
    for(auto& item : fValueVector){
      output->Add(item.second);
    }
    return output;
  }


  //Add value into static vector fValueVector
  static bool AddValue(GValue*, Option_t *opt="");

  bool AppendValue(GValue*);
  bool ReplaceValue(GValue*);

  //virtual void Clear(Option_t *opt="");
  virtual void Print(Option_t *opt="") const;
  virtual void Copy(TObject &obj) const;
  //virtual bool Notify();

  static int Size()  { return fValueVector.size(); }
  std::string PrintToString() const;
  static std::string  WriteToBuffer(Option_t *opt="");



private:
  double fValue;
  EPriority fPriority;
  std::string info;
  static GValue *fDefaultValue;
  static std::map<std::string,GValue*> fValueVector;
  static int  ParseInputData(const std::string input, EPriority priority,
			     Option_t *opt="");
  static void trim(std::string *, const std::string &trimChars=" \f\n\r\t\v");

  ClassDef(GValue,1);
};

#endif
