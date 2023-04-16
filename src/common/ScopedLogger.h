
//==============================================================================
//
//   ScopedLogger.h
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#if !defined(__SCOPPED_LOGGER_H__)
#define __SCOPPED_LOGGER_H__

#define SCOPPEDLOGGER_ID_LEN 128

class ScoppedLogger
{
public:
	ScoppedLogger(const char* pId);
	~ScoppedLogger();
	operator bool()
	{
		return true;
	}
	
private:
	char sId[SCOPPEDLOGGER_ID_LEN];
};
#ifdef _DEBUG
#define	CSBLOCK(x)	ScoppedLogger slogger(x)
#else
#define	CSBLOCK(x)	
#endif
#endif // !defined(__SCOPPED_LOGGER_H__)
