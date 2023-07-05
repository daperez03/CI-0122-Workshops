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
      ssize_t index = -1;
      while (key != this->keys[++index] &&
        (size_t)index < this->count);
      return this->array[index];
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
    
    size_t getCount() {
      return this->count;
    }

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
