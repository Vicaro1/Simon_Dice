// Zona de inclusion de librerias.
#include <Adafruit_GFX.h>     // Libreria de graficos general.
#include <Adafruit_ST7735.h>  // Libreria de harware especifica para el controlador ST7735.
#include <SPI.h>              // Libreria para el protocolo de comunicacion SPI.

// Definicion de los pines relacionados con la pantalla TFT.
const uint8_t TFT_SCLK = 13;  
const uint8_t TFT_MOSI = 11;
const uint8_t TFT_DC = 9;
const uint8_t TFT_RST = 8;   
const uint8_t TFT_CS = 10;

// Se instancia la pantalla mediante la llamada correspondiente. 
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Constantes que ayudaran a controlar los botones y los LEDs.
const uint8_t APAGAR_LEDS = 0;   
const uint8_t ELECCION_NULA = 0; 

// Valor que tienen los diferentes botones al pulsarlos.
const uint8_t ELECCION_ROJO = 1;
const uint8_t ELECCION_VERDE = 2;
const uint8_t ELECCION_AZUL = 4;
const uint8_t ELECCION_AMARILLO = 8;

// Pines conectados a cada LED.
const uint8_t LED_ROJO = 15;
const uint8_t LED_VERDE = 16;
const uint8_t LED_AZUL = 17;
const uint8_t LED_AMARILLO = 14;

// Pines conectados a cada boton.
const uint8_t BOTON_RESET = 2;
const uint8_t BOTON_ROJO = 6;
const uint8_t BOTON_VERDE = 5;
const uint8_t BOTON_AZUL = 4;
const uint8_t BOTON_AMARILLO = 7;

// Pin conectado al zumbador.
const uint8_t ZUMBADOR = 3;

// Se definen el numero de rondas a ganar y el tiempo maximo que puede estar el jugador sin pulsar un boton.
const uint8_t RONDAS_TOTALES = 2+3; 
const uint16_t TIEMPO_LIMITE_INPUT = 3000;

// Se define la velocidad de los LEDs y la dificultad.
uint16_t g_velocidad_leds = 200;
uint16_t g_dificultad = 0;
uint8_t g_vidas = 3;

// Se define el array donde se guardara la combinacion que se tiene que acertar y la ronda actual. 
uint8_t g_secuencia_generada[20]; 
uint8_t g_ronda_actual = 0; 

// Se prepara la ISR donde se reseteara la ronda actual, las vidas y actualizara la pantalla.
void ISR_activar_reset()
{
  g_ronda_actual = 0;
  g_vidas = 3;
  pantalla_vidas();
}


//*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=SETUP=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*//
void setup()
{
  // Los botones tendran un pullup interno y seran pines de entrada.
  pinMode(BOTON_RESET, INPUT_PULLUP);
  pinMode(BOTON_ROJO, INPUT_PULLUP);
  pinMode(BOTON_VERDE, INPUT_PULLUP);
  pinMode(BOTON_AZUL, INPUT_PULLUP);
  pinMode(BOTON_AMARILLO, INPUT_PULLUP);
  
  // Los leds y el zumbador seran pines de salida.
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(ZUMBADOR, OUTPUT);

  // Se configura la interrupcion.
  attachInterrupt(digitalPinToInterrupt(BOTON_RESET), ISR_activar_reset, FALLING);
  
  // Se inicia la pantalla con todo en negro y se muestra la portada.
  tft.initR(INITR_BLACKTAB);  
  pantalla_inicio();
  delay(2500);    
}

//*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=LOOP=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*//
void loop()
{
  // Se llama al procedimiento que escoge la dificultad.
  escoger_dificultad(); 

  // Para indicar que se va a iniciar el juego todos los led se encenderan a la vez durante 1 segundo.
  estado_led(ELECCION_ROJO | ELECCION_VERDE | ELECCION_AZUL | ELECCION_AMARILLO);
  delay(1000);
  estado_led(APAGAR_LEDS);
  delay(250);
  
  // Se muestra la pantalla con las vidas y se indican las 3 vidas disponibles.
  pantalla_vidas();
  g_vidas = 3;
  
  // Se procede a jugar.
  if (simon_dice()) 
  {
    // Si el jugador gana se muestra la pantalla ganadora y la melodia y secuencia de LEDs ganadora.
    pantalla_ganador(); 
    jugador_gana(); 
  }
  else
  {
    // Si el jugador pierde se muestra la pantalla perdedora y la melodia y secuencia de LEDs perdedora.
    pantalla_perdedor();
    jugador_pierde();
  }
  delay(1000);
}

