#ifndef TXMLODB_H
#define TXMLODB_H

// Access ODB info from an XML ODB dump at either the begining of a run
// or from a seperate file.
// 
// Name: TXMLOdb.h
// Author: P.C. Bender
// 
//  
// 

#include "Globals.h"
#include<cstdlib>
#include<cstdio>
#include<stdint.h>

#include <fstream>
#include <string>
#include <vector>

#include "TXMLNode.h"
#include "TXMLDocument.h"
#include "TDOMParser.h"


//#include "VirtualOdb"

class TXMLOdb { //: public TDOMParser { //, public VirtualOdb {

   public:
      TXMLOdb(char *buffer,int size=0);
      virtual ~TXMLOdb();
   
      TXMLDocument *fDoc;
      TDOMParser *fParser;
      TXMLNode *fOdb;

      const char *GetNodeName(TXMLNode*);
      TXMLNode* FindNode(const char *name, TXMLNode *node=0);
//      void DumpTree(TXMLNode*node = NULL, int level = 0);
//      void DumpDirTree(TXMLNode*node = NULL, int level = 0);
//      const char* GetAttrValue(TXMLNode*node,const char*attrName);
      TXMLNode* FindPath(const char* path,TXMLNode*node=0);

//      TXMLNode* FindArrayPath(TXMLNode*node,const char* path,const char* type,int index);

      int ReadInt(const char *path, int index=0, int defaultValue=0xffffffff);
      std::vector<int> ReadIntArray(TXMLNode *node); 
      std::vector<double> ReadDoubleArray(TXMLNode *node); 
      std::vector<std::string> ReadStringArray(TXMLNode *node); 

//      int      odbReadAny(   const char*name, int index, int tid,void* buf, int bufsize=0 );
//      uint32_t odbReadUint32(const char*name, int index, uint32_t defaultValue );
//      unsigned char odbReadByte(const char*name, int index, unsigned char defaultValue );
//      bool     odbReadBool(  const char*name, int index, bool     defaultValue );
//      double   odbReadDouble(const char*name, int index, double   defaultValue );
//      float    odbReadFloat(const char*name, int index, float  defaultValue );
//      const char* odbReadString(const char*name, int index, const char* defaultValue );
//      int      odbReadArraySize(const char*name);

   ClassDef(TXMLOdb,0)

};

#endif
