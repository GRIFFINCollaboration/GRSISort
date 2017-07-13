#ifndef TXMLODB_H
#define TXMLODB_H

/** \addtogroup Sorting
 *  @{
 */

// Access ODB info from an XML ODB dump at either the begining of a run
// or from a seperate file.
//
// Name: TXMLOdb.h
// Author: P.C. Bender
//
//
//

#include <cstdlib>
#include <cstdio>

#include <fstream>
#include <string>
#include <vector>

#include "Globals.h"

#ifdef HAS_XML
#include "TXMLNode.h"
#include "TXMLDocument.h"
#include "TDOMParser.h"

class TXMLOdb {
public:
   TXMLOdb(char* buffer, int size = 0);
   virtual ~TXMLOdb();

   TXMLDocument* fDoc;
   TDOMParser*   fParser;
   TXMLNode*     fOdb;

   const char* GetNodeName(TXMLNode*);
   TXMLNode* FindNode(const char* name, TXMLNode* node = nullptr);
   TXMLNode* FindPath(const char* path, TXMLNode* node = nullptr);

   int ReadInt(const char* path, int index = 0, int defaultValue = 0xffffffff);
   std::vector<int> ReadIntArray(TXMLNode* node);
   std::vector<double> ReadDoubleArray(TXMLNode* node);
   std::vector<std::string> ReadStringArray(TXMLNode* node);

private:
   static char fTextBuffer[256];

   /// \cond CLASSIMP
   ClassDef(TXMLOdb, 0)
   /// \endcond
};
#endif
/*! @} */
#endif
