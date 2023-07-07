// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "translate.h"

#define UserStackSize		1024 	// increase this as necessary!
#define FILE_NAME_SIZE 1000

class Thread;

/// @brief Clase que maneja memoria Fisica/Virtual
class AddrSpace {
  public:
    #ifdef VM
    /// @brief Crea una instancia de AddrSpace
    /// @param executable Ejecutable con el binario ejecutable
    /// @param programName Nombre del archivo ejecutable
    AddrSpace(OpenFile* executable, const char* programName);
    /// @brief Crea una instancia de AddrSpace a partit de otra instancia
    /// @param father Instancia que se desea copiar
    /// @param thread Identificador del hilo que se le atribuye la instancia
    AddrSpace(const AddrSpace& father, Thread* thread);
    /// @brief Habilita una pagina en memoria
    /// @param availablePage Variable en la que se colocara el numero de pagina
    /// @param pageOut Contendra un puntero a la TranslationEntry a remplazar
    void FindPage(int& availablePage, TranslationEntry*& pageOut);
    /// @brief Genera una tabla de punteros a los TranslationEntry
    /// en memoria fisica
    /// @return Puntero a la tabla
    TranslationEntry** getPhysicalPageTable();
    /// @brief Actualiza las tablas de la page table respecto a TLB
    void updataPageTables();
    /// @brief Implementacion de sustitucion secondChance
    /// @param table Table sobre la que se desea realizar el replace
    /// @param index Indice actual del algoritmo
    /// @param range Rango de la tabla
    /// @return Pagina que se debe remplazar
    TranslationEntry* secondChance(
      TranslationEntry* table, int& index, int range);
    /// @brief Implementacion de sustitucion secondChance
    /// @param table Table sobre la que se desea realizar el replace
    /// @param index Indice actual del algoritmo
    /// @param range Rango de la tabla
    /// @return Pagina que se debe remplazar
    TranslationEntry* secondChance(
      TranslationEntry** table, int& index, int range);
    #else
    /// @brief Crea un nuevo address space para un ejecutable
    /// @param executable Archvio ejecutable de Mips
    AddrSpace(OpenFile* executable);	// Create an address space,
    /// @brief Constructor por copia de address space
    /// @details Copia el segmento de codigo y datos del original y agrega un nuvo stack
    /// @param father Address space a copiar
    AddrSpace(const AddrSpace& father);
    #endif

    /// @brief Destructor por default de address space
    ~AddrSpace();  // De-allocate an address space

    /// @brief Inicializa registros a nivel de usuario
    /// @details Usado para poder hacer un context swicth
    void InitRegisters();		// Initialize user-level CPU registers,
      // before jumping to user code

    /// @brief Salva/restaura estados de un address space especifico
    /// @details Usado para poder hacer un context swicth
    void SaveState();  // Save/restore address space-specific

    /// @brief Informacion sobre el context switch
    void RestoreState();  // info on a context switch

    #ifdef VM 
    void PageFaultException(); 
    #endif

  private:
    /// @brief Tabla de paginas
    TranslationEntry* pageTable;	// Assume linear page table translation
      // for now!
    /// @brief Numero de paginas del address space
    unsigned int numPages;		// Number of pages in the virtual 
      // address space
    /// @brief numero de usuarios del address space
    int usage = 1;
    #ifdef VM 
    char file[FILE_NAME_SIZE];
    #endif
};

#endif // ADDRSPACE_H
