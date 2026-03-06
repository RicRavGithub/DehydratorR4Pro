# Software Logic Specification: DRYMASTER PRO R4
**Version:** 1.2 (Implementation-Ready Master Spec)  
**Target Hardware:** Arduino UNO R4 Minima (Renesas RA4M1)  
**Document Type:** Absolute Ground Truth for Code Reconstruction  

## 1. Architettura del Sistema e Dipendenze
Il firmware è strutturato come una Macchina a Stati Finiti (FSM) che orchestra il controllo termico, la sicurezza e l'interfaccia grafica.  
Il sistema opera su un ciclo principale sincronizzato di 2000ms (2 secondi).

### 1.1 Librerie Obbligatorie
* **`R4_TFT_SPI_Touch.h` (Custom):** Libreria fondamentale per la gestione del controller ILI9486 e del touch XPT2046 tramite registri shift 74HC4094. Gestisce la risoluzione 480x320.
* **`pid-autotune.h` (AhmedOsamPIN_PELTIER7):** Implementa l'algoritmo PID e la calibrazione automatica tramite metodo relay.
* **`Adafruit_SHT31.h`:** Gestione del sensore ambientale I2C (SDA/SCL).
* **`OneWire.h` & `DallasTemperature.h`:** Gestione del sensore di sicurezza barra DS18B20 su Pin A1.
* **`EEPROM.h`:** Gestione della persistenza su memoria interna da 8KB.
* **`bmp30x30.h` (Asset):** Contiene le icone bitmap per la dashboard (Temp, Hum, Power).

### 1.2 Assegnazione dei pin
* L'assegnazione dei PIN è obbligata dal circuito fisico in cui opera l'Arduino e pertanto non può essere modificata.  
* Il codice non dovrà mai usare altri termini per identificare un PIN che non siano i nomi nelle define sottostanti.  
Pin definiti nella libreria R4_TFT_SPI_Touch:  
#define PIN_PIR_D2      2  
#define PIN_TP_IRQ      3  
#define PIN_TP_CS       4  
#define PIN_SD_CS       5  
#define PIN_TP_BUSY     6  
#define PIN_LCD_DC      7  
#define PIN_LCD_RST     8  
#define PIN_LCD_BL      9  
#define PIN_LCD_CS      10  
#define PIN_SPI_MOSI    11  
#define PIN_SPI_MISO    12  
#define PIN_SPI_SCK     13  
Pin da definire all'inizio dello sketch:  
#define PIN_PELTIER     A0  
#define PIN_DS18B20     A1  
#define PIN_SHT31_SDA   A4  
#define PIN_SHT31_SCL   A5  

## 2. Global FSM: Stati e Navigazione
Il sistema si trova sempre in uno dei seguenti stati definiti tramite `enum States`:

| Stato | Descrizione | Trigger di Uscita |
| :--- | :--- | :--- |
| **MAIN_PAGE** | Dashboard principale con dati e grafici real-time. | Pressione tasti "SET" o "TUNE". |
| **SETPOINT_PAGE** | Regolazione del target termico (20-90°C). | Tasti "+" e "-", "SAVE" (scrive EEPROM) e "CANCEL". |
| **INFO_PAGE** | Diagnostica: versioni EEPROM, tutti i valori grezzi dei sensori, stato PWM, set point. | Pressione tasto "BACK". |
| **AUTOTUNE_CONFIRM** | Schermata di avviso pre-calibrazione. | "YES" avvia autotune; "NO" torna alla Main. |
| **AUTOTUNE_RUNNING** | Monitoraggio attivo del processo di calibrazione. | Completamento automatico o tasto "ABORT". |
| **ALARM_PAGE** | Blocco di sicurezza (schermo rosso). | Reset manuale con apposito tasto o automatico dopo rientro nei parametri. |

## 3. Elaborazione Sensori e "Aggressive Filtering"
Per compensare il rumore elettrico, il sistema applica una media mobile sugli ultimi 8 valori.

### 3.1 Rolling Average (8 Campioni)
* Ogni 2000ms viene effettuata una lettura dai sensori SHT31 e DS18B20.
* I dati sono inseriti in un buffer circolare di 8 posizioni per ogni variabile (Aria T, Aria RH, Barra T).
* La logica di filtraggio calcola la media aritmetica degli 8 campioni più recenti.
* Il valore risultante è l'unico utilizzato dal PID e visualizzato sulla UI per garantire stabilità.

### 3.2 Matematica dell'Umidità Assoluta ($g/m^3$)
Utilizzando la temperatura media ($T$) e l'umidità relativa media ($RH$), il software calcola:
1. **Pressione di vapore saturo:** $$E_s = 6.112 \times e^{(17.67 \times T) / (T + 243.5)}$$
2. **Umidità Assoluta ($AH$):** $$AH = (E_s \times (RH / 100.0) \times 216.74) / (273.15 + T)$$

## 4. Gestione Memoria (Circular Wear Leveling)
Per proteggere la EEPROM interna, si utilizza un sistema con logica **Circular Buffer Wear Leveling**, che sfrutta tutti gli 8KB di EEPROM disponibili.

