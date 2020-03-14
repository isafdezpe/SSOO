Ejercicios realizados: 12,13

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

Ej 15.



Mª Isabel Fernández Pérez, UO257829
