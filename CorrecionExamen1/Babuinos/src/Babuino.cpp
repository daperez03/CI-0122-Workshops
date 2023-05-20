#include "Babuino.hpp"
#include <iostream>

/// Imprime las estadisticas de una cuerda y el total de Babuinos
void printStats(int cuerda, int count, int countBabuinos) {
  std::cout << "________________________________________" << std::endl;
  std::cout << "Numero de hilos en la cuerda " << cuerda << ": "
    << count << std::endl;
  std::cout << "Numero de hilos total: " << countBabuinos << std::endl;
}

/// Constructor de Babuino, unicamente recibe un struct de datos
/// necesarios para su ejecucion
Babuino::Babuino(struct SharedData& babuinoData):
  babuinoData(babuinoData){}

void Babuino::run(int cuerda) {
  /// Simulamos un monitor con un 'unique_lock' que lo que hacer es
  /// pedir el mutex en el constructor de el y liberarlo en el destructor.
  std::unique_lock<std::mutex> babuinoLock(babuinoData.canAcces);
  ++babuinoData.count[cuerda]; /// Aumentamos el nuevo Babuino en su cuerda
  if (++babuinoData.countBabuinos == B) { /// Aumentamos el total de babuinos
    /// Si los Babuinos son iguales a B hay que liberar una cuerda
    printStats(cuerda, babuinoData.count[cuerda], babuinoData.countBabuinos);
    int mayor = 0;
    for(int i = 0; i < N; ++i) { /// Buscarmos la cuerda con mas Babuinos
      if (babuinoData.count[i] > babuinoData.count[mayor]) mayor = i;
    }
    std::cout << "Liberando cuerda " << mayor << std::endl;
    /// Liberamos la cuerda con mas Babuinos
    babuinoData.ropeWaiting[mayor].notify_all();
    /// Restamos los babuinos liberados del total
    babuinoData.countBabuinos -= babuinoData.count[mayor];
    /// Ponemos la cuerda liberada en cero
    babuinoData.count[mayor] = 0;
    /// Si yo no era parte de la cuerda liberada debo esperar a que
    /// me liberen
    if(mayor != cuerda) {
      babuinoData.ropeWaiting[cuerda].wait(babuinoLock);
    }
  } else {
    /// Si los Babuinos no son iguales a B esperamos
    printStats(cuerda, babuinoData.count[cuerda], babuinoData.countBabuinos);
    babuinoData.ropeWaiting[cuerda].wait(babuinoLock);
  }
}