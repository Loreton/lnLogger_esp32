//
// updated by ...: Loreto Notarantonio
// Date .........: 03-09-2025 11.54.34
//


#ifdef __ln_MODULE_DEBUG_TEST__


#include "lnLogger_Class.h"


void setup() {
    Serial.begin(115200); // Initialize Serial communication
    lnLog.init();         // Initialize the logger's mutex
    // ... rest of your setup code
}


void loop() {
    log_info("This is an info message.");
    log_warn("Something might be wrong here: %d", 42);
    log_error("Critical error occurred in file %s at line %d", __FILE__, __LINE__);
    log_debug("Debugging variable value: %s", VAR_NAME_VALUE(some_variable));
    delay(1000);
}


#endif