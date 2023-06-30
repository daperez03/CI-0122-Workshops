#ifndef BACKINGSTORE_HPP
#define BACKINGSTORE_HPP

#include "Map.hpp"
class Thread;

class BackingStore {
  private:
    typedef struct Key {
      unsigned int vpn;
      Thread* currentThread;
    } Key;
    
    Map<Key, char*> backingStore;
  public:
    BackingStore() = default;
    ~BackingStore() = default;
    // Intercambia con la paginacion
    void pageIn(TranslationEntry* pageIn,
      TranslationEntry* pageOut);
    // Mueve una pagina de la memoria al BackingStore
    void pageOut(TranslationEntry* pageOut);
};
#endif
