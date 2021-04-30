// Sonidos.ino
// Código creado para la manipulación de sonido usando la tiva C y un buzzer
// Código original tomado de:
//      https://henryforceblog.wordpress.com/2015/05/02/blink-example-using-timer-on-a-tiva-launchpad/
// Modificado por Luis Alberto Rivera (Procesamiento de señales)
// Adaptación al proyecto por Steve Chex

//*****************************************************************************************************
//                                          LIBRERIAS
//*****************************************************************************************************

#include "wiring_private.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "pitches.h"

#include <SPI.h>
#include <SD.h>

//*****************************************************************************************************
//                              VARIABLES, CONSTANTES Y DEFINICIONES
//*****************************************************************************************************

#define LED RED_LED
#define LED2 BLUE_LED
#define Buz PE_3
#define SW_1 PF_0
#define SW_2 PF_4

#define MISO PA_4
#define MOSI PA_5
#define CS PA_3
#define SCK PA_2

#define MHZ80 80000000
#define interval 5000

//--------------------------- CONTROL DE CANCIONES --------------------------------

File dir1, dir2;
uint32_t freq = 0;
bool state = false, sta2 = false, stop = false, oneShoot = false, oneRead = false;
uint8_t soundCount = 1, durationCount = 0, beat = 0, durationBase = 0, durationBetweenNotes = 0,
        songLastIndex = 0;
uint8_t changeSong = 1;

//-------------------------------- LECTURA SD -------------------------------------

uint32_t temp = 0, contKeeper = 0, contKeeper2 = 0, contTransformer = 0, valor = 0, valorNoTrans = 0;
int transformer[8] = {};
uint32_t kepa[110] = {}, kepa2[110] = {};
bool noSD = false;

//------------------------------- CONTENEDORES ------------------------------------

uint32_t notes[110] = {};
uint8_t duration[110] = {};
uint32_t newSong[110] = {};
uint8_t newDuration[110] = {};

//-------------------------------- CANCIONES --------------------------------------

//--------------------------- POP GOES THE WEASEL ---------------------------------

uint32_t song1[107] = {1,
                       204081,                                    // 1 Anacruza inicial
                       152963, 0, 152963, 136286, 0, 136286,      // 6
                       121396, 102040, 121396, 152963, 0, 204081, // 6
                       152963, 0, 152963, 136286, 0, 136286,      // 6
                       121396, 121396, 152963, 0, 204081,         // 5
                       152963, 0, 152963, 136286, 0, 136286,      // 6
                       121396, 102040, 121396, 152963, 0, 0,      // 6
                       90909, 0, 136286, 0, 114613,               // 5
                       121396, 0, 152963, 0, 102040,              // 4  + Anacruza de la siguiente parte (43)
                       //----- PARTE 2 -----
                       76408, 0, 76408, 90909, 0, 90909,          // 6
                       80971, 68085, 80971, 102040, 0, 102040,    // 6
                       76408, 0, 76408, 90909, 0, 90909,          // 6
                       80971, 80971, 102040, 0, 102040,           // 5
                       114613, 0, 121396, 114613, 0, 102040,      // 6
                       90909, 80971, 76408, 0,                    // 4
                       90909, 0, 136286, 0, 114613,               // 4
                       121396, 0, 152963, 0,                      // 4  + Anacruza de la siguiente parte (40)
                       1
                      };
uint8_t lenght1[107] = {64,               // suma de fracciones: 1/duration[n]
                        4,                // 0.25 Anacruza
                        4, 4, 4, 4, 4, 4, // 1.5
                        4, 4, 4, 4, 4, 4, // 1.5
                        4, 4, 4, 4, 4, 4, // 1.5
                        2, 4, 4, 4, 4,       // 1.5
                        4, 4, 4, 4, 4, 4, // 1.5
                        4, 4, 4, 4, 4, 4, // 1.5
                        4, 2, 4, 4, 4,    // 1.5
                        4, 2, 4, 4, 4,    // 1.5
                        //----- PARTE 2 -----
                        4, 4, 4, 4, 4, 4, // 1.5
                        4, 4, 4, 4, 4, 4, // 1.5
                        4, 4, 4, 4, 4, 4, // 1.5
                        2, 4, 4, 4, 4,    // 1.5
                        4, 4, 4, 4, 4, 4, // 1.5
                        2, 4, 2, 4,       // 1.5
                        4, 2, 4, 4, 4,    // 1.5
                        4, 2, 4, 4,       // 1.5
                        64
                       };

