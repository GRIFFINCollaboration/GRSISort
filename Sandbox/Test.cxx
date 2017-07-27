// g++ Test.cxx -I${GRSISYS}/include  -L${GRSISYS}/libraries -lNucleus `root-config --cflags --libs`

#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

#include <TObject.h>

#include <TNucleus.h>

class TGRSISourceInfo : public TObject {
public:
   TGRSISourceInfo() {}
   TGRSISourceInfo(const char* name);
   virtual ~TGRSISourceInfo() {}

   bool SetNext()
   {
      if(iter == fGTransition.end()) {
         Reset();
         return false;
      }
      energy    = iter->first;
      intensity = iter->second;
      iter++;
      return true;
   }
   double GetEnergy() { return energy; }
   double GetIntensity() { return intensity; }

   void Reset() { iter = fGTransition.begin(); }

private:
   std::string fName;
   double      energy;
   double      intensity;
   typedef std::map<Double_t, Double_t> TGRSIGammaTransition;
   TGRSIGammaTransition fGTransition;

   TGRSIGammaTransition::iterator iter;

   void        SetSourceInfo(ifstream*);
   static void trim(std::string*, const std::string& trimChars = " \f\n\r\t\v");

   // ClassDef(TGRSISourceInfo,1)
};

TGRSISourceInfo::TGRSISourceInfo(const char* name)
{
   TNucleus    nuc(name);
   std::string sourcename = nuc.GetName();
   sourcename += ".source";
   ifstream sourcefile;
   sourcefile.open(sourcename.c_str());
   if(sourcefile.is_open()) {
      SetSourceInfo(&sourcefile);
   } else {
      printf("Cannot find source info for: %s\n", sourcename.c_str());
   }
}

void TGRSISourceInfo::SetSourceInfo(ifstream* infile)
{
   std::string line;
   int         linenumber = 0;
   double      eng, inten;
   while(getline(*infile, line)) {
      linenumber++;
      trim(&line);
      int comment = line.find("//");
      if(comment != std::string::npos) {
         line = line.substr(0, comment);
      }
      if(line.length() == 0) continue;
      std::stringstream ss(line);
      ss >> eng;
      ss >> inten;
      printf("eng: %.02f\tinten: %.02f\n", eng, inten);
   }
}

void TGRSISourceInfo::trim(std::string* line, const std::string& trimChars)
{
   // Removes the the string "trimCars" from  the string 'line'
   if(line->length() == 0) return;
   std::size_t found                    = line->find_first_not_of(trimChars);
   if(found != std::string::npos) *line = line->substr(found, line->length());
   found                                = line->find_last_not_of(trimChars);
   if(found != std::string::npos) *line = line->substr(0, found + 1);
   return;
}

int main(int argc, char** argv)
{
   if(argc != 2) {
      printf("wrong number of commands.\n");
      return 1;
   }
   printf("argv[1] = %s\n", argv[1]);
   delete(new TGRSISourceInfo(argv[1]));

   return 0;
}
