//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
   Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
   Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
   Con ayuda de: José Guerra
   IE3027: Electrónica Digital 2 - 2019
*/
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// Librerías y definiciones
//***************************************************************************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1

#define PB1 PUSH1
#define PB2 PUSH2
#define PB3 PE_3
#define PB4 PE_4
#define PIN1 PA_2
#define PIN2 PA_3
#define PIN3 PA_4

//***************************************************************************************************************************************
// Definición de variables
//***************************************************************************************************************************************

// lectura del estado de pushbuttons
int PB1State = 1;
int PB2State = 1;
int PB3State = 1;
int PB4State = 1;
int x_1 = 20;
int y_1 = 20;
int nSong = 0;

// variables de la pantalla de inicio
// variable de antirebote de botones
int pressed1 = 0;
int pressed2 = 0;

// variable para selección de nivel
int level = 0;

// color de llenado según el nivel
int fill_color;

// strings para mostrar texto en la pantalla de inicio
String TITLE1 = "FLOPPY";
String TITLE2 = "BIRDN'T";
String play1 = "PLAY MAP 1";
String play2 = "PLAY MAP 2";
String play3 = "PLAY MAP 3";
String text2 = "LICENSED BY";
String text3 = "NITENGO OF AMERICA INC";
String indicador = "+";

// string para almacenar el valor de las vida de cada jugador
char str_vidaj1[1];
char str_vidaj2[1];

// posiciones de J1 y J2 en pantalla
int posx1 = 20;
int posy1 = 100;
int posx2 = 60;
int posy2 = 100;

// Guardar posiciones de las anclas en variables globales
// anclasY_obs[0]: Ancla de Liana
// anclasY_obs[1]: Ancla de Arbol
int anclasY_obs[2] = {0, 0};

// anclas_aviones[0]: Ancla de J1
// anclas_aviones[1]: Ancla de J2
// Solo se necesita su coordenada Y. La coordenada x la entregan las
// variables posx1 y posx2
int anclas_aviones[2] = {0, 0};

// valores pseudoaleatorios para los obstáculos
int randValues[] = {0, 3, 1, 6, 5, 2, 3, 4, 1, 2,
                    0, 6, 6, 2, 1, 4, 0, 0, 3, 2, 
                    0, 6, 0, 5, 2, 0, 1, 4, 3, 6, 
                    0, 3, 0, 3, 1, 2, 2, 4, 5, 6};
int maxPRandom = 39;
int randomControl = 0;
int obstaculosPorVelocidad[] = {3, 3, 3, 4, 4, 4, 4, 5, 7, 9,
                    10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
int obsControl = 0;
bool changeValue = false;

// Coordenada x de los obstáculos
int x_coord_obs = 0;

// bandera de colision de los jugadores
bool colJ1 = false, colJ2 = false;

// velocidad de barrido de obstáculos
int movSpeed = 1;

// altura de obstáculos
int h_tree = 81, h_liana = 78;

// Variables del sistema de vidas
uint8_t vidasJ1 = 3, vidasJ2 = 3;
bool impactoPrevioJ1 = false, impactoPrevioJ2 = false;
bool reiniciarObstaculos = false;

// Variables para control de estados del juego (jugando, fin del juego, reinicio, etc)
bool jugando = false, ganador = false, apagarControlJ1 = false, apagarControlJ2 = false; 


int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};

//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);

/// Funciones definidas para el juego
void x_move(unsigned int rightButtonState, unsigned int leftButtonState, unsigned int xlim1, unsigned int xlim2, unsigned int width, unsigned int height, unsigned char bitmap[]);
void y_move(unsigned int upButtonState, unsigned int downButtonState, unsigned int ylim1, unsigned int ylim2, unsigned int width, unsigned int height, unsigned char bitmap[]);
int jump_1(int buttonState, int ylim1, int width, int height, unsigned char bitmap[]);
int jump_2(int buttonState, int ylim1, int width, int height, unsigned char bitmap[]);
int fall_1(int ylim2, int width, int height, unsigned char bitmap[]);
int fall_2(int ylim2, int width, int height, unsigned char bitmap[]);
void init(void);

