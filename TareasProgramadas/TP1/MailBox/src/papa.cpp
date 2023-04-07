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

#include "MailBox.h"

#define MAXRANDOM 10000

/// @brief Atributos de la papa.
struct Papa{
  size_t players = 10;
  ssize_t value = 2026;
  ssize_t rotacion = -1;
};

/// @brief Dados privados de cada proceso.
struct player_data{
  Papa papa;
  size_t process_id;
  size_t next_process;
  bool live = true;
  MailBox mail;
};



/// @brief Aplica las reglas de Collatz al valor de la papa.
/// Se simula el estallido de la papa cuando el valor retornado
/// por esta función es uno.
/// @param value Valor actual de la papa.
/// @return Nuevo valor de la papa.
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
  int st = -1;
  while (true) {
    ply_data.mail.recv(ply_data.process_id
      ,reinterpret_cast<void*>(&ply_data.papa), sizeof(Papa));
    if (ply_data.live) {
      if (ply_data.papa.players == 1) {
        /// Win
        std::cout << "Proceso " << ply_data.process_id
          << " es el ganador" << std::endl;
        ply_data.papa.value = -1;
        ply_data.mail.send(ply_data.next_process
          ,reinterpret_cast<void*>(&ply_data.papa), sizeof(Papa));
        break;
      } else {
        if (1 == collatz(ply_data.papa.value)) {
          /// Explota
          ply_data.live = false;
          --ply_data.papa.players;
          unsigned int seed = ply_data.process_id;
          ply_data.papa.value = rand_r(&seed) % MAXRANDOM;
          std::cout << "Proceso " << ply_data.process_id
            << " fuera del juego, participantes activos: "
            << ply_data.papa.players << std::endl;
        }
        ply_data.mail.send(ply_data.next_process
          ,reinterpret_cast<void*>(&ply_data.papa), sizeof(Papa));
      }
    } else {
      st = ply_data.mail.send(ply_data.next_process
        ,reinterpret_cast<void*>(&ply_data.papa), sizeof(Papa));
      if (st == -1) break;
    }
  }
}

/// @brief Main process.
/// @param argc Numero de argumentos digitados.
/// @param argv Vector de argumentos.
/// @return Codigo de error.
int main(int argc, char ** argv) {
  int st = -1;
  Papa papa;
  srand(time(NULL));

  if (argc == 4) {  /// Leemos argumentos
    papa.value = atoi(argv[1]);
    papa.players = atoi(argv[2]);
    if (argv[3][0] == 'l') {
      papa.rotacion = -1;
    } else if (argv[3][0] == 'r') {
      papa.rotacion = 1;
    } else {
      perror("Invalid arguments");
      exit(-1);
    }
  } else if (argc > 1) {
    perror("Invalid arguments");
    exit(-1);
  }

  const size_t players = papa.players;

  std::cout << "Creando una ronda de " << players
    <<" participantes" << std::endl;
  for (size_t i = 0; i < players; ++i) {
    st = fork();
    if (!st) {
      player_data ply_data;
      ply_data.process_id = i + 1;
      ply_data.next_process = ply_data.process_id + papa.rotacion;
      if (ply_data.next_process == 0) {
        ply_data.next_process = players;
      } else if (ply_data.next_process == players + 1) {
        ply_data.next_process = 1;
      }
      persona(ply_data);  // Este proceso simula una persona participante
      if (ply_data.live) {
        // Send to main process
        ply_data.mail.send(players + 1
          , reinterpret_cast<void*>(&ply_data.papa), sizeof(Papa));
      }
      break;
    }
  }

  if (st) {
    // Escoge el proceso que comienza el juego
    size_t first = (rand() % players) + 1;
    std::cout << "Receptor del primer mensaje " << first
      << ", valor de la papa " << papa.value << std::endl;
    MailBox mail;
    mail.send(first, reinterpret_cast<void*>(&papa),
      sizeof(Papa));
    /// Main thread esperar hasta que finalicen los procesos
    mail.recv(players + 1 ,reinterpret_cast<void*>(&papa), sizeof(Papa));
  }
  return EXIT_SUCCESS;
}

