/*
// updated by ...: Loreto Notarantonio
// Date .........: 19-02-2026 15.59.49
*/


// >>>lnLogger_Class.h
#pragma once

#include <Arduino.h>
#include <freertos/semphr.h> // Required for FreeRTOS types (SemaphoreHandle_t)

#ifndef fstripHeaderTrue
    #define fMilliSecondsFalse 0
    #define fMilliSecondsTrue  1

    #define fstripHoursTrue  1
    #define fstripHoursFalse  0
#endif


// #########################################################################
// #     Logger CLASS     Logger CLASS     Logger CLASS     Logger CLASS
// #########################################################################

// Class for ESP32 logger
class ESP32Logger { // Renamed from ESP32LoggerMutex for simplicity and clarity
    public:
        ESP32Logger(void);
        void init(const uint8_t line_buffer_len=128, const uint8_t filename_buffer_len=20);
        void write(const char* color, const char* tag, const char* file, const char* function, int line, const char* format, ...);

    private:
        char sharedTimeBUFFER[16];
        bool fIncludeFunction=false; // log -> filename.function:linno
        uint8_t m_LINE_BUFFER_LENGTH; // spazio dedicato tutta la riga di log
        uint8_t m_FILENAME_BUFFER_LENGTH; // spazio dedicato a -> [filename.function:linno]

        bool m_mutexInitialized = false;
        SemaphoreHandle_t m_logMutex = NULL; // The mutex to protect log operations
        const char* getFileLineInfo(char *outBUFFER, const uint16_t OutBUFFER_maxLen, const char* file, const char* function, int line);
        void getNowTime(char* buffer, size_t len);
        // const char* msecToHMS(char *buffer, uint8_t buffer_len, uint32_t millisec, bool addMilliSec=false, bool stripHeader=false);

}; // class ESP32Logger


extern ESP32Logger lnLog; // defined in lnLogger_Class.cpp
/* initializazione del logger
    Serial.begin(115200);
    delay(2000);
    lnLog.init();
*/

