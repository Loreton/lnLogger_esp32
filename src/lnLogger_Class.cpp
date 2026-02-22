/*
// updated by ...: Loreto Notarantonio
// Date .........: 19-02-2026 15.59.37
*/

// >>>lnLogger_Class.cpp
#include <Arduino.h>
#include <ESP32Time.h> // ESP32Time.cpp
#include <freertos/semphr.h> // Required here for mutex implementation

#include <lnLogger_Class.h> // Changed to new header file name

ESP32Time       this_rtc;


// Constructor: Initializes the mutex
ESP32Logger::ESP32Logger(void) { // Changed class name
    // The constructor does the bare minimum, or nothing.
    // The mutex will be created in init().
    m_logMutex = NULL; // Initialize to NULL to indicate it hasn't been created yet
};

// Used so I can send a message if I can't initialize the mutex
void ESP32Logger::init(const uint8_t filename_buffer_len) { // Changed class name
    m_FILENAME_BUFFER_LENGTH = filename_buffer_len;
    if (!m_mutexInitialized) {
        m_logMutex = xSemaphoreCreateMutex();
        if (m_logMutex == NULL) {
            // If mutex creation fails, it's a serious error,
            // but at least Serial is now initialized to try.
            Serial.println("\n\nERRORE: Impossibile creare mutex per il logger! Logging non protetto.\n\n");
        }
        else {
            m_mutexInitialized = true;
            Serial.println("\n\tOK: Logger Mutex inizializzato.\n");
        }
    }
}


// ################################################################
// Converte millisecondi in HH:MM:SS.ms
// ritorna il timestamp del giorno
//    addMilliSec = true: aggiunge .xxx alla fine della stringa
//    stripHeader = true: rimuove hour o minutes se == 0
// ################################################################

const char* ESP32Logger::msecToHMS(char *buffer, uint8_t buffer_len, uint32_t millisec, bool withMilliSec, bool stripHours) {

    uint16_t msec    = (millisec % 1000UL);
    uint32_t seconds = (millisec / 1000UL);

    uint8_t sec      = (seconds  % 60);
    uint8_t min      = (seconds / 60) % 60;
    uint8_t hour     = (seconds / 3600);

    if (withMilliSec) {
        snprintf(buffer, buffer_len, "%02d:%02d:%02d.%03lu", hour, min, sec, msec); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.
    }
    else {
        snprintf(buffer, buffer_len, "%02d:%02d:%02d", hour, min, sec); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.
    }

    if (stripHours && hour == 0)  {
        return buffer+3;
    }

    return buffer;
}

const char* ESP32Logger::msecToHMS(uint32_t millisec, bool withMilliSec, bool stripHours) {
    return msecToHMS(sharedTimeBUFFER, sizeof(sharedTimeBUFFER), millisec, withMilliSec, stripHours);
}


// const char* ESP32Logger::secToHMS(uint32_t seconds, bool stripHours) {
//     char buffer[12];
//     return msecToHMS(buffer, 15, seconds*1000UL, false, stripHours);
// }


/**
 * @brief Formats the file name and line number.
 * The file name is truncated to a maximum length and padded with dots if shorter.
 * @param file The full path of the file (usually __FILE__).
 * @param function  (usually __FUNCTION__).
 * @param line The line number (usually __LINE__).
 * @return A constant string containing the formatted file name, function name and line number.
 */

