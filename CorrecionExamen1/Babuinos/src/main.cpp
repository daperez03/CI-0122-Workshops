#include "Babuino.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <iostream>

#define MAX 500

/// Llama a babuinos.run para iniciar la simulacion de los hilos
void createBabuino(int cuerda, void* babuinosData) {
  struct SharedData* data = (struct SharedData*)babuinosData;
  Babuino babuino(*data);
  babuino.run(cuerda);
}

int main() {
  int status = 0;
  std::thread hilos[MAX];
  struct SharedData babuinosData;
  /// Crea hilos respecto a MAX
  for(int i = 0; i < MAX; ++i)
    hilos[i] = std::thread(createBabuino, (rand() % N), (void*)&babuinosData);
  /// Espera hilos respecto a MAX
  for(int i = 0; i < MAX; ++i)
    hilos[i].join();
}
