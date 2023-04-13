/// Copyright 2023 <Daniel Pérez-Morera>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmath>
#include <iostream>
#include <vector>

#include "Pthread.hpp"

#define RANDOM_MAX 10000

/// @brief Representacion digital de la papa
struct Papa{
  size_t miembros = 100;
  ssize_t value = 2026;
  ssize_t rotacion = -1;
};

/// @brief Dados privados de cada hilo
struct private_data{
  Papa* papa;
  Pthread* hilo;
  private_data* siguiente;
  private_data* anterior;
};

/// Aplica las reglas de Collatz al valor de la papa
/// Se simula el estallido de la papa cuando el valor
/// retornado por esta función es uno
ssize_t cambiarPapa(ssize_t& value) {
  if (1 == (value & 0x1)) {  // papa es impar
    value = (value << 1) + value + 1;  // papa = papa * 2 + papa + 1
  } else {
    value >>= 1;  // n = n / 2, utiliza corrimiento a la derecha, una posicion
  }
  return value;
}

/// @brief Simula los players del la papa caliente
/// @param data Datos privados del proceso
/// @return NULL
void* simulation(void* data) {
  // Se reconstruye los parametros
  assert(data);
  private_data& my_data = *reinterpret_cast<private_data*>(data);

  // Inicio del juego
  while (true) {
    // Todos los hilos deben esperar su turno con la papa
    my_data.hilo->wait();
    if (my_data.papa->miembros == 1
      || cambiarPapa(my_data.papa->value) == 1) {
      // Se aplica collatz para ver si la papa explot
      // Se contabiliza la cantidad de miembros
      if (--my_data.papa->miembros == 1) {
        // Si los miembros son dos tenemos un ganador
        std::cout << "Proceso "
          << my_data.hilo->get_process_number()
          << " fuera del juego" << std::endl;
        my_data.siguiente->hilo->signal();  // Señal para reanudar juego
      } else if (my_data.papa->miembros == 0) {
        // Si solo hay un miebro, el es el ganador
        std::cout << "Proceso "
          << my_data.hilo->get_process_number()
          << " es el ganador" << std::endl;
      } else {  // Si hay mas de dos miembros el juego se debe restablecer
        unsigned int seed = my_data.hilo->get_process_number();
        my_data.papa->value = rand_r(&seed) % RANDOM_MAX;
        my_data.anterior->siguiente = my_data.siguiente;
        my_data.siguiente->anterior = my_data.anterior;
        std::cout << "Proceso "
          << my_data.hilo->get_process_number()
          << " fuera del juego" << std::endl;
        my_data.siguiente->hilo->signal();  // Señal para reanudar juego
      }
      break;
    } else {  // Si la papa no explota se la pazo al siguiente
      my_data.siguiente->hilo->signal();
    }
  }
  return nullptr;
}

/// @brief Funcion incial, inicializa el juego de la papa
/// @param argc argument count
/// @param argv argument vector
/// @return Status of execution
int main(int argc, char** argv) {
  // Valores de la papa
  Papa papa;

  if (argc == 4) {  /// Leemos argumentos
    papa.value = atoi(argv[1]);
    papa.miembros = atoi(argv[2]);
    if (argv[3][0] == 'l') {
      papa.rotacion = -1;
    } else if (argv[3][0] == 'r') {
      papa.rotacion = 1;
    } else {
      perror("Invalid arguments");
      exit(0);
    }
  } else if (argc > 1) {
    perror("Invalid arguments");
    exit(0);
  }

  const size_t miembros_count = papa.miembros;

  // Genera estructura que controlara los players y su estructura de datos
  std::vector<Pthread> miembros(papa.miembros);
  std::vector<private_data> private_data(papa.miembros);

  std::cout << "Creando una ronda de " << miembros_count
    <<" participantes" << std::endl;
  // Inicializa cada hilo con su respectiva posicion y su estrcutura
  for (size_t i = 0; i < miembros_count; ++i) {
    miembros[i] = Pthread(i);
    miembros[i].wait();
    private_data[i].papa = &papa;
    private_data[i].hilo = &miembros[i];
    // Dependiendo de la rotacion asigna el siguiente y anterior de cada jugador
    ssize_t siguiente = ((ssize_t)i + papa.rotacion) % (ssize_t)papa.miembros;
    ssize_t anterior = ((ssize_t)i - papa.rotacion) % (ssize_t)papa.miembros;
    if (siguiente == -1) siguiente = papa.miembros - 1;
    if (anterior == -1) anterior = papa.miembros - 1;
    private_data[i].siguiente = &private_data[siguiente];
    private_data[i].anterior = &private_data[anterior];
    miembros[i].run(simulation, &private_data[i]);
  }

  // Signal para el inicio del juego entre el resto de hilos
  // Se elige quien inicia mediante un random
  srand(time(NULL));
  size_t first = rand() % miembros_count;
  std::cout << "Receptor del primer mensaje " << first
      << ", valor de la papa " << papa.value << std::endl;
  private_data[first].hilo->signal();

  // Hilo principal espera que el resto de hilos finalicen el juego
  for (size_t i = 0; i < miembros_count; ++i) {
    miembros[i].join();
    miembros[i].signal();
  }
  return EXIT_SUCCESS;
}