#if 0
// per eventuali test:
//     /media/loreto/LnDisk_SD_ext4/Filu/GIT-REPO/c-cpp/variEsempi/formatLogFname.cpp
const char* ESP32Logger::getFileLineInfo(char *outBUFFER, const uint16_t outBUFFER_SIZE, const char* file, const char* function, int line) {
    const uint16_t outBUFFER_LEN = outBUFFER_SIZE-1; // per sicurezza prendiamo l'ultimo byte per lo '\0'

    // Estrai il nome del file (senza percorso)
    const char *filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;

    const char paddingChar      = '.'; // carattere di padding
    const uint8_t func_len      = strlen(function);
    const uint8_t line_len      = 1 + 3; // 1 per ':' + 3 per il numero di riga
    const uint8_t fixed_len     = func_len + line_len  + 1;
    const uint8_t file_len      = outBUFFER_LEN - fixed_len; // 1 per il terminatore '\0'

    char lineBuff[line_len+1]; // spazio per '\0'
    snprintf(lineBuff, sizeof(lineBuff) ,":%03d", line); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.


    int8_t j;

    // --- fill in buffer whith paddingChar
    for (j = 0; j < outBUFFER_LEN; j++) { outBUFFER[j] = paddingChar; }
    outBUFFER[outBUFFER_LEN] = '\0'; // terminatore buffer


    // copiamo nome del file in outBUFFER
    char *ptr = outBUFFER;
    for (j = 0; j < file_len && *ptr != '\0'; j++) { // --- verifichiamo anche non superare il limite del buffer '\0' messo precedentemente
        if (filename[j] == '\0' || filename[j] == '_' || filename[j] == '.') {
            break;
        }
        *ptr++ = filename[j]; // riempi con il carattere di padding
    }


    if (fIncludeFunction) {
        // --- copiamo function name to outBUFFER
        *ptr++ = '.'; // separator file.function
        for (j = 0; function[j] != '\0' && *ptr != '\0'; j++) {  // --- verifichiamo anche non superare il limite del buffer '\0' messo precedentemente
            *ptr++ = function[j];
        }
    }



    // --- copiamo function name to outBUFFER
    ptr = &outBUFFER[outBUFFER_LEN - line_len];
    for (j = 0; lineBuff[j] != '\0' && *ptr != '\0'; j++) {  // --- verifichiamo anche non superare il limite del buffer '\0' messo precedentemente
        *ptr++ = lineBuff[j];
    }
   // *ptr = '\0'; // terminatore della stringa (per sicurezza.....)


    return outBUFFER;
}
#endif
const char* ESP32Logger::getFileLineInfo(char *outBUFFER, const uint16_t outBUFFER_SIZE, const char* file, const char* function, int line) {
    if (outBUFFER == nullptr || outBUFFER_SIZE == 0) return "";

    // --- CONFIGURAZIONE ---
    const char paddingChar = ' '; // <--- Cambia questo per modificare il look
    const char file_funct_separator   = '.'; // Separatore tra funzione e file
    const uint16_t outBUFFER_LEN = outBUFFER_SIZE - 1;

    // 1. Estrai e pulisci il nome del file (stop a '_' o '.')
    const char *filenameStart = strrchr(file, '/');
    filenameStart = filenameStart ? filenameStart + 1 : file;

    char cleanFilename[32];
    uint8_t cleanFileLen = 0;
    for (cleanFileLen = 0; cleanFileLen < sizeof(cleanFilename) - 1; cleanFileLen++) {
        char c = filenameStart[cleanFileLen];
        // if (c == '\0' || c == '_' || c == '.') break; //.... altrimenti mi taglia il nome del file tio _Class
        if (c == '\0' || c == '.') break;
        cleanFilename[cleanFileLen] = c;
    }
    cleanFilename[cleanFileLen] = '\0';

    // 2. Prepara la stringa della riga (es: ":054")
    char lineBuff[10];
    int lineLen = snprintf(lineBuff, sizeof(lineBuff), ":%03d", line);

    // 3. Calcola lunghezze per l'allineamento a destra
    uint8_t funcPartLen = fIncludeFunction ? (strlen(function) + 1) : 0; // +1 per il separatore
    uint16_t totalTextLen = funcPartLen + cleanFileLen + lineLen;

    // 4. Reset buffer con il carattere di padding scelto
    memset(outBUFFER, paddingChar, outBUFFER_LEN);
    outBUFFER[outBUFFER_LEN] = '\0';

    // 5. Calcola punto di inizio (Offset)
    int16_t startPos = outBUFFER_LEN - totalTextLen;
    if (startPos < 0) startPos = 0;

    char *ptr = outBUFFER + startPos;

    // 6. Copia i componenti (con protezione overflow)
    // A. Funzione
    if (fIncludeFunction) {
        for (int i = 0; function[i] != '\0' && (ptr - outBUFFER) < outBUFFER_LEN; i++) {
            *ptr++ = function[i];
        }
        if ((ptr - outBUFFER) < outBUFFER_LEN) *ptr++ = file_funct_separator;
    }

    // B. Filename
    for (int i = 0; i < cleanFileLen && (ptr - outBUFFER) < outBUFFER_LEN; i++) {
        *ptr++ = cleanFilename[i];
    }

    // C. Linea
    for (int i = 0; i < lineLen && (ptr - outBUFFER) < outBUFFER_LEN; i++) {
        *ptr++ = lineBuff[i];
    }

    return outBUFFER;
}



