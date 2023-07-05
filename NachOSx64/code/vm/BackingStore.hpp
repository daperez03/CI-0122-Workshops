#ifndef BACKINGSTORE_HPP
#define BACKINGSTORE_HPP

#include "Map.hpp"
#include "../threads/thread.h"
#include "../machine/translate.h"

class BackingStore {
  private:
    Map<TranslationEntry*, char*> backingStore;
  public:
    BackingStore() = default;
    ~BackingStore() = default;
    // Intercambia con la paginacion
    void pageIn(TranslationEntry* pageIn,
      TranslationEntry* pageOut);
    // Mueve una pagina de la memoria al BackingStore
    void pageOut(TranslationEntry* pageOut);
    char* remove(TranslationEntry* pageOut);
};
#endif
