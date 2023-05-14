#include "filesTable.h"
#include <iostream>

NachosOpenFilesTable::NachosOpenFilesTable() {
  this->openFiles = new int(TABLE_SIZE);
  this->openFilesMap = new BitMap(TABLE_SIZE);
  this->usage = 1;
}

NachosOpenFilesTable::~NachosOpenFilesTable() {
  if (this->usage == 1) {
    delete this->openFilesMap;
    delete this->openFiles;
  }
}

int NachosOpenFilesTable::Open(int UnixHandle) {
  OpenFileId fd = this->openFilesMap->Find();
  this->openFiles[fd] = UnixHandle;
  return fd;
}

int NachosOpenFilesTable::Close(int NachosHandle) {
  int status = -1;
  if (this->openFilesMap->Test(NachosHandle)) {
    OpenFileId fd = this->openFiles[NachosHandle];
    this->openFilesMap->Clear(NachosHandle);
    status = close(fd);
  }
  return status;
}

bool NachosOpenFilesTable::isOpened(int NachosHandle) {
  return this->openFilesMap->Test(NachosHandle);
}

int NachosOpenFilesTable::getUnixHandle(int NachosHandle) {
  int status = -1;
  if (this->openFilesMap->Test(NachosHandle)) {
    OpenFileId fd = NachosHandle;
    status = fd;
  }
  return status;
}

void NachosOpenFilesTable::addThread() {
  ++this->usage;
}

void NachosOpenFilesTable::delThread() {
  --this->usage;
}

void NachosOpenFilesTable::Print() {
  printf("nachOS FD\tUnix FD\n");
  for (size_t i = 0; i < TABLE_SIZE; ++i) {
    if (this->openFilesMap->Test(i)) {
      printf("%i\t%i\n", i, this->openFiles[i]);
    }
  }
}
