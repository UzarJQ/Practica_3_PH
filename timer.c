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
#include "Bmp.h"
#include "lcd.h"

/*--- variables globales ---*/
extern int switch_leds;
int timer1_num_int = 0; // Contador de periodos completos por el timer1
volatile int led_event_counter = 0;
volatile int game_started = 0;
volatile int time_counter = 0;
volatile int seconds = 0;
extern CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS];
static int selected_row = 0;
static int selected_column = 0;
static unsigned int TRP = 272016;
static unsigned int TRD = 275251;

/* declaraci�n de funci�n que es rutina de servicio de interrupci�n
https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html */
void timer_ISR(void) __attribute__((interrupt("IRQ")));
void timer1_ISR(void) __attribute__((interrupt("IRQ")));
void timer2_ISR(void) __attribute__((interrupt("IRQ")));

/*--- codigo de las funciones ---*/
void timer_ISR(void)
{
	static int time_counter = 0; // Contador de segundos

	if (game_started)
	{
		time_counter++; // Incrementa el contador de segundos

		if ((time_counter % 1000) == 0) // Reinicia el contador cada 2 segundos
		{
			seconds++;

			LcdClrRect(63, 5, 120, 17, WHITE);
			char time[3];
			time[0] = (seconds / 10) + '0';
			time[1] = (seconds % 10) + '0';
			time[2] = '\0';
			Lcd_DspAscII8x16(65, 5, BLACK, time);
			Lcd_Dma_Trans();
		}
	}
	// Borrar el bit de interrupción para Timer0
	rI_ISPC |= BIT_TIMER0; // BIT_TIMER0 está definido en 44b.h
}

void timer_init(void)
{
	rINTMOD = 0x0;						// Configuración para interrupciones de nivel
	rINTCON = 0x1;						// Habilitar vectores
	rINTMSK &= ~(BIT_TIMER0); // Habilitar interrupción del Timer 0

	pISR_TIMER0 = (unsigned)timer_ISR; // Dirección de la ISR del Timer 0

	rTCFG0 = (79 << 0);	 // Preescalador para Timer0: 79
	rTCFG1 = (0x3 << 0); // Divisor de 8 para Timer0

	rTCNTB0 = 100000; // Valor del contador para 1 Hz (1 segundo)
	rTCMPB0 = 0x0;		// No se usa el comparador

	rTCON |= (1 << 1);	// Actualización manual de los registros
	rTCON &= ~(1 << 1); // Limpiar el bit de actualización manual

	rTCON = (rTCON & ~(0x1 << 3)) | (0x1 << 3) | (0x1 << 0); // Auto-reload y Start
}

void timer1_inicializar(void)
{
	rINTMOD = 0x0; // Configura las lineas como de tipo IRQ
	rINTCON = 0x1; // Habilita int vectorizadas y la linea IRQ (FIQ no)
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
	timer1_num_int++; // Incrementar el contador de ciclos completos
	// Maquina de estados
	switch (button_state)
	{
	case WAITING:
		if (button_flag == 1)
		{
			button_state = PRESSED;
			last_timer_value = timer1_leer();
			button_flag = 0;
		}
		break;

	case PRESSED:
		if ((timer1_leer() - last_timer_value) > TRP)
		{
			switch (sudoku_status)
			{
			case NOT_STARTED:
				break;
			case STARTED:
				push_debug(5, 5, timer1_leer());
				sudoku_candidatos_init_arm(cuadricula, 1);
				push_debug(5, 5, timer1_leer());
				led8_count = 15; // Indicar que se debe seleccionar la fila con una F en el 8led
				sudoku_status = ROW_SELECTION;
				game_started = 1;
				LCD_mostrar_sudoku(cuadricula);
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
					if (led8_count > 9)
					{
						led8_count = 1;
					}

					selected_row = led8_count - 1; // Se resta uno para que el índice de la celda sea correcto (empezamos en cuadricula[0])
					led8_count = 12;							 // Indicar que se debe seleccionar la columna con una C en el 8led
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
					if (led8_count > 9)
					{
						led8_count = 1;
					}

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
					if (cuadricula[selected_row][selected_column] & 0x8000) // Si es una pista
					{
						led8_count = 15; // Indicar que se debe seleccionar la fila con una F en el 8led
						sudoku_status = ROW_SELECTION;
					}
					else
					{
						celda_poner_valor(&cuadricula[selected_row][selected_column], led8_count);
						cuadricula_candidatos_verificar(cuadricula, selected_row, selected_column);

						if (cuadricula[selected_row][selected_column] & 0x4000) // Comprobar si hay errores
						{
							led8_count = 14; // Indicar error con una E en el 8led
						}
						else // Si no hay errores, propagar el valor de la celda
						{
							sudoku_candidatos_propagar_arm(cuadricula, selected_row, selected_column, led8_count);
							led8_count = 15;
							sudoku_status = ROW_SELECTION;
						}
					}
					LCD_mostrar_sudoku(cuadricula);
				}
				break;
			default:
				break;
			}

			D8Led_symbol(led8_count & 0xf);

			if (!(rPDATG & 0x40) || !(rPDATG & 0x80))
			{
				button_state = MAINTAINED; // Cambiar a estado mantenido
			}
			else
			{
				button_state = RELEASED; // Cambiar a estado liberado
			}
			last_timer_value = timer1_leer(); // Actualizar tiempo
		}
		break;

	case MAINTAINED:
		if ((timer1_leer() - last_timer_value) > 50000) // 50 ms
		{
			if ((rPDATG & 0x40) && (rPDATG & 0x80)) // Botón liberado
			{
				last_timer_value = timer1_leer();
				button_state = RELEASED;
			}
		}
		last_timer_value = timer1_leer();
		break;

	case RELEASED:
		if ((timer1_leer() - last_timer_value) > TRD)
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

	if (led_event_counter < 20)
	{
		led2_on();
	}
	else if (led_event_counter < 40)
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

	rTCFG0 = (79 << 8);	 // Preescalador para Timer2 (79)
	rTCFG1 = (0x3 << 8); // Divisor de 8 para Timer2

	rTCNTB2 = 1250; // Valor del contador para 80 Hz
	rTCMPB2 = 0x0;

	rTCON |= (1 << 13); // Actualizaci�n manual
	rTCON &= ~(1 << 15);

	rTCON = (rTCON & ~(0x1 << 13)) | (0x1 << 15) | (0x1 << 12); // Auto-reload y Start
}

// 102000 - 65535 = 36465 C C
// 79669 - 65535 = 14134 C ARM
// 98385 - 65535 = 32850 ARM C
// 76040 - 65535 = 10505 ARM ARM

// O0 = 259287115 - 259262652 = 24463
// O0 = 316368719 - 316344202 = 24517
// O0 = 338126638 - 338102146 = 24492

// O1 = 149768381 - 149751783 = 16598
// O1 = 356993018 - 356976445 = 16573

// O2 = 115017551 - 114952013 = 65538
// O2 = 98764611 - 98699076 = 65535
// O2 = 129369928 - 129304387 = 65541

// O3 = 127272781 - 127207238 = 65543
// O3 = 123078465 - 123012936 = 65529

// Os = 100092058 - 100075339 = 16719
// Os = 572868761 - 572852046 = 16715