//*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=FUNCIONES DEL JUEGO=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*//

/* Funcion booleana principal del juego.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    true: Si el jugador completa todas las rondas exitosamente.
 *    false: Si el jugador no completa todas las rondas exitosamente.
*/
bool simon_dice(void)
{
  // Se genera una semilla aleatoria empleando millis() y se inicia la ronda actual a 0.
  randomSeed(millis());
  g_ronda_actual = 0;
  
  // Mientras las vidas sean distintas de 0:
  while (g_vidas != 0)
  {
    // Mientras que la ronda actual no supere a la ronda final:
    while (g_ronda_actual < RONDAS_TOTALES)
    {
      // Se añaden dos secuencias al principio para empezar en 3 LEDs.
      if (g_ronda_actual == 0)
      {
        generar_secuencia();  
        generar_secuencia();  
        }
        
      generar_secuencia();    // Se añade un nuevo movimiento a la secuencia. 
      visualizar_secuencia(); // Se reproduce la secuencia generada.

      // Se comprueba si la secuencia del usuario coincide con la secuencia generada.
      for (uint8_t movimiento_actual = 0; movimiento_actual < g_ronda_actual; movimiento_actual++)
      {
        // Se espera a la la pulsacion de un boton.
        uint8_t boton_seleccionado = espera_boton(); 
        // Si no se pulsa nada durante el tiempo establecido: 
        if (boton_seleccionado == ELECCION_NULA)
        {
          // Si aun quedan vidas:
          if (g_vidas>0) 
          {
            // Si quedan tres vidas se elimina el primer corazon.
            if (g_vidas==3)
            {
              tft.fillRect(0, 68, 42, 39, ST7735_BLACK);
            }
            // Si quedan dos vidas se elimina el segundo corazon.
            else if (g_vidas==2)
            {
              tft.fillRect(43, 68, 42, 39, ST7735_BLACK);
            }
            // Si queda una vida se elimina el tercer corazon.
            else if (g_vidas==1)
            {
             tft.fillRect(86, 68, 42, 39, ST7735_BLACK);
            }
            // Se quita una vida, se vuelve a empezar y se sale del bucle
            g_vidas--;
            g_ronda_actual = 0;
            break;
          }
          // Si no quedan vidas:
          if (g_vidas==0) 
          {
            // La funcion retorna falso.
            return false; 
          }
        }
        // Si el boton es incorrecto:
        if (boton_seleccionado != g_secuencia_generada[movimiento_actual]) 
        {
          // Si aun quedan vidas:
          if (g_vidas>0) 
          {
            // Si quedan tres vidas se elimina el primer corazon.
            if (g_vidas==3)
            {
              tft.fillRect(0, 68, 42, 39, ST7735_BLACK);
            }
            // Si quedan dos vidas se elimina el segundo corazon.
            else if (g_vidas==2)
            {
              tft.fillRect(43, 68, 42, 39, ST7735_BLACK);
            }
            // Si queda una vida se elimina el tercer corazon.
            else if (g_vidas==1)
            {
             tft.fillRect(86, 68, 42, 39, ST7735_BLACK);
            }
            // Se quita una vida, se vuelve a empezar y se sale del bucle
            g_vidas--;
            g_ronda_actual = 0;
            break;
          }
          // Si no quedan vidas:
          if (g_vidas==0) 
          {
            // La funcion retorna falso.
            return false; 
          }  
        }
      }
      delay(1000); 
    }
    // El jugador ha acertado todo y la funcion devuelve verdadero.
    return true; 
  }
}
/* Procedimiento que genera la secuencia aleatoria que se tiene que acertar.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void generar_secuencia(void)
{
  // Se genera un numero aleatorio entre 0 y 3 (el limite superior queda excluido).
  uint8_t nuevo_boton = random(0, 4); 
  
  // Se convierte este numero generado a un boton.
  if (nuevo_boton == 0) nuevo_boton = ELECCION_ROJO;
  else if (nuevo_boton == 1) nuevo_boton = ELECCION_VERDE;
  else if (nuevo_boton == 2) nuevo_boton = ELECCION_AZUL;
  else if (nuevo_boton == 3) nuevo_boton = ELECCION_AMARILLO;
  
  // Se añade el boton a la secuencia.
  g_secuencia_generada[g_ronda_actual++] = nuevo_boton; 
}

/* Procedimiento que visualiza la secuencia generada.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void visualizar_secuencia(void)
{
  for (uint8_t movimiento_actual = 0 ; movimiento_actual < g_ronda_actual ; movimiento_actual++)
  {
    // Se reproduce un sonido junto con la iluminacion del led.
    toner(g_secuencia_generada[movimiento_actual], 150);
    // Este delay es que establece como de dificil va a ser el juego.
    delay(g_velocidad_leds*g_dificultad); 
  }
}

//*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=FUNCIONES DEL HARDWARE=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*//

