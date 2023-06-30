#define VM
#ifdef VM 

// TODO(us): Adaptar a memoria virtual

// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

#define NO_PHYSYCAL_PAGE -1

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void SwapHeader (NoffHeader *noffH) {
  noffH->noffMagic = WordToHost(noffH->noffMagic);
  noffH->code.size = WordToHost(noffH->code.size);
  noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
  noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
  noffH->initData.size = WordToHost(noffH->initData.size);
  noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
  noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
  noffH->uninitData.size = WordToHost(noffH->uninitData.size);
  noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
  noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile* exec) {
  DEBUG('A', "AddrSpace: executable constructor.\n");
  NoffHeader noffH;
  unsigned int size;
  this->executable = exec;

  stats->numDiskReads += executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
  if ((noffH.noffMagic != NOFFMAGIC) &&
    (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    SwapHeader(&noffH);
  ASSERT(noffH.noffMagic == NOFFMAGIC);

  // how big is address space?
  size = noffH.code.size + noffH.initData.size
    + noffH.uninitData.size + UserStackSize;	// we need to increase the size
  // to leave room for the stack
  numPages = divRoundUp(size, PageSize);
  size = numPages * PageSize;

  // TODO(me): Revisar respecto al espacio del backing store
  // ASSERT(numPages <= NumPhysPages);		// check we're not trying
  // ASSERT((int)numPages <= physicalPageMap->NumClear());
  // to run anything too big --
  // at least until we have
  // virtual memory

  DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
    numPages, size);
  // first, set up the translation
  this->pageTable = new TranslationEntry[numPages];
  unsigned int codeArea = noffH.code.size % PageSize;
  for (size_t i = 0; i < numPages; ++i) {
    this->pageTable[i].virtualPage = i;
    this->pageTable[i].physicalPage = NO_PHYSYCAL_PAGE;
    this->pageTable[i].valid = false;
    this->pageTable[i].use = false;
    this->pageTable[i].dirty = false;
    if (i < codeArea) {
      // if page is only code is read only
      this->pageTable[i].readOnly = true;
    } else {
      // if the code segment was entirely on
      this->pageTable[i].readOnly = false;
    }
  }
}

AddrSpace::AddrSpace(const AddrSpace& father) {
  DEBUG('A', "AddrSpace: Copy constructor.\n");
  // TODO(me): Revisar el manejo de usage
  ++this->usage;
  this->numPages = father.numPages;
  this->pageTable = new TranslationEntry[numPages];
  // int stackPagesInit =
    // this->numPages - divRoundUp(UserStackSize, PageSize);
  // TODO(me): Revisar respecto al espacio del backing store
  // ASSERT(divRoundUp(UserStackSize, PageSize) <= physicalPageMap->NumClear());
  for (unsigned int i = 0; i < this->numPages; ++i) {
    // TODO(us): Verificar como copiar page
    this->pageTable[i].physicalPage = NO_PHYSYCAL_PAGE;
    this->pageTable[i].virtualPage = father.pageTable[i].virtualPage;
    // TODO(me): Poner en invalido
    this->pageTable[i].valid = father.pageTable[i].valid;
    this->pageTable[i].use = father.pageTable[i].use;
    this->pageTable[i].dirty = father.pageTable[i].dirty;
    this->pageTable[i].readOnly = father.pageTable[i].readOnly;
  }
}


//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------
AddrSpace::~AddrSpace() {
  DEBUG('A', "AddrSpace: default destructor.\n");
  // Limpiar memoria
  int globalPage = this->numPages -
    divRoundUp(UserStackSize, PageSize);
  // TODO(me): Revisar el manejo de usage
  if (--this->usage == 0) {
    // Elimina memoria de la seccion de codigo y datos 
    for (int i = 0; i < globalPage; ++i) {
      int page = this->pageTable[i].physicalPage;
      physicalPageMap->Clear(page);
      bzero(machine->mainMemory + page * PageSize, PageSize);
    }
  }
  // Elimina memoria del stack
  for (int i = globalPage; i < (int)this->numPages; ++i) {
    int page = this->pageTable[i].physicalPage;
    if (page != NO_PHYSYCAL_PAGE) {
      physicalPageMap->Clear(page);
      bzero(machine->mainMemory + page * PageSize, PageSize);
    } else {
      // TODO(me): eliminar del backing store
    }
  }
  delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void AddrSpace::InitRegisters() {
  DEBUG('A', "AddrSpace: InitRegisters.\n");
  int i;
  for (i = 0; i < NumTotalRegs; i++)
	  machine->WriteRegister(i, 0);
  // Initial program counter -- must be location of "Start"
  machine->WriteRegister(PCReg, 0);	
  // Need to also tell MIPS where next instruction is, because
  // of branch delay possibility
  machine->WriteRegister(NextPCReg, 4);
  // Set the stack register to the end of the address space, where we
  // allocated the stack; but subtract off a bit, to make sure we don't
  // accidentally reference off the end!
  machine->WriteRegister(StackReg, numPages * PageSize - 16);
  DEBUG('a', "Initializing stack register to %d\n",
    numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() {
  DEBUG('A', "AddrSpace: SaveState.\n");

}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() {
  DEBUG('A', "AddrSpace: RestoreState.\n");
  machine->pageTable = pageTable;
  machine->pageTableSize = numPages;
}

void AddrSpace::PageFaultException() {
  int addr = machine->ReadRegister(BadVAddrReg);
  unsigned int vpn = (unsigned) addr / PageSize;
  TranslationEntry* pageIn = &this->pageTable[vpn];
  TranslationEntry* pageOut = nullptr;
  char buffer[PageSize];
  bzero(buffer, PageSize);
  if (!pageIn->valid || pageIn->readOnly ||
    !pageIn->dirty) {
    int position = noffH.code.inFileAddr + (vpn * PageSize);
    stats->numDiskReads +=
      this->executable->ReadAt(buffer, PageSize, position);
    //TODO(us):Find page replace
  }
  //TODO(me): Algoritmo de remplazo
}

#endif