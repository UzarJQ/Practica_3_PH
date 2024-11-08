/*********************************************************************************************
* Fichero:		timer.c
* Autor:
* Descrip:		funciones de control del timer0 del s3c44b0x
* Version:
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "timer.h"
#include "44b.h"
#include "44blib.h"

/*--- variables globales ---*/
int switch_leds = 0;
int timer1_num_int = 0; // Contador de periodos completos por el timer1

/* declaraci�n de funci�n que es rutina de servicio de interrupci�n
 * https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html */
void timer_ISR(void) __attribute__((interrupt("IRQ")));
void timer1_ISR(void) __attribute__((interrupt("IRQ")));

/*--- codigo de las funciones ---*/
void timer_ISR(void)
{
	switch_leds = 1;

	/* borrar bit en I_ISPC para desactivar la solicitud de interrupci�n*/
	rI_ISPC |= BIT_TIMER0; // BIT_TIMER0 est� definido en 44b.h y pone un uno en el bit 13 que corresponde al Timer0
}

void timer_init(void)
{
	/* Configuracion controlador de interrupciones */
	rINTMOD = 0x0; // Configura las lineas como de tipo IRQ
	rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK &= ~(BIT_TIMER0); // habilitamos en vector de mascaras de interrupcion el Timer0 (bits 26 y 13, BIT_GLOBAL y BIT_TIMER0 est�n definidos en 44b.h)

	/* Establece la rutina de servicio para TIMER0 */
	pISR_TIMER0 = (unsigned) timer_ISR;

	/* Configura el Timer0 */
	rTCFG0 = 255; // ajusta el preescalado
	rTCFG1 = 0x0; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCNTB0 = 65535;// valor inicial de cuenta (la cuenta es descendente)
	rTCMPB0 = 12800;// valor de comparaci�n
	/* establecer update=manual (bit 1) + inverter=on (�? ser� inverter off un cero en el bit 2 pone el inverter en off)*/
	rTCON = 0x2;
	/* iniciar timer (bit 0) con auto-reload (bit 3)*/
	rTCON = 0x09;
}

void timer1_inicializar(void)
{
	// No es necesario volver a configurar rINTMOD y rINTCON si ya se han configurado para timer0
	rINTMSK &= ~(BIT_TIMER1);

	pISR_TIMER1 = (unsigned) timer1_ISR;
	
	// Configuracion del Timer1
	rTCFG0 = 255;
	rTCFG1 = 0x0;

	rTCNTB1 = 64000;
	rTCMPB1 = 0x0;

	rTCON |= 0x100;	// activar manual del timer 1 (bit 9)
	rTCON |= 0x400; // activar auto-reload del timer 1 (bit 11)
} 

void timer1_ISR(void)
{
	timer1_num_int++;  // Aumentar el contador de ciclos completos

	rI_ISPC |= BIT_TIMER1; // Desactivar la solicitud de interrupcion del timer1
}

void timer1_empezar()
{
	// Reiniciar el contador de interrupciones
	timer1_num_int = 0;
	
	// Reiniciar el valor del contador del timer1
	rTCNTB1 = 64000;

	// Establecer update=manual (bit 9) para reiniciar el contador
	rTCON |= 0x100;

	// Iniciar el timer1 con auto-reload (bit 11)
	rTCON |= 0x400;

	// Desactivar el bit de update manual para permitir el funcionamiento normal
	rTCON &= ~0x100;
}

unsigned int timer1_leer()
{
	// Calcular el tiempo transcurrido en el timer1
	// timer1_num_int * 64000 es el total de ticks de los ciclos completos
	// 64000 - rTCNTO1 es el valor de ticks restantes en el ciclo actual
	// (2.0 / 33.0) es el factor de conversion de ticks a microsegundos
	unsigned int total_time = ((timer1_num_int * 64000) + (64000 - rTCNTO1)) * (2.0 / 33.0);

	return total_time;
}

unsigned int timer1_parar(){
	// Parar el timer1
	rTCON &= ~0x100; // Desactivar manual update del timer 1 (bit 9)
	rTCON &= ~0x400; // Desactivar auto-reload del timer 1 (bit 11)

	return timer1_leer();
}
