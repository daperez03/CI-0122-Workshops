/// Copyright 2023 Daniel Perez-Morera
/**
  *  C++ class to encapsulate Unix shared memory intrinsic structures and system calls
  *  Author: Operating systems (Francisco Arroyo)
  *  Version: 2023/Mar/15
  *
  * Ref.: https://en.wikipedia.org/wiki/Shared_memory
  *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>


#include "ShM.h"

#define KEY 0xC15906

/**
  *  Class constructor
  *
  *  Must call "shmget" to create a shared memory segment
  *
  *  ShMkey is your student id number: 0xA12345 (to represent as hexadecimal value)
  *  size = 1
  *  ShMflg: IPC_CREAT | 0600
  *
 **/
ShM::ShM(size_t size) {
  int st;
  this->father_id = getpid();
  st = shmget(KEY, size, IPC_CREAT | 0600);
  if (-1 == st) {
    perror("ShM::ShM");
    exit(1);
  }
  this->id = st;
}


/**
  *   Class destructor
  *
  *   Must call shmctl
  *
 **/
ShM::~ShM() {
  int st = 0;
  if (father_id == getpid()) {
    st = shmctl(this->id, IPC_RMID, NULL);
  }
  if (-1 == st) {
    perror("ShM::~ShM");
    exit(2);
  }
}


/**
  *   Attach method
  *
  *   Need to call ShMat to receive a pointer to shared memory area
  *
 **/
void * ShM::attach() {
  this->area = shmat(this->id, NULL, 0);
  if (reinterpret_cast<void *>(-1) == this->area) {
    perror("ShM::attach");
    exit(2);
  }
  return this->area;
}


/**
  *   Detach method
  *
  *   Need to call shmdt to destroy local pointer
  *
 **/
int ShM::detach() {
  int st = -1;

  st = shmdt(this->area);
  if (-1 == st) {
    perror("ShM::detach");
    exit(3);
  }
  return st;
}

