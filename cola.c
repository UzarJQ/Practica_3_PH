#include "cola.h"
#include "8led.h"
#include "eventos.h"

/*--- Definiciones de la Pila de Depuración ---*/

EventoDebug pila_debug[stack_size];
unsigned int indice_pila = 0;

/*--- Funciones ---*/

// Agregar un evento a la pila de depuración
void push_debug(int ID_evento, uint32_t datos_complementarios, unsigned int momento)
{

  pila_debug[indice_pila].ID_evento = ID_evento;
  pila_debug[indice_pila].datos_complementarios = datos_complementarios;
  pila_debug[indice_pila].momento = momento;

  // Incrementar el �ndice de manera circular
  indice_pila = (indice_pila + 1) % stack_size;
}
