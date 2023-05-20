# Correcion del Examen #1

## Enunciado
En el parque nacional Kruger en Sudáfrica hay un cañón muy profundo que cuenta con varias cuerdas (N) para cruzarlo. Los babuinos necesitan cruzar este cañón utilizando alguna de las cuerdas que les sera asignada de manera aleatoria. Como los babuinos son muy agresivos, hay que esperar a que haya B babuinos para cruzar en un lado del canon, de otra manera, menos que B babuinos, harán pleito, terminarán cayendo por el cañón y muriendo.  

Si al momento de llegar al cañón un babuino (Babuino ( int cuerda)) determina que no hay B babuidos, incluyendole, espera en la cuerda respectiva; de otra manera determina cuál es la cuerda que tiene más babuinos (Ci) y deja pasar a todos los que esperan por esa cuerda. Mientras están cruzando los babuinos de la cuerda Ci cualquier otro que llegue a esta misma cuerda debera esperar.

Escriba una solucion con monitores que le permita a los babuinos cruzar sin exponerse a los problemas indicados.
Resuelva el problema en pseudo-código construyendo un monitor, debe entregar el cuerpo del monitor con los metodos descritos, el constructor, el destructor y cualquier otro que considere apropiado. Ademas debe escribir la rutina que representa a cada babuino y que utilice su monitor.
No debe escribir la rutina principal que genera los babuinos.

## Aspectos a tener en cuenta
Como en c++ no existe monitores se simulan mediante mutex.  
Ademas se incluye un main para efectos practicos de poder realizar pruebas sobre el codigo. Sin embargo, como el codigo esta orientado a que nunca sea detenido entonces se va quedar n cantidad de hilos esperando a ser liberados, esto unicamente es con mero proposito ilustrativo de la funcion del programa.

## Compilar
Para compilar esta simulacion unicamente debe realizar el siguiente comando:
```
make run
```

## Ejecutar
Para ejecutar esta simulacion unicamente debe realizar el siguiente comando:

```
./bin/Babuino.run
```
