ESP32 context
i
Una libreria di logging leggera, thread-safe e altamente configurabile per ESP32, progettata per l'uso con l'ecosistema **PlatformIO** e il framework **Arduino**.

## 🚀 Caratteristiche Principali

-   **Thread-Safe**: Utilizza i Mutex di FreeRTOS per garantire che i log da diversi Task non si sovrappongano.
-   **Colori ANSI**: Supporto integrato per colori nel terminale (es. Rosso per errori, Giallo per warning).
-   **Livelli di Log Granulari**: 8 livelli di log (da `NONE` a `TRACE`) per filtrare le informazioni necessarie.
-   **Timestamp Intelligente**: Gestisce automaticamente l'ora solare (NTP) se disponibile, altrimenti effettua il fallback sui `millis()` dall'avvio.
-   **Metadata Automatici**: Include nome del file, funzione e riga del codice in un formato allineato e pulito.
-   **Efficienza**: I log esclusi tramite il livello di compilazione non occupano spazio in memoria Flash né cicli CPU (grazie alle macro).
-   **Utility Time**: Funzioni integrate per convertire millisecondi in formati leggibili (`HH:MM:SS.ms`).

---

## 🛠 Configurazione PlatformIO

Per utilizzare la libreria nel tuo progetto PlatformIO, assicurati di configurare correttamente il file `platformio.ini`.

```ini
[env:esp32]
platform = espressif32 @ ^6.8.0
framework = arduino
monitor_speed = 115200
monitor_filters = colorize  ; Indispensabile per vedere i colori ANSI

build_flags = 
    ; Imposta il livello di log globale (es. 5 = INFO)
    -D lnLOG_LEVEL_DEFAULT=5
```

---

## 📖 Guida all'uso

### 1. Inizializzazione
Nel tuo file `main.cpp` (o nel setup del test), inizializza il logger dopo il `Serial.begin()`.

```cpp
#include <lnLogger_Class.h>

void setup() {
    Serial.begin(115200);
    delay(1000); 

    // Inizializza il logger:
    // Parametro 1: lunghezza massima riga di log (default 128)
    // Parametro 2: lunghezza massima info file/linea (default 20)
    lnLog.init(128, 25);
    
    lnLOG_INFO("Sistema avviato con successo!");
}
```

### 2. Macro di Logging
Utilizza le macro predefinite per stampare messaggi a diversi livelli. Supportano la formattazione stile `printf`.

| Macro | Livello | Colore | Uso tipico |
| :--- | :--- | :--- | :--- |
| `lnLOG_ERROR(...)` | ERROR | Rosso | Errori critici che bloccano il flusso. |
| `lnLOG_WARNING(...)` | WARN | Giallo | Situazioni anomale ma non bloccanti. |
| `lnLOG_INFO(...)` | INFO | Verde | Informazioni generali sullo stato. |
| `lnLOG_NOTIFY(...)` | NTFY | Viola | Notifiche di eventi importanti. |
| `lnLOG_DEBUG(...)` | DEBG | Cyan | Debug dettagliato per lo sviluppo. |
| `lnLOG_TRACE(...)` | TRAC | Bianco | Tracciamento granulare del codice. |

**Esempio:**
```cpp
int sensoreVal = 42;
lnLOG_DEBUG("Lettura sensore: %d", sensoreVal);
lnLOG_ERROR("Connessione Wi-Fi fallita! Tentativo: %d", 3);
```

### 3. Utility Timestamp
Puoi convertire millisecondi in stringhe formattate usando `msecToHMS`:

```cpp
char timeBuf[16];
uint32_t ms = millis();
lnLog.msecToHMS(timeBuf, sizeof(timeBuf), ms, true, false); 
// Risultato: "00:01:23.456"
```

---

## 🔍 Dettagli Tecnici

### Livelli di Log
I livelli sono definiti come segue:
1. `lnLOG_LEVEL_ERROR`
2. `lnLOG_LEVEL_WARN`
3. `lnLOG_LEVEL_SPECIAL`
4. `lnLOG_LEVEL_NOTIFY`
5. `lnLOG_LEVEL_INFO`
6. `lnLOG_LEVEL_DEBUG`
7. `lnLOG_LEVEL_TRACE`

Se un modulo non definisce `lnLOG_MODULE_LEVEL`, verrà utilizzato il valore di `lnLOG_LEVEL_DEFAULT`.

### Formattazione del Nome File
Il logger pulisce automaticamente il percorso del file (estraendo solo il nome del file da `__FILE__`) e lo allinea a destra per una leggibilità superiore nel monitor seriale.

**Esempio Output:**
```text
[00:00:45][   main:054][INFO] Sistema avviato
[00:00:46][ sensor:022][DEBG] Valore: 25.4
```

---

## 📂 Struttura del Progetto
- `src/lnLogger_Class.h`: Definizione della classe, macro e costanti colore.
- `src/lnLogger_Class.cpp`: Logica di gestione Mutex, formattazione stringhe e gestione tempo.
- `test/`: Contiene esempi di test per verificare il timestamp e le funzionalità del logger.

---

## ⚠️ Note
- Il logger richiede l'ambiente **FreeRTOS** (nativo su ESP32).
- Assicurarsi che il terminale seriale supporti le sequenze di escape ANSI per una visualizzazione corretta dei colori.
- Se il tempo di sistema non è sincronizzato via NTP, il logger mostrerà il tempo trascorso dall'avvio (uptime).

---
*Developed by Loreto Notarantonio*
