/// Copyright 2023 Daniel Perez-Morera
/** class to encapsulate Unix semaphore intrinsic structures and system calls
  *  Author: Operating systems (Francisco Arroyo)
  *  Version: 2023/Mar/15
  *
  * Ref.: https://en.wikipedia.org/wiki/Semaphore_(programming)
  *
 **/

/// @brief Clase encapsuladora de la funcion sem de UNIX
class Semaphore {
 public:
  /// @brief Constructor multifuncional
  /// @param sem_count Numero de semaforos que se desea crear
  /// @param initialValue Valor inicial de los semaforos
  explicit Semaphore(int sem_count = 1, int initialValue = 0);
  // Parameter is initial value
  ///@brief Destructor de los semaforos creados
  ~Semaphore();
  /// @brief Incrementa el valor de un semaforo en 1
  /// @param sem_number Numero del semaforo que se quiere incrementar
  /// @return Error code
  int Signal(int sem_number = 0);
  // Is the same as V method in Dijkstra definition
  /// @brief Decrementa el valor de un semaforo en 1
  /// @param sem_number Semaforo el cual se desea decrementar
  int Wait(int sem_number = 0);
  // Is the same as P method in Dijkstra definition

 private:
  int id;  // Semaphore indentifier;
  int father_id;  // IP of father
  int sem_count;  // Number of semaphore
};

