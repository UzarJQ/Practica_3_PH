/*********************************************************************************************
 * Fichero:		timer.h
 * Autor:
 * Descrip:		funciones de control del timer0 del s3c44b0x
 * Version:
 *********************************************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_

/*--- variables visibles del m�dulo timer.c/timer.h ---*/
int switch_leds;

/*--- declaracion de funciones visibles del m�dulo timer.c/timer.h ---*/
void timer_init(void);
void timer1_inicializar(void);
unsigned int timer1_leer(void);
unsigned int timer1_parar(void);
void timer2_init(void);
void timer2_ISR(void);

#endif /* _TIMER_H_ */