//------------------------------- KOROBEINIKI -------------------------------------
uint32_t song2[107] = {1,
                       121396, 161943, 152963, 136286, 152963, 161943, // 6
                       181818, 181818, 152963, 121396, 136286, 152963, // 6
                       161943, 161943, 152963, 136286, 121396,         // 5
                       152963, 181818, 181818, 0,                      // 4
                       0, 136286, 114613 , 90909, 102040, 114613,      // 6
                       121396, 0, 152963, 121396, 136286, 152963,      // 6
                       161943, 161943, 152963, 136286, 121396,         // 5
                       152963, 181818, 181818, 0,                      // 4
                       //----- REPETICION -----
                       121396, 161943, 152963, 136286, 152963, 161943, // 6
                       181818, 181818, 152963, 121396, 136286, 152963, // 6
                       161943, 161943, 152963, 136286, 121396,         // 5
                       152963, 181818, 181818, 0,                      // 4
                       0, 136286, 114613 , 90909, 102040, 114613,      // 6
                       121396, 0, 152963, 121396, 136286, 152963,      // 6
                       161943, 161943, 152963, 136286, 121396,         // 5
                       152963, 181818, 181818, 0,                      // 4
                       //------- PUENTE -------
                       121396, 152963, 136286, 161943,
                       152963, 181818 , 192771, 161943,
                       121396, 152963, 136286, 161943,
                       152963, 121396 , 90909 , 90909, 96269, 96269, 0,
                       1
                      };
uint8_t lenght2[107] = {64,               // suma de fracciones: 1/duration[n]
                        2, 4, 4, 2, 4, 4, // 2 en todas las filas
                        2, 4, 4, 2, 4, 4,
                        2, 4, 4, 2, 2,
                        2, 2, 2, 2,
                        4, 2, 4, 2, 4, 4,
                        2, 4, 4, 2, 4, 4,
                        2, 4, 4, 2, 2,
                        2, 2, 2, 2,
                        //----- REPETICION -----
                        2, 4, 4, 2, 4, 4, // 2 en todas las filas
                        2, 4, 4, 2, 4, 4,
                        2, 4, 4, 2, 2,
                        2, 2, 2, 2,
                        4, 2, 4, 2, 4, 4,
                        2, 4, 4, 2, 4, 4,
                        2, 4, 4, 2, 2,
                        2, 2, 2, 2,
                        //------- PUENTE -------
                        1, 1, 1, 1,
                        1, 1, 1, 1,
                        1, 1, 1, 1,
                        2, 2, 2, 2, 1, 2, 4,
                        64
                       };

//--------------------------- FRAGMENTO DE MARIO ----------------------------------

uint32_t song3[107] = {1,
                       80971, 76408, 80971, 76408, 90909, 76408, 80971, 90909,
                       102040, 108108, 102040, 121396, 121396, 136286, 121396,
                       114613, 121396, 114613, 161943, 152963, 136286, 152963,
                       152963, 204081, 323886, 0,
                       1
                      };
uint8_t lenght3[107] = {64,               // suma de fracciones: 1/duration[n]
                        4,
                        4, 4, 4, 2, 4, 4, 4,  // 2
                        4, 4, 4, 2, 4, 4, 4,  // 2
                        4, 4, 4, 2, 4, 4, 2,  // 2
                        4, 2, 2, 2,
                        64
                       };

unsigned long lastTime = 0;

//*****************************************************************************************************
//                                          PROTOTIPOS
//*****************************************************************************************************

void loadSong(int songNum);
void configureTimer1A(void);
uint8_t songLength(uint32_t listOfTones[]);
void loadSong(int songNum);

//*****************************************************************************************************
//                                            SETUP
//*****************************************************************************************************

void setup() {
  Serial.begin(250000);
  configureTimer1A(); // llamado a configuración del timer.

  //pinMode(Pot, INPUT); //CANALES PARA CONVERSION A/D
  pinMode(SW_1, INPUT_PULLUP);
  pinMode(SW_2, INPUT_PULLUP);
  pinMode(Buz, OUTPUT);
  pinMode(PE_2, INPUT_PULLUP); // Pines para control de música.
  
  loadSong(1);     // Se carga la canción 1 en la memoria.
  oneShoot = true; // La primera cancion solo sonará una vez: La intro

  // Iniciando memoria SD
  SPI.setModule(0); // Se elige el puerto SPI numero 0 de la Tiva

  pinMode(CS, OUTPUT); // Pin para Chip Select del maestro
  Serial.println("Iniciando tarjeta");
  if (!SD.begin(CS)) { // Comprueba que la tarjeta SD esté conectada
    Serial.println("initialization failed!");
    noSD = true; // Bandera que indica si hay una tarjeta SD
    return; // Si no la hay, deja de configurar
  } else {
    Serial.println("initialization Succesfull!");
  }
}

