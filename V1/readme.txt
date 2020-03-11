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



Mª Isabel Fernández Pérez, UO257829
