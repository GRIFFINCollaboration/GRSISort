#ifdef HAS_XML

#include <stdexcept>

#include "TXMLOdb.h"

#include "TList.h"
#include "TXMLAttr.h"

/// \cond CLASSIMP
ClassImp(TXMLOdb)
/// \endcond

char TXMLOdb::fTextBuffer[256];

TXMLOdb::TXMLOdb(char* buffer, int size)
{
   fOdb = nullptr;
   fDoc = nullptr;
   std::ifstream input;
   input.open(buffer);
   fParser = new TDOMParser;
   fParser->SetValidate(false);
   if(input.is_open()) {
      fParser->ParseFile(buffer);
   } else {
      // printf("0x%08x\t%i\n",buffer,size);
      fParser->ParseBuffer(buffer, size);
   }
   fDoc = fParser->GetXMLDocument();
   if(fDoc == nullptr) {
      fprintf(stderr, "XmlOdb::XmlOdb: Malformed ODB dump: cannot get XML document\n");
      return;
   }
   fOdb = fDoc->GetRootNode();
   if(strcmp(fOdb->GetNodeName(), "odb") != 0) {
      fprintf(stderr, "XmlOdb::XmlOdb: Malformed ODB dump: cannot find <odb> tag\n");
      return;
   }
}

TXMLOdb::~TXMLOdb()
{
   if(fParser != nullptr) {
      delete fParser;
   }
}

TXMLNode* TXMLOdb::FindNode(const char* name, TXMLNode* node)
{
   if(node == nullptr) {
      if(fOdb == nullptr) {
         return nullptr;
      }
      node = fOdb; //->GetChildren();
   }
   if(!node->HasChildren()) {
      return nullptr;
   }
   node = node->GetChildren();
   while(node != nullptr) {
      std::string nodename = GetNodeName(node);
      if(nodename.compare(name) == 0) {
         return node;
      }
      node = node->GetNextNode();
   }

   return nullptr;
}

TXMLNode* TXMLOdb::FindPath(const char* path, TXMLNode* node)
{
   if(node == nullptr) {
      if(fOdb == nullptr) {
         return nullptr;
      }
      node = fOdb; //->GetChildren();
   }
   // if(!node->HasChildren())
   //   return 0;
   // node = node->GetChildren();

   std::string              pathname = path;
   std::vector<std::string> elems;
   std::size_t              last  = 0;
   std::size_t              slash = pathname.find_first_of('/');
   if(slash == 0) {
      last = 1;
   }
   while(true) {
      slash = pathname.find_first_of('/', last);
      elems.push_back(pathname.substr(last, slash - last));
      //      printf("last = %i\tslash = %i\n",last,slash);
      last = slash + 1;
      if(slash == std::string::npos) {
         break;
      }
   }

   for(auto& elem : elems) {
      node = FindNode(elem.c_str(), node);
      if(node != nullptr) {
         //         printf("elem[%i]\t= %s\tnode = %s\n",x,elems.at(x).c_str(),GetNodeName(node));
      } else {
         node = nullptr;
         break;
      }
   }

   /*
      if(firstslash != std::string::npos) {
         elem = pathname.substr(0,firstslash-1);
         firstslash = pathname.find_first_of('/',firstslash+1);

         pathname = pathname.substr(firstslash);
      }

      printf("pathname = %s\n",pathname.c_str());
      printf("elem     = %s\n",pathname.c_str());


      node = FindNode(elem.c_str(),node);
      if(pathname.length()!=0)
         FindPath(pathname.c_str(),node);
   */
   return node;
}

const char* TXMLOdb::GetNodeName(TXMLNode* node)
{
   // std::string TXMLOdb::GetNodeName(TXMLNode* node) {
   TList* list = node->GetAttributes();
   if(list != nullptr) {
      std::string buffer = (static_cast<TXMLAttr*>(list->At(0)))->GetValue();
      // list->Delete();
      strlcpy(fTextBuffer, buffer.c_str(), sizeof(fTextBuffer));
      return ((const char*)fTextBuffer); // buffer.c_str();
   }
   return "";
}

int TXMLOdb::ReadInt(const char* path, int, int defaultValue)
{
   TXMLNode* node = FindPath(path);
   if(node == nullptr) {
      return defaultValue;
   }
   return 0;
}

