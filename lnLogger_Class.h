/*
// updated by ...: Loreto Notarantonio
// Date .........: 11-09-2025 14.28.33
*/

#pragma once

#include <Arduino.h>
#include <freertos/semphr.h> // Required for FreeRTOS types (SemaphoreHandle_t)
#include "esp_timer.h" // For esp_timer_get_time()
#include <ESP32Time.h> // ESP32Time.cpp

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
        void init(void);
        void write(const char* color, const char* tag, const char* file, const char* function, int line, const char* format, ...);
        const char* msecToHMS(char *buffer, uint8_t buffer_len, uint32_t millisec, bool addMilliSec=false, bool stripHeader=false);
        const char* msecToHMS(uint32_t millisec, bool withMilliSec=false, bool stripHours=false); // usa un buffer interno
        // const char* secToHMS(uint32_t millisec, bool stripHours=false); // usa un buffer interno

    private:
        ESP32Time rtc;
        struct tm      m_timeinfo;
        char sharedTimeBUFFER[16];

        bool m_mutexInitialized = false;
        SemaphoreHandle_t m_logMutex = NULL; // The mutex to protect log operations
        const char* getFileLineInfo(char *outBUFFER, const uint16_t OutBUFFER_maxLen, const char* file, const char* function, int line);
}; // class ESP32Logger


extern ESP32Logger lnLog; // defined in lnLogger.cpp



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
    #ifndef LOG_LEVEL_DEFAULT
        // Log Levels --- definiti come BUILD_FLAGS nel file: /home/loreto/lnProfile/liveProduction/piorun.sh
        #define    LOG_LEVEL_NONE       0
        #define    LOG_LEVEL_ERROR      1
        #define    LOG_LEVEL_WARN       2
        #define    LOG_LEVEL_SPECIAL    3
        #define    LOG_LEVEL_NOTIFY     4
        #define    LOG_LEVEL_INFO       5
        #define    LOG_LEVEL_DEBUG      6
        #define    LOG_LEVEL_TRACE      7
        #pragma message "LOG_LEVEL_DEFAULT not DEFINED. Defaulting to LOG_LEVEL_WARN."
        #define LOG_LEVEL_DEFAULT LOG_LEVEL_WARN
    #endif

    // --- per ogni modulo posso decidere il livello di log
    #ifndef LOG_MODULE_LEVEL
        #define LOG_MODULE_LEVEL LOG_LEVEL_DEFAULT
    #endif
    // ---


    // Convenience macros for logging.
    // These macros check LOG_LEVEL at pre-compilation time
    // and call write only if the level is enabled, otherwise
    // they expand to `do {} while(0)` to generate no code.
    #if LOG_MODULE_LEVEL >= LOG_LEVEL_SPECIAL
        // #define LOG_SPEC(fmt, ...)     lnLog.write(LogColors::BLUEH, "SPC", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define LOG_SPEC(fmt, ...)     lnLog.write(LogColors::BLUEH, "SPC", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define log_spec LOG_SPEC
    #else
        #define LOG_SPEC(...) do {} while (0)
        #define log_spec LOG_SPEC
    #endif

    #if LOG_MODULE_LEVEL >= LOG_LEVEL_ERROR
        #define LOG_ERROR(fmt, ...)    lnLog.write(LogColors::REDH, "ERR", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define log_error LOG_ERROR
    #else
        #define LOG_ERROR(...) do {} while (0)
        #define log_error LOG_ERROR
    #endif

    #if LOG_MODULE_LEVEL >= LOG_LEVEL_WARN
        #define LOG_WARN(fmt, ...)     lnLog.write(LogColors::YELLOWH, "WRN", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define log_warn LOG_WARN
    #else
        #define LOG_WARN(...) do {} while (0)
        #define log_warn LOG_WARN
    #endif


    #if LOG_MODULE_LEVEL >= LOG_LEVEL_INFO
        #define LOG_INFO(fmt, ...)     lnLog.write(LogColors::GREENH, "INF", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define log_info LOG_INFO
    #else
        #define LOG_INFO(...) do {} while (0)
        #define log_info LOG_INFO
    #endif

    #if LOG_MODULE_LEVEL >= LOG_LEVEL_NOTIFY
        #define LOG_NOTIFY(fmt, ...)  lnLog.write(LogColors::PURPLEH, "NFY", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define log_notify LOG_NOTIFY
    #else
        #define LOG_NOTIFY(...) do {} while (0)
        #define log_notify LOG_NOTIFY
    #endif

    #if LOG_MODULE_LEVEL >= LOG_LEVEL_DEBUG
        #define LOG_DEBUG(fmt, ...)    lnLog.write(LogColors::CYANH, "DBG", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define log_debug LOG_DEBUG
    #else
        #define LOG_DEBUG(...) do {} while (0)
        #define log_debug LOG_DEBUG
    #endif

    #if LOG_MODULE_LEVEL >= LOG_LEVEL_TRACE
        #define LOG_TRACE(fmt, ...)    lnLog.write(LogColors::WHITEH, "TRC", __FILE__, __FUNCTION__ , __LINE__, fmt, ##__VA_ARGS__)
        #define log_trace LOG_TRACE
    #else
        #define LOG_TRACE(...) do {} while (0)
        #define log_trace LOG_TRACE
    #endif

    #undef LOG_MODULE_LEVEL

    // #endif  // end else del NO_MODULE_LOG