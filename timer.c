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
#include "sudoku_2024.h"
#include "cola.h"

/*--- variables globales ---*/
extern int switch_leds;
int timer1_num_int = 0; // Contador de periodos completos por el timer1
volatile int led_event_counter = 0;
extern CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS];
static int selected_row = 0;
static int selected_column = 0;
static int modifying_value = 0;

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
	push_debug(3, 0, timer1_leer());
}

void timer1_ISR(void)
{
	timer1_num_int++; // Incrementar el contador de ciclos completos
	// Máquina de estados
	switch (button_state)
	{
	case WAITING:
		if (button_flag == 1) // Detectar una nueva pulsación
		{
			button_state = PRESSED;
			last_timer_value = timer1_leer();
			button_flag = 0;
		}
		break;

	case PRESSED:
		if ((timer1_leer() - last_timer_value) > 272016) // TRP
		{
			switch (sudoku_status)
			{
			case NOT_STARTED:
				break;
			case STARTED:
				sudoku_candidatos_init_arm(cuadricula, 1);
				led8_count = 15;
				sudoku_status = ROW_SELECTION;
				break;
			case ROW_SELECTION:
				if (!(rPDATG & 0x40)) // Incrementar con el boton izquierdo
				{
					led8_count++;

					if (led8_count > 9)
					{
						led8_count = 1;
					}
				}

				if (!(rPDATG & 0x80))
				{
					selected_row = led8_count - 1; // Se resta uno para que el índice sea correcto (empezamos en 0)
					led8_count = 12;
					sudoku_status = COLUMN_SELECTION;
				}
				break;
			case COLUMN_SELECTION:
				if (!(rPDATG & 0x40)) // Incrementar con el boton izquierdo
				{
					led8_count++;

					if (led8_count > 9)
					{
						led8_count = 1;
					}
				}

				if (!(rPDATG & 0x80))
				{
					selected_column = led8_count - 1; // Se resta uno para que el índice sea correcto (empezamos en 0)

					if (cuadricula[selected_row][selected_column] & 0x8000) // comprobar si la celda es una pista
					{
						sudoku_status = ROW_SELECTION;
						led8_count = 15;
					}
					else
					{
						sudoku_status = VALUE_SELECTION;
						led8_count = 0;
					}
				}
				break;
			case VALUE_SELECTION:
				if (!(rPDATG & 0x40)) // Incrementar con el boton izquierdo
				{
					led8_count++;
					if (led8_count > 9)
					{
						led8_count = 0;
					}
				}

				if (!(rPDATG & 0x80))
				{
					celda_poner_valor(&cuadricula[selected_row][selected_column], led8_count);
					cuadricula_candidatos_verificar(cuadricula, selected_row, selected_column);

					if (cuadricula[selected_row][selected_column] & 0x4000) // Comprobar si hay errores
					{
						led8_count = 14; // Indicar error con una E en el 8led
						modifying_value = 1;
						break;
					}
					else // Si no hay errores, propagar el valor de la celda
					{
						if (modifying_value)
						{
							sudoku_candidatos_init_arm(cuadricula, 1);
							modifying_value = 0;
						}
						else
						{
							sudoku_candidatos_propagar_arm(cuadricula, selected_row, selected_column, led8_count);
						}

						led8_count = 15;
						sudoku_status = ROW_SELECTION;
					}
				}

				break;
			default:
				break;
			}

			D8Led_symbol(led8_count & 0xf);

			if (!(rPDATG & 0x40) || !(rPDATG & 0x80))
			{
				button_state = MANTAINED; // Cambiar a estado mantenido
			}
			else
			{
				button_state = RELEASED; // Cambiar a estado liberado
			}
			last_timer_value = timer1_leer(); // Actualizar tiempo
		}
		break;

	case MANTAINED:
		if ((timer1_leer() - last_timer_value) > 50000) // 50 ms
		{
			if ((rPDATG & 0x40) && (rPDATG & 0x80)) // Botón liberado
			{
				last_timer_value = timer1_leer();
				button_state = RELEASED;
			}
		}
		break;

	case RELEASED:
		if ((timer1_leer() - last_timer_value) > 2752512) // TRD
		{
			push_debug(RELEASED_IRQ, button_id, timer1_leer());
			button_state = WAITING;
			rEXTINTPND = 0xF;						// Limpiar bits en EXTINTPND
			rINTMSK &= ~(BIT_EINT4567); // Volver a habilitar las interrupciones de botones
		}
		break;

	default:
		button_state = WAITING; // Reiniciar en caso de error
		break;
	}

	rI_ISPC |= BIT_TIMER1; // Limpiar interrupción
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
	led_event_counter++;

	if (led_event_counter < 200)
	{
		led2_on();
	}
	else if (led_event_counter < 400)
	{
		led2_off();
	}
	else
	{
		led_event_counter = 0; // Reiniciar el contador después de 160 eventos (2 segundos)
	}

	rI_ISPC |= BIT_TIMER2; // Limpiar la interrupción
}

void timer2_init(void)
{
	rINTMOD = 0x0;
	rINTCON = 0x1;
	rINTMSK &= ~(BIT_TIMER2);

	pISR_TIMER2 = (unsigned)timer2_ISR;

	rTCFG0 = 0x8;
	rTCFG1 = 0x0;

	rTCNTB2 = 100000;
	rTCMPB2 = 0x0;

	rTCON |= (1 << 13); // Actualización manual
	rTCON &= ~(1 << 15);

	rTCON = (rTCON & ~(0x1 << 13)) | (0x1 << 15) | (0x1 << 12); // Auto-reload y Start
}

// 102000 - 65535 = 36465 C C
// 79669 - 65535 = 14134 C ARM
// 98385 - 65535 = 32850 ARM C
// 76040 - 65535 = 10505 ARM ARM