/*********************************************************************************************
 * Fichero:	main.c
 * Autor:
 * Descrip:	punto de entrada de C
 * Version:      <P4-ARM.timer-leds>
 *********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "8led.h"
#include "button.h"
#include "led.h"
#include "timer.h"
#include "44blib.h"
#include "44b.h"
#include "cola.h"

/*--- variables globales ---*/

int switch_leds = 0;

/*--- codigo de funciones ---*/
void Main(void)
{
	/* Inicializa controladores */
	sys_init();						// Inicialización de la placa, interrupciones y puertos
	//timer_init();					// Inicialización del temporizador Timer0
	timer1_inicializar(); // Inicialización del Timer1
	timer2_init();				// Inicialización del Timer2 para el latido del LED
	Eint4567_init();			// Inicializar los pulsadores
	D8Led_init();					// Inicializar el display 8 LED

	/* Valor inicial de los leds */
	leds_off(); // Apagar todos los LEDS
	led1_on();	// Encender el LED derecho inicialmente

	while (1)
	{
		/* Cambia los leds con cada interrupcion del temporizador */
		//		if (switch_leds == 1)
		//		{
		//			leds_switch();
		//			switch_leds = 0;
		//		}

		/* Llamada para gestionar el estado del botón */
	}
}
