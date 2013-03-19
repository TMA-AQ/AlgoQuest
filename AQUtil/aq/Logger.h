#ifndef __LOG_H__
#define __LOG_H__

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>

#if defined(__FreeBSD__)
# include <syslog.h>
# include <pthread.h>
#endif

#define LOG_FILE  -1

#define SYSLOG		0x01
#define LOCALFILE	0x02
#define STDOUT		0x04

#define LOGBUFFER 8192

namespace aq
{

  class Logger
  {
  public:

    static Logger& getInstance();
    static Logger& getInstance(const char * ident, int mode);

    ~Logger();

		void setLocalFile(const char *);

    void setLockMode(bool value)      { this->lockMode = value; }
    void setColorMode(bool value)     { this->colorLog = value; }
    void setDateMode(bool value)      { this->dateMode = value; }
    void setPidMode(bool value)       { this->pidMode = value; }
    void setMode(char _mode)          { this->mode = _mode; }
    void setLevel(int _level)         { this->level = _level; }
    void setIdent(const char *_ident) { 
      ::memset(this->ident, 0, sizeof(ident)); 
      ::strncpy_s(this->ident, _ident, sizeof(this->ident)); 
    }

    char getMode() const { return this->mode; }
    const char * getIdent() const { return this->ident; }

    void log(const char *file, const char *function, unsigned int line, int facility, const char * forma, ...) const;

  private:  
    Logger(const char * ident, int mode);
    Logger(const Logger& );
    Logger& operator=(const Logger& source);

    void printStdOut(const char * but, int facility) const;
    void printWithColor(const char * buf, int facility) const;
    int  openFile(const char *);
    void closeFile(void);

    int level;
    FILE * localFile;
    char mode;
    char ident[64];
    char statsIdent[64];
    char httpIdent[64];

    mutable boost::mutex mutex;

    bool colorLog;
    bool lockMode;
    bool dateMode;
    bool pidMode;
  };

}

#ifndef __FreeBSD__

enum log_facilities {
  LOG_CRIT = 2,
  LOG_ERR,
  LOG_WARNING,
  LOG_NOTICE,
  LOG_INFO,
  LOG_DEBUG,
};

#endif

#define AQ_CRITICAL __FILE__, __FUNCTION__, __LINE__, LOG_CRIT
#define AQ_ERROR __FILE__, __FUNCTION__, __LINE__, LOG_ERR
#define AQ_WARNING __FILE__, __FUNCTION__, __LINE__, LOG_WARNING
#define AQ_NOTICE __FILE__, __FUNCTION__, __LINE__, LOG_NOTICE
#define AQ_INFO __FILE__, __FUNCTION__, __LINE__, LOG_INFO
#define AQ_DEBUG __FILE__, __FUNCTION__, __LINE__, LOG_DEBUG

#ifdef _DEBUG
# define AQ_LOG_DEBUG(args) \
  do {                   \
    aq::Logger::getInstance().log(AQ_DEBUG, args);                \
  } while (false) ;
#else
# define AQ_LOG_DEBUG(inst)
#endif

#endif