//*****************************************************************************************************
//                                            LOOP
//*****************************************************************************************************

void loop() {
  uint8_t song1 = digitalRead(PE_2);
  if (song1 == 0) {
    oneRead = true;
  } else {
    oneRead = false;
  }

  if (millis() > lastTime + interval) { // Funcion usada para depurar
    if (lastTime <= 0) {
      //readSongSD(1);
      //loadSong(2);
    }

    //digitalWrite(LED2, state);
    lastTime = millis();
  }

  //DESACTIVAR CANCION SONANDO
  //if (digitalRead(SW_1) == 0) {
  if (song1 == 1) {
    ROM_TimerDisable(TIMER2_BASE, TIMER_A); // Desactivar Timer 2A
    ROM_TimerDisable(TIMER1_BASE, TIMER_A); // Desactivar  Timer 1A
    ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, freq); // Asignar la frecuencia inicial de la cancion

    //Reiniciar las variables de control de la canción
    beat = 0;
    soundCount = 1;
    durationBase = 128 / duration[soundCount];
    durationBetweenNotes = durationBase * 2   / 10;
    freq = notes[soundCount];
    oneShoot = false;
    stop = true;
  }

  // ACTIVAR CANCION SONANDO
  if (song1 == 0) {
    if (stop) {
      switch (changeSong) {
        case 1:
          changeSong = 2;
          loadSong(changeSong);
          oneShoot = false;
          break;
        case 2:
          changeSong = 3;
          loadSong(changeSong);
          oneShoot = true;
          break;
        case 3:
          changeSong = 1;
          loadSong(changeSong);
          oneShoot = true;
          break;
        default:
          changeSong = 4;
          break;
      }
      stop = false;
      ROM_TimerEnable(TIMER2_BASE, TIMER_A); // Start Timer 2A
      ROM_TimerEnable(TIMER1_BASE, TIMER_A); // Start Timer 1A
    }
  }
}

//*****************************************************************************************************
//                                        INTERRUPCIONES
//*****************************************************************************************************

// Handler (ISR) de la interrupción del Timer
void Timer1AHandler(void) {
  //Required to launch next interrupt

  digitalWrite(Buz, state); // CAMBIAR EL ESTADO DEL BUZZER
  if (state) {
    state = false;
  } else {
    state = true;
  }
  ROM_TimerIntClear(TIMER1_BASE, TIMER_A);

}

void Timer2AHandler(void) {
  //Required to launch next interrupt
  ROM_TimerIntClear(TIMER2_BASE, TIMER_A);

  beat++; // CONTADOR PRINCIPAL DE RITMO

  //durationBase = 128 / duration[soundCount];
  //durationBetweenNotes = durationBase * 2   / 10;

  if (beat >= (128 / duration[soundCount]) - durationBetweenNotes) {
    freq = 0; // Generador de silencios
  }

  if (beat >= durationBase) { // Control de duracion de notas
    beat = 0;
    soundCount++;
    if (soundCount > songLastIndex) {
      soundCount = 1;
      if (oneShoot) {
        ROM_TimerDisable(TIMER1_BASE, TIMER_A);
        ROM_TimerDisable(TIMER2_BASE, TIMER_A);
      }
    }
    durationBase = 128 / duration[soundCount];
    durationBetweenNotes = durationBase * 2   / 10;
    freq = notes[soundCount];
    ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, freq); // El último argumento es el CustomValue
  }
}

//*****************************************************************************************************
//                                          FUNCIONES
//*****************************************************************************************************

