/*--- Archivo : button.c ---*/
#include "8led.h"
#include "led.h"
#include "44blib.h"
#include "44b.h"
#include "def.h"
#include "button.h"
#include "cola.h"

/*--- Variables globales ---*/
static ButtonState button_state = WAITING;
static unsigned int last_timer_value = 0;
static Event evento = PRESSED_IRQ;
static int led8_count = 0;
static int button_id;

/* Declaración de función para ISR de Eint4567 */
void Eint4567_ISR(void) __attribute__((interrupt("IRQ")));

/* Código de funciones */
void Eint4567_ISR(void)
{
  rINTMSK |= BIT_EINT4567; // Deshabilitar interrupciones de EINT4567
  int which_int = rEXTINTPND;
  button_state = PRESSED;

  if (button_state == PRESSED)
  {
    if ((timer1_leer() - last_timer_value) > 20201) // TRP
    {

      if (!(rPDATG & 0x40) || !(rPDATG & 0x80))
      {
        button_state = MANTAINED;
      }
      else
      {
        button_state = RELEASED;
      }

      switch (which_int)
      {
      case 4:
        led8_count++;
        button_id = 6;
        last_timer_value = timer1_leer();
        push_debug(evento, button_id, last_timer_value);
        break;
      case 8:
        led8_count--;
        button_id = 7;
        last_timer_value = timer1_leer();
        push_debug(evento, button_id, last_timer_value);
        break;
      default:
        break;
      }
      last_timer_value = timer1_leer();
    }
  }

  if (button_state == MANTAINED)
  {

    if ((timer1_leer() - last_timer_value) > 50000) // Monitorizar cada 50 ms
    {
      if ((rPDATG & 0x40) || (rPDATG & 0x80))
      {
        button_state = RELEASED;
      }
      else
      {
        button_state = MANTAINED;
      }
    }
    last_timer_value = timer1_leer();

    last_timer_value = timer1_leer();
  }

  if (button_state == RELEASED)
  {

    if ((timer1_leer() - last_timer_value) > 20201) // TRD
    {
      evento = RELEASED_IRQ;
      push_debug(evento, button_id, last_timer_value);
      button_state = WAITING;
    }
  }

  D8Led_symbol(led8_count & 0xf);
  rEXTINTPND = 0xf;           // Borra los bits en EXTINTPND
  rI_ISPC |= BIT_EINT4567;    // Borra el bit pendiente en INTPND
  rINTMSK &= ~(BIT_EINT4567); // Habilitar interrupciones de EINT4567
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