extern uint8_t getready[];
extern uint8_t j1win[];
extern uint8_t j2win[];

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  //--------------------------------------------------
  // Obtención de numero aleatorio usando analogRead
  //--------------------------------------------------
  int randVal = analogRead(PE_2);
  randomControl = map(randVal, 600, 900, 0, 38);
  if (randomControl < 0){
    randomControl = 0;
  } else if (randomControl > 38) {
    randomControl = 38;
  }
  
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  //Serial.println("Inicio");
  LCD_Init();
  //Serial.println(randomControl);
  // configuración de botones
  pinMode(PB1, INPUT_PULLUP);
  pinMode(PB2, INPUT_PULLUP);
  pinMode(PB3, INPUT);
  pinMode(PB4, INPUT);
  
  pinMode(PIN1, OUTPUT);
  pinMode(PIN2, OUTPUT);
  pinMode(PIN3, OUTPUT);

  FillRect(0, 0, 320, 240, 0x0000);   // llenado de pantalla de inicio
  init();                             // llamada al texto e imágenes de inicio
  
  LCD_Print(indicador, 105, 140, 1, 0xFBE4, 0x0000);  // indicador de selección
    digitalWrite(PIN1, HIGH);
    digitalWrite(PIN1, LOW);
    digitalWrite(PIN1, HIGH);
    digitalWrite(PIN1, LOW);
    
  while(PB1State == 1){
    PB1State = digitalRead(PB1);
    PB2State = digitalRead(PB2);
    LCD_Print(indicador, 105, 140+level, 1, 0xFBE4, 0x0000);
    FillRect(105, 125+60, 5, 10, 0x0000);
    //Serial.println(level);
    if(level > 30){
      level = 0;
    }
    if (PB2State == HIGH){
      pressed1 = 1;
    }
    if (PB2State == LOW && pressed1 == 1){
      level = level + 15;   
      FillRect(105, 125+level, 5, 10, 0x0000);
      pressed1 = 0;
    }
  }
  //--------------------------------------------------
  // Asignación de colores según el mapa seleccionado
  //--------------------------------------------------
  if (level == 0){
    //Serial.println("Mapa 1 seleccionado");
    fill_color = 0x7e3d;  // COLOR HEX: 78c6ec
  }
  if (level == 15){
    //Serial.println("Mapa 2 seleccionado");
    fill_color = 0x21C8;  // COLOR HEX: 203945
  }
  if (level == 30){
    //Serial.println("Mapa 3 seleccionado");
    fill_color = 0xCC69;  // COLOR HEX: ef870c
  }
  //-------------------------------------------------

  //----------------------------------------------
  // Creación del cielo y el suelo (fondo)       
  //----------------------------------------------
  FillRect(0, 0, 320, 240, fill_color); 

  for (int x = 0; x < 319; x++) {
    LCD_Bitmap(x, 210, 80, 30, tile1); // Imprimir suelo
    x += 79;
  }
  //----------------------------------------------

  //----------------------------------------------
  // Marcador de vidas de ambos jugadores
  //----------------------------------------------
  FillRect(10, 215, 115, 20, 0xDB25); 
  H_line(10, 215, 115, 0x0000);
  H_line(10, 235, 115, 0x0000);
  V_line(10, 215, 20, 0x0000);
  V_line(10+115, 215, 20, 0x0000);
  LCD_Bitmap(20, 219, 10, 9, heart);
  String vidaj1 = "J1:";
  String vidaj2 = "J2:";
  LCD_Print(vidaj1, 35, 218, 1, 0xffff, 0xDB25);
  LCD_Print(vidaj2, 80, 218, 1, 0xffff, 0xDB25);

  sprintf(str_vidaj1,"%1d", vidasJ1);
  sprintf(str_vidaj2,"%1d", vidasJ2);
  LCD_Print(str_vidaj1, 60, 218, 1, 0xffff, 0xDB25);
  FillRect(65, 218, 10, 15, 0xDB25); 
  LCD_Print(str_vidaj2, 105, 218, 1, 0xffff, 0xDB25);
  //----------------------------------------------

  //------------------------------------------------------------
  // Posiciones iniciales de los jugadores y mensaje de inicio
  //------------------------------------------------------------
  LCD_Bitmap(posx1, posy1, 35, 28, planej1);
  LCD_Bitmap(posx2, posy2, 35, 28, planej2);
  LCD_Bitmap(130, 100, 150, 40, getready);
  delay(1000);
  FillRect(130, 100, 150, 40, fill_color); 
  digitalWrite(PIN1, HIGH);
  delay(10);
  digitalWrite(PIN1, LOW);
  
  //------------------------------------------------------------
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {

  PB1State = digitalRead(PB1);
  PB2State = digitalRead(PB2);

  x_move_obs(&x_coord_obs, &anclasY_obs[1], &anclasY_obs[0]); // Mueve los obstáculos específicos de un nivel

  // Control de movimientos y desactivación de controles cuando un jugador pierde 
  if (PB1State == 0 && !apagarControlJ2){
    anclas_aviones[1] = jump_2(PB1State, 10, 35, 28, planej2);
  } else {
    if (apagarControlJ2){
      FillRect(posx2, anclas_aviones[1], 38, 28, fill_color);
    } else {
      anclas_aviones[1] = fall_2(180, 35, 28, planej2);
    }
  }
  if (PB2State == 0 && !apagarControlJ1){
    anclas_aviones[0] = jump_1(PB2State, 10, 35, 28, planej1);
  } else {
    if (apagarControlJ1){
      FillRect(posx1, anclas_aviones[0], 38, 28, fill_color);
    } else {
      anclas_aviones[0] = fall_1(180, 35, 28, planej1);
    }
  }
  
  //depuracion(); // Linea para mostrar parametros de depuración.
  
  // -------- Detección de colisión ----------
  // Previamente se definieron anclas, que son la coordenada inicial a partir de la cual se imprimió el objeto.
  // Si se detecta que el Area del ancla del avion colisiona con el area del ancla de algun obstáculo, entonces
  // las variables serán de valor "true". En caso contrario, serán de valor "false". 
  // Estas variables controlan un if más adelante. Por lo que será alli donde se programará cualquier funcionalidad necesaria. 
  // Los numeros que se suman a las coordenadas son parametros de ajuste, para hacer que la colision no sea demasiado precisa
  colJ1 = (anclas_aviones[0] + 6 < anclasY_obs[0] || anclas_aviones[0] + 22 > anclasY_obs[1]) && 
          !(posx1 + 50 < x_coord_obs || posx1 + 8 > x_coord_obs)
  ;
  colJ2 = (anclas_aviones[1] + 6 < anclasY_obs[0] || anclas_aviones[1] + 22 > anclasY_obs[1]) &&
          !(posx2 + 50 < x_coord_obs || posx2 + 8 > x_coord_obs)
  ;
  // -------- --------------------- ----------  
  if (x_coord_obs < 120) {
    reiniciarObstaculos = true;  
    if (colJ1){
      if (!impactoPrevioJ1){
        vidasJ1--;
        if (vidasJ1 > 5){
          vidasJ1 = 0;
        }
        sprintf(str_vidaj1,"%1d", vidasJ1);
        LCD_Print(str_vidaj1, 60, 218, 1, 0xffff, 0xDB25);
        FillRect(65, 218, 10, 15, 0xDB25); 
      }
      impactoPrevioJ1 = true;
    }
    if (colJ2){
      if (!impactoPrevioJ2){
        vidasJ2--;
        if (vidasJ1 > 5){
          vidasJ1 = 0;
        }
        sprintf(str_vidaj2,"%1d", vidasJ2);
        LCD_Print(str_vidaj2, 105, 218, 1, 0xffff, 0xDB25);
      }
      impactoPrevioJ2 = true;
    }
  } 
  
  // Detección de un ganador al reiniciarse la posición de los obstáculos
  if (!reiniciarObstaculos){
    if (vidasJ1 == 0 && vidasJ2 == 0){
      LCD_Print("DRAW", 60, 0, 1, 0xffff, 0x01EB); // Para depuración
      apagarControlJ1 = true;
      apagarControlJ2 = true;
      movSpeed = 0;
      digitalWrite(PIN1, HIGH);
      delay(10);
      digitalWrite(PIN1, LOW);
      delay(1000);
      while(true){}
    } else if (vidasJ1 > 0 && vidasJ2 == 0){
      LCD_Print("-J1-", 60, 0, 1, 0xffff, 0x01EB); // Para depuración
      apagarControlJ2 = true;
      movSpeed = 0;
      digitalWrite(PIN1, HIGH);
      delay(10);
      digitalWrite(PIN1, LOW);
      LCD_Bitmap(130, 100, 120, 40, j1win); 
      delay(1000);
      while(true){}
    } else if (vidasJ1 ==  0 && vidasJ2 > 0){
      LCD_Print("-J2-", 60, 0, 1, 0xffff, 0x01EB); // Para depuración
      apagarControlJ1 = true;
      movSpeed = 0;
      digitalWrite(PIN1, HIGH);
      delay(10);
      digitalWrite(PIN1, LOW);
      LCD_Bitmap(130, 100, 120, 40, j2win); 
      delay(1000);
      while(true){}
    }
  }
} // end Loop()

