/*
// updated by ...: Loreto Notarantonio
// Date .........: 19-02-2026 15.59.37
*/

// >>>lnLogger_Class.cpp
#include <Arduino.h>
#include <time.h>

#include <freertos/semphr.h> // Required here for mutex implementation

#include <lnLogger_Class.h> // Changed to new header file name


// Constructor: Initializes the mutex
ESP32Logger::ESP32Logger(void) { // Changed class name
    // The constructor does the bare minimum, or nothing.
    // The mutex will be created in init().
    m_logMutex = NULL; // Initialize to NULL to indicate it hasn't been created yet
};

// Used so I can send a message if I can't initialize the mutex
void ESP32Logger::init(const uint8_t line_buffer_len, const uint8_t filename_buffer_len) { // Changed class name
    m_FILENAME_BUFFER_LENGTH = filename_buffer_len;
    m_LINE_BUFFER_LENGTH = line_buffer_len;
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


void ESP32Logger::getNowTime(char* buffer, size_t len) {
    time_t now = time(nullptr);

    if (now < 100000) { // Se NTP non è ancora sincronizzato
        // fallback su millis()
        uint32_t s = millis() / 1000;
        uint8_t hh = (s / 3600) % 24;
        uint8_t mm = (s / 60) % 60;
        uint8_t ss = s % 60;

        snprintf(buffer, len, "%02d:%02d:%02d", hh, mm, ss);
        return;
    }

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    strftime(buffer, len, "%H:%M:%S", &timeinfo);
}



/**
 * @brief Formats the file name and line number.
 * The file name is truncated to a maximum length and padded with dots if shorter.
 * @param file The full path of the file (usually __FILE__).
 * @param function  (usually __FUNCTION__).
 * @param line The line number (usually __LINE__).
 * @return A constant string containing the formatted file name, function name and line number.
 */
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

        char nowTimeBUFFER[16];
        char fnameBUFFER[m_FILENAME_BUFFER_LENGTH];
        char logLineBUFFER[m_LINE_BUFFER_LENGTH];


        va_list args;
        va_start(args, format);
        int len = vsnprintf(logLineBUFFER, sizeof(logLineBUFFER), format, args);
        va_end(args);

        getNowTime(nowTimeBUFFER, sizeof(nowTimeBUFFER));

        // coloriamo solo il testo
        Serial.printf("%s[%s][%s]%s[%-4s] %s%s\n",
                      LogColors::GREEN,
                      nowTimeBUFFER,
                      this->getFileLineInfo(fnameBUFFER, sizeof(fnameBUFFER), file, function, line),
                      color,
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
        Serial.println();
    }
}

ESP32Logger lnLog; // Definizione oggetto lnLog