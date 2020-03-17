Ej 6. Ejecuta el simulador con el programa prog-V1-E6, que especifica un tamaño de proceso que supera el 
máximo posible por proceso. Prueba a cambiar el tamaño a un valor válido, por ejemplo 50, y comprueba lo 
que pasa. ¿Por qué hace lo que hace con el tamaño 65?
Porque supera el tamaño disponible en memoria.

Ej 13. Estudia la función OperatingSystem_SaveContext():
a. ¿Por qué hace falta salvar el valor actual del registro PC del procesador y de la PSW?
Para poder continuar en un futuro la ejecución del proceso en el estado anterior a dejar de ejecutarse.
b. ¿Sería necesario salvar algún valor más?
El registro acumulador.
c. A la vista de lo contestado en los apartados a) y b) anteriores, ¿sería necesario realizar
alguna modificación en la función OperatingSystem_RestoreContext()? ¿Por qué?
Sí, para recuperar también el valor del acumulador.
d. ¿Afectarían los cambios anteriores a la implementación de alguna otra función o a la definición 
de alguna estructura de datos?
Al Struct PCB, habría que añadir el campo copyOfAccumulatorRegister.

Ej 14. Estudia lo que hace la función OperatingSystem_TerminateProcess y en particular lo que pasa
cuando el que termina es el SystemIdleProcess.
Si no hay procesos no terminados y se está ejecutando el SystemIdleProcess, se finaliza el proceso y se
apaga el simulador. Si hay procesos no terminados, se selecciona cuál será el siguiente en ejecutarse y 
se asigna este al procesador.

16. Ya sabemos lo que pasa cuando se ejecuta una instrucción HALT. Ahora ejecuta el simulador con
programas que tengan instrucciones OS e IRET. Estudia el comportamiento en ambos casos. ¿Por
qué hace lo que hace?
El programa con instrucción OS llama al manejador de la interrupción indicada, y el programa con IRET 
hace el retorno de un tratamiento de interrupción.


Mª Isabel Fernández Pérez, UO257829
