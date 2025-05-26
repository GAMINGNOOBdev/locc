#ifndef __BARANIUM__LOGGING_H_
#define __BARANIUM__LOGGING_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

typedef unsigned char loglevel_t;

#define LOGLEVEL_INFO       (loglevel_t)0
#define LOGLEVEL_DEBUG      (loglevel_t)1
#define LOGLEVEL_ERROR      (loglevel_t)2
#define LOGLEVEL_WARNING    (loglevel_t)3

#define LOG logStr
#define LOGINFO(...) logStr(LOGLEVEL_INFO, stringf(__VA_ARGS__))
#define LOGDEBUG(...) logStr(LOGLEVEL_DEBUG, stringf(__VA_ARGS__))
#define LOGERROR(...) logStr(LOGLEVEL_ERROR, stringf(__VA_ARGS__))
#define LOGWARNING(...) logStr(LOGLEVEL_WARNING, stringf(__VA_ARGS__))

/**
 * @brief Like printf but for building a string together
 * 
 * @param[in] formatString string which has format information
 * @param[in] ... any other arguments
 * 
 * @returns the new formatted string
*/
const char* stringf(const char* formatString, ...);

/**
 * @brief En-/Disable debug messages showing up
 * 
 * @note By default debug messages are off
 * 
 * @param val "Boolean" value, 1 = debug messages show up, 0 = no debug messages, -1 = no change
 *
 * @returns The current log state
 */
uint8_t logEnableDebugMsgs(uint8_t val);

/**
 * @brief En-/Disable messages showing up in stdout
 * 
 * @note By default stdout messages are on
 * 
 * @param val "Boolean" value, 1 = messages show up on stdout, 0 = they do not show up on stdout
 */
void logEnableStdout(uint8_t val);

/**
 * @brief Set an output stream for log messages
 * 
 * @param stream The output stream to which will be written, NULL to disable logging
 */
void logSetStream(FILE* stream);

/**
 * @brief Log a message onto the cmd line
 * 
 * @note Logging it turned off by default, set the output stream first before logging messages
 * 
 * @param lvl Logging level
 * @param msg Log message
 */
void logStr(loglevel_t lvl, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
