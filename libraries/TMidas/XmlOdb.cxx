//
// ALPHA ROOT analyzer
//
// Name: XmlOdb.cxx
// Author: K.Olchanski, 11-July-2006
//
// $Id: XmlOdb.cxx 91 2012-04-12 18:36:17Z olchansk $
//
// $Log: XmlOdb.cxx,v $
// Revision 1.1  2006/07/11 18:53:18  alpha
// KO- code to access to ODB from XML ODB dumps in MIDAS data files
//
//

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <fstream>

#include "XmlOdb.h"

#include <TList.h>

XmlOdb::XmlOdb(const char*xbuf,int bufLength) //ctor
{
	printf("xmlodb thingie running...\n");

	
  fOdb = NULL;
  fParser = new TDOMParser();
  fParser->SetValidate(false);

  char*buf = (char*)malloc(bufLength);
  memcpy(buf, xbuf, bufLength);
  for (int i=0; i<bufLength; i++)
    if (!isascii(buf[i]))
      buf[i] = 'X';
    else if (buf[i]=='\n')
      0;
    else if (buf[i]=='\r')
      0;
    else if (!isprint(buf[i]))
      buf[i] = 'X';
    else if (buf[i] == 0x1D)
      buf[i] = 'X';

  char* xend = strstr(buf,"odb>");
  if (xend)
    xend[4] = 0;

  //printf("end: %s\n", buf+bufLength-5);

//	std::ofstream out("goobers.xml");
//	out << xbuf;
//		out.close();
		
  fParser->ParseBuffer(buf,bufLength);
  
	
  TXMLDocument* doc = fParser->GetXMLDocument();
  if (!doc)
    {
      fprintf(stderr,"XmlOdb::XmlOdb: Malformed ODB dump: cannot get XML document\n");
      return;
    }

  fOdb = FindNode(doc->GetRootNode(),"odb");
  if (!fOdb)
    {
      fprintf(stderr,"XmlOdb::XmlOdb: Malformed ODB dump: cannot find <odb> tag\n");
      return;
    }
}

XmlOdb::XmlOdb(const char* filename) //ctor
{
  fOdb = NULL;

	std::ifstream ifile;
	ifile.open(filename);

	ifile.seekg(0, ifile.end);
	int bufLength = ifile.tellg();
	ifile.seekg(0,ifile.beg);

  char * xbuf = new char [bufLength];
  ifile.read (xbuf,bufLength);
  ifile.close();


  fOdb = NULL;
  fParser = new TDOMParser();
  fParser->SetValidate(false);

  //char*buf = (char*)malloc(bufLength);
  //memcpy(buf, xbuf, bufLength);
  for (int i=0; i<bufLength; i++)
    if (!isascii(xbuf[i]))
      xbuf[i] = 'X';
    else if (xbuf[i]=='\n')
      0;
    else if (xbuf[i]=='\r')
      0;
    else if (!isprint(xbuf[i]))
      xbuf[i] = 'X';
    else if (xbuf[i] == 0x1D)
      xbuf[i] = 'X';

  char* xend = strstr(xbuf,"odb>");
  if (xend)
    xend[4] = 0;

  //printf("end: %s\n", buf+bufLength-5);

//	std::ofstream out("goobers.xml");
//	out << xbuf;
//		out.close();
		
  fParser->ParseBuffer(xbuf,bufLength);
  
	
  TXMLDocument* doc = fParser->GetXMLDocument();
  if (!doc)
    {
      fprintf(stderr,"XmlOdb::XmlOdb: Malformed ODB dump: cannot get XML document\n");
      return;
    }

  fOdb = FindNode(doc->GetRootNode(),"odb");
  if (!fOdb)
    {
      fprintf(stderr,"XmlOdb::XmlOdb: Malformed ODB dump: cannot find <odb> tag\n");
      return;
    }

	delete[] xbuf;

/*
  fParser = new TDOMParser();
  fParser->SetValidate(false);

  int status = fParser->ParseFile(filename);
  if (status != 0)
    {
      fprintf(stderr,"XmlOdb::XmlOdb: Failed to parse XML file \'%s\', ParseFile() returned %d\n", filename, status);
      return;
    }

  TXMLDocument* doc = fParser->GetXMLDocument();
  if (!doc)
    {
      fprintf(stderr,"XmlOdb::XmlOdb: Malformed ODB dump: cannot get XML document\n");
      return;
    }

  fOdb = FindNode(doc->GetRootNode(),"odb");
  if (!fOdb)
    {
      fprintf(stderr,"XmlOdb::XmlOdb: Malformed ODB dump: cannot find <odb> tag\n");
      return;
    }
*/
}

XmlOdb::~XmlOdb() // dtor
{
	if(fParser)
	  delete fParser;
  fParser = NULL;
}

