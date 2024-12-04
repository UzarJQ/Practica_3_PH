/*********************************************************************************************
 * Fichero:	button.h
 * Autor:
 * Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
 * Version:
 *********************************************************************************************/

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <stdint.h>
#include "sudoku_2024.h"
typedef enum
{
  WAITING,
  PRESSED,
  RELEASED,
  MAINTAINED
} ButtonState;

// stack variables
extern volatile int button_id;
extern volatile ButtonState button_state;
extern volatile unsigned int last_timer_value;

// sudoku flags
extern volatile int button_flag;
extern volatile int led8_count;

/*--- declaracion de funciones visibles del modulo button.c/button.h ---*/
void Eint4567_init();
void gestionar_boton(); // Gestionar el estado del boton
int button_active();    // Identificar boton precionado

#endif /* _BUTTON_H_ */