// Función que configura el timer
void configureTimer1A(void) {
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // Enable Timer 1 Clock
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2); // Enable Timer 2 Clock
  ROM_IntMasterEnable(); // Enable Interrupts
  ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
  ROM_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic

  // Configure Timer Frequency
  // El tercer argumento ("CustomValue") de la siguiente función debe ser un entero (uint32_t),
  // NO un float. Ese valor determina la frecuencia (y por lo tanto el período) del timer.
  // La frecuecia está dada por: MasterClock / CustomValue
  // En la Tiva C, con las configuraciones actuales, el MasterClock es de 80 MHz.
  // Ejemplos:
  // Si se quiere una frecuencia de 1 Hz, el CustomValue debe ser 80 000 000: 80MHz/80M = 1 Hz
  // Si se quiere una frecuencia de 1 kHz, el CustomValue debe ser 80000: 80MHz/80k = 1 kHz

  ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, 8); // Se inicia el buzzer con una frecuencia inaudible
  ROM_TimerLoadSet(TIMER2_BASE, TIMER_A, 400000); // "Beat" inicial: 80M/N = X Hz =~ X/2 BPM
  // Para Koribeiniki
  // 640000 (slow) / 550000 (presto) / 400000 (fast)

  // Al parecer, no hay función ROM_TimerIntRegister definida. Usar la de memoria FLASH
  // El prototipo de la función es:
  //    extern void TimerIntRegister(uint32_t ui32Base, uint32_t ui32Timer, void (*pfnHandler)(void));
  // Con el tercer argumento se especifica el handler de la interrupción (puntero a la función).
  // Usar esta función evita tener que hacer los cambios a los archivos internos de Energia,
  // sugeridos en la página de donde se tomó el código original.
  TimerIntRegister(TIMER1_BASE, TIMER_A, &Timer1AHandler);
  TimerIntRegister(TIMER2_BASE, TIMER_A, &Timer2AHandler);

  ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Timer 1A Interrupt when Timeout
  ROM_TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT); // Timer 2A Interrupt when Timeout
  ROM_TimerEnable(TIMER1_BASE, TIMER_A); // Start Timer 1A
  ROM_TimerEnable(TIMER2_BASE, TIMER_A); // Start Timer 2A
  ROM_IntEnable(INT_TIMER1A);  // Enable Timer 1A Interrupt
  ROM_IntEnable(INT_TIMER2A);  // Enable Timer 2A Interrupt
}

uint8_t songLength(uint32_t listOfTones[]) { // Detectar el final de la canción cargada a la memoria.
  uint8_t lastIndex = 0, index = 0;
  while (lastIndex == 0) {
    index++;
    if (listOfTones[index] == 1) { // Cuando encuentre el primer 1, habrá encontrado el final
      lastIndex = index;
    }
  }
  return lastIndex;
}

void loadSong(int songNum) { // Cargar cancion a la memoria
  ROM_TimerDisable(TIMER1_BASE, TIMER_A);
  ROM_TimerDisable(TIMER2_BASE, TIMER_A);
  switch (songNum) {
    case 1: // La canción 1 ya está en memoria, solo se carga al array correcto.
      songLastIndex = songLength(song1);
      for (int i = 0; i < songLastIndex; i++) {
        notes[i] = song1[i];
        duration[i] = lenght1[i];
      }
      if (songLastIndex < 110) {
        for (int i = songLastIndex; i < 110; i++) {
          notes[i] = 1;
          duration[i] = 64;
        }
      }
      freq = notes[1];
      songLastIndex = songLength(notes);
      durationBase = 128 / duration[soundCount];
      durationBetweenNotes = durationBase * 2   / 10;
      break;
    case 2:
      readSongSD(1); // Se lee desde la SD.
      songLastIndex = songLength(newSong);
      for (int i = 0; i < songLastIndex; i++) {
        notes[i] = newSong[i]; // Carga cada nota al array de notas y
        duration[i] = newDuration[i]; // cada duracion al array de duraciones
      }
      if (songLastIndex < 110) {
        for (int i = songLastIndex; i < 110; i++) {
          notes[i] = 1;
          duration[i] = 64;
        }
      }
      freq = notes[1];
      songLastIndex = songLength(notes);
      durationBase = 128 / duration[soundCount];
      durationBetweenNotes = durationBase * 2   / 10;
      break;
    case 3: 
      readSongSD(2);
      songLastIndex = songLength(newSong);
      for (int i = 0; i < songLastIndex; i++) {
        notes[i] = newSong[i];
        duration[i] = newDuration[i];
      }
      if (songLastIndex < 110) {
        for (int i = songLastIndex; i < 110; i++) {
          notes[i] = 1;
          duration[i] = 64;
        }
      }
      freq = notes[1];
      songLastIndex = songLength(notes);
      durationBase = 128 / duration[soundCount];
      durationBetweenNotes = durationBase * 2   / 10;
      break;
    default:
      notes[0] = 1;
      notes[1] = 0;
      notes[2] = 1;
      duration[0] = 1;
      duration[1] = 1;
      duration[2] = 1;
      if (songLastIndex < 110) { // Rellena los espacios sobrantes con frecuencias inaudibles.
        for (int i = songLastIndex; i < 110; i++) {
          notes[i] = 1;
          duration[i] = 64;
        }
      }
      break;
  }
  // Reinicia variables de control y carga los datos a los timers.
  beat = 0;
  soundCount = 1;

  ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, freq);

  ROM_TimerEnable(TIMER1_BASE, TIMER_A);
  ROM_TimerEnable(TIMER1_BASE, TIMER_A);
}

