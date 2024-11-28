/*--- Archivo : button.c ---*/
#include "8led.h"
#include "led.h"
#include "44blib.h"
#include "44b.h"
#include "def.h"
#include "button.h"
#include "cola.h"

/*--- Variables globales ---*/
volatile int led8_count = 0;

volatile int button_id;

volatile unsigned int last_timer_value;
volatile ButtonState button_state = WAITING;
volatile int button_flag = 0;

/* Declaración de función para ISR de Eint4567 */
void Eint4567_ISR(void) __attribute__((interrupt("IRQ")));

/* Código de funciones */
void Eint4567_ISR(void)
{
  rINTMSK |= BIT_EINT4567; // Deshabilitar interrupciones de EINT4567

  switch (rEXTINTPND)
  {
  case 4:
    button_id = 6;
    push_debug(PRESSED_IRQ, button_id, timer1_leer());
    break;
  case 8:
    button_id = 7;
    push_debug(PRESSED_IRQ, button_id, timer1_leer());
    break;
  default:
    break;
  }

  button_flag = 1;
  rI_ISPC |= BIT_EINT4567;
}

void Eint4567_init(void)
{
  // Configuración del controlador de interrupciones
  rI_ISPC = 0x3ffffff;        // Borra INTPND escribiendo 1s en I_ISPC
  rEXTINTPND = 0xf;           // Borra EXTINTPND escribiendo 1s en el propio registro
  rINTMOD = 0x0;              // Configura las líneas como de tipo IRQ
  rINTCON = 0x1;              // Habilita interrupciones vectorizadas y la línea IRQ (FIQ no)
  rINTMSK &= ~(BIT_EINT4567); // Habilitar interrupciones de EINT4567

  // Establece la rutina de servicio para Eint4567
  pISR_EINT4567 = (unsigned)Eint4567_ISR;

  // Configuración del puerto G
  rPCONG = 0xffff;                                                        // Establece la función de los pines (EINT0-7)
  rPUPG = 0x0;                                                            // Habilita el "pull-up" del puerto
  rEXTINT = rEXTINT | (0x2 << 0) | (0x2 << 4) | (0x2 << 8) | (0x2 << 12); // Configura las líneas de interrupción como de flanco de bajada

  rEXTINTPND = 0xf;        // borra los bits en EXTINTPND
  rI_ISPC |= BIT_EINT4567; // borra el bit pendiente en INTPND
}
