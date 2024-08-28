# Simon_Dice
El proyecto consistirá en prototipar una versión del famoso juego Simon [1]. Para llevar a cabo la implementación se necesitarán los siguientes componentes:
 
Una protoboard.
Un microcontrolador: Arduino o ESP32.
3 LEDs de colores: rojo, verde y amarillo.
3 resistencias de 330 ohmios para los LEDs.
4 pulsadores.
4 resistencias de 4.7 K para los pulsadores.

El comportamiento del juego será el siguiente:
1) Aparecerá un menú para que el usuario pueda elegir el nivel de dificultad. Habrá tres niveles. A más dificultad más rápido se encenderán los LEDs.
   
2) Una vez seleccionada la dificultad:
a.Se encenderá una secuencia de colores de forma aleatoria. Por defecto empezará en 3.
b.El usuario deberá acertar que secuencia se de colores se ha generado a través de los pulsadores. Cada LED tendrá asociado un pulsador.
c.Si el usuario acierta se incrementa el número de LEDs que se encienden en la secuencia (el máximo es 10) y se vuelve al punto a.
d.Si el usuario falla pierde una vida se vuelve al punto a y la secuencia de LEDs que se muestran vuelve a su número inicial que es 3.

3) Si el usuario pierde todas las vidas parpadearán todos los LEDs.

4) Hay un cuarto botón para reiniciar el juego.

Mi versión expande los LEDs a cuatro junto a cinco botones (cuatro para los LEDs y uno de reinicio) 
y un zumbador activo tal y como se ve en el juego verdadero, además le he añadido una pantalla TFT de 1.8 pulgadas.
El conexionado de los LED se realiza con resistencias de 220 Ω en vez de con 330 Ω.
El conexionado de los botones se realiza solo con el cable de señal y uno a GND gracias al pullup interno que ofrece Arduino en sus pines.

[1] https://es.wikipedia.org/wiki/Simon_%28juego%29
