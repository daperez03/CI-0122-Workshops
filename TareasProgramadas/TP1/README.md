# Tarea Programada 1

## Estudiante

Nombre: [Daniel Pérez-Morera](mailto:daniel.perezmorera@ucr.ac.cr)  
Carnet: C15906  

## Profesor

Rafael Francisco-Arroyo

## Descripción de la tarea

En este tarea hemos buscado la manera en la cual generar un simulador del juego de la papa caliente,
en el cual podamos comunicarnos con procesos externos al principal y de esa forma generar un control
de flujo de los procesos y los datos. Para este caso en específico hemos implementado tres modelos,
los cuales son:

1. MailBox: en este primer modelo se utiliza una cola compartida entre los procesos, la cual permite el
intercambio de datos y a su vez poder generar que un proceso espera hasta que reciba su mensaje sobre esta
cola.

2. Pthreads: en este modelo hemos utilizados hilos y semáforos de unix, los cuales nos permitieron generar
una estructura de memoria compartida común para cada hilo y a su vez el semáforo el cual iba a regular el paso
de los procesos sobre esta estructura.

3. ShM & Semaphore: Para finalizar hemos creado un último modelo con las funciones shM y sem de UNIX. Esta
funciones nos permitieron generar un espacio compartido, el cual contendrá los atributos de la papa y mediante
el semáforo regularía la lectura y escritura sobre esta papa.

## Manual de compilacion y ejecucion

En este breve apartado explicaremos los pasos a seguir para lograr compilar y ejecutar de manera
adecuada nuestros modelos. Cabe recalcar que los pasos son idénticos para los tres modelos.
Estos pasos son:

1. Se debe dirigir desde la terminal a una de las carpetas que contiene el programa a ejecutar.
2. Se debe ejecutar el comando `make`
3. Ejecutar el programa con el comando `./bin/<ejecutable> <vida> <players> <rotación>`
	* Si se desea ejecutar el programa de manera default no colocaremos ningún argumento.
	* Si se desea modificar los parámetros del programa deberemos ingresar los tres parámetros
	de manera obligatoria.
    	* vida: Indiaca la vida con la cual iniciara la papa el juego.
    	* players: Indica la cantidad de procesos que jugarán el juego.
    	* dirección: Indica el sentido de rotación del juego, donde se
    	deberá escoger entre `l`(left) o `r`(right).