//***************************************************************************************************************************************
// Funciones de movimiento
//***************************************************************************************************************************************
//****************************************
// Inicio
//****************************************
// :::: Despliega mensaje de inicio y titulo

void init(void){
  LCD_Print(play1, 118, 140, 1, 0xFBE4, 0x0000);
  LCD_Print(play2, 118, 155, 1, 0xFBE4, 0x0000);
  LCD_Print(play3, 118, 170, 1, 0xFBE4, 0x0000);
  LCD_Print(text2, 114, 200, 1, 0xffff, 0x0000);
  LCD_Print(text3, 70, 210, 1, 0xffff, 0x0000);
  LCD_Bitmap(220, 80, 45, 36, plane_init);
  LCD_Bitmap(60, 80, 150, 37, title);
};
  
//****************************************
// Caída
//****************************************

// Jugador 1
int fall_1(int ylim2, int width, int height, unsigned char bitmap[]) {
  int anim = (posy1 / 35) % 2;
  if (posy1 < ylim2) {
    posy1 = posy1 + 2;
  }
  LCD_Bitmap(posx1, posy1, width, height, bitmap);
  LCD_Sprite(posx1 + width, posy1 + 3, 3, 21, helice, 5, anim, 0, 0);
  H_line(posx1, posy1 - 1, width, fill_color);
  H_line(posx1, posy1 - 1 - 1, width, fill_color);
  return posy1;
};

