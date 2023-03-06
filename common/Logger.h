#ifndef __FASTCGRA_LOGGER__
#define __FASTCGRA_LOGGER__

#include "Common.h"

#define COUT    SimpleLogger::cout()
#define CLOG    SimpleLogger::clog()
#define CERR    SimpleLogger::cerr()
#define CNUL    StreamLogger("/dev/null")
#define CTMP    StreamLogger("./tmp/tmp.log")
#define FLOG(x) StreamLogger(x)

#ifndef NDEBUG
    #define ASSERT(x, y) \
        if(!(x)) \
        { \
            CERR << "Line " << __LINE__ << " of " << __FILE__ << ": \n -> Assertion of " << #x << " failed. " << "\n -> Info: " << (y); \
            exit(1); \
        } 
#else
    #define ASSERT(x, y)
#endif

#ifdef LOGLEVEL0
    #define LOGLEVEL 0
#endif

#ifdef LOGLEVEL1
    #define LOGLEVEL 1
#endif

#ifdef LOGLEVEL2
    #define LOGLEVEL 2
#endif

#if LOGLEVEL == 0
    #define PRINT  COUT
    #define NOTE   CLOG
    #define WARN   CERR
    #define ERROR  CERR
#elif LOGLEVEL == 1
    #define PRINT  COUT
    #define NOTE   NoneLogger::nolog
    #define WARN   CLOG
    #define ERROR  CERR
#elif LOGLEVEL == 2
    #define PRINT  COUT
    #define NOTE   NoneLogger::nolog
    #define WARN   NoneLogger::nolog
    #define ERROR  CERR
#else
    #define PRINT  COUT
    #define NOTE   CLOG
    #define WARN   CERR
    #define ERROR  CERR
#endif

namespace FastCGRA
{

class SimpleLogger
{
private: 
    std::ostream &_out; 

public: 
    SimpleLogger(): _out(std::clog) {}
    SimpleLogger(std::ostream &out): _out(out) {}
    SimpleLogger(const SimpleLogger &logger): _out(logger._out) {}
    ~SimpleLogger() {_out << std::endl; }
    
    template<typename Type> const SimpleLogger &operator << (const Type &datum) const {_out << datum; return *this; }
    
    static SimpleLogger cout() {return std::cout; }
    static SimpleLogger clog() {return std::clog; }
    static SimpleLogger cerr() {return std::cerr; }
    
}; 

class StreamLogger
{
private: 
    std::ostream *_out; 
    bool         _delete; 

public: 
    StreamLogger()                          : _out(&std::clog),              _delete(false)          {}
    StreamLogger(std::ostream &out)         : _out(&out),                    _delete(false)          {}
    StreamLogger(std::ostream *out)         : _out(out),                     _delete(false)          {}
    StreamLogger(const std::string &file)   : _out(new std::ofstream(file, std::ios::app)), _delete(true) {}
    StreamLogger(const StreamLogger &logger): _out(logger._out), _delete(logger._delete) {const_cast<StreamLogger&>(logger)._delete = false; }
    
    ~StreamLogger() {*_out << std::endl; if(_delete) {delete _out; }}
    
    template<typename Type> const StreamLogger &operator << (const Type &datum) const {*_out << datum; return *this; }
    
}; 

class NoneLogger
{
private: 

public: 
    NoneLogger()                         {}
    NoneLogger(std::ostream &out)        {}
    NoneLogger(std::ostream *out)        {}
    NoneLogger(const std::string &file)  {}
    NoneLogger(const NoneLogger &logger) {}
    
    template<typename Type> const NoneLogger &operator << (const Type &datum) const {return *this; }
    
    static NoneLogger nolog; 
    
}; 


}

#endif
