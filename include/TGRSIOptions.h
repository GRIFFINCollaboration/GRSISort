#ifndef TGRSIOPTIONS_H
#define TGRSIOPTIONS_H

#include <cstdio>
#include <string>

#include <TObject.h>

namespace TGRSIOptions {

      //TGRSIOptions();
   
     // virtual ~TGRSIOptions();

      extern std::string fhostname;
      extern std::string fexptname;
   
      extern std::vector<std::string> fInputRootFile;
      extern std::vector<std::string> fInputMidasFile;
      extern std::vector<std::string> fInputCalFile;
      extern std::vector<std::string> fInputOdbFile;

		extern bool fCloseAfterSort;
	        extern bool fLogErrors;
		extern bool fUseMidFileOdb;
		extern bool fMakeAnalysisTree;
                extern bool fProgressDialog;
      std::string GetHostName();
      std::string GetExptName();
     
      std::vector<std::string> GetInputRoot();  
      std::vector<std::string> GetInputMidas(); 
      std::vector<std::string> GetInputCal();   
      std::vector<std::string> GetInputOdb();   

		const char *GetXMLODBFile(int runnumber=0,int subrunnumber=-1);
		 const char *GetCalFile(int runnumber=0,int subrunnumber=-1);


		 void SetCloseAfterSort(bool flag=true); 
		 bool CloseAfterSort();                  

		 void SetLogErrors(bool flag=true);      
		 bool LogErrors();			
	
                 void SetProgressDialog(bool flag=true); 
                 bool ProgressDialog();                  
	
		 void SetUseMidFileOdb(bool flag=true);  
		 bool UseMidFileOdb();                   

		 void SetMakeAnalysisTree(bool flag=true);
		 bool MakeAnalysisTree();                

       void SetHostName(std::string &host);
       void SetExptName(std::string &expt); 

       void AddInputRootFile(std::string &input);  
       void AddInputMidasFile(std::string &input); 
       void AddInputCalFile(std::string &input);   
       void AddInputOdbFile(std::string &input);   


       void Print(Option_t *opt = "");
       void Clear(Option_t *opt = "");

 


}

#endif


