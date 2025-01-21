#include "sudoku_2024.h"
#include <stdio.h>
#include <stdlib.h>
#include "Bmp.h"
#include "lcd.h"

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

	if ((cuadricula[row][col] & 0x8000) == 0) // Verifica solo las celdas que no sean una pista
	{
		int displace = 3 + (int)valor;

		if (valor != 0x0000) // Si el valor ingresado es distinto de 0, se verifica que este en la lista de candidatos
		{
			if ((cuadricula[row][col] & (1 << displace)) == 0) // Si el valor no pertenece a los posibles candidatos, se marca como error
			{
				cuadricula[row][col] |= 0x4000;
				errors++;
			}
			else
			{
				cuadricula[row][col] &= ~0x4000;
			}
		}
		else // Si se limpia la celda (poner valor = 0) se limpia el bit de error
		{
			cuadricula[row][col] &= ~0x4000;
		}
	}
}
/*********************************************************************************************
 * name:		LCD_mostrar_sudoku()
 * func:		Muestra la cuadricula del sudoku en la pantalla LCD
 * desc:		Imprime la cuadricula 9x9 y los candidatos 3x3 de cada celda.
 * 					Si la celda tiene un valor, lo imprime en el centro de la celda.
 * 					Si la celda no tiene valor, imprime los candidatos.
 *          Si la celda contiene un error, imprime una "E" en la celda.
 *********************************************************************************************/
