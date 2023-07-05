// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "copyright.h"
#include "addrspace.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include "bitmap.h"
#include "OSTable.h"

//____________________________Macros______________________________________________
/// @brief Lee los valores de parametro en los registros
#define READ_PARAM(t) machine->ReadRegister(t + 3)
/// @brief Retornar un valor despues de un system call
#define RETURN(t) machine->WriteRegister(2, t)
/// @brief Define el tamaño default de los buffer
#define BUFFER_SIZE 1000

//___________________________Common Functions______________________________________
/// @brief Mueve los registros a la siguiente instruccion
void getNextInstruction() {
  machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
  machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
  machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

/// @brief Leer un conjunto n de bytes de la memoria de NachOS
/// @param addr Direccion logica de NachOS
/// @param bytes Numero de bytes a leer
/// @param dest Buffer de destino de los datos
/// @return Estado de la funcion ejecutada: 0 = Exit Success; -1 = Exit Failer
int ReadMem(int addr, int bytes, char* dest) {
  memset(dest, 0, bytes);
  int count = 0;
  for(int addrIterator = addr; addrIterator < addr + bytes; ++addrIterator) {
    int value = 0;
    while (!machine->ReadMem(addrIterator, 1, &value));
    dest[count] = (char)value;
    if (dest[count++] == 0) break;
  }
  return 0;
}

/// @brief Leer un conjunto de caracteres de la memoria de NachOS
/// @param addr Direccion logica de NachOS
/// @param bytes Numero de bytes del buffer
/// @param dest Buffer de destino de los datos
/// @return Estado de la funcion ejecutada: 0 = Exit Success; -1 = Exit Failer
int ReadMemSTR(int addr, int bytes, char* dest) {
  memset(dest, 0, bytes);
  int count = 0;
  for(int addrIterator = addr; addrIterator < addr + bytes; ++addrIterator) {
    int value = 0;
    while (!machine->ReadMem(addrIterator, 1, &value));
    dest[count] = (char)value;
    if (dest[count++] == 0) break;
  }
  return 0;
}

/// @brief Escribir un conjunto de bytes sobre memoria de NachOS
/// @param addr Direccion logica de NachOS
/// @param bytes Numeros de bytes a escribir
/// @param src Buffer con los datos
/// @return Estado de la funcion ejecutada: 0 = Exit Success; -1 = Exit Failer
int WriteMem(int addr, int bytes, char* src) {
  int count = 0;
  for(int addrIterator = addr; addrIterator < addr + bytes; ++addrIterator) {
    int value = src[count++];
    while (!machine->WriteMem(addrIterator, 1, value));
  }
  return 0;
}

/// @brief Comparte recursos entre threads.
/// @param src Thread con los recursos
/// @param dst Thread destino
void ShareResources(Thread* src, Thread* dst) {
  dst->fileTable = src->fileTable;
  dst->fileTable->addThread();
  dst->threadTable = src->threadTable;
  dst->threadTable->addThread();
  dst->semTable = src->semTable;
  dst->semTable->addThread();
  dst->lockTable = src->lockTable;
  dst->lockTable->addThread();
  dst->condTable = src->condTable;
  dst->condTable->addThread();
}

/// @brief Mueve el contador de instrucciones sobre una funcion especifica
/// @param addr Funcion sobre la que se quiere mover el contador
void NachOSForkThread(void * addr) { // for 64 bits version
  // Tomamos el space del hilo actual
  AddrSpace* space = currentThread->space;
  space->InitRegisters(); // set the initial register values
  space->RestoreState(); // load page table register
  // Set the return address for this thread to the same as the main thread
  // This will lead this thread to call the exit system call and finish
  machine->WriteRegister(RetAddrReg, 4);
  machine->WriteRegister(PCReg, (long)addr);
  machine->WriteRegister(NextPCReg, (long)addr + 4);
  machine->Run(); // jump to the user progam
  ASSERT(false);
}

/// @brief Carga un programa sobre un thread
/// @param argv nombre del programa
void execProcess(void* param) {
  void** argv = (void**)param;
  #ifdef VM
  currentThread->space =
    new AddrSpace((OpenFile*)argv[0], (char*)argv[1]);
  #else
  currentThread->space = new AddrSpace((OpenFile*)argv[0]);
  #endif
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();
  machine->Run();
  ASSERT(false);
}

//________________________________System calls_______________________________________
/// @brief System call interface: Halt()
void NachOS_Halt() {		// System call 0
  DEBUG('S', "System Call: NachOS_Halt.\n");
  DEBUG('a', "Shutdown, initiated by user program.\n");
  interrupt->Halt();
}

/// @brief System call interface: void Exit( int )
/// @details Finaliza un proceso, revisa recursos abierto y los elimina
void NachOS_Exit() {		// System call 1
  DEBUG('S', "System Call: NachOS_Exit.\n");
  int status = READ_PARAM(1); // Leemos el estado de ejecucion
  currentThread->Yield(); // Quitamos la CPU
  // Finish the thread
  currentThread->Finish();
  if (errno) NachOS_Halt();
  RETURN(status);
}

/// @brief System call interface: SpaceId Exec( char * )
/// @details Inicia la ejecucion de un programa
/// @param programName Nombre del programa
/// @return ID del nuevo hilo creado o -1 en caso de error
void NachOS_Exec() {		// System call 2
  DEBUG('S', "System Call: NachOS_Exec.\n");
  int status = -1;
  char programName[BUFFER_SIZE];
  status = ReadMemSTR(READ_PARAM(1), BUFFER_SIZE, programName);
  OpenFile* executable = fileSystem->Open(programName);
  if (status != -1 && executable != nullptr) {
    Thread* newThread = new Thread ("New process");
    status = currentThread->threadTable->Open(newThread);
    ShareResources(currentThread, newThread);
    void* argv[2] = {executable, programName};
    newThread->Fork(execProcess, argv);
  } else status = -1;
  RETURN(status);
}

/// @brief System call interface: int Join( SpaceId )
/// @details Espera que un hilo finalice su ejecucion
/// @param thread_id Hilo por el que se desea esperar 
/// @return Estado de la funcion ejecutada: 0 = Exit Success; -1 = Exit Failer
void NachOS_Join() {		// System call 3
  DEBUG('S', "System Call: NachOS_Join.\n");
  int status = -1;
  SpaceId thread_id = READ_PARAM(1);
  if (currentThread->threadTable->isOpened(thread_id)) {
    Thread* thread = currentThread->threadTable->getObject(thread_id);
    if (thread->semaphore == NULL)
      thread->semaphore = new Semaphore("Join Semaphore", 0);
    thread->semaphore->P();
  }
  RETURN(status);
}


/*
 *  System call interface: void Create( char * )
 *  Crea un nuevo documento
 */

/// @brief System call interface: void Create( char * )
/// @details Crea un nuevo documento de texto
/// @param buffer_addr Nombre del ejecutable
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_Create() {		// System call 4
  DEBUG('S', "System Call: NachOS_Create.\n");
  int status = -1;
  int buffer_addr = READ_PARAM(1);
  char buffer[BUFFER_SIZE];
  status = ReadMemSTR(buffer_addr, BUFFER_SIZE, buffer);
  if(status == EXIT_SUCCESS) {
    OpenFileId fd = creat(buffer, S_IRWXU);
    status = fd;
  }
  RETURN(status);
}

/// @brief System call interface: OpenFileId Open( char * )
/// @details Abre un documento de texto
/// @param buffer_addr Nombre del documento
/// @return File descriptor o -1 en caso de error
void NachOS_Open() {		// System call 5
  DEBUG('S', "System Call: NachOS_Open.\n");
  int status = -1;
  int buffer_addr = READ_PARAM(1);
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  status = ReadMemSTR(buffer_addr, BUFFER_SIZE, buffer);
  if(status == EXIT_SUCCESS) {
    OpenFileId fd = open(buffer, O_RDWR);
    if (fd != -1)
      fd = currentThread->fileTable->Open(fd);
    status = fd;
  }
  RETURN(status);
}

/// @brief System call interface: OpenFileId Write( char *, int, OpenFileId )
/// @details Permite escribe sobre un documento
/// @param buffer_addr Bytes a escribir
/// @param size Tamaño del buffer
/// @param fd File descriptor
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_Write() {		// System call 6
  DEBUG('S', "System Call: NachOS_Write.\n");
  int status = -1;
  // Obtenemos los parametros del System call
  int buffer_addr = READ_PARAM(1);
  int size = READ_PARAM(2);
  OpenFileId fd = READ_PARAM(3);
  canAccessConsole->Lock();
  // Verificamos que no escriba sobre el archivo de entrada
  if (fd != ConsoleInput && currentThread->fileTable->isOpened(fd)) {
    // Pasamos datos de una direccion de memoria de NachOS a una de linux
    // char* buffer = new char(size);
    char buffer[size + 1];
    memset(buffer, 0, size + 1);
    status = ReadMem(buffer_addr, size, buffer);
    fd = currentThread->fileTable->getObject(fd);
    if (status == EXIT_SUCCESS) {
      if (fd == ConsoleOutput) {
        printf("%s", buffer);
        fflush(stdout);
        stats->numConsoleCharsWritten += sizeof(buffer);
      } else if (fd == ConsoleError) fprintf(stderr, "%d\n", buffer_addr);
      else status = write(fd, buffer, size);
    }
  }
  canAccessConsole->Unlock();
  RETURN(status);
}

/// @brief System call interface: OpenFileId Read( char *, int, OpenFileId )
/// @details Lee un documento
/// @param buffer_addr Buffer destino de lo que se leera
/// @param size Tamaño del buffer
/// @param fd File descriptor
/// @return Numero de bytes leidos, 0 en caso de EOF, -1 en caso de error
void NachOS_Read() {  // System call 7
  DEBUG('S', "System Call: NachOS_Read.\n");
  int status = -1;
  // Obtenemos los parametros del System call
  int buffer_addr = READ_PARAM(1);
  int size = READ_PARAM(2);
  OpenFileId fd = READ_PARAM(3);
  canAccessConsole->Lock();
  // Verificamos que no lea sobre el archivo de salida o error
  if (fd != ConsoleOutput && fd != ConsoleError
    && currentThread->fileTable->isOpened(fd)) {
    // Leemos datos desde linux
    // char* buffer = new char(size);
    char buffer[size];
    memset(buffer, 0, size);
    fd = currentThread->fileTable->getObject(fd);
    status = read(fd, buffer, size);
    if (status != -1) {
      // Pasamos los datos a la memoria de NachOS
      if (READ_PARAM(3) == ConsoleInput)
        stats->numConsoleCharsWritten += status;
      status = WriteMem(buffer_addr, status, buffer) == -1 ? -1 : status;
    }
  }
  canAccessConsole->Unlock();
  RETURN(status);
}

/// @brief System call interface: void Close( OpenFileId )
/// @details Cierra un documento
/// @param fd File descriptor
/// @return 0 en caso de exito, -1 en caso de erro
void NachOS_Close() {		// System call 8
  DEBUG('S', "System Call: NachOS_Close.\n");
  int status = -1;
  OpenFileId fd = READ_PARAM(1);
  if (currentThread->fileTable->isOpened(fd)) {
    status = currentThread->fileTable->getObject(fd);
    status = close(status);
    status = currentThread->fileTable->Close(fd);
  }
  RETURN(status);
}

/// @brief System call interface: int Fork( void (*func)() )
/// @details Crea un nuevo thread sobre una subrutina
/// @param funccion Subrutina en la que se ejecutara el nuevo thread
/// @return ID del nuevo hilo o -1 en caso de error
void NachOS_Fork() {		// System call 9
  DEBUG('S', "System Call: NachOS_Fork.\n");
  DEBUG( 'u', "Entering Fork System call\n" );
  Thread* newThread = new Thread("Child to execute Fork");
  #ifdef VM
  newThread->space =
    new AddrSpace(*currentThread->space, newThread);
  #else
  newThread->space = new AddrSpace(*currentThread->space);
  #endif
  currentThread->threadTable->Open(newThread);
  ShareResources(currentThread, newThread);
  newThread->Fork(NachOSForkThread, (void*)(long)READ_PARAM(1));
  DEBUG( 'u', "Exiting Fork System call\n" );
}

/// @brief System call interface: void Yield()
/// @details Cede la CPU a otro proceso
void NachOS_Yield() {		// System call 10
  DEBUG('S', "System Call: NachOS_Yield.\n");
  currentThread->Yield();
}

/// @brief System call interface: Sem_t SemCreate( int )
/// @details Crea un semafor
/// @param sem_init Numero en el cual el semaforo iniciara
/// @return ID del semafor o -1 en caso de error
void NachOS_SemCreate() {		// System call 11
  DEBUG('S', "System Call: NachOS_SemCreate.\n");
  Semaphore* sem = new Semaphore("Semaphore", READ_PARAM(1));
  Sem_t sem_id = currentThread->semTable->Open(sem);
  RETURN(sem_id);
}

/// @brief System call interface: int SemDestroy( Sem_t )
/// @details Crea un semaforo
/// @param sem_id ID del semaforo
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_SemDestroy() {		// System call 12
  DEBUG('S', "System Call: NachOS_SemDestroy.\n");
  int status = -1;
  Sem_t sem_id = READ_PARAM(1);
  if (currentThread->semTable->isOpened(sem_id)) {
    delete currentThread->semTable->getObject(sem_id);
    currentThread->semTable->Close(sem_id);
    status = 0;
  }
  RETURN(status);
}

/// @brief System call interface: int SemSignal( Sem_t )
/// @details Hace un signal al semaforo
/// @param sem_id ID del semaforo
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_SemSignal() {		// System call 13
  DEBUG('S', "System Call: NachOS_SemSignal.\n");
  int status = -1;
  Sem_t sem_id = READ_PARAM(1);
  if (currentThread->semTable->isOpened(sem_id)) {
    currentThread->semTable->getObject(sem_id)->V();
    status = 0;
  }
  RETURN(status);
}

/// @brief System call interface: int SemWait( Sem_t )
/// @details Hace un wait sobre el semaforo
/// @param sem_id ID del semaforo
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_SemWait() {		// System call 14
  DEBUG('S', "System Call: NachOS_SemWait.\n");
  int status = -1;
  Sem_t sem_id = READ_PARAM(1);
  if (currentThread->semTable->isOpened(sem_id)) {
    currentThread->semTable->getObject(sem_id)->P();
    status = 0;
  }
  RETURN(status);
}


/*
 *  System call interface: Lock_t LockCreate()
 *  Crea un Lock
 */

/// @brief System call interface: Lock_t LockCreate()
/// @details Crea un lock
/// @return ID del lock, -1 en caso de error
void NachOS_LockCreate() {		// System call 15
  DEBUG('S', "System Call: NachOS_LockCreate.\n");
  Lock* lock = new Lock("Lock");
  Lock_t lock_id = currentThread->lockTable->Open(lock);
  RETURN(lock_id);
}

/// @brief System call interface: int LockDestroy( Lock_t )
/// @details Destruye un lock
/// @param lock_id ID del lock
/// @return 0 en caso de exito, -1 en caso de erro 
void NachOS_LockDestroy() {		// System call 16
  DEBUG('S', "System Call: NachOS_LockDestroy.\n");
  int status = -1;
  Lock_t lock_id = READ_PARAM(1);
  if (currentThread->lockTable->isOpened(lock_id)) {
    delete currentThread->lockTable->getObject(lock_id);
    currentThread->lockTable->Close(lock_id);
    status = 0;
  }
  RETURN(status);
}

/// @brief System call interface: int LockAcquire( Lock_t )
/// @details Produce un acquire sobre un Lock
/// @param lock_id ID del lock
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_LockAcquire() {		// System call 17
  DEBUG('S', "System Call: NachOS_LockAcquire.\n");
  int status = -1;
  Lock_t lock_id = READ_PARAM(1);
  if (currentThread->lockTable->isOpened(lock_id)) {
    currentThread->lockTable->getObject(lock_id)->Acquire();
    status = 0;
  }
  RETURN(status);
}

/// @brief System call interface: int LockRelease( Lock_t )
/// @details Produce un release sobre un lock
/// @param lock_id ID del lock
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_LockRelease() {		// System call 18
  DEBUG('S', "System Call: NachOS_LockRelease.\n");
  int status = -1;
  Lock_t lock_id = READ_PARAM(1);
  if (currentThread->lockTable->isOpened(lock_id)) {
    currentThread->lockTable->getObject(lock_id)->Release();
    status = 0;
  }
  RETURN(status);
}

/// @brief System call interface: Cond_t LockCreate( )
/// @details Crea una variable de condicion
/// @return ID de la variable de condicion
void NachOS_CondCreate() {		// System call 19
  DEBUG('S', "System Call: NachOS_CondCreate.\n");
  Condition* cond = new Condition("Condition");
  Cond_t cond_id = currentThread->condTable->Open(cond);
  RETURN(cond_id);
}

/// @brief System call interface: int CondDestroy( Cond_t )
/// @details Destruye una variable de condicion
/// @param cond_id ID de la variable de condicion
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_CondDestroy() {		// System call 20
  DEBUG('S', "System Call: NachOS_CondDestroy.\n");
  int status = -1;
  Cond_t cond_id = READ_PARAM(1);
  if (currentThread->condTable->isOpened(cond_id)) {
    delete currentThread->condTable->getObject(cond_id);
    currentThread->condTable->Close(cond_id);
    status = 0;
  }
  RETURN(status);
}

/// @brief System call interface: int CondSignal(Cond_t, Lock_t)
/// @details Aplica un signal sobre una variable de condicion
/// @param cond_id ID de la variable de condicion
/// @param lock_id ID del lock
/// @return 0 en caso de exito, -1 en caso de error 
void NachOS_CondSignal() {		// System call 21
  DEBUG('S', "System Call: NachOS_CondSignal.\n");
  int status = -1;
  Cond_t cond_id = READ_PARAM(1);
  Lock_t lock_id = READ_PARAM(2);
  if (currentThread->condTable->isOpened(cond_id) &&
    currentThread->lockTable->isOpened(lock_id)) {
    Lock* lock = currentThread->lockTable->getObject(lock_id);
    currentThread->condTable->getObject(cond_id)->Signal(lock);
    status = 0;
  }
  RETURN(status);
}

/// @brief System call interface: int CondWait(Cond_t, Lock_t)
/// @details Aplica un wait sobre una variable de condicion
/// @param cond_id ID de la variable de condicion
/// @param lock_id ID del lock
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_CondWait() {		// System call 22
  DEBUG('S', "System Call: NachOS_CondWait.\n");
  int status = -1;
  Cond_t cond_id = READ_PARAM(1);
  Lock_t lock_id = READ_PARAM(2);
  if (currentThread->condTable->isOpened(cond_id) &&
    currentThread->lockTable->isOpened(lock_id)) {
    Lock* lock = currentThread->lockTable->getObject(lock_id);
    currentThread->condTable->getObject(cond_id)->Wait(lock);
    status = 0;
  }
  RETURN(status);
}

/// @brief System call interface: int CondBroadcast(Cond_t, Lock_t)
/// @details Aplica un brodcast sobre una variable de condicion
/// @param cond_id ID de la variable de condicion
/// @param lock_id ID del lock
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_CondBroadcast() {		// System call 23
  DEBUG('S', "System Call: NachOS_CondBrodcast.\n");
  int status = -1;
  Cond_t cond_id = READ_PARAM(1);
  Lock_t lock_id = READ_PARAM(2);
  if (currentThread->condTable->isOpened(cond_id) &&
    currentThread->lockTable->isOpened(lock_id)) {
    Lock* lock = currentThread->lockTable->getObject(lock_id);
    currentThread->condTable->getObject(cond_id)->Broadcast(lock);
    status = 0;
  }
  RETURN(status);
}

/// @brief System call interface: Socket_t Socket( int, int )
/// @param family Familia a la que pertenece el socket
/// @param type Tipo de socket
/// @return ID del socket, -1 en caso de error
void NachOS_Socket() {			// System call 30
  DEBUG('S', "System Call: NachOS_Socket.\n");
  int status = -1;
  int family = READ_PARAM(1); // Get family
  int type = READ_PARAM(2); // Get socket type
  bool canCreateSocket = (family == AF_INET_NachOS ||
    family == AF_INET6_NachOS) && (type == SOCK_DGRAM_NachOS
    || type == SOCK_STREAM_NachOS); // if all are ok return true
  if (canCreateSocket) { // if canCreateSocket is true, so make a socket
    family = family == AF_INET_NachOS ? AF_INET : AF_INET6;
    type = type == SOCK_DGRAM_NachOS ? SOCK_DGRAM : SOCK_STREAM;
    int socket_id = socket(family, type, 0);
    if (socket_id != -1)
      status = currentThread->fileTable->Open(socket_id);
  }
  RETURN(status); // return socket id or error
}

/// @brief System call interface: Socket_t Connect( char *, int )
/// @details Realiza un conect a un servidor
/// @param socketFD ID del socket
/// @param addrHost Direccion del servidor
/// @param port Puerto
/// @return 0 en caso de exito, -1 en caso de error 
void NachOS_Connect() {		// System call 31
  DEBUG('S', "System Call: NachOS_Connect.\n");
  int status = -1;
  int socketFD = READ_PARAM(1);
  int addrHost = READ_PARAM(2);
  int port = READ_PARAM(3);
  char host[BUFFER_SIZE];
  status = ReadMemSTR(addrHost, BUFFER_SIZE, host);
  if (strcmp(host, "localhost") == 0) strcpy(host, "127.0.0.1");
  if (status == EXIT_SUCCESS &&
    currentThread->fileTable->isOpened(socketFD)) {
      socketFD = currentThread->fileTable->getObject(socketFD);
    struct sockaddr addr;
    socklen_t len = sizeof(addr);
    struct sockaddr* ha;
    struct sockaddr_in6  hostIPv6;
    struct sockaddr_in  hostIPv4;
    getsockname(socketFD, &addr, &len);
    if (addr.sa_family == AF_INET6) {
      memset(&hostIPv6, 0, sizeof(hostIPv6));
      hostIPv6.sin6_family = AF_INET6;
      hostIPv6.sin6_port = htons(port);
      status = inet_pton(AF_INET6, host, &hostIPv6.sin6_addr);
      ha = (struct sockaddr *) &hostIPv6;
      len = sizeof(hostIPv6);
    } else {
      memset(&hostIPv4, 0, sizeof(hostIPv4));
      hostIPv4.sin_family = AF_INET;
      hostIPv4.sin_port = htons(port);
      status = inet_pton(AF_INET, host, &hostIPv4.sin_addr);
      ha = (struct sockaddr *) &hostIPv4;
      len = sizeof(hostIPv4);
    }
    if (status != -1) status = connect(socketFD, ha, len);
  }
  RETURN(status);
}

/// @brief System call interface: int Bind( Socket_t, int )
/// @param socketFD ID del socket
/// @param port Puerto
/// @return 0 en caso de exito, 0 en caso de error
void NachOS_Bind() {		// System call 32
  DEBUG('S', "System Call: NachOS_Bind.\n");
  int status = -1;
  int socketFD = READ_PARAM(1);
  int port = READ_PARAM(2);
  if (currentThread->fileTable->isOpened(socketFD)) {
    socketFD = currentThread->fileTable->getObject(socketFD);
    struct sockaddr addr;
    socklen_t len = sizeof(addr);
    struct sockaddr* ha;
    struct sockaddr_in6  hostIPv6;
    struct sockaddr_in  hostIPv4;
    getsockname(socketFD, &addr, &len);
    if (addr.sa_family == AF_INET6) {
      hostIPv6.sin6_family = AF_INET6;
      hostIPv6.sin6_port = htons(port);
      hostIPv6.sin6_addr = in6addr_any;
      ha = (struct sockaddr *) &hostIPv6;
      len = sizeof(hostIPv4);
    } else {
      hostIPv4.sin_family = AF_INET;
      hostIPv4.sin_port = htons(port);
      hostIPv4.sin_addr.s_addr = INADDR_ANY;
      ha = (struct sockaddr *) &hostIPv4;
      len = sizeof(hostIPv4);
    }
    status = bind(socketFD, (sockaddr*)ha, len);
  }
  RETURN(status);
}

/// @brief System call interface: int Listen( Socket_t, int )
/// @details Escucha conexiones en un puerto
/// @param socketFD ID del socket
/// @param numConnection  Numero de conexiones permitidas
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_Listen() {		// System call 33
  DEBUG('S', "System Call: NachOS_Listen.\n");
  int status = -1;
  int socketFD = READ_PARAM(1);
  int numConnection = READ_PARAM(2);
  if (currentThread->fileTable->isOpened(socketFD)) {
    socketFD = currentThread->fileTable->getObject(socketFD);
    status = listen(socketFD, numConnection);
  }
  RETURN(status);
}

/// @brief System call interface: int Accept( Socket_t )
/// @details Acepta una conexion
/// @param socketFD ID del socket
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_Accept() {		// System call 34
  DEBUG('S', "System Call: NachOS_Accept.\n");
  int  status = -1;
  int socketFD = READ_PARAM(1);
  if (currentThread->fileTable->isOpened(socketFD)) {
    socketFD = currentThread->fileTable->getObject(socketFD);
    struct sockaddr addr;
    socklen_t len = sizeof(addr);
    getsockname(socketFD, &addr, &len);
    struct sockaddr* ha;
    struct sockaddr_in6 peer6;
    struct sockaddr_in peer4;
    socklen_t peerLen;
    if (addr.sa_family == AF_INET6) {
      peerLen = sizeof(peer6);
      ha = (sockaddr*) &peer6;
    } else {
      peerLen = sizeof(peer4);
      ha = (sockaddr*) &peer4;
    }
    status = accept(socketFD, ha, &peerLen);
    if (status != -1) {
      status = currentThread->fileTable->Open(status);
    }
  }
  RETURN(status);
}


/*
 *  System call interface: int Shutdown( Socket_t, int )
 *  Realiza un Shutdown sobre un socket
 */

/// @brief System call interface: int Shutdown( Socket_t, int )
/// @details Realiza un Shutdown sobre un socket
/// @param socket ID del socket
/// @param how Tipo de shutdown
/// @return 0 en caso de exito, -1 en caso de error
void NachOS_Shutdown() {	// System call 25
  DEBUG('S', "System Call: NachOS_Shutdown.\n");
  int status = -1;
  Socket_t socket = READ_PARAM(1); // Obtenemos el socket
  int how = READ_PARAM(2); // Obtenemos el how
  if (currentThread->fileTable->isOpened(socket)) {
    socket = currentThread->fileTable->getObject(socket);
    if (socket != -1) status = shutdown(socket, how);
    currentThread->fileTable->Close(socket);
  }
  RETURN(status);
}


//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
void ExceptionHandler(ExceptionType which) {
  int type = machine->ReadRegister(2);
  switch ( which ) {
    case SyscallException:
      switch ( type ) {
        case SC_Halt:		// System call # 0
          NachOS_Halt();
          break;
        case SC_Exit:		// System call # 1
          NachOS_Exit();
          break;
        case SC_Exec:		// System call # 2
          NachOS_Exec();
          break;
        case SC_Join:		// System call # 3
          NachOS_Join();
          break;
        case SC_Create:		// System call # 4
          NachOS_Create();
          break;
        case SC_Open:		// System call # 5
          NachOS_Open();
          break;
        case SC_Read:		// System call # 6
          NachOS_Read();
          break;
        case SC_Write:		// System call # 7
          NachOS_Write();
          break;
        case SC_Close:		// System call # 8
          NachOS_Close();
          break;
        case SC_Fork:		// System call # 9
          NachOS_Fork();
          break;
        case SC_Yield:		// System call # 10
          NachOS_Yield();
          break;
        case SC_SemCreate:         // System call # 11
          NachOS_SemCreate();
          break;
        case SC_SemDestroy:        // System call # 12
          NachOS_SemDestroy();
          break;
        case SC_SemSignal:         // System call # 13
          NachOS_SemSignal();
          break;
        case SC_SemWait:           // System call # 14
          NachOS_SemWait();
          break;
        case SC_LckCreate:         // System call # 15
          NachOS_LockCreate();
          break;
        case SC_LckDestroy:        // System call # 16
          NachOS_LockDestroy();
          break;
        case SC_LckAcquire:         // System call # 17
          NachOS_LockAcquire();
          break;
        case SC_LckRelease:           // System call # 18
          NachOS_LockRelease();
          break;
        case SC_CondCreate:         // System call # 19
          NachOS_CondCreate();
          break;
        case SC_CondDestroy:        // System call # 20
          NachOS_CondDestroy();
          break;
        case SC_CondSignal:         // System call # 21
          NachOS_CondSignal();
          break;
        case SC_CondWait:           // System call # 22
          NachOS_CondWait();
          break;
        case SC_CondBroadcast:           // System call # 23
          NachOS_CondBroadcast();
          break;
        case SC_Socket:	// System call # 30
          NachOS_Socket();
          break;
        case SC_Connect:	// System call # 31
          NachOS_Connect();
          break;
        case SC_Bind:	// System call # 32
          NachOS_Bind();
          break;
        case SC_Listen:	// System call # 33
          NachOS_Listen();
          break;
        case SC_Accept:	// System call # 32
          NachOS_Accept();
          break;
        case SC_Shutdown:	// System call # 33
          NachOS_Shutdown();
          break;
        default:
          printf("Unexpected syscall exception %d\n", type );
          ASSERT( false );
          break;
      }
      getNextInstruction();
      break;
    case PageFaultException:
      #ifdef VM 
      currentThread->space->PageFaultException();
      #endif 
      break;
    case ReadOnlyException:
      printf( "Read Only exception (%d)\n", which );
      ASSERT( false );
      break;
    case BusErrorException:
      printf( "Bus error exception (%d)\n", which );
      ASSERT( false );
      break;
    case AddressErrorException:
      printf( "Address error exception (%d)\n", which );
      ASSERT( false );
      break;
    case OverflowException:
      printf( "Overflow exception (%d)\n", which );
      ASSERT( false );
      break;
    case IllegalInstrException:
      printf( "Ilegal instruction exception (%d)\n", which );
      ASSERT( false );
      break;
    default:
      printf( "Unexpected exception %d\n", which );
      ASSERT( false );
      break;
  }
}
