#ifndef MAP_CPP
#define MAP_CPP

#include <cstdio>
#include <cstdlib>

#define ERROR_CODE -1
#define SUCCES_CODE 0

/// @brief Mapa dinamico
/// @tparam Key Llave del mapa
/// @tparam Object Contenido que almacena
template<typename Key, typename Object>
class Map {
  private:
    /// @brief Vector con llaves actuales
    Key* keys = nullptr;
    /// @brief Vector con los objetos actuales
    Object* array = nullptr;
    /// @brief Capacidad inicial del mapa
    size_t capacity = 300;
    /// @brief Numero de elementos actuales
    size_t count = 0;
  public:
    /// @brief Constructor por defecto
    Map() {
      this->keys = new Key[this->capacity];
      this->array = new Object[this->capacity];
    }

    /// @brief Destructor por defecto
    ~Map() {
      delete [] this->keys;
      delete [] this->array;
    }

    /// @brief Sobrecarga de operador []
    /// @param key Llave utilizada como indice
    /// @return Objeto relacionado
    Object& operator[](const Key key) {
      ssize_t index = -1;
      while (key != this->keys[++index] &&
        (size_t)index < this->count);
      return this->array[index];
    }

    /// @brief Inserta una nueva dupla en el mapa
    /// @param key Llave utilizada como index
    /// @param object Objeto relacionado con la llave
    void insert(Key key, Object object) {
      if (this->count >= this->capacity) {
        this->capacity = this->capacity * 2;
        this->keys = (Key*)
          realloc((void*)this->keys, capacity * sizeof(Key));
        this->array = (Object*)
          realloc((void*)this->array, capacity * sizeof(Object));
      }
      this->keys[this->count] = key;
      this->array[this->count++] = object;
    }

    /// @brief Remueve un elemento del mapa
    /// @param key Llave del elemento que se remueve
    void remove(Key key) {
      ssize_t index = -1;
      while (key != this->keys[++index]);
      for (size_t i = index; i < count; ++i) {
        if (i + 1 < count) {
          this->keys[i] = this->keys[i + 1];
          this->array[i] = this->array[i + 1];
        }
      }
      --count;
    }

    /// @brief Obtiene numero de elementos del mapa
    /// @return Numero de elementos del mapa
    size_t getCount() {
      return this->count;
    }

    /// @brief Obtiene lista de llaves en el mapa
    /// @return Lista de llaves
    Key* getKeys() {
      Key* copyKeys = nullptr;
      if (this->count > 0) {
        copyKeys = new Key[this->count];
        for (size_t i = 0; i < this->count; ++i)
          copyKeys[i] = this->keys[i];
      }
      return copyKeys;
    }

};

#endif