/**
 * @brief Internal function for actual log output.
 * This function is called by the logging macros.
 * @param color ANSI color code for output.
 * @param tag Log level label (e.g. "ERR", "INF").
 * @param file The source file path.
 * @param line The line number in the source file.
 * @param format Printf-like format string.
 * @param ... Variable arguments for the format string.
 */
// void ESP32Logger::write(const char* color, const char* tag, const char* file, int line, const char* format, ...) { // Changed class name
void ESP32Logger::write(const char* color, const char* tag, const char* file, const char* function, int line, const char* format, ...) { // Changed class name
    if (!m_mutexInitialized) {
        // If not initialized, we cannot use the mutex or Serial.
        // You might print a crude warning message or discard the log.
        // This is the most critical situation: a log before Serial.init() and myLog.init().
        // For now, we print it to Serial but without guarantees of visibility.
        // For very early logs, you might consider a FIFO buffer or JTAG/SWD.
        Serial.printf("AVVISO: Logging prima dell'inizializzazione: (lanciare il init())");
        // Serial.printf(format, ##__VA_ARGS__);
        Serial.println();
        return; // Exit to avoid issues
    }

    // Try to acquire the mutex. Wait indefinitely (portMAX_DELAY) if it's already locked.
    if (m_logMutex != NULL && xSemaphoreTake(m_logMutex, portMAX_DELAY) == pdTRUE) {
        // const uint8_t nowTIME_SIZE = 16;
        // const uint8_t fname_SIZE   = 30;
        // const uint16_t logLine_SIZE = 512;
        char nowTimeBUFFER[16];
        // const uint8_t fnameBUFFER_len = fIncludeFunction ? 32 : 16; // +1 per il separatore
        char fnameBUFFER[m_FILENAME_BUFFER_LENGTH];
        char logLineBUFFER[512];


        va_list args;
        va_start(args, format);
        int len = vsnprintf(logLineBUFFER, sizeof(logLineBUFFER), format, args);
        va_end(args);

        if (len >= sizeof(logLineBUFFER)) {
            logLineBUFFER[sizeof(logLineBUFFER)-1] = '\0';  // EOS
        }

        struct tm timeinfo = this_rtc.getTimeStruct();
        snprintf(nowTimeBUFFER, sizeof(nowTimeBUFFER), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.

        Serial.printf("%s[%s][%s][%s] %s%s\n",
                      color,
                      nowTimeBUFFER,
                      this->getFileLineInfo(fnameBUFFER, sizeof(fnameBUFFER), file, function, line),
                      tag,
                      logLineBUFFER,
                      LogColors::RESET);

        xSemaphoreGive(m_logMutex); // Release the mutex, allowing other tasks to log
    } else {
        // This part of the code is executed only if the mutex was not created
        // or if mutex acquisition fails (extremely rare with portMAX_DELAY).
        // You might still print a basic message to not lose the log,
        // but it won't be thread-safe.
        Serial.printf("!!! Failed to acquire log mutex or mutex not initialized: ");
        // Serial.printf(format, ##__VA_ARGS__); // Print the original unformatted message
        Serial.println();
    }
}

ESP32Logger lnLog; // Definizione oggetto lnLog