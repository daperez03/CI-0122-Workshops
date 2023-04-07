/// Copyright 2023 <Daniel Pérez-Morera>
#ifndef PTHREAD
#define PTHREAD

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

/// @brief Encapsulacion de las funciones de Pthread(Unix)
class Pthread{
 public:
  /// @brief Constructor de clase Pthread
  /// @param process_number Numero del hilo creado
  explicit Pthread(size_t process_number = 0);
  /// Destructor de clase Pthread
  ~Pthread();
  /// @brief Inicializa la ejecucion del hilo
  /// @param subroutine Subrutina que se debe ejecutar
  /// @param data datos requeridos por el proceso
  /// @return Codigo de ejecucion, Exit success = 0
  size_t run(void*(*subroutine)(void*), void* data);
  /// @brief Espera que el hilo finalice su ejecucion
  /// @return Datos devueltos por la subrutina
  void* join();
  /// @brief Bloque una region del codigo para otros hilos
  /// @return Codigo de ejecucion, Exit success = 0
  size_t wait();
  /// @brief Avilita region de codigo bloqueada con wait
  /// @return Codigo de ejecucion, Exit success = 0
  size_t signal();
  /// @brief Brinda el numero de proceso de este Pthread
  /// @return Numero de proceso
  size_t get_process_number();

 private:
  /// @brief Numero de proceso
  size_t process_number;
  /// @brief ID del hilo
  pthread_t thread_id;
  /// @brief Mutex para aplicar wait & signal
  pthread_mutex_t mutex;
};

#endif
