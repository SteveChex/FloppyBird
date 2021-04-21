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
#define Pot PD_0
#define Buz PD_1

#define MHZ80 80000000

#define interval 100

uint32_t freq = 0;
bool state = false, sta2 = false;
uint8_t soundCount = 0, durationCount = 0, beat = 0;

uint32_t notes[12] = {121396, 161943, 152963, 136286, 152963, 161943,
                      181818, 181818, 152963, 121396, 136286, 152963
                     };
uint8_t duration[12] = {2, 1, 1, 2, 1, 1,
                        2, 1, 1, 2, 1, 1
                       };

unsigned long lastTime = 0;

//*****************************************************************************************************
//                                            SETUP
//*****************************************************************************************************

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
  configureTimer1A(); // llamado a configuración del timer.

  pinMode(Pot, INPUT); //CANALES PARA CONVERSION A/D

  pinMode(Buz, OUTPUT);


  Serial.begin(115200);
  delay(500);
  freq = notes[0];
}

//*****************************************************************************************************
//                                            LOOP
//*****************************************************************************************************

void loop() {
  if (millis() > lastTime + interval) {

    //digitalWrite(LED2, state);
    lastTime = millis();
  }
  Serial.println(freq);
  //digitalWrite(LED, HIGH);
  delay(100);
  //digitalWrite(LED, LOW);
  delay(100);
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
  freq = notes[soundCount];
  ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, freq); // El último argumento es el CustomValue
  ROM_TimerIntClear(TIMER1_BASE, TIMER_A);

}

void Timer2AHandler(void) {
  //Required to launch next interrupt
  ROM_TimerIntClear(TIMER2_BASE, TIMER_A);
  if (sta2) {
    sta2 = false;
  } else {
    sta2 = true;
  }
  digitalWrite(LED2, sta2);
  beat++;
  if (beat >= 128) {
    beat = 0;
    soundCount++;
    if (soundCount > 11) {
      soundCount = 0;
    }
  }
}

//*****************************************************************************************************
//                                          FUNCIONES
//*****************************************************************************************************


// Función que configura el timer (1A en este ejemplo)
void configureTimer1A() {
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // Enable Timer 1 Clock
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2); // Enable Timer 1 Clock
  ROM_IntMasterEnable(); // Enable Interrupts
  ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
  ROM_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic

  // Configure Timer Frequency
  // El tercer argumento ("CustomValue") de la siguiente función debe ser un entero (uint32_t),
  // NO un float. Ese valor determina la frecuencia (y por lo tanto el período) del timer.
  // La frecuecia está dada por: MasterClock / CustomValue
  // En el Tiva C, con las configuraciones actuales, el MasterClock es de 80 MHz.
  // Ejemplos:
  // Si se quiere una frecuencia de 1 Hz, el CustomValue debe ser 80 000 000: 80MHz/80M = 1 Hz
  // Si se quiere una frecuencia de 1 kHz, el CustomValue debe ser 80000: 80MHz/80k = 1 kHz
  ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, 80000); // El último argumento es el CustomValue
  ROM_TimerLoadSet(TIMER2_BASE, TIMER_A, 640000); // El último argumento es el CustomValue

  // Al parecer, no hay función ROM_TimerIntRegister definida. Usar la de memoria FLASH
  // El prototipo de la función es:
  //    extern void TimerIntRegister(uint32_t ui32Base, uint32_t ui32Timer, void (*pfnHandler)(void));
  // Con el tercer argumento se especifica el handler de la interrupción (puntero a la función).
  // Usar esta función evita tener que hacer los cambios a los archivos internos de Energia,
  // sugeridos en la página de donde se tomó el código original.
  TimerIntRegister(TIMER1_BASE, TIMER_A, &Timer1AHandler);
  TimerIntRegister(TIMER2_BASE, TIMER_A, &Timer2AHandler);

  ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Timer 1A Interrupt when Timeout
  ROM_TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT); // Timer 1A Interrupt when Timeout
  ROM_TimerEnable(TIMER1_BASE, TIMER_A); // Start Timer 1A
  ROM_TimerEnable(TIMER2_BASE, TIMER_A); // Start Timer 1A
  ROM_IntEnable(INT_TIMER1A);  // Enable Timer 1A Interrupt
  ROM_IntEnable(INT_TIMER2A);  // Enable Timer 1A Interrupt
}
