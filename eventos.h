#ifndef _EVENTOS_H_
#define _EVENTOS_H_

typedef enum
{
  PRESSED_IRQ = 1,
  PRESSED_FIQ = 2,
  RELEASED_IRQ = 3,
  RELEASED_FIQ = 4,
  TIMER1_IRQ = 5,
  TIMER1_FIQ = 6
} Event;

#endif