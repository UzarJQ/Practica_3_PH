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
#include "eventos.h"

/*--- variables globales ---*/
extern int switch_leds;
int timer1_num_int = 0; // Contador de periodos completos por el timer1
Event timer_event = TIMER1_IRQ;

/* declaraci�n de funci�n que es rutina de servicio de interrupci�n
https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html */
void timer_ISR(void) __attribute__((interrupt("IRQ")));
void timer1_ISR(void) __attribute__((interrupt("IRQ")));
void timer2_ISR(void) __attribute__((interrupt("IRQ")));

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
	rINTMOD = 0x0;						// Configura las lineas como de tipo IRQ
	rINTCON = 0x1;						// Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK &= ~(BIT_TIMER0); // habilitamos en vector de mascaras de interrupcion el Timer0 (bits 26 y 13, BIT_GLOBAL y BIT_TIMER0 est�n definidos en 44b.h)

	/* Establece la rutina de servicio para TIMER0 */
	pISR_TIMER0 = (unsigned)timer_ISR;

	/* Configura el Timer0 */
	rTCFG0 = 255;		 // ajusta el preescalado
	rTCFG1 = 0x0;		 // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCNTB0 = 65535; // valor inicial de cuenta (la cuenta es descendente)
	rTCMPB0 = 12800; // valor de comparaci�n
	/* establecer update=manual (bit 1) + inverter=on (�? ser� inverter off un cero en el bit 2 pone el inverter en off)*/
	rTCON = 0x2;
	/* iniciar timer (bit 0) con auto-reload (bit 3)*/
	rTCON = 0x09;
}

void timer1_inicializar(void)
{
	rINTMOD = 0x0; // Configura las lineas como de tipo IRQ
	rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK &= ~(BIT_TIMER1);

	pISR_TIMER1 = (unsigned)timer1_ISR;

	// Configuracion del Timer1
	rTCFG0 = 255;
	rTCFG1 = 0x0;

	rTCNTB1 = 64000;
	rTCMPB1 = 0x0;

	rTCON |= (1 << 9);																				// Set bit 9 (update=manual)
	rTCON &= ~(1 << 11);																			// Clear bit 11 (auto-reload off)
	rTCON = (rTCON & ~(0x1 << 9)) | (0x1 << 11) | (0x1 << 8); // Clear bit 9 and set bit 11 (update=manual, auto-reload on)
}

void timer1_ISR(void)
{
	timer1_num_int++; // Aumentar el contador de ciclos completos

	rI_ISPC |= BIT_TIMER1; // Desactivar la solicitud de interrupcion del timer1

	//	gestionar_boton();
}

void timer1_empezar()
{
	// Reiniciar el contador de interrupciones
	timer1_num_int = 0;

	// Reiniciar el valor del contador del timer1
	rTCNTB1 = 64000;

	// Establecer update=manual (bit 9) para reiniciar el contador
	rTCON |= (0x1 << 9);

	// Iniciar el timer1 (bit 8)
	rTCON |= (0x1 << 8);

	// Desactivar el bit de update manual para permitir el funcionamiento normal
	rTCON &= ~0x100;
}

unsigned int timer1_leer()
{

	unsigned int valor_cuenta = rTCNTO1; // Leer el valor actual del contador
	// Calcular el tiempo transcurrido en el timer1
	// timer1_num_int * 64000 es el total de ticks de los ciclos completos
	// (2.0 / 33.0) es el factor de conversion de ticks a microsegundos
	unsigned int tiempo_transcurrido = (timer1_num_int * 65536) + (65535 - valor_cuenta);

	return tiempo_transcurrido;
}

unsigned int timer1_parar()
{
	rTCON &= ~(0x1 << 8); // Desactivar el timer 1 (bit 9 y 11)

	return timer1_leer();
}

void timer2_ISR(void)
{
	static int led_status = 0; // Estado del LED: 0 = apagado, 1 = encendido

	if (led_status == 0)
	{
		led2_on(); // Enciende el LED derecho
		led_status = 1;
	}
	else
	{
		led2_off(); // Apaga el LED derecho
		led_status = 0;
	}

	// Borrar la solicitud de interrupción
	rI_ISPC |= BIT_TIMER2; // BIT_TIMER2 está definido y pone un 1 en el bit correspondiente para desactivar la interrupción
}

void timer2_init(void)
{
	// Configuración del controlador de interrupciones
	rINTMOD = 0x0;						// Configura las líneas como de tipo IRQ
	rINTCON = 0x1;						// Habilita interrupciones vectorizadas y la línea IRQ
	rINTMSK &= ~(BIT_TIMER2); // Habilitar interrupciones del timer2

	// Establece la rutina de servicio para TIMER2
	pISR_TIMER2 = (unsigned)timer2_ISR;

	// Configuración del Timer2
	// Usaremos una frecuencia más baja para asegurarnos de que el temporizador funciona como esperamos
	rTCFG0 = 255;					 // Preescalado, para dividir la frecuencia de entrada
	rTCFG1 &= ~(0xf << 8); // Seleccionar un divisor de 1/2 para Timer2

	// Queremos 80 interrupciones por segundo, así que necesitamos calcular el valor para rTCNTB2
	// Si el reloj de entrada tiene una frecuencia de 50 MHz:
	// Frecuencia de reloj = 50,000,000 / (prescaler + 1) / divisor_mux
	//                      = 50,000,000 / 256 / 2
	//                      = 97656.25 Hz
	// Necesitamos 80 interrupciones por segundo:
	// Valor de rTCNTB2 = 97656.25 / 80 ≈ 1220
	rTCNTB2 = 1220; // Configuración para obtener la frecuencia de interrupción deseada
	rTCMPB2 = 0;		// No utilizamos el comparador en este caso

	// Configurar update manual (bit 13) para cargar el valor del buffer y luego iniciar el timer
	rTCON |= (1 << 13);							// Establecer update=manual (bit 13)
	rTCON &= ~(1 << 13);						// Desactivar el bit de update manual
	rTCON |= (1 << 12) | (1 << 15); // Iniciar el timer2 (bit 12) y habilitar auto-reload (bit 15)
}