// #########################################################################
// #     Logger MACROS      Logger MACROS      Logger MACROS      Logger MACROS
// #########################################################################

    #define VALUE_TO_STRING(x) #x
    #define VALUE(x) VALUE_TO_STRING(x)
    #define VAR_NAME_VALUE(var) #var "="  VALUE(var)

    // ANSI Color Definitions
    namespace LogColors {
        const char* const RESET   = "\x1B[0m";

        const char* const RED     = "\x1B[0;31m"; const char* const REDH    = "\x1B[1;31m";
        const char* const GREEN   = "\x1B[0;32m"; const char* const GREENH  = "\x1B[1;32m";
        const char* const YELLOW  = "\x1B[0;33m"; const char* const YELLOWH = "\x1B[1;33m";
        const char* const BLUE    = "\x1B[0;34m"; const char* const BLUEH   = "\x1B[1;34m";
        const char* const PURPLE  = "\x1B[0;35m"; const char* const PURPLEH = "\x1B[1;35m";
        const char* const CYAN    = "\x1B[0;36m"; const char* const CYANH   = "\x1B[1;36m";
        const char* const GRAY    = "\x1B[0;37m"; const char* const WHITEH  = "\x1B[1;37m";

    } // namespace LogColors



    // Set the global log level
    #ifndef lnLOG_LEVEL_DEFAULT
        #define    lnLOG_LEVEL_NONE       0
        #define    lnLOG_LEVEL_ERROR      1
        #define    lnLOG_LEVEL_WARN       2
        #define    lnLOG_LEVEL_SPECIAL    3
        #define    lnLOG_LEVEL_NOTIFY     4
        #define    lnLOG_LEVEL_INFO       5
        #define    lnLOG_LEVEL_DEBUG      6
        #define    lnLOG_LEVEL_TRACE      7
        // Log Levels --- definiti come BUILD_FLAGS nel file: /home/loreto/filu/lnEnv/start_proc/piorun.sh
        // ma li metto qui nel caso non fossero intercettati correttamente
        #pragma message "lnLOG_LEVEL_DEFAULT not DEFINED. Defaulting to lnLOG_LEVEL_WARN."
        #define lnLOG_LEVEL_DEFAULT lnLOG_LEVEL_WARN
    #endif

    // --- per ogni modulo posso decidere il livello di log
    #ifndef lnLOG_MODULE_LEVEL
        #define lnLOG_MODULE_LEVEL lnLOG_LEVEL_DEFAULT
        // #pragma message "LOG_MODULE_LEVEL not_defined"
    #else
        #pragma message "lnLOG_MODULE_LEVEL defined"
    #endif
    // ---


    // Convenience macros for logging.
    // These macros check lnLOG_LEVEL at pre-compilation time
    // and call write only if the level is enabled, otherwise
    // they expand to `do {} while(0)` to generate no code.
    #if lnLOG_MODULE_LEVEL >= lnLOG_LEVEL_SPECIAL
        // #define LOG_SPEC(fmt, ...)     lnLog.write(LogColors::BLUEH, "SPC", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define lnLOG_SPECIAL(fmt, ...)     lnLog.write(LogColors::BLUEH, "SPEC", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define lnlog_special lnLOG_SPECIAL
        #define lnLOG_SUCCESS lnLOG_SPECIAL
    #else
        #define lnLOG_SPECIAL(...) do {} while (0)
        #define lnlog_special lnLOG_SPECIAL
        #define lnLOG_SUCCESS lnLOG_SPECIAL
    #endif

    #if lnLOG_MODULE_LEVEL >= lnLOG_LEVEL_ERROR
        #define lnLOG_ERROR(fmt, ...)    lnLog.write(LogColors::REDH, "ERRO", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define lnlog_error lnLOG_ERROR
    #else
        #define lnLOG_ERROR(...) do {} while (0)
        #define lnlog_error lnLOG_ERROR
    #endif

    #if lnLOG_MODULE_LEVEL >= lnLOG_LEVEL_WARN
        #define lnLOG_WARNING(fmt, ...)     lnLog.write(LogColors::YELLOWH, "WARN", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define lnLOG_WARN lnLOG_WARNING
        #define lnlog_warning lnLOG_WARNING
    #else
        #define lnLOG_WARNING(...) do {} while (0)
        #define lnLOG_WARN lnLOG_WARNING
        #define lnlog_warning lnLOG_WARNING
    #endif


    #if lnLOG_MODULE_LEVEL >= lnLOG_LEVEL_INFO
        #define lnLOG_INFO(fmt, ...)     lnLog.write(LogColors::GREENH, "INFO", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define lnlog_info lnLOG_INFO
    #else
        #define lnLOG_INFO(...) do {} while (0)
        #define lnlog_info lnLOG_INFO
    #endif

    #if lnLOG_MODULE_LEVEL >= lnLOG_LEVEL_NOTIFY
        #define lnLOG_NOTIFY(fmt, ...)  lnLog.write(LogColors::PURPLEH, "NTFY", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define lnlog_notify lnLOG_NOTIFY
    #else
        #define lnLOG_NOTIFY(...) do {} while (0)
        #define lnlog_notify lnLOG_NOTIFY
    #endif

    #if lnLOG_MODULE_LEVEL >= lnLOG_LEVEL_DEBUG
        #define lnLOG_DEBUG(fmt, ...)    lnLog.write(LogColors::CYANH, "DEBG", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define lnlog_debug lnLOG_DEBUG
    #else
        #define lnLOG_DEBUG(...) do {} while (0)
        #define lnlog_debug lnLOG_DEBUG
    #endif

    #if lnLOG_MODULE_LEVEL >= lnLOG_LEVEL_TRACE
        #define lnLOG_TRACE(fmt, ...)    lnLog.write(LogColors::WHITEH, "TRAC", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define lnlog_trace lnLOG_TRACE
    #else
        #define lnLOG_TRACE(...) do {} while (0)
        #define lnlog_trace lnLOG_TRACE
    #endif

    #undef lnLOG_MODULE_LEVEL

    // #endif  // end else del NO_MODULE_LOG