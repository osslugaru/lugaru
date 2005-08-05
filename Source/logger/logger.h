// ---------------------------------------------------------------------------------------------------------------------------------
//  _                                 _     
// | |                               | |    
// | | ___   __ _  __ _  ___ _ __    | |__  
// | |/ _ \ / _` |/ _` |/ _ \ '__|   | '_ \ 
// | | (_) | (_| | (_| |  __/ |    _ | | | |
// |_|\___/ \__, |\__, |\___|_|   (_)|_| |_|
//           __/ | __/ |                    
//          |___/ |___/                     
//
// Generic informational logging class
//
// ---------------------------------------------------------------------------------------------------------------------------------
//
// Restrictions & freedoms pertaining to usage and redistribution of this software:
//
//  * This software is 100% free
//  * If you use this software (in part or in whole) you must credit the author.
//  * This software may not be re-distributed (in part or in whole) in a modified
//    form without clear documentation on how to obtain a copy of the original work.
//  * You may not use this software to directly or indirectly cause harm to others.
//  * This software is provided as-is and without warrantee. Use at your own risk.
//
// For more information, visit HTTP://www.FluidStudios.com
//
// ---------------------------------------------------------------------------------------------------------------------------------
// Originally created on 07/06/2000 by Paul Nettle
//
// Copyright 2000, Fluid Studios, Inc., all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------

#ifndef	_H_LOGGER
#define _H_LOGGER

// ---------------------------------------------------------------------------------------------------------------------------------
// The global logger
// ---------------------------------------------------------------------------------------------------------------------------------

class	Logger;
extern	Logger	logger;

// ---------------------------------------------------------------------------------------------------------------------------------
// Macros (necessary evil to take advantage of __LINE__ and __FILE__)
// ---------------------------------------------------------------------------------------------------------------------------------

#define	LOG		logger.sourceLine() = __LINE__, logger.sourceFile() = __FILE__,logger.logTex
#define	HEX		logger.sourceLine() = __LINE__, logger.sourceFile() = __FILE__,logger.logHex
#define	RAW		logger.sourceLine() = __LINE__, logger.sourceFile() = __FILE__,logger.logRaw
#define	INDENT		logger.sourceLine() = __LINE__, logger.sourceFile() = __FILE__,logger.indent
#define	UNDENT		logger.sourceLine() = __LINE__, logger.sourceFile() = __FILE__,logger.undent
#define	LOGBLOCK	logger.sourceLine() = __LINE__, logger.sourceFile() = __FILE__;LogBlock __lb__

// If you compiler supports __FUNCTION__, then replace the "#if 1" with "#if 0". Note that this will change the usage of the macro

#if 0
#define	LOGFUNC		logger.sourceLine() = __LINE__, logger.sourceFile() = __FILE__;LogFlow __lf__
#else
#define	LOGFUNC		logger.sourceLine() = __LINE__, logger.sourceFile() = __FILE__;LogFlow __lf__(__FUNCTION__)
#endif

// ---------------------------------------------------------------------------------------------------------------------------------
// The logger class: does the actual logging
// ---------------------------------------------------------------------------------------------------------------------------------

class	Logger
{
public:
	// Enumerations

	enum	LogFlags
	{
				LOG_INDENT = 0x00000001,
				LOG_UNDENT = 0x00000002,
				LOG_FLOW   = 0x00000004,
				LOG_BLOK   = 0x00000008,
				LOG_DATA   = 0x00000010,
				LOG_INFO   = 0x00000012,
				LOG_WARN   = 0x00000014,
				LOG_ERR    = 0x00000018,
				LOG_CRIT   = 0x00000020,
				LOG_ALL    = 0xFFFFFFFF
	};

	// Construction/Destruction

inline				Logger()
				: _sourceLine(0), _indentCount(0), _indentChars(4), _fileSizeLimit(-1), _logMask(LOG_ALL),
				  _logStarted(false), _lineCharsFlag(false), _logFile("logger.log")
				{
				}

virtual				~Logger()
				{
					stop();
				}

	// Operators

inline		void		operator +=(const string &s)	{logTex(s);}

	// Accessors

inline	const	bool		&lineCharsFlag() const	{return _lineCharsFlag;}
inline		bool		&lineCharsFlag() 	{return _lineCharsFlag;}

inline	const	int		&fileSizeLimit() const	{return _fileSizeLimit;}
inline		int		&fileSizeLimit()	{return _fileSizeLimit;}

inline	const	unsigned int	&logMask() const	{return _logMask;}
inline		unsigned int	&logMask()		{return _logMask;}

inline	const	string		&logFile() const	{return _logFile;}
inline		string		&logFile()		{return _logFile;}

inline	const	unsigned int	&sourceLine() const	{return _sourceLine;}
inline		unsigned int	&sourceLine()		{return _sourceLine;}

inline	const	string		&sourceFile() const	{return _sourceFile;}
inline		string		&sourceFile()		{return _sourceFile;}

inline		bool		logStarted() const	{return _logStarted;}

	// Utilitarian (public)

virtual		void		start(const bool reset);
virtual		void		stop();
virtual		void		logTex(const string &s, const LogFlags logBits = LOG_INFO);
virtual		void		logRaw(const string &s);
virtual		void		logHex(const char *buffer, const unsigned int count, const LogFlags logBits = LOG_INFO);
virtual		void		indent(const string &s, const LogFlags logBits = LOG_INDENT);
virtual		void		undent(const string &s, const LogFlags logBits = LOG_UNDENT);

private:
	// Utilitarian (private)

virtual		void		limitFileSize() const;
virtual	const	string		&headerString(const LogFlags logBits) const;

	// Data

		string		_logFile;
		string		_sourceFile;
		unsigned int	_sourceLine;
		int		_indentCount;
		int		_indentChars;
		int		_fileSizeLimit;
		unsigned int	_logMask;
		bool		_logStarted;
		bool		_lineCharsFlag;
};

// ---------------------------------------------------------------------------------------------------------------------------------
// The LogBlock class: used for automatic indentation
// ---------------------------------------------------------------------------------------------------------------------------------

class	LogBlock
{
public:
inline				LogBlock(const string &s)	{str = s;logger.indent("Begin block: " + str, Logger::LOG_INDENT);}
inline				~LogBlock()			{logger.undent("", Logger::LOG_UNDENT);}
private:
		string		str;
};

// ---------------------------------------------------------------------------------------------------------------------------------
// The LogFlow class: used for logging code flow
// ---------------------------------------------------------------------------------------------------------------------------------

class	LogFlow
{
public:
inline				LogFlow(const string &function)	{str = function;logger.indent(str, Logger::LOG_FLOW);}
inline				~LogFlow()			{logger.undent("", Logger::LOG_FLOW);}
private:
		string		str;
};

#endif // _H_LOGGER
// ---------------------------------------------------------------------------------------------------------------------------------
// logger.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------