void readSongSD(uint8_t index) { // Lee datos desde la SD
  contKeeper = 0;
  contKeeper2 = 0;
  
  // Nombres de los archivos esperados en la SD.
  
  char name1[] = "/S2N.txt";
  char name2[] = "/S2S.txt";
  char name3[] = "/S3N.txt";
  char name4[] = "/S3S.txt";
  switch (index) {
    case 1:
      dir1 = SD.open(name1);
      break;
    case 2:
      dir1 = SD.open(name3);
      break;
    default:
      break;
  }
  // Leyendo Dir1
  if (dir1) {
    while (dir1.available()) {
      temp = dir1.read();

      if (temp == 44) { // Cada vez que recibe una coma, transforma los numeros anteriores recibidos en un entero
        uint32_t multiplicador = 10000000;
        if (contTransformer < 8) { // usado para la suma más adelante. Coloca la unidad "en su lugar" en base 10
          for (int i = contTransformer; i < 8; i++) {
            transformer[i] = 0;
            multiplicador /= 10;
          }
        }

        valor = 0;

        for (int i = 0; i < contTransformer; i++) {
          valor += multiplicador * transformer[i]; // Colocando cada número en donde corresponde en base 10.
          multiplicador /= 10;
        }

        newSong[contKeeper] = valor; // Guardar valor en array de notas

        contTransformer = 0;
        contKeeper++;
      } else  {

        uint32_t translateValue = translate(temp);
        transformer[contTransformer] = translateValue;

        contTransformer++;
      }
    }
  }
  dir1.close();

  // Ajustando manualmente el ultimo valor de notas
  newSong[contKeeper] = 1;

  contTransformer = 0;

  // Leyendo Dir2
  switch (index) {
    case 1:
      dir1 = SD.open(name2);
      break;
    case 2:
      dir1 = SD.open(name4);
      break;
    default:
      break;
  }
  if (dir1) {
    while (dir1.available()) {
      temp = dir1.read();

      if (temp == 44) {
        uint32_t multiplicador = 10000000;
        if (contTransformer < 8) {
          for (int i = contTransformer; i < 8; i++) {
            transformer[i] = 0;
            multiplicador /= 10;
          }
        }

        valor = 0;

        for (int i = 0; i < contTransformer; i++) {
          valor += multiplicador * transformer[i];
          multiplicador /= 10;
        }

        newDuration[contKeeper2] = valor;

        contTransformer = 0;
        contKeeper2++;
      } else  {

        uint32_t translateValue = translate(temp);
        transformer[contTransformer] = translateValue;

        contTransformer++;
      }
    }
  }
  dir1.close();

  // Ajustando manualmente el ultimo valor de duracion
  newDuration[contKeeper2] = 64;


  for (int i = 0; i < contKeeper + 1; i++) {
    Serial.print(newSong[i]);
    Serial.print(" - ");
    Serial.println(newDuration[i]);
  }
}

uint8_t translate(uint32_t data) { // Traduce los datos ascii recibidos a numeros.
  uint8_t contenedor = 0;
  switch (data) {
    case 48:
      contenedor = 0;
      break;
    case 49:
      contenedor = 1;
      break;
    case 50:
      contenedor = 2;
      break;
    case 51:
      contenedor = 3;
      break;
    case 52:
      contenedor = 4;
      break;
    case 53:
      contenedor = 5;
      break;
    case 54:
      contenedor = 6;
      break;
    case 55:
      contenedor = 7;
      break;
    case 56:
      contenedor = 8;
      break;
    case 57:
      contenedor = 9;
      break;
  }
  return contenedor;
}
