# Proyecto de Sistemas Operativos(Etapa 2)

## Implementación

### System Call

Esta etapa está enfocada en el desarrollo de system calls, las cuales deben permitir ejecutar programas de usuario. Específicamente se desarrollaron los siguientes llamados al sistema:

1. **NachOS_Halt:** Detiene un proceso.
2. **NachOS_Exit:** Elimina recursos y finaliza un proceso.
3. **NachOS_Exec:** Crea un nuevo thread sobre otro ejecutable.
4. **NachOS_Join:** Espera por la llegada de un thread.
5. **NachOS_Create:** Crea un nuevo documento.
6. **NachOS_Open:** Abre un documento ya existente.
7. **NachOS_Read:** Lee un documento.
8. **NachOS_Write:** Escribe sobre un documento.
9. **NachOS_Close:** Cierra un documento abierto.
10. **NachOS_Fork:** Crea un nuevo thread sobre una función del programa.
11. **NachOS_Yield:** Entrega la CPU a otros procesos.
12. **NachOS_SemCreate:** Crea un semáforo.
13. **NachOS_SemDestroy:** Destruye un semáforo.
14. **NachOS_SemSignal:** Realiza un signal sobre un semáforo.
15. **NachOS_SemWait:** Realiza un wait sobre un semáforo.
16. **NachOS_LockCreate:** Crea un lock.
17. **NachOS_LockDestroy:** Destruye un lock.
18. **NachOS_LockAcquire:** Realiza un acquire sobre un lock.
19. **NachOS_LockRelease:** Realiza un release sobre un lock.
20. **NachOS_CondCreate:** Crea una variable de condición.
21. **NachOS_CondDestroy:** Destruye una variable de condición.
22. **NachOS_CondSignal:** Realiza un signal sobre una variable de condición.
23. **NachOS_CondWait:** Realiza un wait sobre una variable de condición.
24. **NachOS_CondBroadcast:** Realiza un wait signal broadcast sobre una variable de condición.
25. **NachOS_Socket:** Crea un socket.
26. **NachOS_Connect:** Establece una conexión con un servidor.
27. **NachOS_Bind:** Permite atar un puerto.
28. **NachOS_Listen:** Permite escuchar request de clientes.
29. **NachOS_Accept:** Permite aceptar conexiones.
30. **NachOS_Shutdown:** Cierra una conexión.

### Address Space

Seguidamente de la implementación de estos casos, reestructuró el **manejo de memoria de los procesos**
dentro de la clase *AddrSpace*, la cual es la encargada de asignar memoria física y lógica a cada página de los procesos, esta clase cuenta con los siguientes métodos:
1. Constructor por ejecutable: Permite construir un address space a partir de un ejecutable MIPS.
2. Constructor por copia: Permite construir un nuevo address space que comparte segmento de código
y de datos con el padre.

### Adicionales

Además para el manejo de las páginas físicas, específicamente saber cuales paginas se tienen disponibles se utilizó una clase *BitMap*, por otro lado para el manejo de ID de semáforos, lock, files y variables de condición, se utilizó una clase *OSTbale*, la cual cumple la función de un mapa, asociando un objeto a un número entero el cual corresponde al id asignado a ese objeto.

### Cambio en el hardware

Por motivos de prueba se amplió el número de páginas disponibles de la memoria física, para poder ejecutar programas más grandes. Específicamente el test `todos` combinado con `shell`, todo esto fue aprobado por el profesor correspondiente.

## Manual de usuario

### Compilación
Para poder compilar este programa es importante primero contar con unas dependencias establecidas,
para poder contar con ellas primero debemos ejecutar el siguiente comando:
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
./nachos -x ../test/addrspacetest
```
```
./nachos -x ../test/todos
```

### Casos de prueba
Para los casos de prueba se tiene habilitado una función dentro del *Makefile* un conjunto de
comandos llamado *test*, el cual permite comparar dos programas deterministas, mediante la salida
esperada, para poder llevar esto acabo se debe ejecutar el comando:
```
make test
```

## Autor
**Nombre:**  
Daniel Pérez Morera.
**Carnet:**  
C15906.
**Correo Institucional:**  
daniel.perezmorera@ucr.ac.cr
