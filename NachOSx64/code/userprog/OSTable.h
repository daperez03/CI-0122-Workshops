#ifndef FILESTABLE_CC
#define FILESTABLE_CC

#include "bitmap.h"

#define TABLE_SIZE 100

template <typename DataType>
class ControlTable {
 public:
  // Initialize
  ControlTable() {
    this->openObjects = new DataType[TABLE_SIZE];
    this->logicalMap = new BitMap(TABLE_SIZE);
    this->usage = 1;
  }
  // De-allocate
  ~ControlTable() {
    if (this->usage == 1) {
      // for (int i = 0; i < TABLE_SIZE; ++i)
      //   if(this->logicalMap->Test(i)) delete this->openObjects[i];
      delete this->logicalMap;
      delete this->openObjects;
    }
  }
  // Register the file handle
  int Open(DataType object) {
    int id = this->logicalMap->Find();
    this->openObjects[id] = object;
    return id;
  }
  // Unregister the file handle
  DataType Close(int id) {
    ASSERT(id < TABLE_SIZE && id >= 0);
    DataType object;
    if (this->logicalMap->Test(id)) {
      object = this->openObjects[id];
      this->logicalMap->Clear(id);
    }
    return object;
  }
  /// Return true if the id is in the table
  bool isOpened(int id) {
    ASSERT(id < TABLE_SIZE && id >= 0);
    return this->logicalMap->Test(id);
  }
  /// Return object of id
  DataType getObject(int id) {
    ASSERT(id < TABLE_SIZE && id >= 0);
    DataType object;
    if (this->logicalMap->Test(id)) {
      object = this->openObjects[id];
    }
    return object;
  }
  /// Return id of object
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
  // If a user thread is using this table, add it
  void addThread() {
    ++this->usage;
  }
  // If a user thread is using this table, delete it
  void delThread() {
    --this->usage;
  }
  int getThreadCount() {
    return usage;
  }
 private:
  // A vector with user opened files
  DataType * openObjects;
  // A bitmap to control our vector
  BitMap * logicalMap;
  // How many threads are using this table
  int usage;
};

#endif