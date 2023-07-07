#include "BackingStore.hpp"
#include "../threads/system.h"


#define NO_PHYSYCAL_PAGE -1

void BackingStore::pageIn(TranslationEntry* pageIn,
  TranslationEntry* pageOut) {
  void* address = (void*)
    &machine->mainMemory[pageOut->physicalPage * PageSize];
  pageIn->physicalPage = pageOut->physicalPage;
  pageOut->physicalPage = NO_PHYSYCAL_PAGE;
  pageOut->use = true;
  pageOut->valid = true;
  pageIn->use = true;
  pageIn->valid = true;
  if (!pageOut->readOnly) {
    char* buffer = new char[PageSize];
    bzero(buffer, PageSize);
    memcpy(buffer, address, PageSize);
    ASSERT(!memcmp(buffer, address, PageSize));
    this->backingStore.insert(pageOut, buffer);
    char* src = this->backingStore[pageIn];
    ASSERT(src);
    bzero(address, PageSize);
    memcpy(address, src, PageSize);
    ASSERT(!memcmp(address, src, PageSize));
    this->backingStore.remove(pageIn);
    delete src;
  }
}

void BackingStore::pageOut(TranslationEntry* pageOut) {
  void* address = (void*)
    &machine->mainMemory[pageOut->physicalPage * PageSize];
  physicalPageMap->Clear(pageOut->physicalPage);
  pageOut->physicalPage = NO_PHYSYCAL_PAGE;
  pageOut->use = true;
  pageOut->valid = true;
  if (!pageOut->readOnly) {
    char* buffer = new char[PageSize];
    bzero(buffer, PageSize);
    memcpy(buffer, address, PageSize);
    ASSERT(!memcmp(address, buffer, PageSize));
    this->backingStore.insert(pageOut, buffer);
    bzero(address, PageSize);
  }
}

char* BackingStore::remove(TranslationEntry* page) {
  char* buffer = this->backingStore[page];
  this->backingStore.remove(page);
  return buffer;
}