// Jugador 2
int fall_2(int ylim2, int width, int height, unsigned char bitmap[]) {
  int anim = (posy2 / 35) % 2;
  if (posy2 < ylim2) {
    posy2 = posy2 + 2;
  }
  LCD_Bitmap(posx2, posy2, width, height, bitmap);
  LCD_Sprite(posx2 + width, posy2 + 3, 3, 21, helice, 5, anim, 0, 0);
  H_line(posx2, posy2 - 1, width, fill_color);
  H_line(posx2, posy2 - 1 - 1, width, fill_color);
  return posy2;
};

//****************************************
// Salto
//****************************************
// Jugador 1
int jump_1(int buttonState, int ylim1, int width, int height, unsigned char bitmap[]) {
  int anim = (posy1 / 35) % 2;
  if (buttonState == 0 & posy1 - 5 > ylim1) {
    posy1 = posy1 - 5;
    H_line(posx1, posy1 + height, width, fill_color);
    LCD_Bitmap(posx1, posy1, width, height, bitmap);
    LCD_Sprite(posx1 + width, posy1 + 3, 3, 21, helice, 5, anim, 0, 0);
    for (int i = 0; i < 5; i++) {
      H_line(posx1, posy1 + 28 + i, width, fill_color);
    }
  }
  V_line(posx1 + width+1, posy1 + 28, 3, fill_color);
  return posy1;
};

