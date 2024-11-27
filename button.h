/*********************************************************************************************
 * Fichero:	button.h
 * Autor:
 * Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
 * Version:
 *********************************************************************************************/

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <stdint.h>

typedef enum
{
  WAITING,
  PRESSED,
  RELEASED,
  MANTAINED
} ButtonState;

/*--- declaracion de funciones visibles del modulo button.c/button.h ---*/
void Eint4567_init();
void gestionar_boton(); // Gestionar el estado del boton
int button_active();    // Identificar boton precionado

#endif /* _BUTTON_H_ */
