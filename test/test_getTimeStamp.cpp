#include <stdio.h>
#include <stdbool.h>
#define uint32_t unsigned long
#define uint16_t unsigned long
#define uint8_t unsigned int

// const uint8_t BUFFER_LEN = 16;
char timeBuffer[16]; // Static buffer for the timestamp



char* getTimeStamp(bool trimHeader, uint32_t millisec) { // Changed class name
    snprintf(timeBuffer, sizeof(timeBuffer), "%s", "ciao mi chiamo loreto");
    uint16_t msec    = (millisec % 1000); // value from 0-999
    uint32_t seconds = (millisec / 1000); // could be long
    uint8_t sec      = (seconds % 60);
    uint8_t min      = (seconds / 60) % 60;
    uint8_t hour     = (seconds / 3600);
    if (trimHeader) {
        if (hour > 0) {
            snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d.%03lu", hour, min, sec, msec); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.
        } else if (min > 0) {
            snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d.%03lu", min, sec, msec); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.
        } else {
            snprintf(timeBuffer, sizeof(timeBuffer), "%02d.%03lu", sec, msec); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.
        }
    }
    else {
        snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d.%03lu", hour, min, sec, msec); // snprintf() scrive al massimo n-1 caratteri più il terminatore nul (\0) in dest.
    }
    return timeBuffer;
}


int main() {
    // Write C code here
    uint32_t ms_to_next_level=179600;
    uint32_t m_elapsed=411;
    printf("Try programiz.pro\n");
    printf("\telapsed: %lu - next level in: %lu\n", m_elapsed, ms_to_next_level);
    printf("\telapsed: %s - next level in: %s\n", getTimeStamp(true, m_elapsed), getTimeStamp(true, ms_to_next_level));

    return 0;
}