TXMLNode* XmlOdb::FindNode(TXMLNode*node, const char*name)
{
  for (; node != NULL; node = node->GetNextNode())
    {
      //printf("node name: \"%s\"\n",node->GetNodeName());
      if (strcmp(node->GetNodeName(),name) == 0)
	return node;
      
      if (node->HasChildren())
	{
	  TXMLNode* found = FindNode(node->GetChildren(),name);
	  if (found)
	    return found;
	}
    }
  
  return NULL;
}

void XmlOdb::DumpTree(TXMLNode*node,int level)
{
  if (!node)
    node = fOdb;

  if (!node)
    {
      fprintf(stderr,"XmlOdb::DumpTree: node is NULL!\n");
      return;
    }

  while (node)
    {
      for (int i=0; i<level; i++)
	printf(" ");
      printf("node name: \"%s\"\n",node->GetNodeName());
      TList* attrs = node->GetAttributes();
      TIter next(attrs);                           
      while (TXMLAttr *attr = (TXMLAttr*)next())                                
	{
	  for (int i=0; i<level; i++)
	    printf(" ");
	  printf("attribute name: \"%s\", value: \"%s\"\n",attr->GetName(),attr->GetValue());
	}
      const char*text = node->GetText();
      if (text)
	{
	  for (int i=0; i<level; i++)
	    printf(" ");
	  printf("node text: \"%s\"\n",node->GetText());
	}
      if (node->HasChildren())
	DumpTree(node->GetChildren(),level + 1);
      node = node->GetNextNode();
    }
  //printf("no more next nodes...\n");
}

void XmlOdb::DumpDirTree(TXMLNode*node,int level)
{
  if (!node)
    node = fOdb;

  if (!node)
    {
      fprintf(stderr,"XmlOdb::DumpDirTree: node is NULL!\n");
      return;
    }

  for (; node != NULL; node = node->GetNextNode())
    {
      const char* name = node->GetNodeName();
      
      if (strcmp(name,"dir") != 0)
	continue;
      
      for (int i=0; i<level; i++)
	printf(" ");
      printf("node name: \"%s\"\n",node->GetNodeName());
      TList* attrs = node->GetAttributes();
      TIter next(attrs);                           
      while (TXMLAttr *attr = (TXMLAttr*)next())                                
	{
	  for (int i=0; i<level; i++)
	    printf(" ");
	  printf("attribute name: \"%s\", value: \"%s\"\n",attr->GetName(),attr->GetValue());
	}
      if (node->HasChildren())
	DumpDirTree(node->GetChildren(),level + 1);
    }
  //printf("no more next nodes...\n");
}

//
// Return the value of the named attribute
//
const char* XmlOdb::GetAttrValue(TXMLNode*node,const char*attrName)
{
  TList* attrs = node->GetAttributes();
  TIter next(attrs);                           
  while (TXMLAttr *attr = (TXMLAttr*)next())                                
    {
      //printf("attribute name: \"%s\", value: \"%s\"\n",attr->GetName(),attr->GetValue());
      
      if (strcmp(attr->GetName(),attrName)==0)
	return attr->GetValue();
    }
  return NULL;
}

//
// Follow the ODB path through the XML DOM tree
//
TXMLNode* XmlOdb::FindPath(TXMLNode*node,const char* path)
{
  if (!fOdb)
    return NULL;

  if (!node)
    node = fOdb->GetChildren();
  
  while (1)
    {
      // skip leading slashes
      while (*path == '/')
        path++;
      
      if (*path == 0)
        return node;
      
      const int kElemSize = 256;
      char elem[kElemSize+1];
      memset(elem,0,kElemSize+1);
      
      // copy the next path element into "elem"-
      // copy "path" until we hit "/" or end of string
      for (int i=0; i<kElemSize; i++)
        {
          if (*path==0 || *path=='/')
            break;
          elem[i] = *path++;
        }
      
      //printf("looking for \"%s\" more \"%s\"\n",elem,path);
      
      for (; node != NULL; node = node->GetNextNode())
        {
          const char* nodename = node->GetNodeName();
          const char* namevalue = GetAttrValue(node,"name");
	  
          //printf("node name: \"%s\", \"name\" value: \"%s\"\n",node->GetNodeName(),namevalue);
	  
          bool isDir = strcmp(nodename,"dir") == 0;
          bool isKey = strcmp(nodename,"key") == 0;
          bool isKeyArray = strcmp(nodename,"keyarray") == 0;
	  
          if (!isKey && !isDir && !isKeyArray)
            continue;
	  
          //
          // compare directory names
          //
	  
          if (strcasecmp(elem,namevalue) == 0)
            {
              if (isDir)
                {
                  // found the right subdirectory, descend into it
                  node = node->GetChildren();
                  break;
                }
              else if (isKey || isKeyArray)
                {
                  return node;
                }
            }
        }
    }
}