// Jugador 2
int jump_2(int buttonState, int ylim1, int width, int height, unsigned char bitmap[]) {
  int anim = (posy2 / 35) % 2;
  if (buttonState == 0 & posy2 - 5 > ylim1) {
    posy2 = posy2 - 5;
    H_line(posx2, posy2 + height, width, fill_color);
    LCD_Bitmap(posx2, posy2, width, height, bitmap);
    LCD_Sprite(posx2 + width, posy2 + 3, 3, 21, helice, 5, anim, 0, 0);
    for (int i = 0; i < 5; i++) {
      H_line(posx2, posy2 + 28 + i, width, fill_color);
    }
  }
  V_line(posx2 + width + 1, posy2 + 28, 3, fill_color);
  return posy2;
};
//****************************************
// Movimiento horizontal de los obstaculos
//
// La función retorna valores int para
// entregar coordenadas al ciclo principal.
//****************************************
void x_move_obs(int *xcoordObs, int *ycoordBitmap1, int *ycoordBitmap2) {
  
  x_1 += movSpeed; // Control de velocidad de obstaculos
  
  // Esta instrucción puede provocar que los obstaculos no se borren adecuadamente al salir de la pantalla.
  if (x_1 > 340 ) { // Reinicio de la variable contadora de coordenada x
    x_1 = 0;
    reiniciarObstaculos = false; // Reinicio de variables de control de obstáculos y velocidad
    impactoPrevioJ1 = false;
    impactoPrevioJ2 = false;
    randomControl++;
    if (randomControl > maxPRandom){
      randomControl = 0;
    }
    obsControl++;
    if (obsControl > obstaculosPorVelocidad[movSpeed]){
      obsControl = 0;
      if (movSpeed != 15){
        movSpeed++;
      }
    }
  }
  int tempHeightTree = 210, tempHeightLiana = 0;
  uint8_t valorPrincipal = randValues[randomControl]; 
  if (valorPrincipal == 0){ 
    valorPrincipal = 1; 
  }
  // Imprimir obstaculos
  //LCD_Bitmap(320 - x_1, 210 - height1, width1, height1, bitmap1);
  //LCD_Bitmap(320 - x_1, 0, width2, height2, bitmap2);
  LCD_Bitmap(320 - x_1, tempHeightTree - 28, 20, 28, tree_b3);
  tempHeightTree -= 28;
  for (uint8_t i = 1; i < valorPrincipal*3; i++){
    LCD_Bitmap(320 - x_1, tempHeightTree - 6, 20, 6, tree_b2);
    tempHeightTree -= 6;
  }
  LCD_Bitmap(320 - x_1, tempHeightTree - 11, 20, 11, tree_b1);
  tempHeightTree -= 11;

  LCD_Bitmap(320 - x_1, 0, 20, 3, liana_b1);
  tempHeightLiana += 3;
  for (uint8_t i = 1; i < 6 - valorPrincipal; i++){
    LCD_Bitmap(320 - x_1, tempHeightLiana, 20, 23, liana_b2);
    tempHeightLiana += 23;
  }
  LCD_Bitmap(320 - x_1, tempHeightLiana, 20, 15, liana_b3);
  tempHeightLiana += 15;
  
  // Borrar rastro de la pantalla. Borra más lineas con el incremento de la variable "speed"
  for (uint8_t i = 1; i < movSpeed + 1; ++i) {
    V_line(340 - x_1 - 2 + i, 0, 210, fill_color);
  }
  
  // Valores de retorno
  *xcoordObs = 340 - x_1 ;
  *ycoordBitmap1 = tempHeightTree;
  *ycoordBitmap2 = tempHeightLiana;
}
//****************************************
// Función de depuración
//****************************************
void depuracion(void) {
  // -------- Lineas de depuración ----------
  // Despliegan visualmente la coordenada x de los obstáculos y lo muestran en un cuadro en pantalla.
  V_line(100, 0, 240, 0x0000);
  char str[3];
  sprintf(str,"%d", x_coord_obs);
  FillRect(0, 0, 100, 12, 0x01EB);
  LCD_Print(str, 0, 0, 1, 0xffff, 0x01EB);
  // -------- -------------------- ----------
  if (x_coord_obs > 120) {
    LCD_Print("---", 25, 0, 1, 0xffff, 0x01EB); // Para depuración
  } else {    
    LCD_Print("COL", 25, 0, 1, 0xffff, 0x01EB); // Para depuración
    if (colJ1){
      LCD_Print("J1", 60, 0, 1, 0xffff, 0x01EB); // Para depuración
    }
    if (colJ2){
      LCD_Print("J2", 80, 0, 1, 0xffff, 0x01EB); // Para depuración
    }
  }
}
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
/*void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
  }
*/

