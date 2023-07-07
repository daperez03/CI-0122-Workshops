#ifndef BACKINGSTORE_HPP
#define BACKINGSTORE_HPP

#include "Map.hpp"
#include "../threads/thread.h"
#include "../machine/translate.h"

/// @brief Clase encargada de generar funciones de Swap
class BackingStore {
  private:
    /// @brief Mapa en el que se almacena las paginas
    Map<TranslationEntry*, char*> backingStore;
  public:
    /// @brief Constructor por defecto
    BackingStore() = default;
    /// @brief Destructor por defecto
    ~BackingStore() = default;
    /// @brief Intercambia una pagina en memoria con una en la estructura
    /// @param pageIn Pagina que ingresara a memoria
    /// @param pageOut Pagina que sale de memoria y se almacena en la estructura
    void pageIn(TranslationEntry* pageIn,
      TranslationEntry* pageOut);
    /// @brief Mueve una pagina de la memoria al BackingStore
    /// @param pageOut Pagina que se sacara de memoria
    void pageOut(TranslationEntry* pageOut);
    /// @brief Remueve elementos de la estructura de datos
    /// @param pageOut Indica la pagina que se desea remover
    /// @return Conjunto de datos que almacena
    char* remove(TranslationEntry* pageOut);
};
#endif