//
// Follow the ODB path through the XML DOM tree
//
TXMLNode* XmlOdb::FindArrayPath(TXMLNode*node,const char* path,const char* type,int index)
{
  if (!fOdb)
    return NULL;

  if (!node)
    node = fOdb->GetChildren();

  node = FindPath(node, path);

  if (!node)
    return NULL;

  const char* nodename = node->GetNodeName();
  const char* num_values = GetAttrValue(node,"num_values");

  const char* typevalue = GetAttrValue(node,"type");

  if (!typevalue || (strcasecmp(typevalue,type) != 0))
    {
      fprintf(stderr,"XmlOdb::FindArrayPath: Type mismatch: \'%s\' has type \'%s\', we expected \'%s\'\n", path, typevalue, type);
      return NULL;
    }

  bool isKeyArray = (num_values!=NULL) && (strcmp(nodename,"keyarray")==0);

  if (!isKeyArray)
    {
      if (index != 0)
        {
          fprintf(stderr,"XmlOdb::FindArrayPath: Attempt to access array element %d, but \'%s\' is not an array\n", index, path);
          return NULL;
        }

      return node;
    }

  int max_index = atoi(num_values);

  if (index < 0 || index >= max_index)
    {
      fprintf(stderr,"XmlOdb::FindArrayPath: Attempt to access array element %d, but size of array \'%s\' is %d\n", index, path, max_index);
      return NULL;
    }

  //printf("nodename [%s]\n", nodename);

  TXMLNode* elem = node->GetChildren();

  for (int i=0; elem!=NULL; )
    {
      const char* name = elem->GetNodeName();
      const char* text = elem->GetText();
      //printf("index %d, name [%s] text [%s]\n", i, name, text);

      if (strcmp(name,"value") == 0)
        {
          if (i == index)
            return elem;
          i++;
        }

      elem = elem->GetNextNode();
    }
  

  return node;
}

int      XmlOdb::odbReadAny(   const char*name, int index, int tid,void* buf, int bufsize )
{ assert(!"Not implemented!"); }


unsigned char XmlOdb::odbReadByte(const char*name, int index, unsigned char defaultValue)
{
  TXMLNode *parent = NULL;
  TXMLNode *node = FindArrayPath(parent,name,"BYTE",index);
  if (!node)
    return defaultValue;
  const char* text = node->GetText();
  if (!text)
    return defaultValue;
  return text[0]-'0';//strtoul(text,NULL,0);
}


uint32_t XmlOdb::odbReadUint32(const char*name, int index, uint32_t defaultValue)
{
  TXMLNode *parent = NULL;
  TXMLNode *node = FindArrayPath(parent,name,"DWORD",index);
  if (!node)
    return defaultValue;
  const char* text = node->GetText();
  if (!text)
    return defaultValue;
  return strtoul(text,NULL,0);
}

double   XmlOdb::odbReadDouble(const char*name, int index, double defaultValue)
{
  TXMLNode *parent = NULL;
  TXMLNode *node = FindArrayPath(parent,name,"DOUBLE",index);
  if (!node)
    return defaultValue;
  const char* text = node->GetText();
  if (!text)
    return defaultValue;
  return atof(text);
}

float  XmlOdb::odbReadFloat(const char*name, int index, float defaultValue)
{
  TXMLNode *parent = NULL;
  TXMLNode *node = FindArrayPath(parent,name,"FLOAT",index);
  if (!node)
    return defaultValue;
  const char* text = node->GetText();
  if (!text)
    return defaultValue;
  return atof(text);
}

int      XmlOdb::odbReadInt(   const char*name, int index, int      defaultValue)
{
  TXMLNode *parent = NULL;
  TXMLNode *node = FindArrayPath(parent,name,"INT",index);
  if (!node)
    return defaultValue;
  const char* text = node->GetText();
  if (!text)
    return defaultValue;
  return atoi(text);
  //printf("for \'%s\', type is \'%s\', text is \'%s\'\n", name, typevalue, text);
  //DumpTree(node);
  //exit(1);
  return 0;
}

bool     XmlOdb::odbReadBool(  const char*name, int index, bool     defaultValue)
{
  TXMLNode *parent = NULL;
  TXMLNode *node = FindArrayPath(parent,name,"BOOL",index);
  if (!node)
    return defaultValue;
  const char* text = node->GetText();
  if (!text)
    return defaultValue;
  if (*text == 'n')
    return false;
  return true;
}

const char* XmlOdb::odbReadString(const char* name, int index, const char* defaultValue)
{
  TXMLNode *parent = NULL;
  TXMLNode *node = FindArrayPath(parent, name, "STRING", index);
  if (!node)
    return defaultValue;
  const char* text = node->GetText();
  if (!text)
    return defaultValue;
  return text;
}

int      XmlOdb::odbReadArraySize(const char*name)
{
  TXMLNode *parent = NULL;
  TXMLNode *node = FindPath(parent,name);
  if (!node)
    return 0;
  const char* num_values = GetAttrValue(node,"num_values");
  if (!num_values)
    return 1;
  return atoi(num_values);
}









//end