extern void LCD_mostrar_sudoku(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{
	Lcd_Clr();
	Lcd_Active_Clr();
	Lcd_DspAscII8x16(5, 5, BLACK, "Tiempo: ");

	int i, j, k, l;

	// Numeración de filas
	for (i = 0; i < NUM_FILAS; i++)
	{
		INT16 filaX = 25;							 // Desplazamos los números de las filas un poco más a la izquierda
		INT16 filaY = 28 + 22 * i + 8; // Bajamos los números de las filas para centrarlos más
		char filaValor[2] = {i + '1', '\0'};
		Lcd_DspAscII8x16(filaX, filaY, BLACK, filaValor);
	}

	// Numeración de columnas
	for (j = 0; j < NUM_COLUMNAS - 7; j++) // Excluye las columnas extra
	{
		INT16 colX = 35 + 30 * j + 12; // Ajuste para mover la numeración de las columnas más a la derecha
		INT16 colY = 20;							 // Subimos los números de las columnas para que no estén sobre la cuadrícula
		char colValor[2] = {j + '1', '\0'};
		Lcd_DspAscII8x16(colX, colY, BLACK, colValor);
	}

	// Dibuja la cuadrícula
	for (i = 0; i < NUM_FILAS; i++) // Recorre las filas
	{
		for (j = 0; j < NUM_COLUMNAS - 7; j++) // Recorre las columnas
		{
			INT16 usLeft = 35 + 30 * j; // Posición de la izquierda de cada celda
			INT16 usTop = 35 + 22 * i;	// Posición de la parte superior de cada celda
			INT16 usRight = usLeft + 29;
			INT16 usBottom = usTop + 21;

			Lcd_Draw_Box(usLeft, usTop, usRight, usBottom, BLACK);

			uint8_t valor = celda_leer_valor(cuadricula[i][j]);
			if (valor == 0)
			{
				// Dibuja una sub-cuadrícula de 3x3 dentro de cada cuadro grande
				for (k = 0; k < 3; k++)
				{
					for (l = 0; l < 3; l++)
					{
						INT16 subLeft = usLeft + 9 * l;
						INT16 subTop = usTop + 7 * k;
						INT16 subRight = subLeft + 8;
						INT16 subBottom = subTop + 6;

						int displace = 4 + (k * 3) + l;

						uint16_t candidato = (cuadricula[i][j] >> displace) & 1;

						if (candidato == 1)
						{
							INT16 centerX = subLeft + (subRight - subLeft) / 2 - 2;
							INT16 centerY = subTop + (subBottom - subTop) / 2 - 7;
							Lcd_DspAscII8x16(centerX, centerY, BLACK, "x");
						}
					}
				}
			}
			else
			{
				INT16 centerX = usLeft + 10;
				INT16 centerY = usTop + 4;

				if (cuadricula[i][j] & 0x8000)
				{
					LcdClrRect(usLeft, usTop, usRight, usBottom, LIGHTGRAY);
				}

				if (cuadricula[i][j] & 0x4000)
				{
					INT16 errorX = usRight - 8;
					INT16 errorY = usTop + 2;
					Lcd_DspAscII8x16(errorX, errorY, BLACK, "e");
				}

				char string_value[2] = {valor + '0', '\0'};
				Lcd_DspAscII8x16(centerX, centerY, BLACK, string_value);
			}
		}
	}

	BitmapView(125, 135, Stru_Bitmap_gbMouse);
	Lcd_Dma_Trans();
}

// extern void LCD_mostrar_sudoku(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
// {
// 	Lcd_Clr();
// 	Lcd_Active_Clr();

// 	int i, j, k, l;

// 	for (i = 0; i < NUM_FILAS; i++) // Numeración de filas
// 	{
// 		INT16 filaX = 28;					 // Posición fija de la numeración de filas
// 		INT16 filaY = 28 + 22 * i; // La posición de la fila se ajusta a la cuadrícula
// 		char filaValor[2] = {i + '1', '\0'};
// 		Lcd_DspAscII8x16(filaX, filaY, BLACK, filaValor);
// 	}

// 	for (j = 0; j < NUM_COLUMNAS - 7; j++) // Numeración de columnas
// 	{
// 		INT16 colX = 35 + 30 * j; // Posición de la numeración de columnas
// 		INT16 colY = 28;					// Fija la posición de la numeración en la parte superior
// 		char colValor[2] = {j + '1', '\0'};
// 		Lcd_DspAscII8x16(colX, colY, BLACK, colValor);
// 	}

// 	for (i = 0; i < NUM_FILAS; i++) // Recorre las filas
// 	{
// 		for (j = 0; j < NUM_COLUMNAS - 7; j++) // Recorre las columnas
// 		{
// 			INT16 usLeft = 35 + 30 * j; // Cambié el índice de 'i' a 'j'
// 			INT16 usTop = 35 + 22 * i;	// Cambié el índice de 'j' a 'i'
// 			INT16 usRight = usLeft + 29;
// 			INT16 usBottom = usTop + 21;

// 			Lcd_Draw_Box(usLeft, usTop, usRight, usBottom, BLACK);

// 			uint8_t valor = celda_leer_valor(cuadricula[i][j]);
// 			if (valor == 0)
// 			{
// 				// Dibuja una sub-cuadrícula de 3x3 dentro de cada cuadro grande
// 				for (k = 0; k < 3; k++)
// 				{
// 					for (l = 0; l < 3; l++)
// 					{
// 						INT16 subLeft = usLeft + 9 * l;
// 						INT16 subTop = usTop + 7 * k;
// 						INT16 subRight = subLeft + 8;
// 						INT16 subBottom = subTop + 6;

// 						int displace = 4 + (k * 3) + l;

// 						uint16_t candidato = (cuadricula[i][j] >> displace) & 1;

// 						if (candidato == 1)
// 						{
// 							INT16 centerX = subLeft + (subRight - subLeft) / 2 - 2;
// 							INT16 centerY = subTop + (subBottom - subTop) / 2 - 7;
// 							Lcd_DspAscII8x16(centerX, centerY, BLACK, "x");
// 						}
// 					}
// 				}
// 			}
// 			else
// 			{
// 				INT16 centerX = usLeft + 10;
// 				INT16 centerY = usTop + 4;

// 				// if (cuadricula[i][j] & 0x4000)
// 				// {
// 				// 	Lcd_DspAscII8x16(centerX, centerY, BLACK, "E");
// 				// }
// 				// else
// 				// {
// 				char string_value[2] = {valor + '0', '\0'};
// 				Lcd_DspAscII8x16(centerX, centerY, BLACK, string_value);
// 				// }
// 			}
// 		}
// 	}

// 	BitmapView(125, 135, Stru_Bitmap_gbMouse);
// 	Lcd_Dma_Trans();
// }

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