/* Procedimiento que enciende o apaga un LEDs.
 *  Argumentos de Entrada:
 *    (uint8_t) led -> Led que se quiere encender
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void estado_led(uint8_t led)
{
  // Si el led seleccionado es el rojo se enciende ese led, sino se apaga.
  if ((led & ELECCION_ROJO) != 0) digitalWrite(LED_ROJO, HIGH);
  else digitalWrite(LED_ROJO, LOW);
  // Si el led seleccionado es el verde se enciende ese led, sino se apaga.
  if ((led & ELECCION_VERDE) != 0) digitalWrite(LED_VERDE, HIGH);
  else digitalWrite(LED_VERDE, LOW);
  // Si el led seleccionado es el azul se enciende ese led, sino se apaga.
  if ((led & ELECCION_AZUL) != 0) digitalWrite(LED_AZUL, HIGH);
  else digitalWrite(LED_AZUL, LOW);
  // Si el led seleccionado es el amarillo se enciende ese led, sino se apaga.
  if ((led & ELECCION_AMARILLO) != 0) digitalWrite(LED_AMARILLO, HIGH);
  else digitalWrite(LED_AMARILLO, LOW);
}

/* Funcion que espera a que un boton sea pulsado:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (uint8_t) ELECCION_ROJO -> Si se pulsa el boton rojo. 
 *    (uint8_t) ELECCION_VERDE -> Si se pulsa el boton verde.
 *    (uint8_t) ELECCION_AZUL -> Si se pulsa el boton azul.
 *    (uint8_t) ELECCION_AMARILLO -> Si se pulsa el boton amarillo.
 *    (uint8_t) ELECCION_NULA -> Si no se pulsa ningun boton. 
*/
uint8_t espera_boton(void)
{
  // Se implementa una espera no bloqueante (Falso delay).
  uint32_t tiempo_actual = millis(); // Se guarda el instante en el entramos en el lazo.
  while ((millis() - tiempo_actual) < TIEMPO_LIMITE_INPUT) // Si no ha pasado el tiempo limite:
  {
    uint8_t boton = comprobar_boton(); // Guardo en una variable el boton pulsado.
    // Si se ha pulsado un boton:
    if (boton != ELECCION_NULA)
    {
      toner(boton, 150); // Se reproduce el sonido con la iluminacion correspondiente al boton pulsado.
      // Se espera a que el boton deje de estar pulsado.
      while (comprobar_boton() != ELECCION_NULA); 
      delay(10); 
      return boton; // Se devuelve la eleccion correspondiente al boton pulsado.
    }
  }
  return ELECCION_NULA; // Si se ha sobrepasado el tiempo establecido se devuelve eleccion nula.
}

