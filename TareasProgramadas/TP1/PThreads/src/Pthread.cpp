/// Copyright 2023 <Daniel Pérez-Morera>

#include <iostream>

#include "Pthread.hpp"

Pthread::Pthread(size_t process_number) {
  this->process_number = process_number;
  if (EXIT_SUCCESS != pthread_mutex_init(&this->mutex, nullptr)) {
    perror("Cannot init mutex");
    exit(EXIT_FAILURE);
  }
}

Pthread::~Pthread() {
  if (EXIT_SUCCESS != pthread_mutex_destroy(&this->mutex)) {
    perror("Cannot destroy mutex");
    std::cout << this->process_number << std::endl;
    exit(EXIT_FAILURE);
  }
}

size_t Pthread::run(void*(*subroutine)(void*), void* data) {
  if (EXIT_SUCCESS != pthread_create(&this->thread_id
    , nullptr, subroutine, data)) {
    perror("Cannot create pthread");
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}

void* Pthread::join() {
  if (EXIT_SUCCESS != pthread_join(this->thread_id, nullptr)) {
    perror("Cannot join with pthread");
    exit(EXIT_FAILURE);
  }
  return nullptr;
}

size_t Pthread::wait() {
  if (EXIT_SUCCESS != pthread_mutex_lock(&this->mutex)) {
    perror("Cannot call wait");
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}

size_t Pthread::signal() {
  if (EXIT_SUCCESS != pthread_mutex_unlock(&this->mutex)) {
    perror("Cannot call signal");
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
size_t Pthread::get_process_number() {
  return this->process_number;
}
