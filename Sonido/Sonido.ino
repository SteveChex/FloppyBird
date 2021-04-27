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

//*****************************************************************************************************
//                              VARIABLES, CONSTANTES Y DEFINICIONES
//*****************************************************************************************************

#define LED RED_LED
#define LED2 BLUE_LED
#define Buz PE_3
#define SW_1 PF_0
#define SW_2 PF_4

#define MHZ80 80000000

#define interval 5000

uint32_t freq = 0;
bool state = false, sta2 = false, stop = false;
uint8_t soundCount = 1, durationCount = 0, beat = 0, durationBase = 0, durationBetweenNotes = 0,
        songLastIndex = 0;
uint8_t changeSong = 1;


//------------------------------- CONTENEDORES ------------------------------------

uint32_t notes[110] = {};
uint8_t duration[110] = {};

//-------------------------Ni idea de como se llama -------------------------------

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

//--------------------------- CANCION DE GANADOR ----------------------------------

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
  Serial.begin(115200);
  delay(500);

  configureTimer1A(); // llamado a configuración del timer.

  //pinMode(Pot, INPUT); //CANALES PARA CONVERSION A/D
  pinMode(SW_1, INPUT_PULLUP);
  pinMode(SW_2, INPUT_PULLUP);
  pinMode(Buz, OUTPUT);
  loadSong(1);
}

//*****************************************************************************************************
//                                            LOOP
//*****************************************************************************************************

void loop() {
  if (millis() > lastTime + interval) {
    if (lastTime <= 0) {
      Serial.println("CAMBIANDO!");
      //loadSong(2);
    }

    //digitalWrite(LED2, state);
    lastTime = millis();
  }

  if (digitalRead(SW_1) == 0) {
    ROM_TimerDisable(TIMER2_BASE, TIMER_A); // Desactivar Timer 2A
    ROM_TimerDisable(TIMER1_BASE, TIMER_A); // Desactivar  Timer 1A
    ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, freq); // Asignar la frecuencia inicial de la cancion

    //Reiniciar las variables de control de la canción
    beat = 0;
    soundCount = 1;
    durationBase = 128 / duration[soundCount];
    durationBetweenNotes = durationBase * 2   / 10;
    freq = notes[soundCount];
    stop = true;
  }

  if (digitalRead(SW_2) == 0) {
    if (stop) {
      switch (changeSong) {
        case 1:
          changeSong = 2;
          Serial.println(changeSong);
          loadSong(changeSong);
          break;
        case 2:
          changeSong = 3;
          Serial.println(changeSong);
          loadSong(changeSong);
          break;
        case 3:
          changeSong = 1;
          Serial.println(changeSong);
          loadSong(changeSong);
          break;
        default:
          changeSong = 4;
          Serial.print("DEFAULT");
          Serial.println(changeSong);
          break;
      }
      stop = false;
    }
    ROM_TimerEnable(TIMER2_BASE, TIMER_A); // Start Timer 2A
    ROM_TimerEnable(TIMER1_BASE, TIMER_A); // Start Timer 1A
  }

}

//*****************************************************************************************************
//                                        INTERRUPCIONES
//*****************************************************************************************************

// Handler (ISR) de la interrupción del Timer
void Timer1AHandler(void) {
  //Required to launch next interrupt

  digitalWrite(Buz, state);
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

  beat++;

  //durationBase = 128 / duration[soundCount];
  //durationBetweenNotes = durationBase * 2   / 10;

  if (beat >= (128 / duration[soundCount]) - durationBetweenNotes) {
    freq = 0;
    //ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, freq); // El último argumento es el CustomValue
  }

  if (beat >= durationBase) {
    beat = 0;
    soundCount++;
    if (soundCount > songLastIndex) {
      soundCount = 1;
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
  ROM_TimerLoadSet(TIMER2_BASE, TIMER_A, 550000); // "Beat" inicial: 80M/N = X Hz =~ X/2 BPM
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

uint8_t songLength(uint32_t listOfTones[]) {
  uint8_t lastIndex = 0, index = 0;
  while (lastIndex == 0) {
    index++;
    if (listOfTones[index] == 1) {
      lastIndex = index;
    }
  }
  return lastIndex;
}

void loadSong(int songNum) {
  ROM_TimerDisable(TIMER1_BASE, TIMER_A);
  ROM_TimerDisable(TIMER2_BASE, TIMER_A);
  switch (songNum) {
    case 1:
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
      /*for (int i = 0; i < 110; i++) {
        Serial.print(notes[i]);
        Serial.print("-");
        Serial.println(duration[i]);
        }*/
      freq = notes[1];
      songLastIndex = songLength(notes);
      durationBase = 128 / duration[soundCount];
      durationBetweenNotes = durationBase * 2   / 10;
      break;
    case 2:
      songLastIndex = songLength(song2);
      for (int i = 0; i < songLastIndex; i++) {
        notes[i] = song2[i];
        duration[i] = lenght2[i];
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
      songLastIndex = songLength(song3);
      for (int i = 0; i < songLastIndex; i++) {
        notes[i] = song3[i];
        duration[i] = lenght3[i];
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
      if (songLastIndex < 110) {
        for (int i = songLastIndex; i < 110; i++) {
          notes[i] = 1;
          duration[i] = 64;
        }
      }
      break;
  }
  beat = 0;
  soundCount = 1;

  ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, freq);

  ROM_TimerEnable(TIMER1_BASE, TIMER_A);
  ROM_TimerEnable(TIMER1_BASE, TIMER_A);
}