void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + w;
  y2 = y + h;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = w * h * 2 - 1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);

      //LCD_DATA(bitmap[k]);
      k = k - 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  //Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    //Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      if(level == 15){
        if(bitmap[k] == 0x7e && bitmap[k+1] == 0x3d){ 
          LCD_DATA(0x21); // 0x21C8
          LCD_DATA(0xc8);
        }
        else if ((bitmap[k] == 0x1e && bitmap[k+1] == 0x26) || (bitmap[k] == 0x2d && bitmap[k+1] == 0xc7)|| (bitmap[k] == 0x25 && bitmap[k+1] == 0xa7)){
          LCD_DATA(0x0a); // 0x2365 512D
          LCD_DATA(0xa2);
        }
        else if ((bitmap[k] == 0x4e && bitmap[k+1] == 0xe6) || (bitmap[k] == 0x4f && bitmap[k+1] == 0xe5)|| (bitmap[k] == 0x47 && bitmap[k+1] == 0x63)){
          LCD_DATA(0x2c); // 0x2365 512D
          LCD_DATA(0x62);
        }
        else if ((bitmap[k] == 0x56 && bitmap[k+1] == 0xca)){
          LCD_DATA(0x13); // 0x2365 512D
          LCD_DATA(0x35);
        }
        else if ((bitmap[k] == 0x05 && bitmap[k+1] == 0x29)){
          LCD_DATA(0x01); // 0x2365 512D
          LCD_DATA(0x06);
        }
        else{
          LCD_DATA(bitmap[k]);
          LCD_DATA(bitmap[k + 1]);
        }
      }
      if(level == 30){
        if(bitmap[k] == 0x7e && bitmap[k+1] == 0x3d){ 
          LCD_DATA(0xcc); // 0xEC21
          LCD_DATA(0x69);
        }
        else if ((bitmap[k] == 0x1e && bitmap[k+1] == 0x26) || (bitmap[k] == 0x2d && bitmap[k+1] == 0xc7)|| (bitmap[k] == 0x25 && bitmap[k+1] == 0xa7)){
          LCD_DATA(0x9a); // 0x2365 512D
          LCD_DATA(0xe4);
        }
        else if ((bitmap[k] == 0x4e && bitmap[k+1] == 0xe6) || (bitmap[k] == 0x4f && bitmap[k+1] == 0xe5)|| (bitmap[k] == 0x47 && bitmap[k+1] == 0x63)){
          LCD_DATA(0xfc); // 0x2365 512D
          LCD_DATA(0x21);
        }
        else if ((bitmap[k] == 0x56 && bitmap[k+1] == 0xca)){
          LCD_DATA(0x98); // 0x2365 512D
          LCD_DATA(0xe3);
        }
        else if ((bitmap[k] == 0x05 && bitmap[k+1] == 0x29)){
          LCD_DATA(0x40); // 0x2365 512D
          LCD_DATA(0x20);
        }
        else{
          LCD_DATA(bitmap[k]);
          LCD_DATA(bitmap[k + 1]);
        }
      }
      if(level == 0){
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
      }
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        if(level == 15){
        if(bitmap[k] == 0x7e && bitmap[k+1] == 0x3d){ 
          LCD_DATA(0x21); // 0x21C8
          LCD_DATA(0xc8);
        }
        else{
          LCD_DATA(bitmap[k]);
          LCD_DATA(bitmap[k + 1]);
        }
      }
      if(level == 30){
        if(bitmap[k] == 0x7e && bitmap[k+1] == 0x3d){ 
          LCD_DATA(0xcc); // 0xEC21
          LCD_DATA(0x69);
        }
        else{
          LCD_DATA(bitmap[k]);
          LCD_DATA(bitmap[k + 1]);
        }
      }
      if(level == 0){
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
      }
        k = k + 2;
      }
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
