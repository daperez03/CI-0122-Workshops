/// Copyright 2023 Daniel Perez-Morera
/**
 *  CI0122 Sistemas Operativos
 *  Grupo 2, 2022-ii
 *
 *  Problema de la "papa caliente"
 *
 *  Se crea una ronda de n procesos (fork) que se intercambian mensajes por medio de buzones
 *
 **/

#include <stdio.h>  // printf function
#include <stdlib.h>
#include <unistd.h>  // _exit function
#include <time.h>
#include <iostream>
#include <vector>

#include "Semaphore.h"
#include "ShM.h"

#define MAXRANDOM 10000

/// @brief Atributos de la papa->
struct Papa{
  size_t players = 100;
  ssize_t value = 2026;
  ssize_t rotacion = -1;
};

/// @brief Dados privados de cada proceso.
struct player_data{
  Papa* papa;
  size_t process_id;
  ssize_t next_process;
  bool live = true;
  Semaphore* semaphores;
};



/// @brief Aplica las reglas de Collatz al valor de la papa->
/// Se simula el estallido de la papa cuando el valor retornado
/// por esta función es uno.
/// @param value Valor actual de la papa->
/// @return Nuevo valor de la papa->
ssize_t collatz(ssize_t& value) {
  if (1 == (value & 0x1)) {  // papa es impar
    value = (value << 1) + value + 1;  // papa = papa * 2 + papa + 1
  } else {
    value >>= 1;  // n = n / 2, utiliza corrimiento a la derecha, una posicion
  }
  return value;
}


/// @brief Procedimiento para simular una persona participante en la ronda.
/// Recibe la identificación del buzón y la identificación de la persona.
/// @param ply_data Datos de cada proceso.
void persona(player_data& ply_data) {
  while (true) {
    ply_data.semaphores->Wait(ply_data.process_id);
    if (ply_data.live) {
      if (ply_data.papa->players == 1) {
        /// Win
        std::cout << "Proceso " << ply_data.process_id
          << " es el ganador" << std::endl;
        ply_data.papa->value = -1;
        ply_data.semaphores->Signal(ply_data.next_process);
        break;
      } else {
        if (1 == collatz(ply_data.papa->value)) {
          /// Explota
          ply_data.live = false;
          --ply_data.papa->players;
          unsigned int seed = ply_data.process_id;
          ply_data.papa->value = rand_r(&seed) % MAXRANDOM;
          std::cout << "Proceso " << ply_data.process_id
            << " fuera del juego, participantes activos: "
            << ply_data.papa->players << std::endl;
        }
        ply_data.semaphores->Signal(ply_data.next_process);
      }
    } else {
      if (ply_data.papa->value < 0) {
        ply_data.semaphores->Signal(ply_data.process_id);
        ply_data.semaphores->Signal(ply_data.next_process);
        break;
      } else {
        ply_data.semaphores->Signal(ply_data.next_process);
      }
    }
  }
}

/// @brief Main process.
/// @param argc Numero de argumentos digitados.
/// @param argv Vector de argumentos.
/// @return Codigo de error.
int main(int argc, char ** argv) {
  int st = -1;
  ShM papa_shm(sizeof(Papa));
  Papa* papa = reinterpret_cast<Papa*>(papa_shm.attach());
  papa->players = 100;
  papa->rotacion = -1;
  papa->value = 2023;
  srand(time(NULL));

  if (argc == 4) {  /// Leemos argumentos
    papa->value = atoi(argv[1]);
    papa->players = atoi(argv[2]);
    if (argv[3][0] == 'l') {
      papa->rotacion = -1;
    } else if (argv[3][0] == 'r') {
      papa->rotacion = 1;
    } else {
      perror("Invalid arguments");
      exit(0);
    }
  } else if (argc > 1) {
    perror("Invalid arguments");
    exit(0);
  }
  const size_t players = papa->players;
  Semaphore semaphores(players + 1, 0);

  std::cout << "Creando una ronda de " << players
    <<" participantes" << std::endl;
  for (size_t i = 0; i < players; ++i) {
    st = fork();
    if (!st) {
      player_data ply_data;
      ply_data.semaphores = &semaphores;
      ply_data.papa = papa;
      ply_data.process_id = i;
      ply_data.next_process = i + papa->rotacion;
      if (ply_data.next_process == -1) {
        ply_data.next_process = players - 1;
      } else {
        ply_data.next_process %= players;
      }
      persona(ply_data);  // Este proceso simula una persona participante
      if (ply_data.live) semaphores.Signal(players);
      break;
    }
  }

  if (st) {
    // Escoge el proceso que comienza el juego
    size_t first = rand() % players;
    std::cout << "Receptor del primer mensaje " << first
      << ", valor de la papa " << papa->value << std::endl;
    semaphores.Signal(first);
    /// Main thread esperar hasta que finalicen los procesos
    semaphores.Wait(players);
    // Elimina los recursos compartidos
    papa_shm.detach();
  }
  return EXIT_SUCCESS;
}

