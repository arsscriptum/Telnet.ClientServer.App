
//==============================================================================
//
//   myexception.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================



#ifndef __MY_EXCEPTION_H__
#define __MY_EXCEPTION_H__

// using standard exceptions
#include <iostream>
#include <exception>
using namespace std;

class myexception: public exception
{
  virtual const char* what() const throw()
  {
    return "My exception happened";
  }
} myex;


#endif //__MY_EXCEPTION_H__

