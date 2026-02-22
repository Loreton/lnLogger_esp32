//
// updated by ...: Loreto Notarantonio
// Date .........: 22-02-2026 16.04.22
//


// #ifdef __ln_MODULE_DEBUG_TEST__


#include <lnLogger_Class.h>


void setup() {
    Serial.begin(115200); // Initialize Serial communication
    lnLog.init(20);         // Initialize the logger's mutex
    // ... rest of your setup code
}


void loop() {
    LOG_INFO("This is an info message.");
    LOG_WARN("Something might be wrong here: %d", 42);
    LOG_ERROR("Critical error occurred in file %s at line %d", __FILE__, __LINE__);
    LOG_DEBUG("Debugging variable value: %s", VAR_NAME_VALUE(some_variable));
    delay(1000);
}


// #endif