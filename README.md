# Proyecto DAA – Extracción Iterativa de Conos Invertidos Positivos en un Modelo de Bloques 3D

**Entrega 2 – Diseño algorítmico, análisis y validación preliminar**

## Integrantes

- Camilo Jofré Tapia
- Jhoan Montero Huarachi
- Sebastián Hernández Lara

**Profesor:** Aldo Quelopana Retamal

---

# Estructura del proyecto

```text
Grupo_Entrega2.docx          Informe de la Entrega 2

src/
├── common.hpp               Estructuras principales, carga de instancias y cálculo de precedencias
├── baseline.cpp             Implementación de la línea base (Entrega 1)
├── improved.cpp             Implementación del algoritmo Floating Cone Incremental
├── generate_instance.cpp    Generador de instancias sintéticas
└── validate.cpp             Validador independiente

data/
├── I1_3x3x2_seed42.csv
├── I2_5x5x5_seed123.csv
├── I3_10x10x10_seed2026.csv
├── I4_15x15x15_seed2026.csv
└── casos/                   Casos manuales de prueba

results/
├── benchmark.csv            Resultados experimentales
├── escalabilidad.csv        Datos del análisis de escalabilidad
└── escalabilidad.png        Gráfico de tiempos
```

---

# Compilación

Se requiere un compilador **g++** con soporte para **C++17**.

```bash
cd src

g++ -O2 -std=c++17 -o generate_instance generate_instance.cpp
g++ -O2 -std=c++17 -o baseline baseline.cpp
g++ -O2 -std=c++17 -o improved improved.cpp
g++ -O2 -std=c++17 -DNO_MAIN -o validate validate.cpp baseline.cpp improved.cpp
```

---

# Ejecución

## Generar una instancia sintética

```bash
./generate_instance <dx> <dy> <dz> <seed> <salida.csv>
```

Ejemplo:

```bash
./generate_instance 10 10 10 2026 ../data/mi_instancia.csv
```

## Ejecutar la línea base

```bash
./baseline ../data/mi_instancia.csv
```

## Ejecutar el algoritmo mejorado

```bash
./improved ../data/mi_instancia.csv
```

## Validar una solución

```bash
./validate ../data/mi_instancia.csv
```

El validador ejecuta ambos algoritmos y comprueba que las soluciones obtenidas respeten las restricciones de precedencia del problema.

---

# Formato de entrada

Durante el desarrollo del proyecto se utilizaron dos formatos de entrada.

## Formato definido inicialmente en el enunciado

```text
x,y,z,v
```

donde `v` corresponde al valor económico del bloque.

## Formato definido posteriormente por el profesor

```text
x,y,z,Tonelada,Metal_1,Metal_2,Roca
```

En este caso, el valor económico del bloque se calcula automáticamente utilizando los parámetros económicos establecidos en el enunciado del proyecto.

---

# Notas

- Todas las instancias sintéticas se generan utilizando una semilla fija para garantizar la reproducibilidad de los experimentos.
- El código fue desarrollado por el grupo. Se utilizó inteligencia artificial como herramienta de apoyo para consultas puntuales, depuración y mejora de la documentación, manteniendo siempre la revisión y comprensión de las soluciones implementadas.