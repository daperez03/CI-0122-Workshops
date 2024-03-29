// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue) {
    name = (char *)debugName;
    value = initialValue;
    queue = new List<Thread*>;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore() {
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
        queue->Append(currentThread);		// so go to sleep
        currentThread->Sleep();
    }
    value--;  // semaphore available, 
    // consume its value
    interrupt->SetLevel(oldLevel);		// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V() {
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = queue->Remove();
    if (thread != NULL)  // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

#ifdef USER_PROGRAM
//----------------------------------------------------------------------
// Semaphore::Destroy
// 	Destroy the semaphore, freeing the waiting threads
//	This is used to destroy a user semaphore
//----------------------------------------------------------------------

void
Semaphore::Destroy()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    while ( (thread = queue->Remove() ) != NULL )	// make thread ready
	scheduler->ReadyToRun(thread);

    interrupt->SetLevel(oldLevel);
}

#endif


// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(const char* debugName) {
    this->name = (char*)debugName;
    this->queue = new List<Thread*>;
}


Lock::~Lock() {
    if(this->queue != nullptr) delete this->queue;
}


void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    while (!this->available) {
        this->queue->Append(currentThread);
        currentThread->Sleep();
    }
    this->available = false;
    this->owner = currentThread;
    interrupt->SetLevel(oldLevel);
}


void Lock::Release() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    Thread *thread;
    thread = queue->Remove();
    if (thread != NULL)
        scheduler->ReadyToRun(thread);
    this->available = true;
    interrupt->SetLevel(oldLevel);
}


bool Lock::isHeldByCurrentThread() {
   return this->owner == currentThread;
}


Condition::Condition(const char* debugName) {
    this->name = (char*)debugName;
    this->queue = new List<Thread*>;
}


Condition::~Condition() {
    if (this->queue != nullptr) delete queue;
}


void Condition::Wait(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(conditionLock->isHeldByCurrentThread()) conditionLock->Release();
    while (this->isBusy) {
        this->queue->Append(currentThread);
        currentThread->Sleep();
    }
    this->isBusy = true;
    conditionLock->Acquire();
    interrupt->SetLevel(oldLevel);
}


void Condition::Signal(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if (conditionLock->isHeldByCurrentThread()) {
        Thread* thread = queue->Remove();
        if (thread != NULL) {
            scheduler->ReadyToRun(thread);
            this->isBusy = false;
        }
    }
    interrupt->SetLevel(oldLevel);
}


void Condition::Broadcast( Lock * conditionLock) {
    if (conditionLock->isHeldByCurrentThread()) {
        while (this->queue->IsEmpty()) {
            this->Signal(conditionLock);
        }
    }
}


// Mutex class
Mutex::Mutex( const char * debugName ) {
    this->name = (char*)debugName;
    value = true;
    queue = new List<Thread*>;
}

Mutex::~Mutex() {
    if(this->queue != nullptr) delete this->queue;
}

void Mutex::Lock() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    while (!this->value) {
        this->queue->Append(currentThread);
        currentThread->Sleep();
    }
    this->value = false;
    interrupt->SetLevel(oldLevel);
}

void Mutex::Unlock() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    Thread *thread;
    thread = queue->Remove();
    if (thread != NULL)
        scheduler->ReadyToRun(thread);
    this->value = true;
    interrupt->SetLevel(oldLevel);
}


// Barrier class
Barrier::Barrier(const char * debugName, int size_p) {
    // this->name;
    this->size = (size_t)size_p;
    this->queue = new List<Thread*>;
}

Barrier::~Barrier() {
    if(this->queue != nullptr) delete this->queue;
}

void Barrier::Wait() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if (this->count++ < this->size) {
        this->queue->Append(currentThread);
        currentThread->Sleep();
    } else {
        for(size_t i = 0; i < size; ++i) {
            oldLevel = interrupt->SetLevel(IntOff);
            Thread * thread = queue->Remove();
            if (thread != NULL)	   // make thread ready, consuming the V immediately
            scheduler->ReadyToRun(thread);
            this->count = 0;
        }
    }
    interrupt->SetLevel(oldLevel);
}

