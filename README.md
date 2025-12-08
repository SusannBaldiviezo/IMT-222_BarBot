🥤 BarBot – Sistema Automatizado de Preparación de Bebidas (ESP32 + FreeRTOS + Control + FSM)
📌 Descripción general del proyecto

BarBot es un sistema embebido desarrollado con ESP32 que automatiza la selección, posicionamiento y dispensado de bebidas utilizando servos, motor DC y una máquina de estados finita (FSM).
La gestión temporal del sistema se organiza mediante FreeRTOS, logrando un funcionamiento estable, modular y sin bloqueos.

El proyecto es parte del desarrollo académico de la Universidad Católica Boliviana (UCB – Tarija), integrando conceptos de Sistemas Embebidos, Control I, FreeRTOS, y Arquitectura modular basada en tareas.

🧩 Objetivos del proyecto
🎯 General

Desarrollar un prototipo funcional capaz de preparar bebidas automáticamente, integrando control de actuadores, manejo de tareas y organización modular del software.

🎯 Específicos

Implementar control básico de posición de servos y motor.

Diseñar una máquina de estados (FSM) para gestionar el flujo del BarBot.

Integrar FreeRTOS para tareas concurrentes.

Organizar el código de forma modular (src, inc, docs, img).

Desarrollar recetas configurables.

Presentar el avance del proyecto con fundamentos de control, embebidos y buenas prácticas.

⚙️ Características principales

Microcontrolador: ESP32

Sistema operativo: FreeRTOS

Arquitectura:

Máquina de estados (FSM)

Tareas concurrentes

Código modular por componentes

Actuadores:

Servo de giro

Servo de dispensado

Servo de entrega (servicio)

Motor DC para carrusel

Interfaz:

Botones (UP / DOWN / OK)

LCD I2C

LED Heartbeat

Recetas configurables

Control básico proporcional (Control I)

🛠️ Estructura del repositorio
IMT-222_BarBot/
│
├── src/              → Código fuente (.cpp)
├── inc/              → Headers (.h)
├── docs/             → Documentos, informes, diagramas
├── img/              → Imágenes y recursos
├── README.md         → Este archivo
└── .gitignore

🧠 Arquitectura del software

El sistema está dividido en módulos independientes:

Módulo	Función principal
FSM	Control de estados del BarBot (MENU, SERVING, DONE)
Hardware	Control de servos, motor, LCD, botones
Buttons	Manejo de entradas + antirrebote
Recipes	Base de datos de recetas y tiempos
FreeRTOS	Gestión de tareas concurrentes
Main	Inicialización del sistema y tareas FreeRTOS
🧵 Tareas en FreeRTOS

El sistema usa dos tareas principales:

🟦 TaskBarbot

Prioridad: 1

Periodo: 50 ms

Ejecuta la FSM

Maneja servos, recetas y botones

Controla todo el flujo del BarBot

🟩 TaskHeartbeat

Prioridad: 0

Periodo: 500 ms

Parpadeo del LED

Indica que el sistema está vivo

El loop() queda vacío, FreeRTOS controla todo el flujo.

🧩 Máquina de estados (FSM)
Estados principales:

MENU: selección de recetas

SERVING: giro, posicionamiento y dispensado

DONE: espera confirmación para volver al menú

Cada estado ejecuta una parte lógica del sistema, manteniendo el código limpio y modular.

🔧 Control aplicado (Control I)
Modelo del servo

Aproximado como primer orden:

G(s) = K / (τs + 1)
K = 1
τ ≈ 0.15 s

Control utilizado

Control Proporcional (P):

u(t) = Kp (r - y)


Aplicado para movimientos suaves y estables.

📈 Avance actual del proyecto
Componente	Estado
Control de servos	✔️ Completo
Control del motor DC	✔️ Completo (open-loop calibrado)
FSM funcional	✔️ Completo
FreeRTOS	✔️ Completo
Organización modular	✔️ Completa
README	✔️ Actualizado
Documentación técnica	🔄 En progreso
Ensamblado mecánico	🔄 En progreso
Simulación y control	✔️ Aplicado (Control I)
📌 Conclusiones

El BarBot cumple con los principios fundamentales de un sistema embebido bien estructurado.

FreeRTOS permite separar el sistema en tareas estables y no bloqueantes.

La máquina de estados simplifica la lógica de operación y permite escalabilidad.

El sistema de control aplicado (servo/motor) refleja correctamente los contenidos teóricos de Control I.

El proyecto está en una etapa sólida para seguir avanzando y agregar características como WiFi, calibración automática, telemetría o más recetas.