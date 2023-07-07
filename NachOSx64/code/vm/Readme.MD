# Proyecto de Sistemas Operativos(Etapa 3)

## Implementación

### TLB

En este caso la implementación de TLB ya viene predefinida por NachOS, por lo que lo único que corresponde a esta apartado es indicar que se agregó un índice llamado `tlbIndex` para la implementación del algoritmo de reemplazo.

### Tabla invertida

Para el manejo de memoria entre multihilos fue necesario implementar una tabla invertida, en este caso esta fue implementada mediante un mapa con la tupla `<Thread* , AddrSpace*>`.

### Backing Store

Se llevó a cabo el desarrollo de una clase `BackingStore` la cual es capaz de almacenar páginas de la memoria principal a esta estructura de datos. Esta cuenta con un realloc que le permite no tener capacidad definida. Esta clase implementa los siguientes métodos:

1. `BackingStore()`: Constructor por defecto
2. `~BackingStore()`: Destructor por defecto
3. `pageIn()`: Intercambia una página en memoria con una en la estructura
4. `pageOut()`: Mueve una página de la memoria al BackingStore
5. `remove()`: Remueve elementos de la estructura de datos

### Address Space

Respecto al manejo de memoria que realizamos en la etapa anterior tuvimos que generar una nueva reestructuración. En este caso utilizaremos paginación por demanda, por lo que inicialmente no existe nada cargado en memoria ni en la TLB.
Para poder llevar esto a cabo también se tuvo que generar el manejo del error `PageFault`. Dentro de las nuevas modificaciones que se realizamos tenemos las siguientes:
1. `AddrSpace()`: Al constructor general de `AddrSpace` se le agregó el nombre del programa.
2. `AddrSpace(const AddrSpace&)`: Al constructor por copia de `AddrSpace` se le agregó el nombre del hilo para que pueda ser identificado dentro de la tabla invertida.
3. `FindPage()`: Habilita una página en memoria.
4. `updataPageTables()`: Actualiza las tablas de la page table respecto a TLB.
5. `secondChance()`: Implementa el algoritmo de sustitución second chance.

### Adicionales

Se implementó una clase `Map` con la intención de que fuera útil para implementar la tabla invertida y la page table.

## Manual de usuario

### Compilación
Primero es necesario movilizarse a la carpeta `vm`, mediante la terminal de shell.
Para poder compilar este programa es importante primero contar con unas dependencias establecidas. Para contar con ellas primero debemos ejecutar el siguiente comando:
```
make depend
```
Una vez instaladas las dependencia se puede seguir con la compilación del programa, para esto debemos ejecutar el siguiente comando:
```
make
```

### Ejecución
Una vez ya contemos con un ejecutable del programa podemos seguir con la compilación, este este caso
debemos ejecutar el siguiente comando:

```
./nachos <parámetros> -x <ejecutable>
```

**Parámetros de NachOS:**  
-d <Flag\>: modo de depuración.
-x <executable\>: ejecuta programas de usuario.

**Ejemplo de ejecución:**  
```
./nachos -x ../test/matmult
```
```
./nachos -x ../test/RW
```

### Mejoras
Este proyecto llegó a una etapa muy avanzada, sin embargo aún se sigue generando un error de instrucción invalida, respecto al caso de prueba de `addrspacetest`.

## Autor
**Nombre:**  
Daniel Pérez Morera.
**Carnet:**  
C15906.
**Correo Institucional:**  
daniel.perezmorera@ucr.ac.cr
