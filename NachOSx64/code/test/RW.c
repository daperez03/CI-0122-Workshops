#include "syscall.h"


int main() {
  char buffer[100];
  int i = 0;
  Create("../test/prueba.txt");
  int fd = Open("../test/prueba.txt");
  Read(buffer, 100, 0);
  Write(buffer, 100, 1);
  Write(buffer, 100, fd);
  Close(fd);
}
