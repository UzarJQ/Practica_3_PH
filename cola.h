#ifndef COLA_H
#define COLA_H

#include <stdint.h>
#include "eventos.h"

#define stack_size 100 // Tamaño de la pila de depuración

typedef struct
{
  Event ID_evento;
  uint32_t datos_complementarios;
  unsigned int momento;
} EventoDebug;

// Declaración de funciones para la pila de depuración
void push_debug(uint8_t ID_evento, uint32_t datos_complementarios, unsigned int momento);

#endif // COLA_H
