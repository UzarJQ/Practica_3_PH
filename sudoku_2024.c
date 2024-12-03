#include "sudoku_2024.h"
#include <stdio.h>
#include <stdlib.h>

volatile char *ready;
volatile int celdas_vacias;
volatile SudokuStates sudoku_status = NOT_STARTED;
volatile int errors = 0;
/* *****************************************************************************
 * Funciones privadas (static)
 * (no pueden ser invocadas desde otro fichero) */

/* modifica el valor almacenado en la celda indicada */
extern void
celda_poner_valor(CELDA *celdaptr, uint8_t val)
{
	*celdaptr = (*celdaptr & 0xFFF0) | (val & 0x000F);
}

/* extrae el valor almacenado en los 16 bits de la celda */
extern uint8_t
celda_leer_valor(CELDA celda)
{
	return (celda & 0x000F);
}

/* Propaga el valor de una determinada celda para actualizar las listas de candidatos en su fila, columna y region */
extern void sudoku_candidatos_propagar_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], int fila, int columna, uint8_t valor)
{
	if (valor != 0)
	{
		int displace = 3 + (int)valor;

		/* recorrer cada fila desactivando el candidato de la lista */
		int row = 0;
		while (row < NUM_FILAS)
		{
			uint16_t celda = cuadricula[row][columna];
			if (((celda & 0x8000) != 0x8000) || row != fila)
			{
				celda &= ~(1 << displace);
				cuadricula[row][columna] = celda;
			}
			row++;
		}

		/* recorrer cada columna desactivando el candidato de la lista */
		int col = 0;
		while (col < NUM_COLUMNAS - 7)
		{
			uint16_t celda = cuadricula[fila][col];
			if (((celda & 0x8000) != 0x8000) || col != columna)
			{
				celda &= ~(1 << displace); // Desactivar el candidato de la lista con una operaci�n NAND
				cuadricula[fila][col] = celda;
			}
			col++;
		}

		/* Calcular la posicion inicial para cada region 3x3 correspondiente */
		int row_start = (fila / 3) * 3;
		int col_start = (columna / 3) * 3;

		/* Recorrer la region desactivando el candidato de la lista */
		row = row_start;
		while (row < (row_start + 3))
		{
			int col = col_start;
			while (col < (col_start + 3))
			{
				uint16_t celda = cuadricula[row][col];
				if (((celda & 0x8000) != 0x8000) || row != fila || col != columna)
				{
					celda &= ~(1 << displace);
					cuadricula[row][col] = celda;
				}
				col++;
			}
			row++;
		}
	}
}
/* *****************************************************************************
 * calcula todas las listas de candidatos (9x9)
 * necesario tras borrar o cambiar un valor (listas corrompidas)
 * retorna el numero de celdas vacias */
extern int
sudoku_candidatos_init_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], int version_propagar)
{
	/* Recorrer la cuadricula celda a celda inicializando la lista de candidatos */
	int row = 0;
	while (row < NUM_FILAS)
	{
		int col = 0;
		while (col < NUM_COLUMNAS - 7)
		{
			int pista = cuadricula[row][col] & 0x8000;
			if (pista == 0)
			{
				cuadricula[row][col] |= 0x1FF0;
			}
			col++;
		}
		row++;
	}

	/* Recorer la cuadricula celda a celda:
	 * Si la celda tiene valor => sudoku_candidatos_propagar_c
	 * Si no tiene valor => actualizar contador de celdas vac�as
	 */
	int celdas_vacias = 0;
	row = 0;
	while (row < NUM_FILAS)
	{
		int col = 0;
		while (col < NUM_COLUMNAS - 7)
		{
			uint8_t valor_actual = celda_leer_valor(cuadricula[row][col]);
			if (valor_actual == 0x0000)
			{
				celdas_vacias++;
			}
			else
			{
				if (version_propagar == 0)
				{
					sudoku_candidatos_propagar_c(cuadricula, row, col, valor_actual);
				}
				if (version_propagar == 1)
				{
					sudoku_candidatos_propagar_arm(cuadricula, row, col, valor_actual);
				}
				if (version_propagar == 2)
				{
					//					sudoku_candidatos_propagar_thumb(cuadricula, row, col, valor_actual);
				}
			}
			col++;
		}
		row++;
	}

	return celdas_vacias;
}

extern void
cuadricula_candidatos_verificar(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], int row, int col)
{

	uint8_t valor = celda_leer_valor(cuadricula[row][col]);

	/* Si el valor es distinto de 0, revisa que si dicho valor esta en la lista de candidatos) */
	if ((cuadricula[row][col] & 0x8000) == 0) // Si la celda es una pista, no se verifica
	{
		int displace = 3 + (int)valor;
		if (valor != 0x0000)
		{
			if ((cuadricula[row][col] & (1 << displace)) == 0)
			{
				cuadricula[row][col] |= 0x4000; // Marcar la celda con error
				errors++;
			} // Si el valor no esta en la lista de candidatos, marcar la celda con error
			else
			{
				cuadricula[row][col] &= ~0x4000;
			}
		}
		else
		{
			cuadricula[row][col] &= ~0x4000;
		}
	}
}

/* Recorre la cuadricula y para cada celda llama a cuadricula_candidatos_verificar */
static int
verificar_lista_calculada(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{
	int row = 0;
	while (row < NUM_FILAS)
	{
		int col = 0;
		while (col < NUM_COLUMNAS - 7)
		{
			cuadricula_candidatos_verificar(cuadricula, row, col);
			col++;
		}
		row++;
	}
	return errors;
}

/* *****************************************************************************
 * Funciones publicas
 * (pueden ser invocadas desde otro fichero) */

/* *******************************************cuadricula[NUM_FILAS][NUM_COLUMNAS]**********************************
 * programa principal del juego que recibe el tablero,
 * y la senyal de ready que indica que se han actualizado fila y columna */
void sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], char *ready)
{

	// Dos configuraciones para *init*

	celdas_vacias = sudoku_candidatos_init_c(cuadricula, 0); // Version C + C

	// verificar_lista_calculada(cuadricula);

	// celdas_vacias = sudoku_candidatos_init_c(cuadricula, 1); // Version C + ARM
	// verificar_lista_calculada(cuadricula);

	// celdas_vacias = sudoku_candidatos_init_c(cuadricula, 2); // Version C + Thumb
	// verificar_lista_calculada(cuadricula);

	// celdas_vacias = sudoku_candidatos_init_arm(cuadricula, 0); // Version ARM + C
	// verificar_lista_calculada(cuadricula);

	// celdas_vacias = sudoku_candidatos_init_arm(cuadricula, 1); // Version ARM + ARM
	// verificar_lista_calculada(cuadricula);

	// celdas_vacias = sudoku_candidatos_init_arm(cuadricula, 2); // Version ARM + Thumb
	// verificar_lista_calculada(cuadricula);
}