/* Funcion que comprueba que boton se ha pulsado.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (uint8_t) ELECCION_ROJO -> Si se pulsa el boton rojo. 
 *    (uint8_t) ELECCION_VERDE -> Si se pulsa el boton verde.
 *    (uint8_t) ELECCION_AZUL -> Si se pulsa el boton azul.
 *    (uint8_t) ELECCION_AMARILLO -> Si se pulsa el boton amarillo.
 *    (uint8_t) ELECCION_NULA -> Si no se pulsa ningun boton.
*/
uint8_t comprobar_boton(void)
{
  if (digitalRead(BOTON_ROJO) == 0) return (ELECCION_ROJO);
  else if (digitalRead(BOTON_VERDE) == 0) return (ELECCION_VERDE);
  else if (digitalRead(BOTON_AZUL) == 0) return (ELECCION_AZUL);
  else if (digitalRead(BOTON_AMARILLO) == 0) return (ELECCION_AMARILLO);
  return (ELECCION_NULA); 
}

/* Procedimiento para encender un LED con su correspondiente sonido.
 *  Rojo     440Hz - 2.272ms - 1.136ms pulso
 *  Verde    880Hz - 1.136ms - 0.568ms pulso
 *  Azul     587.33Hz - 1.702ms - 0.851ms pulso
 *  Amarillo 784Hz - 1.276ms - 0.638ms pulso
 *  Argumentos de Entrada:
 *    (uint8_t) boton -> Boton que se ha pulsado.
 *    (uint16_t) duracion_zumbido_ms -> Duracion total del zumbido.
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void toner(uint8_t boton, uint16_t duracion_zumbido_ms)
{
  // Se enciende el LED asociado al boton pulsado.
  estado_led(boton);
  // Se reproduce el sonido asociado al boton
  switch (boton)
  {
    case ELECCION_ROJO:
      buzz_sound(duracion_zumbido_ms, 1136);
      break;
    case ELECCION_VERDE:
      buzz_sound(duracion_zumbido_ms, 568);
      break;
    case ELECCION_AZUL:
      buzz_sound(duracion_zumbido_ms, 851);
      break;
    case ELECCION_AMARILLO:
      buzz_sound(duracion_zumbido_ms, 638);
      break;
  }
  // Se apaga el LED
  estado_led(APAGAR_LEDS); 
}

/* Cambia de estado el zumbador cada perido_zumbido_us durante la duracion_zumbido_ms.
 *  Argumentos de Entrada:
 *    (uint16_t) duracion_zumbido_ms -> Duracion total del zumbido
 *    (uint16_t) perido_zumbido_us -> medio perido de la onda
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void buzz_sound(uint16_t duracion_zumbido_ms, uint16_t perido_zumbido_us)
{
  // Se convierte la duracion del zumbido de ms a us.
  long duracion_zumbido_us = duracion_zumbido_ms * (long)1000;
  // Bucle hasta que el tiempo que esta sonando el zumbador sea menor que un solo periodo de onda
  while (duracion_zumbido_us > (perido_zumbido_us * 2))
  {
    // Se decrementa el tiempo que esta sonando el zumbador.
    duracion_zumbido_us -= perido_zumbido_us * 2; 
    // Cambia de estado el zumbador a varias velocidades.
    digitalWrite(ZUMBADOR, HIGH);
    delayMicroseconds(perido_zumbido_us);
    digitalWrite(ZUMBADOR, LOW);
    delayMicroseconds(perido_zumbido_us);
  }
}

/* Procedimiento para hacer sonar la melodia ganadora junto a los LEDs.
 * Todos los LEDs pardearan un par de veces.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void jugador_gana(void)
{
  estado_led(ELECCION_VERDE | ELECCION_ROJO);
  sonido_ganador();
  estado_led(ELECCION_AZUL | ELECCION_AMARILLO);
  sonido_ganador();
  estado_led(ELECCION_VERDE | ELECCION_ROJO);
  sonido_ganador();
  estado_led(ELECCION_AZUL | ELECCION_AMARILLO);
  sonido_ganador();
}

/* Procedimiento para hacer sonar la melodia ganadora.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void sonido_ganador(void)
{
  // Cambia de estado el zumbador a varias velocidades.
  for (uint8_t x = 250 ; x > 70 ; x--)
  {
    for (uint8_t y = 0 ; y < 3 ; y++)
    {
      digitalWrite(ZUMBADOR, HIGH);
      delayMicroseconds(x);
      digitalWrite(ZUMBADOR, LOW);
      delayMicroseconds(x);
    }
  }
}

/* Procedimiento para hacer sonar la melodia perdedora junto a los LEDs.
 * Todos los LEDs pardearan un par de veces.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void jugador_pierde(void)
{
  estado_led(ELECCION_ROJO | ELECCION_VERDE | ELECCION_AZUL | ELECCION_AMARILLO);
  buzz_sound(300, 2000);
  estado_led(APAGAR_LEDS);
  buzz_sound(300, 2000);
  estado_led(ELECCION_ROJO | ELECCION_VERDE | ELECCION_AZUL | ELECCION_AMARILLO);
  buzz_sound(300, 2000);
  estado_led(APAGAR_LEDS);
  buzz_sound(300, 2000);
}

/* Procedimiento para escoger la dificultad mediante los pulsadores.
 * Todos los LEDs iran encendiendose y apagandose secuencialmente a la espera de una pulsación.
 * dificultad:  Azul -> Facil
 *              Verde -> Medio
 *              Rojo -> Dificil
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void escoger_dificultad()
{
  pantalla_dificultad();
  // Bucle infinito.
  while (true)
  {
    // Se enciende el led azul y se comprueba si se ha pulsado un boton
    estado_led(ELECCION_AZUL);
    if (comprobar_boton() == ELECCION_AZUL) {g_dificultad=3; return;}
    else if (comprobar_boton() == ELECCION_VERDE){g_dificultad=2; return;}
    else if (comprobar_boton() == ELECCION_ROJO) {g_dificultad=1; return;}
    delay(100);
    
    // Se enciende el led verde y se comprueba si se ha pulsado un boton
    estado_led(ELECCION_VERDE);
    if (comprobar_boton() == ELECCION_AZUL) {g_dificultad = 3; return;}
    else if (comprobar_boton() == ELECCION_VERDE){g_dificultad =2 ; return;}
    else if (comprobar_boton() == ELECCION_ROJO) {g_dificultad = 1; return;}
    delay(100);
    
    // Se enciende el led rojo y se comprueba si se ha pulsado un boton
    estado_led(ELECCION_ROJO);
    if (comprobar_boton() == ELECCION_AZUL) {g_dificultad = 3; return;}
    else if (comprobar_boton() == ELECCION_VERDE){g_dificultad = 2; return;}
    else if (comprobar_boton() == ELECCION_ROJO) {g_dificultad = 1; return;}
    delay(100);
    
    // Se enciende el led amarillo y se comprueba si se ha pulsado un boton
    estado_led(ELECCION_AMARILLO);
    if (comprobar_boton() == ELECCION_AZUL) {g_dificultad = 3; return;}
    else if (comprobar_boton() == ELECCION_VERDE){g_dificultad = 2; return;}
    else if (comprobar_boton() == ELECCION_ROJO) {g_dificultad = 1; return;}
    delay(100);
  }
}  

/* Procedimiento que muestra la pantalla de inicio.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void pantalla_inicio() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(7, 15); tft.setTextColor(ST7735_BLUE); tft.setTextSize(4);
  tft.println("SI");
  tft.setCursor(53, 15); tft.setTextColor(ST7735_GREEN); tft.setTextSize(4);
  tft.println("MON");
  tft.setCursor(20, 110); tft.setTextColor(ST7735_RED); tft.setTextSize(4);
  tft.println("DI");
  tft.setCursor(64, 110); tft.setTextColor(ST7735_YELLOW); tft.setTextSize(4);
  tft.println("CE");
  tft.setCursor(10, 70); tft.setTextColor(ST7735_WHITE); tft.setTextSize(1);
  tft.println("Victor Caro Pastor");
}

/* Procedimiento que muestra la pantalla de seleccion de dificultad.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void pantalla_dificultad() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(10, 10); tft.setTextColor(ST7735_BLUE); tft.setTextSize(2);
  tft.println("SI");
  tft.setCursor(33, 10); tft.setTextColor(ST7735_GREEN); tft.setTextSize(2);
  tft.println("MON");
  tft.setCursor(75, 10); tft.setTextColor(ST7735_RED); tft.setTextSize(2);
  tft.println("DI");
  tft.setCursor(98, 10); tft.setTextColor(ST7735_YELLOW); tft.setTextSize(2);
  tft.println("CE");
  tft.setCursor(8, 40); tft.setTextColor(ST7735_WHITE); tft.setTextSize(1);
  tft.println("SELECTOR DIFICULTAD");
  tft.fillCircle(20, 70, 10, ST7735_BLUE);
  tft.setCursor(40, 63); tft.setTextColor(ST7735_WHITE); tft.setTextSize(2);
  tft.println("FACIL");
  tft.fillCircle(20, 100, 10, ST7735_GREEN);
  tft.setCursor(40, 93); tft.setTextColor(ST7735_WHITE); tft.setTextSize(2);
  tft.println("NORMAL");
  tft.fillCircle(20, 130, 10, ST7735_RED);
  tft.setCursor(40, 123); tft.setTextColor(ST7735_WHITE); tft.setTextSize(2);
  tft.println("DIFICIL");
}

/* Procedimiento que muestra la pantalla con las vidas totales.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void pantalla_vidas() {
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(10, 15); tft.setTextColor(ST7735_WHITE); tft.setTextSize(2);
    tft.println("  VIDAS");
    tft.println(" RESTANTES");
    
    // CORAZON 1
    tft.fillCircle(11, 80, 10, ST7735_RED);
    tft.fillCircle(29, 80, 10, ST7735_RED);
    tft.fillTriangle(2, 84, 38, 84, 20, 103, ST7735_RED);
    // CORAZON 2 
    tft.fillCircle(54, 80, 10, ST7735_RED);
    tft.fillCircle(72, 80, 10, ST7735_RED);
    tft.fillTriangle(45, 84, 81, 84, 63, 103, ST7735_RED);
    // CORAZON 3
    tft.fillCircle(97, 80, 10, ST7735_RED);
    tft.fillCircle(115, 80, 10, ST7735_RED);
    tft.fillTriangle(88, 84, 124, 84, 106, 103, ST7735_RED);

    if (g_dificultad == 3)
    {
      tft.setCursor(22, 125); tft.setTextColor(ST7735_BLUE); tft.setTextSize(3);
      tft.println("FACIL");
    }
    else if (g_dificultad == 2)
    {
      tft.setCursor(12, 125); tft.setTextColor(ST7735_GREEN); tft.setTextSize(3);
      tft.println("NORMAL");
    }
    else if (g_dificultad == 1)
    {
      tft.setCursor(2, 125); tft.setTextColor(ST7735_RED); tft.setTextSize(3);
      tft.println("DIFICIL");
    }
}

/* Procedimiento que muestra la pantalla cuando el jugador pierde.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void pantalla_perdedor() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(10, 40); tft.setTextColor(ST7735_WHITE); tft.setTextSize(3);
  tft.println(" GAME");
  tft.setCursor(5, 70);tft.setTextColor(ST7735_RED); tft.setTextSize(5);
  tft.println("OVER");
}

/* Procedimiento que muestra la pantalla cuando el jugador gana.
 *  Argumentos de Entrada:
 *    (void) Ninguno
 *  Argumentos de Salida:
 *    (void) Ninguno
*/
void pantalla_ganador() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(10, 25); tft.setTextColor(ST7735_YELLOW); tft.setTextSize(3);
  tft.println("WINNER");
  tft.setCursor(10, 65); tft.setTextColor(ST7735_YELLOW); tft.setTextSize(3);
  tft.println("WINNER");
  tft.setCursor(10, 105); tft.setTextColor(ST7735_YELLOW); tft.setTextSize(3);
  tft.println("WINNER");
}
