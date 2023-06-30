#ifndef MAP_CPP
#define MAP_CPP

#include <cstdio>
#include <cstdlib>

#define ERROR_CODE -1
#define SUCCES_CODE 0

template<typename Key, typename Object>
class Map {
  private:
    Key* keys = nullptr;
    Object* array = nullptr;
    size_t capacity = 300;
    size_t count = 0;
  public:
    Map() {
      this->keys = new Key[this->capacity];
      this->array = new Object[this->capacity];
    }

    ~Map() {
      delete [] this->keys;
      delete [] this->array;
    }

    Object& operator[](const Key key) {
      size_t index = ERROR_CODE;
      for (size_t i = 0; i < this->capacity; ++i) {
        if (key == this->keys[i]) {
          index = i;
          break;
        }
      }
      if (index != ERROR_CODE) {
        return this->array[index]; 
      }
      Object* error = new Object();
      return *error;
    }

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
};

#endif
