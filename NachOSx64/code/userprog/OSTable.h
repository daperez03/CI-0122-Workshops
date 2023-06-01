#ifndef FILESTABLE_CC
#define FILESTABLE_CC

#include "bitmap.h"

#define TABLE_SIZE 100

/// @brief Tabla de Control
/// @tparam DataType Tipo de dato de la tabla
template <typename DataType>
class ControlTable {
 public:
  /// @brief Constructor por default de una tabla de control
  ControlTable() {
    this->openObjects = new DataType[TABLE_SIZE];
    this->logicalMap = new BitMap(TABLE_SIZE);
    memset(this->openObjects, 0, TABLE_SIZE);
    this->usage = 1;
  }

  /// @brief Destructor por defualt de una tabla de control
  ~ControlTable() {
    if (this->usage == 1) {
      // for (int i = 0; i < TABLE_SIZE; ++i)
      //   if(this->logicalMap->Test(i)) delete this->openObjects[i];
      delete this->logicalMap;
      delete this->openObjects;
    }
  }

  /// @brief Agrega un nuevo elemento a la tabla de control
  /// @param object Elemento nuevo
  /// @return ID relacionado con el nuevo elemento
  int Open(DataType object) {
    int id = this->logicalMap->Find();
    this->openObjects[id] = object;
    return id;
  }

  /// @brief Borra un elemento de la tabla de control
  /// @param id ID del elemento que se desea borrar
  /// @return elemento que se saco de la tabla
  DataType Close(int id) {
    ASSERT(id < TABLE_SIZE && id >= 0);
    DataType object;
    if (this->logicalMap->Test(id)) {
      object = this->openObjects[id];
      this->logicalMap->Clear(id);
      memset(&this->openObjects[id],0, sizeof(DataType));
    }
    return object;
  }

  /// @brief Indica si un id exste en la tabla
  /// @param id ID del objeto que se quiere verificar
  /// @return True si el elemento se encuentra, false si no se encontro el elemento
  bool isOpened(int id) {
    ASSERT(id < TABLE_SIZE && id >= 0);
    return this->logicalMap->Test(id);
  }

  /// @brief Retorna el objeto relacionado con un ID
  /// @param id ID relacionado con el objeto
  /// @return Objeto hallado
  DataType getObject(int id) {
    ASSERT(id < TABLE_SIZE && id >= 0);
    DataType object;
    if (this->logicalMap->Test(id)) {
      object = this->openObjects[id];
    }
    return object;
  }

  /// @brief Retorna el ID relacionado a un objeto
  /// @param object Objeto del cual deseamos obtener un ID
  /// @return ID del objeto, -1 en caso de error
  int getID(DataType object) {
    int id = -1;
    for (int i = 0; i < TABLE_SIZE; ++i) {
      if(this->openObjects[i] == object) {
        id = i;
        break;
      }
    }
    return id;
  }

  /// @brief Aumenta la variable de usuarios, que indica cuantos procesos estan utilizando la tabla
  void addThread() {
    ++this->usage;
  }

  /// @brief Decrementa la variable de usuarios, que indica cuantos procesos estan utilizando la tabla
  void delThread() {
    --this->usage;
  }

  /// @brief Retorna el numero de procesos utilizando la tabla de control
  /// @return Numero de proceos utilizando la tabal de control
  int getThreadCount() {
    return usage;
  }
 private:
  /// @brief Vector con los objetos 
  DataType * openObjects;

  /// @brief BitMap para controlar los id y espacios disponibles dentro de la tabla de control
  BitMap * logicalMap;
  
  /// @brief Numero de procesos utilizando la tabla de control
  int usage;
};

#endif