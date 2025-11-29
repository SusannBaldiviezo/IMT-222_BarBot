# IMT-222_BarBot
 1. Descripción General

BarBot es un sistema automatizado capaz de preparar bebidas mezcladas utilizando un carrusel motorizado, sensores de posición, un servo presionador y una interfaz de usuario basada en LCD y botones. El sistema está desarrollado sobre un ESP32, estructurado con FreeRTOS, programado en C++ modular y controlado mediante una Máquina de Estados Finita (FSM).

El proyecto integra los principales conceptos vistos en la materia Sistemas Embebidos I, incluyendo:

Control de actuadores

Sensores digitales

Comunicación I²C

FreeRTOS y multitarea

Diseño FSM

Antirrebote de botones

Modularización en .cpp / .h

Integración hardware–software

Documentación profesional

 2. Objetivo del Proyecto

Diseñar e implementar un sistema embebido capaz de seleccionar, mover, posicionar y dispensar dos líquidos diferentes para formar bebidas mezcladas (ej. Ron + Cola, Vodka + Sprite), manteniendo proporciones definidas (70% mezclador, 30% alcohol).

3. Objetivos Específicos

Implementar una máquina de estados (FSM) que controle el flujo completo del sistema.

Controlar un motor DC mediante finales de carrera para posicionar un carrusel rotatorio.

Servir líquidos mediante un servo accionado por pulsos PWM precisos.

Diseñar un sistema de navegación por menú con botones + LCD I²C.

Implementar antirrebote por software usando técnicas vistas en clase.

Desarrollar el sistema usando FreeRTOS para garantizar concurrencia estable.

Modularizar el software siguiendo buenas prácticas de ingeniería.

Documentar arquitectura, diagramas y flujo del sistema.

 4. Arquitectura General del Sistema

La arquitectura del BarBot se divide en cinco capas:

1. Capa de Entrada

Botones: UP, DOWN y OK (cambiaron recetas y confirman selección).

Final de carrera HOME → marca posición cero del carrusel.

Final de carrera SLOT → detecta cada botella (6 posiciones).

2. Capa de Procesamiento

ESP32 + FreeRTOS

Máquina de Estados (FSM)

Timers basados en millis()

Antirrebote por software

Lógica de recetas (proporciones 30–70)

3. Capa de Control

Cálculo de movimiento del motor

Conteo de posiciones

Control de servo mediante pulsos

Control no bloqueante (RTOS)

4. Capa de Actuadores

Motor DC tipo limpiaparabrisas (giro constante).

Servo SG90 / MG996R para presionar dispensador.

5. Capa de Salida

LCD 16×2 I²C

Indicación de estado actual

“Trago listo”

Movimientos, selección y mensajes de homing

5. Materiales
Electrónica

ESP32 DevKit 30 pines

Pantalla LCD 16×2 con módulo I²C

Servo SG90 o MG996R

Motor DC tipo limpiaparabrisas

Driver de motor (L298N o MOSFET + diodos)

2 finales de carrera

3 botones (UP, DOWN, OK)

Fuente 5V/3A (servo + motor)

Fuente 12V (motor)

Regulador 5V → 3.3V (si se requiere)

Mecánica

Carrusel para 6 botellas

Dispensadores tipo botellón

Acoples para servo

Estructura impresa o fabricada

Vasos de prueba

6. Descripción Completa de la FSM

La FSM controla todo el flujo del BarBot:

ST_IDLE

Sistema inicial sin operación.

ST_BOOT

Inicialización de hardware, LCD, servo y variables.

ST_HOME

Movimiento del carrusel hasta encontrar HOME.
Define currentPos = 0.

ST_MENU

Selección de bebida con UP/DOWN.
Confirmación con OK.

ST_MOVE1

Mover carrusel hasta bottlePos1 (alcohol).
Usa flancos del sensor SLOT.

ST_DISPENSE1

Accionar servo durante pourMs1.
Sirve el 30% del volumen total.

ST_MOVE2

Mover carrusel a bottlePos2 (mezclador).

ST_DISPENSE2

Accionar servo durante pourMs2.
Sirve el 70% del volumen.

ST_DONE

Mensaje “Trago listo”.
Retorna a menú.

7. Flujo del Sistema (Paso a Paso)

El usuario enciende el BarBot.

El sistema hace homing para encontrar posición 0.

Se muestra el menú de bebidas.

El usuario selecciona una receta.

El carrusel se mueve a la botella del alcohol.

El servo presiona el dispensador durante un tiempo calculado.

El carrusel se mueve a la segunda botella.

El servo presiona nuevamente (70%).

Se muestra “Trago listo”.

El usuario vuelve al menú.

8. Lógica de Mezcla 70/30

Suponiendo un caudal constante:

𝑉
=
𝑄
⋅
𝑡
V=Q⋅t

Alcohol: 30% del total

Mezclador: 70%

Por eso:

pourMs1 → tiempo del alcohol

pourMs2 → tiempo del mezclador

Esto puede recalibrarse fácilmente cambiando valores en recipes.cpp.

9. FreeRTOS y Tareas

Se usan dos tareas:

TaskBarbot

Ejecuta fsmStep() cada 10 ms

Procesa sensores

Actualiza estado

Controla motor y servo

TaskHeartbeat

Mantiene un LED parpadeando

Indica que el sistema está vivo

Ayuda al debug entre fallos

 10. Modularización del Código
src/
  - BarBot_FreeRTOS.ino
  - fsm.cpp
  - hardware.cpp
  - buttons.cpp
  - recipes.cpp

inc/
  - fsm.h
  - hardware.h
  - buttons.h
  - recipes.h

Beneficios:

✔ código mantenible
✔ fácil lectura
✔ escalable
✔ profesional
✔ perfecto para presentaciones

11. Pruebas Realizadas

Validación de sensores

Validación de motor + flancos

Prueba del servo con dispensadores

Ajuste de proporciones

Simulación en Wokwi

 12. Estado Actual del Proyecto
Casi completo

✔ FSM funcional
✔ Menú con antirrebote
✔ Recetas configurables
✔ FreeRTOS integrado
✔ Simulación estable
✔ Documentación IEEE
✔ Diagramas

Pendiente

🔲 Montaje final del prototipo
🔲 Calibración de tiempos reales
🔲 Diseño mecánico definitivo

13. Conclusiones

BarBot demuestra la integración total de conceptos de sistemas embebidos, incluyendo:

Sensado

Actuación

Control

Programación modular

RTOS

Diseño FSM

Interacción humano–máquina (HMI)

El proyecto es escalable, robusto, educativo y perfectamente justificable como dominio de la materia.

14. Autores

Susann Bladiviezo-validación de códigos .h
Florencia Frigero-Validación de códigos.h e .ino
Benjamín Soruco-Simulación en KiCad
Alejandro Bejarano-Diseño mecánico y documentación