* **`Config struct`:** Struttura di 15 byte, che contiene `uint16_t version`, `uint8_t setpoint` e `float kp, ki, kd`.
* **Boot Scan:** All'avvio, scansiona la EEPROM cercando il record con la `version` più alta.
* Se non viene trovato (memoria vergine), vengono caricati i valori di default.
* La logica gestisce anche il passaggio per lo 0: se sono presenti sia version 0xFFFF che 0x0000, cerca il valore più alto del nuovo ciclo.
* **Save Logic:** Il salvataggio avviene solo su conferma utente ("SAVE") o al termine di un Autotune riuscito. Il nuovo record viene scritto nell'indirizzo successivo, garantendo una distribuzione uniforme dei cicli di scrittura.

## 5. Controllo Termico (PID & Slow PWM)
### 5.1 Slow PWM Logic (Pin PIN_PELTIER)
Data l'inerzia termica, il MOSFET è pilotato con una frequenza di 0.5Hz.
* **Finestra temporale:** 2000ms.
* **Duty Cycle:** Il PID calcola un valore di output compreso tra 0 e 2000, l'output viene applicato al pin PIN_PELTIER, che resta HIGH per un numero di ms pari al valore di output del pid e low per i restanti fino ad arrivare a 2000ms.
* **Safety Lock:** In caso di allarme, il Pin PIN_PELTIER viene forzato a LOW indipendentemente dal PID.

### 5.2 PID Autotune
Viene utilizzato il metodo dell'oscillazione a relè per calcolare i coefficienti ottimali. Durante questa fase, il setpoint viene mantenuto fisso e il sistema monitora i picchi di temperatura per derivare il guadagno e il periodo critico.

## 6. Graphical User Interface (GUI) & Touch
### 6.1 Layout Dashboard (MAIN_PAGE)
* **Colori:** ogni grandezza fisica ha assegnato il proprio colore, da usare per scritte e bordi nei quattro box in alto, nelle tracce del grafico e come colore dei pulsanti footer:

| Grandezza | Alias #define nello sketch | Nome colore nella libreria R4_TFT_SPI_Touch |
| :--- | :--- | :--- |
| Temperatura Aria | COL_TEMP_ARIA | COLOR_RED_VIVID |
| Relative Humidity (%) | COL_REL_HUM | COLOR_BLUE_CYAN |
| Absolute Humidity | COL_ABS_HUM | COLOR_GREEN_EMERALD |
| PWM (%) | COL_PWM | COLOR_YELLOW_VIVID |
| Temperatura Barra | COL_BAR | COLOR_WHITE |

* **Top Bar (Icons):** 4 box (120x40px) con icone 30x30 a destra e valori, con unità di misura per Temperatura Aria, Relative Humidity %, Absolute Humidity, PWM (%).
* **Rolling Chart:** Grafico centrale aggiornato ogni 10 secondi con 235 campioni storici.
    * **Tracce:** Temperatura Aria, Relative Humidity %, Absolute Humidity, PWM (%).
    * **Setpoint:** Una linea tratteggiata, anch'essa di colore COL_TEMP_ARIA, rappresenta il valore target impostato, permettendo di valutare visivamente l'errore di inseguimento.
* **Pulsanti Footer:** Disegnati con funzione `drawButton3D()` per effetto rilievo. A sinistra la Temperatura Barra, a destra il Setpoint
* **Titolo:** In mezzo tra i due Pulsanti Footer, la scritta "DRYMASTER PRO R4"

### 6.2 Gestione Retroilluminazione (D9)
* **Trigger**: Attivato dal sensore PIR (D2) o dal tocco sul display.
* **Fade-In**: Aumento rapido della luminosità PWM fino al 100%.
* **Fade-Out**: Dopo 5 minuti (300.000ms) di inattività, la luminosità decresce lentamente fino allo spegnimento totale.

### 6.3 Logica dei Tocchi (XPT2046)
Il software mappa le coordinate touch per gestire le interazioni:
* **SET:** In MAIN_PAGE, `x > 340, y > 270` (Passa a `SETPOINT_PAGE`).
* **TUNE:** In MAIN_PAGE, `x < 140, y > 270` (Passa a `AUTOTUNE_CONFIRM`).
* **INFO:** In MAIN_PAGE,  `140 < x < 340, y > 270` (Passa a `INFO_PAGE`).
* **Setpoint +/-, SAVE; CANCEL:** In SETPOINT_PAGE, Regolazione di `tempSetpointTmp` prima del salvataggio, salvataggio, ritorno a MIAN_PAGE senza cambiare il Setpoint.

## 7. Watchdog e Sicurezza Hardware
La logica di sicurezza ha la precedenza su tutte le altre funzioni del loop.
Il sistema esegue un controllo critico ad ogni iterazione del loop se si verifica almeno una delle seguenti:
1. **Soglia Barra:** Se Barra T > 115°C.
2. **Soglia Aria:** Se Aria T > 95°C.
3. **Integrità:** Errore di comunicazione con SHT31 o DS18B20 (NaN).
4. **Lentezza:** La temperatura della barra non sale abbastanza in fretta: a pieno carico, la soglia è 1°C/minuto, per altri valori di PWM, in proporzione; la condizione serve ad identificare un possibile distacco di qualche elemento.
* **Azione:** Salto immediato a `ALARM_PAGE`, Pin A0 LOW, disabilitazione interrupt e segnale visivo rosso.