std::vector<int> TXMLOdb::ReadIntArray(TXMLNode* node)
{
   std::vector<int> temp;
   if(node == nullptr) {
      return temp;
   }
   if(!node->HasChildren()) {
      return temp;
   }
   TList* list = node->GetAttributes();
   if(list == nullptr) {
      return temp;
   }
   TIter iter(list);
   int   size = 0;
   while(TXMLAttr* attr = static_cast<TXMLAttr*>(iter.Next())) {
      if(strcmp(attr->GetName(), "num_values") == 0) {
         size = atoi(attr->GetValue());
      }
   }
   //   printf("size = %i\n",size);
   temp.assign(size, 0);
   TXMLNode* child   = node->GetChildren();
   int       counter = 0;
   while(true) {
      if(TList* index = child->GetAttributes()) {
         // printf("index = %i\n",atoi(((TXMLAttr*)(index->At(0)))->GetValue()));
         // printf("value = %s\t%i\n",child->GetText(),atoi(child->GetText()));
         int indexnum = atoi((static_cast<TXMLAttr*>(index->At(0)))->GetValue());
         int value    = atoi(child->GetText());
         //         printf("indexnum %i : value 0x%08x\n",indexnum,value);
         temp.at(indexnum) = value;
      } else if(child->GetText() != nullptr) {
         int indexnum = counter++;
         // printf("%i/%i\n",counter,size);
         temp.at(indexnum) = atoi(child->GetText());
         // printf("text: %s   int: %i\n",child->GetText(),temp.at(indexnum));
      }
      child = child->GetNextNode();
      if(child == nullptr) {
         break;
      }
   }
   return temp;
}

std::vector<std::string> TXMLOdb::ReadStringArray(TXMLNode* node)
{
   std::vector<std::string> temp;
   if(node == nullptr) {
      return temp;
   }
   if(!node->HasChildren()) {
      return temp;
   }
   TList* list = node->GetAttributes();
   if(list == nullptr) {
      return temp;
   }
   TIter iter(list);
   int   size = 0;
   while(TXMLAttr* attr = static_cast<TXMLAttr*>(iter.Next())) {
      if(strcmp(attr->GetName(), "num_values") == 0) {
         size = atoi(attr->GetValue());
      }
   }
   //   printf("size = %i\n",size);
   temp.assign(size, "");
   TXMLNode* child   = node->GetChildren();
   int       counter = 0;
   while(true) {
      //     printf("here\n");
      if(TList* index = child->GetAttributes()) {
         // printf("index = %i\n",atoi(((TXMLAttr*)(index->At(0)))->GetValue()));
         // printf("value = %s\t%i\n",child->GetText(),atoi(child->GetText()));
         int         indexnum = atoi((static_cast<TXMLAttr*>(index->At(0)))->GetValue());
         const char* value;
         value = child->GetText();
         // printf("indexnum %i : value 0x%08x\n",indexnum,value.c_str());

         // Make sure we actually read a word
         std::string value_str;
         if(value == nullptr) {
            value_str = "";
         } else {
            value_str = value;
         }
         temp.at(indexnum) = value_str;
      } else if(child->GetText() != nullptr) {
         int indexnum = counter++;
         // printf("%i/%i\n",counter,size);
         temp.at(indexnum).assign(child->GetText());
      }
      child = child->GetNextNode();
      if(child == nullptr) {
         break;
      }
   }
   return temp;
}

std::vector<double> TXMLOdb::ReadDoubleArray(TXMLNode* node)
{
   std::vector<double> temp;
   if(node == nullptr) {
      return temp;
   }
   if(!node->HasChildren()) {
      return temp;
   }
   TList* list = node->GetAttributes();
   if(list == nullptr) {
      return temp;
   }
   TIter iter(list);
   int   size = 0;
   while(TXMLAttr* attr = static_cast<TXMLAttr*>(iter.Next())) {
      if(strcmp(attr->GetName(), "num_values") == 0) {
         size = atoi(attr->GetValue());
      }
   }
   //   printf("size = %i\n",size);
   temp.assign(size, 0.0);
   TXMLNode* child   = node->GetChildren();
   int       counter = 0;
   while(true) {
      //      printf("here\n");
      if(TList* index = child->GetAttributes()) {
         // printf("index = %i\n",atoi(((TXMLAttr*)(index->At(0)))->GetValue()));
         // printf("value = %s\t%i\n",child->GetText(),atoi(child->GetText()));
         int    indexnum = atoi((static_cast<TXMLAttr*>(index->At(0)))->GetValue());
         double value    = atof(child->GetText());
         //         printf("indexnum %i : value 0x%08x\n",indexnum,value);
         temp.at(indexnum) = value;
      } else if(child->GetText() != nullptr) {
         int indexnum      = counter++;
         temp.at(indexnum) = atof(child->GetText());
      }
      child = child->GetNextNode();
      if(child == nullptr) {
         break;
      }
   }
   return temp;
}
#endif
