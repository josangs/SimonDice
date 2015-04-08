# SimonDice
Código realizado por mi de el conocido juego Simon.

Este código esta implementado sobre un LaunchPad MSP430 (MSP-EXP430G2) y el entorno de software IAR Embedded Workbench. La 
aplicación necesitará del añadadido de un HW externo, consistente en 3 pulsadores y 3 leds. Como el LaunchPad trae de por si dos 
leds y dos pulsadores, aunque solo podemos utilizar uno de ellos, dado que el otro tiene implementado la función de reset. Pues 
solo nos haría falta tener 1 led y 2 pulsadores.

Una breve explicación de su funcionamiento, sería esta: Al inicio los 3 leds deberán estar iluminados, y el sistema en modo de bajo 
consumo esperando la pulsación de una tecla cualquiera. Tras la pulsación los leds se apagarán y se iluminarán de uno en uno, de 
forma consecutiva, en intervalos de 330 ms (son 3 leds si fueran 4 sería en intervalos de 250 ms). Finalmente quedarán apagados 
duarante 1 segundo, tras el que el sistema propondrá la primera secuencia de tamaño 1.

En caso de acertar la secuencia, el sistema esperará un intervalo de 2 segundos para proponer la misma secuencia, pero añadiendo
una nueva luz escogida de forma aleatoria.

NOTA: Yo no he iniciado la semilla (srand), por lo que las secuencias se repiten siempre igual.

