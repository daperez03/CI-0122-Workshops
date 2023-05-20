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
#include "fileTable.h"

//____________________________Macros______________________________________________
/// Utilizado para obtener los valores de parametro
#define READ_PARAM(t) machine->ReadRegister(t + 3)
// Utilizado para retornar un valor despues de un system call
#define RETURN(t) machine->WriteRegister(2, t)
// Define el tamaño default de los buffer
#define BUFFER_SIZE 1000

//___________________________Common Functions______________________________________
void getNextInstruction() {
  machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
  machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
  machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

int ReadMem(int addr, int bytes, char* dest) {
  memset(dest, 0, bytes);
  bool error = false;
  int count = 0;
  for(int addrIterator = addr; addrIterator < addr + bytes; ++addrIterator) {
    int value = 0;
    error = !machine->ReadMem(addrIterator, 1, &value) || error;
    dest[count++] = (char)value;
  }
  return (error ? -1 : 0);
}

int WriteMem(int addr, int bytes, char* src) {
  bool error = false;
  int count = 0;
  for(int addrIterator = addr; addrIterator < addr + bytes; ++addrIterator) {
    int value = src[count++];
    error = !machine->WriteMem(addrIterator, 1, value) || error;
  }
  return (error ? -1 : 0);
}

// TODO(me) FINISH FUNCCION
void ShareResources(Thread* src, Thread* dst) {
  dst->fileTable = src->fileTable;
  dst->fileTable->addThread();
}

// Pass the user routine address as a parameter for this function
// This function is similar to "StartProcess" in "progtest.cc" file under "userprog"
// Requires a correct AddrSpace setup to work well
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

// TODO(me) FINISH FUNCCION
void removeResources(Thread* thread) {
  thread->fileTable->delThread();
  delete thread->fileTable;
  delete thread->space;
  thread->space = nullptr;
}


//________________________________System calls_______________________________________
/*
 *  System call interface: Halt()
 */
void NachOS_Halt() {		// System call 0
  DEBUG('a', "Shutdown, initiated by user program.\n");
  interrupt->Halt();
}


/*
 *  System call interface: void Exit( int )
 */
/// TODO(ME) completar
void NachOS_Exit() {		// System call 1
  // Debemos desacer los recursos y etc
  // Destruir tabla de FD
  // Limpiar memoria de AddrSpace
  int status = READ_PARAM(1); // Leemos el estado de ejecucion
  currentThread->Yield(); // Quitamos la CPU
  removeResources(currentThread); // Eliminamos los recursos del thread
  NachOS_Halt();
  // Finish the thread
   currentThread->Finish();
}


/*
 *  System call interface: SpaceId Exec( char * )
 */
/// TODO(ME) completar
void NachOS_Exec() {		// System call 2
}


/*
 *  System call interface: int Join( SpaceId )
 */
/// TODO(ME) completar
void NachOS_Join() {		// System call 3
}


/*
 *  System call interface: void Create( char * )
 */
void NachOS_Create() {		// System call 4
  int status = -1;
  int buffer_addr = READ_PARAM(1);
  char buffer[BUFFER_SIZE]; //TODO: no se debe pasar el size del buffer?
  status = ReadMem(buffer_addr, BUFFER_SIZE, buffer);
  if(status == EXIT_SUCCESS) {
    OpenFileId fd = creat(buffer, S_IRWXU);
    status = fd;
  }
  RETURN(status);
}

/*
 *  System call interface: OpenFileId Open( char * )
 */
void NachOS_Open() {		// System call 5
  int status = -1;
  int buffer_addr = READ_PARAM(1);
  char buffer[BUFFER_SIZE]; //TODO: no se debe pasar el size del buffer?
  memset(buffer, 0, BUFFER_SIZE);
  status = ReadMem(buffer_addr, BUFFER_SIZE, buffer);
  if(status == EXIT_SUCCESS) {
    OpenFileId fd = open(buffer, O_RDWR);
    if (fd != -1) fd = currentThread->fileTable->Open(fd);
    status = fd;
  }
  RETURN(status);
}

/*
 *  System call interface: OpenFileId Write( char *, int, OpenFileId )
 */
void NachOS_Write() {		// System call 6
  int status = -1;
  // Obtenemos los parametros del System call
  int buffer_addr = READ_PARAM(1);
  int size = READ_PARAM(2);
  OpenFileId fd = READ_PARAM(3);
  // Verificamos que no escriba sobre el archivo de entrada
  if (fd != ConsoleInput && currentThread->fileTable->isOpened(fd)) {
    // Pasamos datos de una direccion de memoria de NachOS a una de linux
    // char* buffer = new char(size);
    char buffer[size];
    status = ReadMem(buffer_addr, size, buffer);
    fd = currentThread->fileTable->getUnixHandle(fd);
    if (status == EXIT_SUCCESS) {
      if (fd == ConsoleOutput) printf("%s", buffer);
      else if (fd == ConsoleError) fprintf(stderr, "%s", buffer);
      else status = write(fd, buffer, size);
    }
  }
  RETURN(status);
}

/*
 *  System call interface: OpenFileId Read( char *, int, OpenFileId )
 */
void NachOS_Read() {  // System call 7
  int status = -1;
  // Obtenemos los parametros del System call
  int buffer_addr = READ_PARAM(1);
  int size = READ_PARAM(2);
  OpenFileId fd = READ_PARAM(3);
  // Verificamos que no lea sobre el archivo de salida o error
  if (fd != ConsoleOutput && fd != ConsoleError
    && currentThread->fileTable->isOpened(fd)) {
    // Leemos datos desde linux
    // char* buffer = new char(size);
    char buffer[size];
    memset(buffer, 0, size);
    fd = currentThread->fileTable->getUnixHandle(fd);
    status = read(fd, buffer, size);
    if (status != EXIT_FAILURE) {
      // Pasamos los datos a la memoria de NachOS
      status = WriteMem(buffer_addr, size, buffer);
    }
  }
  RETURN(status);
}


/*
 *  System call interface: void Close( OpenFileId )
 */
void NachOS_Close() {		// System call 8
  int status = -1;
  OpenFileId fd = READ_PARAM(1);
  if (currentThread->fileTable->isOpened(fd)) {
    fd = currentThread->fileTable->getUnixHandle(fd);
    status = close(fd);
  }
  RETURN(status);
}


/*
 *  System call interface: void Fork( void (*func)() )
 */
/// TODO(ME) debuguear
void NachOS_Fork() {		// System call 9
  DEBUG( 'u', "Entering Fork System call\n" );
  // We need to create a new kernel thread to execute the user thread
  Thread* newThread = new Thread("Child to execute Fork");
  // Child and father will also share the same address space, except for the stack
  // Text, init data and uninit data are shared, a new stack area must be created
  // for the new child
  // We suggest the use of a new constructor in AddrSpace class,
  // This new constructor will copy the shared segments (space variable) from currentThread, passed
  // as a parameter, and create a new stack for the new child
  newThread->space = new AddrSpace(*currentThread->space);
  // We need to share the Open File Table structure with this new child
  ShareResources(currentThread, newThread);
  // We (kernel)-Fork to a new method to execute the child code
  // Pass the user routine address, now in register 4, as a parameter
  // Note: in 64 bits register 4 need to be casted to (void *)
  newThread->Fork(NachOSForkThread, (void*)READ_PARAM(1));
  DEBUG( 'u', "Exiting Fork System call\n" );
}


/*
 *  System call interface: void Yield()
 */
/// TODO(ME) completar
void NachOS_Yield() {		// System call 10
}


/*
 *  System call interface: Sem_t SemCreate( int )
 */
/// TODO(ME) completar
void NachOS_SemCreate() {		// System call 11
}


/*
 *  System call interface: int SemDestroy( Sem_t )
 */
/// TODO(ME) completar
void NachOS_SemDestroy() {		// System call 12
}


/*
 *  System call interface: int SemSignal( Sem_t )
 */
/// TODO(ME) completar
void NachOS_SemSignal() {		// System call 13
}


/*
 *  System call interface: int SemWait( Sem_t )
 */
/// TODO(ME) completar
void NachOS_SemWait() {		// System call 14
}


/*
 *  System call interface: Lock_t LockCreate( int )
 */
/// TODO(ME) completar
void NachOS_LockCreate() {		// System call 15
}


/*
 *  System call interface: int LockDestroy( Lock_t )
 */
/// TODO(ME) completar
void NachOS_LockDestroy() {		// System call 16
}


/*
 *  System call interface: int LockAcquire( Lock_t )
 */
/// TODO(ME) completar
void NachOS_LockAcquire() {		// System call 17
}


/*
 *  System call interface: int LockRelease( Lock_t )
 */
void NachOS_LockRelease() {		// System call 18
}


/*
 *  System call interface: Cond_t LockCreate( int )
 */
/// TODO(ME) completar
void NachOS_CondCreate() {		// System call 19
}


/*
 *  System call interface: int CondDestroy( Cond_t )
 */
/// TODO(ME) completar
void NachOS_CondDestroy() {		// System call 20
}


/*
 *  System call interface: int CondSignal( Cond_t )
 */
void NachOS_CondSignal() {		// System call 21
}


/*
 *  System call interface: int CondWait( Cond_t )
 */
/// TODO(ME) completar
void NachOS_CondWait() {		// System call 22
}


/*
 *  System call interface: int CondBroadcast( Cond_t )
 */
/// TODO(ME) completar
void NachOS_CondBroadcast() {		// System call 23
}


/*
 *  System call interface: Socket_t Socket( int, int )
 */
void NachOS_Socket() {			// System call 30
  int status = -1;
  int family = READ_PARAM(1); // Get family
  int type = READ_PARAM(1); // Get socket type
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


/*
 *  System call interface: Socket_t Connect( char *, int )
 */
void NachOS_Connect() {		// System call 31
  int status = -1;
  int socketFD = READ_PARAM(1);
  int addrHost = READ_PARAM(2);
  int port = READ_PARAM(3);
  char host[BUFFER_SIZE];
  status = ReadMem(addrHost, BUFFER_SIZE, host);
  if (status == EXIT_SUCCESS &&
    currentThread->fileTable->isOpened(socketFD)) {
      socketFD = currentThread->fileTable->getUnixHandle(socketFD);
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


/*
 *  System call interface: int Bind( Socket_t, int )
 */
void NachOS_Bind() {		// System call 32
  int status = -1;
  int socketFD = READ_PARAM(1);
  int port = READ_PARAM(2);
  if (currentThread->fileTable->isOpened(socketFD)) {
    socketFD = currentThread->fileTable->getUnixHandle(socketFD);
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


/*
 *  System call interface: int Listen( Socket_t, int )
 */
void NachOS_Listen() {		// System call 33
  int status = -1;
  int socketFD = READ_PARAM(1);
  int numConnection = READ_PARAM(2);
  if (currentThread->fileTable->isOpened(socketFD)) {
    socketFD = currentThread->fileTable->getUnixHandle(socketFD);
    status = listen(socketFD, numConnection);
  }
  RETURN(status);
}


/*
 *  System call interface: int Accept( Socket_t )
 */
void NachOS_Accept() {		// System call 34
  int  status = -1;
  int socketFD = READ_PARAM(1);
  if (currentThread->fileTable->isOpened(socketFD)) {
    socketFD = currentThread->fileTable->getUnixHandle(socketFD);
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
 */
void NachOS_Shutdown() {	// System call 25
  int status = -1;
  Socket_t socket = READ_PARAM(1); // Obtenemos el socket
  int how = READ_PARAM(2); // Obtenemos el how
  if (currentThread->fileTable->isOpened(socket)) {
    socket = currentThread->fileTable->getUnixHandle(socket);
    if (socket != -1) status = shutdown(socket, how);
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
