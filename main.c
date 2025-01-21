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
#include "Bmp.h"
#include "lcd.h"

/*--- variables globales ---*/

int switch_leds = 0;

/*--- codigo de funciones ---*/
void Main(void)
{
	int seconds = 0;
	char time[3];
	/* Inicializa controladores */
	sys_init();						// Inicialización de la placa, interrupciones y puertos
	timer_init();					// Inicialización del temporizador Timer0
	timer1_inicializar(); // Inicialización del Timer1
	timer2_init();				// Inicialización del Timer2 para el latido del LED
	Eint4567_init();			// Inicializar los pulsadores
	D8Led_init();					// Inicializar el display 8 LED

	/* initial LCD controller */
	Lcd_Init();
	/* clear screen */
	Lcd_Clr();
	Lcd_Active_Clr();

	Lcd_DspAscII8x16(10, 0, BLACK, "SUDOKU 2024");
	Lcd_DspAscII8x16(10, 15, BLACK, "INSTRUCCIONES:");
	Lcd_DspAscII8x16(10, 30, BLACK, "El sudoku funciona mediante un contador");
	Lcd_DspAscII8x16(10, 45, BLACK, "que va de 1 a 9.  Utiliza el boton");
	Lcd_DspAscII8x16(10, 60, BLACK, "izquierdo para incrementarlo.");
	Lcd_DspAscII8x16(10, 75, BLACK, "Cuando el contador llegue a 9 se");
	Lcd_DspAscII8x16(10, 90, BLACK, "reiniciara a 1.  Utiliza el boton");
	Lcd_DspAscII8x16(10, 105, BLACK, "derecho para confirmar el valor");
	Lcd_DspAscII8x16(10, 120, BLACK, "Primero se debe seleccionar la fila,");
	Lcd_DspAscII8x16(10, 135, BLACK, "luego la columna y finalmente el valor.");
	Lcd_DspAscII8x16(10, 150, BLACK, "Cada celda tiene posibles valores ");
	Lcd_DspAscII8x16(10, 165, BLACK, "candidatos que seran representados");
	Lcd_DspAscII8x16(10, 180, BLACK, "mediante un cuadro.");

	Lcd_DspAscII8x16(10, 200, BLACK, "Pulsa un boton para empezar.");
	// Actualiza la pantalla
	// BitmapView(125, 135, Stru_Bitmap_gbMouse);
	Lcd_Dma_Trans();

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
