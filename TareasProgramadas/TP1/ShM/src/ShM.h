/// Copyright 2023 Daniel Perez-Morera
#ifndef SHM
#define SHM
/** class to encapsulate Unix shared memory intrinsic structures and system calls
  *  Author: Operating systems (Francisco Arroyo)
  *  Version: 2023/Mar/15
  *
  * Ref.: https://en.wikipedia.org/wiki/Shared_memory
  *
 **/

/// @brief Clase encapsuladora de la funciones shm de UNIX
class ShM {
 public:
  /// @brief  Constructor de memoria compartida
  /// @param  size Size de la memoria compartida
  explicit ShM(size_t = 0);  // Parameter represent segment size
  /// @brief Destructor de memoria compartida
  ~ShM();
  /// @brief Indicador de la ubicacion de memoria compartida
  /// @return Puntero a la estructura de datos compartida
  void * attach();
  /// @brief Elimina el area de memoria compartida
  /// @return Error code
  int detach();

 private:
  int id;  // shared memory indentifier
  int father_id;  // IP of father
  void * area;  // pointer to shared memory area
};

#endif
