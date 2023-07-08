// #define VM
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
#include "addrspace.h"
#include "noff.h"
#include "system.h"

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

AddrSpace::AddrSpace(OpenFile* executable, const char* programName) {
  DEBUG('A', "AddrSpace: executable constructor.\n");
  NoffHeader noffH;
  unsigned int size;
  strcpy(this->file, programName);
  stats->numDiskReads +=
    executable->ReadAt((char*)&noffH, sizeof(noffH), 0);
  if ((noffH.noffMagic != NOFFMAGIC) &&
    (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    SwapHeader(&noffH);
  ASSERT(noffH.noffMagic == NOFFMAGIC);

  // how big is address space?
  size = noffH.code.size + noffH.initData.size
    + noffH.uninitData.size + UserStackSize;	// we need to increase the size
  // to leave room for the stack
  this->numPages = divRoundUp(size, PageSize);
  size = this->numPages * PageSize;

  // TODO(me): Revisar respecto al espacio del backing store
  // ASSERT(numPages <= NumPhysPages);		// check we're not trying
  // ASSERT((int)numPages <= physicalPageMap->NumClear());
  // to run anything too big --
  // at least until we have
  // virtual memory

  DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
    this->numPages, size);
  // first, set up the translation
  this->pageTable = new TranslationEntry[this->numPages];
  unsigned int codeSize = noffH.code.size % PageSize;
  for (size_t i = 0; i < numPages; ++i) {
    this->pageTable[i].virtualPage = i;
    this->pageTable[i].physicalPage = NO_PHYSYCAL_PAGE;
    this->pageTable[i].valid = false;
    this->pageTable[i].use = false;
    this->pageTable[i].dirty = false;
    if (i < codeSize) { // TODO(me): revisar
      this->pageTable[i].readOnly = false;
    } else { // if the code segment was entirely on
      this->pageTable[i].readOnly = false;
    }
  }
  invertedTable.insert(currentThread, this);
}

AddrSpace::AddrSpace(const AddrSpace& father, Thread* thread) {
  DEBUG('A', "AddrSpace: Copy constructor.\n");
  // TODO(me): Revisar el manejo de usage
  ++this->usage;
  this->numPages = father.numPages;
  this->pageTable = new TranslationEntry[numPages];
  strcpy(this->file, father.file);
  // int stackPagesInit =
    // this->numPages - divRoundUp(UserStackSize, PageSize);
  // TODO(me): Revisar respecto al espacio del backing store
  // ASSERT(divRoundUp(UserStackSize, PageSize) <= physicalPageMap->NumClear());
  for (unsigned int i = 0; i < this->numPages; ++i) {
    // TODO(us): Verificar como copiar page
    this->pageTable[i].physicalPage = NO_PHYSYCAL_PAGE;
    this->pageTable[i].virtualPage = i;
    // TODO(me): Poner en invalido
    this->pageTable[i].valid = false;
    this->pageTable[i].use = false;
    this->pageTable[i].dirty = false;
    this->pageTable[i].readOnly =
      father.pageTable[i].readOnly;
  }
  invertedTable.insert(thread, this);
}


//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------
AddrSpace::~AddrSpace() {
  DEBUG('A', "AddrSpace: default destructor.\n");
  // Limpiar memoria
  for (size_t i = 0; i < this->numPages; ++i) {
    int currentPage = this->pageTable[i].physicalPage;
    if (currentPage != NO_PHYSYCAL_PAGE) {
      physicalPageMap->Clear(currentPage);
      void* address =
        &machine->mainMemory[currentPage * PageSize];
      bzero(address, PageSize);
    } else if (this->pageTable[i].valid &&
      this->pageTable[i].dirty) {
      char* buffer = backingStore->remove(&this->pageTable[i]);
      delete buffer;
    }
  }
  invertedTable.remove(currentThread);
  delete this->pageTable;
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
  this->updataPageTables();
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
  for (size_t i = 0; i < TLBSize; ++i) {
    machine->tlb[i].valid = false;
  }
}

void AddrSpace::PageFaultException() {
  this->updataPageTables();
  ++stats->numPageFaults;
  int addr = machine->ReadRegister(BadVAddrReg);
  unsigned int vpn = (unsigned) addr / PageSize;
  int position = vpn * PageSize;
  TranslationEntry* pageIn = &this->pageTable[vpn];
  // Algoritmo de remplazo a TLB
  TranslationEntry* pageOut = this->secondChance(machine->tlb,
    tlbIndex, TLBSize);

  DEBUG('D', "PageIn->virtualPage = %i\n", pageIn->virtualPage);
  DEBUG('D', "PageIn->valid = %i\n", pageIn->valid);
  DEBUG('D', "PageIn->dirty = %i\n", pageIn->dirty);
  DEBUG('D', "PageOut->virtualPage = %i\n", pageOut->virtualPage);
  DEBUG('D', "PageOut->valid = %i\n", pageOut->valid);
  DEBUG('D', "PageOut->dirty = %i\n", pageOut->dirty);

  if (pageIn->physicalPage == NO_PHYSYCAL_PAGE) {
    if (pageIn->valid && pageIn->dirty && !pageIn->readOnly) {

      DEBUG('D', "Busca en el backingStore\n\n");

      // Buscar en backingStore
      TranslationEntry** table = this->getPhysicalPageTable();
      TranslationEntry* pageOutPT =
        this->secondChance(table, physicalIndex, NumPhysPages);
      backingStore->pageIn(pageIn, pageOutPT);
      delete table;

    } else {
      TranslationEntry* pageOutPT = nullptr;
      int availablePage = NO_PHYSYCAL_PAGE;
      // Pagina a remplazar o utilizar
      FindPage(availablePage, pageOutPT);
      if (availablePage == NO_PHYSYCAL_PAGE) {

        DEBUG('D', "Se coloca la pageOutPT en swap\n");
        DEBUG('D', "pageOutPT->virtualPage = %i\n", pageOutPT->virtualPage);
        DEBUG('D', "pageOutPT->valid = %i\n", pageOutPT->valid);
        DEBUG('D', "pageOutPT->dirty = %i\n", pageOutPT->dirty);

        availablePage = pageOutPT->physicalPage;
        backingStore->pageOut(pageOutPT);
      }

      OpenFile* executable = fileSystem->Open(this->file);
      NoffHeader noffH;
      stats->numDiskReads +=
        executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
      pageIn->physicalPage = availablePage;
      char* into = &machine->mainMemory[availablePage * PageSize];
      int physicalPosition = noffH.code.inFileAddr + position;
      int executableSize = noffH.code.size + noffH.initData.size;
      bzero(into, PageSize);
      if ((int)vpn < divRoundUp(executableSize, PageSize)) {
        DEBUG('D', "Busca en el ejecutable %s\n\n", this->file);

        stats->numDiskReads +=
          executable->ReadAt(into, PageSize, physicalPosition);
      } else DEBUG('D', "Limpia zona de stack/DNI\n\n");
      delete executable;
    }
  } else DEBUG('D', "Ya la pagina existe en memoria\n\n");
  ASSERT(pageOut);
  ASSERT(pageIn);
  pageIn->valid = true;
  pageIn->use = true;
  // Escribe en cache su remplazo
  memcpy(pageOut, pageIn, sizeof(TranslationEntry));
}

void AddrSpace::FindPage(int& availablePage,
  TranslationEntry*& pageOut) {
  availablePage = physicalPageMap->Find();
  if (availablePage == NO_PHYSYCAL_PAGE) {
    TranslationEntry** table = this->getPhysicalPageTable();
    pageOut = this->secondChance(table,
      physicalIndex, NumPhysPages);
    delete table;
  }
}

TranslationEntry** AddrSpace::getPhysicalPageTable() {
  TranslationEntry** PhysicalPageTable =
    new TranslationEntry*[NumPhysPages];
  int iterator = 0;
  Thread** keys = invertedTable.getKeys();
  size_t size = invertedTable.getCount();
  for (size_t i = 0; i < size; ++i) {
    AddrSpace* addrSpace = invertedTable[keys[i]];
    TranslationEntry* table = addrSpace->pageTable;
    for (size_t j = 0; j < addrSpace->numPages; ++j) {
      if (table[j].physicalPage != NO_PHYSYCAL_PAGE) {
        PhysicalPageTable[iterator++] = &table[j];
      }
    }
  }
  delete[] keys;
  return PhysicalPageTable;
}

TranslationEntry* AddrSpace::secondChance(
  TranslationEntry* table, int& index, int range) {
  ASSERT(table);
  while (table[index].use && table[index].valid) {
    ASSERT(index < range);
    if (table[index].physicalPage != NO_PHYSYCAL_PAGE) {
      table[index].use = false;
    }
    index = (index + 1) % range;
  }
  TranslationEntry* swapPage = &table[index];
  index = (index + 1) % range;
  return swapPage;
}

TranslationEntry* AddrSpace::secondChance(
  TranslationEntry** table, int& index, int range) {
  ASSERT(table);
  while (table[index]->use && table[index]->valid) {
    ASSERT(index < range);
    if (table[index]->physicalPage != NO_PHYSYCAL_PAGE) {
      table[index]->use = false;
    }
    index = (index + 1) % range;
  }
  return table[index];
}


void AddrSpace::updataPageTables() {
  Thread** keys = invertedTable.getKeys();
  size_t size = invertedTable.getCount();
  if (keys != nullptr) {
    for (size_t it1 = 0; it1 < TLBSize; ++it1) {
      TranslationEntry* currentPage = &machine->tlb[it1];
      for (size_t it2 = 0; it2 < size; ++it2) {
        AddrSpace* addrSpace = invertedTable[keys[it2]];
        TranslationEntry* table = addrSpace->pageTable;
        for (size_t it3 = 0; it3 < addrSpace->numPages; ++it3) {
          if (table[it3].physicalPage == currentPage->physicalPage) {
            table[it3].dirty = currentPage->dirty;
            table[it3].valid = currentPage->valid;
          }
        }
      }
    }
    delete keys;
  }
}

#endif