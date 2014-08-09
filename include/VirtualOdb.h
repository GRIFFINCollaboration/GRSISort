//
// Virtual ODB access methods: online or offline from XML file
//
// Name: VirtualOdb.h
//
// $Id: VirtualOdb.h 91 2012-04-12 18:36:17Z olchansk $
//
//



#ifndef INCLUDE_VirtualOdb_H
#define INCLUDE_VirtualOdb_H

/// Interface class for ODB access
#include <stdint.h>

class TXMLNode;

class VirtualOdb
{
  public:
  // ODB functions

  /// Read size of an array
  virtual int      odbReadArraySize(const char*name) = 0;
  /// Read value of arbitrary type
  virtual int      odbReadAny(   const char*name, int index, int tid,void* buf, int bufsize = 0) = 0;
  /// Read an integer value, midas type TID_INT

  virtual unsigned char  odbReadByte(   const char*name, int index = 0, unsigned char defaultValue = 0) = 0;
  /// Read an unsigned char value, midas type TID_BYTE

  virtual int      odbReadInt(   const char*name, int index = 0, int      defaultValue = 0) = 0;
  /// Read an unsigned 32-bit integer value, midas type TID_DWORD
  virtual uint32_t odbReadUint32(const char*name, int index = 0, uint32_t defaultValue = 0) = 0;
  /// Read an 32-bit floating point value, midas type TID_FLOAT
  virtual float   odbReadFloat(const char*name, int index = 0, float   defaultValue = 0) = 0;
  /// Read an 64-bit floating point value, midas type TID_DOUBLET
  virtual double   odbReadDouble(const char*name, int index = 0, double   defaultValue = 0) = 0;
  /// Read a boolean value, midas type TID_BOOL
  virtual bool     odbReadBool(  const char*name, int index = 0, bool     defaultValue = false) = 0;
  /// Read a string value, midas type TID_STRING
  virtual const char* odbReadString(const char*name, int index = 0,const char* defaultValue = NULL) = 0;
  /// Destructor has to be virtual
  virtual ~VirtualOdb() { /* empty */ }; // dtor
};

#endif
// end

