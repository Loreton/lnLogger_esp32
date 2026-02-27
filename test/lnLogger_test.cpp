//
// updated by ...: Loreto Notarantonio
// Date .........: 22-02-2026 16.04.22
//


// #ifdef __ln_MODULE_DEBUG_TEST__


#include <lnLogger_Class.h>


void setup() {
    Serial.begin(115200); // Initialize Serial communication
    lnLog.init(128, 20);  // line_buffer_len, filename_buffer_len
    // ... rest of your setup code
}


void loop() {
    lnLOG_INFO("This is an info message.");
    lnLOG_WARNING("Something might be wrong here: %d", 42);
    lnLOG_ERROR("ERROR occurred in file %s at line %d", __FILE__, __LINE__);
    lnLOG_DEBUG("Debugging variable value: %s", VAR_NAME_VALUE(some_variable));
    // lnLOG_CRITICAL("CRITICAL variable value: %s", VAR_NAME_VALUE(some_variable));
    delay(1000);
}


// #endif
