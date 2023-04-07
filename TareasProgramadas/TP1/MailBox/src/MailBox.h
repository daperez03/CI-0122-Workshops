/// Copyright 2023 Daniel Perez-Morera
/**
  *   C++ class to encapsulate Unix message passing intrinsic structures and system calls
  *
 **/

#define MAXDATA 1024

/// @brief Clase encapsuladroe de las funciones de msg en UNIX
class MailBox {
 public:
  /// @brief Constructor default
  /// Utiliza msgget para obtener una cola compartida
  MailBox();
  /// @brief Destructor de la cola compartida
  /// Utliza la funcion msgctl para eliminar
  /// la cola establecida en el constructor
  ~MailBox();
  /// @brief Envia un buffer a la cola
  /// @param type Tipo de dato el cual envio, esto para poder diferenciar entre
  /// los distintos mensajes
  /// @param buffer Un puntero a los datos los cuales se desan enviar
  /// @param capacity Capacidad e bytes del buffer
  /// @return Error code
  int send(long type, void * buffer, int capacity);
  /// @brief Espera un la cola el envio de un dato.
  /// @param type Indica el tipo especifico de dato que espera.
  /// @param buffer Buffer donde se desea almacenar el dato.
  /// @param capacity Capacidad e bytes del buffer.
  /// @return Bytes leidos en el mensaje.
  int recv(long type, void * buffer, int capacity);

 private:
  /// @brief ID de la cola
  int id;  // mailbox id
  int father_ip;
};

