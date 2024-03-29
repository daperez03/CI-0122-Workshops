// system.h
// All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "bitmap.h"
#include "synch.h"
#include "../vm/Map.hpp"
#include "../vm/BackingStore.hpp"

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
// called before anything else
extern void Cleanup();				// Cleanup, called when
// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
/// @brief user program memory and registers
extern Machine* machine;
// @brief Bit map de control de la memoria fisica
extern BitMap* physicalPageMap;
/// @brief Manejo de consola
extern Mutex* canAccessConsole;
#endif

#ifdef VM
/// @brief Tabla invertida de page table
extern Map<Thread*, AddrSpace*> invertedTable;
/// @brief Backing Store para almacenar las pages
extern BackingStore* backingStore;
/// @brief Index de la tlb para implementar secondChance
extern int tlbIndex;
/// @brief Index de la memoria para implementar secondChance
extern int physicalIndex;
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk* synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
