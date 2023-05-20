#ifndef BABUINO_HPP
#define BABUINO_HPP

#define N 10
#define B 40

#include <mutex>
#include <thread>
#include <condition_variable>

/// @brief Estructura compartida de los Babuinos
struct SharedData {
  std::mutex canAcces;
  std::condition_variable ropeWaiting[N];
  int count[N] = {0};
  int countBabuinos = 0;
};


class Babuino{
 private:
  /// @brief Datos compartidos entre Babuinos
  struct SharedData& babuinoData;
 public:
  /// Constructor de Babuino, unicamente recibe un struct de datos
  /// necesarios para su ejecucion
  Babuino(struct SharedData& babuinoData);
  ~Babuino() = default;
  /// Simulacion de los babuinos, como dice en el enunciado solicita
  /// una cuerda y inicia su simulacion
  void run(int cuerda);
};
#endif
