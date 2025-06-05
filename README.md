# Proyecto-Fin-Grado
# Comparativa de Consumo Energético y Calidad de Soluciones en Algoritmos Evolutivos

**Proyecto Fin de Grado - Doble Grado en Ingeniería Informática y Administración y Dirección de Empresas**

**Autor:** Francisco Javier Luque Hernández  
**Directores:** Pablo García Sánchez, Sergio Iván Aquino Britez  
**Universidad:** Universidad de Granada  
**Año:** 2025

## 📋 Descripción

Este proyecto evalúa la **eficiencia energética** de cuatro frameworks populares de algoritmos evolutivos desde una perspectiva medioambiental, comparando su rendimiento en dos arquitecturas diferentes: un portátil (Intel i7-7500U) y un servidor (Intel i9-12900KF).

### 🎯 Objetivo Principal

Cuantificar la relación entre potencia computacional y eficiencia energética en algoritmos evolutivos, introduciendo la métrica unificada **η = fitness/kWh** para evaluar la rentabilidad energética.

## 🔬 Metodología Experimental

### Frameworks Evaluados
- **ParadisEO** (C++)
- **ECJ** (Java) 
- **DEAP** (Python)
- **Inspyred** (Python)

### Benchmarks Utilizados
- **OneMax** - Problema binario unimodal
- **Sphere** - Optimización continua separable
- **Rosenbrock** - Valle curvado no separable  
- **Schwefel** - Función multimodal

### Configuraciones Experimentales
- **Tamaños de población:** 2⁶, 2¹⁰, 2¹⁴ individuos
- **Probabilidades de cruce:** 0,01; 0,20; 0,80
- **Réplicas:** 10 ejecuciones por configuración
- **Tiempo límite:** 2 minutos por ejecución
- **Total:** 2.880 experimentos (96 horas de ejecución)

### Arquitecturas de Hardware
| Máquina | Procesador | Núcleos/Hilos | TDP | RAM |
|---------|------------|---------------|-----|-----|
| **Portátil** | Intel i7-7500U | 2/4 | 15W | 7,7 GiB |
| **Servidor** | Intel i9-12900KF | 16/24 | 241W | 125 GiB |

## 📊 Hallazgos Principales

### 🔋 Eficiencia Energética
- **El portátil es 5-7 veces más eficiente** que el servidor según la métrica η
- **Punto óptimo:** N = 2¹⁰ maximiza la eficiencia en todos los frameworks
- **Factor de escalado:** El servidor consume 4-7x más energía pero solo acelera 2,5x las generaciones

### 🏆 Ranking de Frameworks
1. **ParadisEO** - Mejor equilibrio eficiencia/estabilidad
2. **Inspyred** - Excelente balance facilidad/rendimiento  
3. **DEAP** - Eficiente en portátil, penalizado en servidor
4. **ECJ** - Máxima velocidad pero menor eficiencia energética

### 📈 Métricas Clave
- **Consumo energético:** Medido con contadores RAPL + CodeCarbon
- **Emisiones CO₂:** Conversión automática con factores regionales
- **Calidad de solución:** Fitness inicial, variación, máximo alcanzado
- **Eficiencia:** η = fitness/kWh (métrica unificada propuesta)

## 📁 Estructura del Repositorio

```
📂 DATOS_RECABADOS/
├── 📂 BBDD_ONEMAX/         # Datos experimentales OneMax
├── 📂 BBDD_SPHERE/         # Datos experimentales Sphere  
├── 📂 BBDD_ROSENBROCK/     # Datos experimentales Rosenbrock
└── 📂 BBDD_SCHWEFEL/       # Datos experimentales Schwefel

📂 DEAP/                    # Implementaciones Python DEAP
├── 📓 DEAP_sphere.ipynb
├── 📓 ejecuciones_DEAP_PYTHON_ROSENBROCK_SCHWEFEL.ipynb
└── 📓 OneMax_DEAP.ipynb

📂 ECJ/                     # Implementaciones Java ECJ
├── 📓 ejecuciones_y_guardado_datos_*.ipynb
├── ⚙️ *.params            # Archivos de configuración
├── ☕ *.java              # Clases Java
└── ☕ TimeConstrainedEvaluator.java

📂 INSPYRED/               # Implementaciones Python Inspyred
├── 📓 ejecuciones_INSPYRED_*.ipynb
├── 📓 inspyred_ONEMAX.ipynb
└── 📓 INSPYRED_Sphere.ipynb

📂 ParadisEO/              # Implementaciones C++ ParadisEO
├── 📓 ejecuciones_*.ipynb
├── 🔧 onemax.cpp
├── 🔧 rosenbrock.cpp
├── 🔧 schwefel.cpp
└── 🔧 sphere_sbx.cpp
```

## 🛠️ Tecnologías y Herramientas

### Medición Energética
- **CodeCarbon** - Medición de consumo y emisiones CO₂
- **Jupyter Notebooks** - Análisis y visualización

### Frameworks de Algoritmos Evolutivos
- **ParadisEO 2.x** - Framework C++ de alto rendimiento
- **ECJ 27** - Toolkit Java para computación evolutiva  
- **DEAP 1.3** - Biblioteca Python para algoritmos evolutivos
- **Inspyred 1.0** - Framework Python minimalista

### Operadores Genéticos Implementados
- **Selección:** Torneo binario
- **Cruce:** SBX (Simulated Binary Crossover) para problemas continuos
- **Mutación:** Bit-flip (OneMax), Polinómica (problemas continuos)
- **Reemplazo:** Generacional puro

## 🚀 Instalación y Uso

### Prerrequisitos
```bash
# Python 3.8+
pip install deap inspyred codecarbon numpy pandas matplotlib

# Java 11+ (para ECJ)
# C++ compiler con soporte C++17 (para ParadisEO)
```

### Ejecución de Experimentos

#### DEAP/Inspyred (Python)
```bash
cd DEAP/
jupyter notebook OneMax_DEAP.ipynb
```

#### ECJ (Java)
```bash
cd ECJ/
# Compilar
javac -cp ecj.jar:. *.java
# Ejecutar
java -cp ecj.jar:. RunOneMax
```

#### ParadisEO (C++)
```bash
cd ParadisEO/
g++ -O3 -std=c++17 onemax.cpp -o onemax -lparadiseo
./onemax -p 1024 -c 0.8 -m 0.1
```

## 📈 Reproducción de Resultados

### Análisis de Datos
Los notebooks de Jupyter en cada directorio permiten:
- **Cargar datos experimentales** desde archivos Excel
- **Calcular métricas** de eficiencia energética (η)
- **Generar gráficas** comparativas entre frameworks
- **Exportar resultados** en formatos estándar

### Datos Disponibles
Cada directorio `BBDD_*` contiene:
- **codecarbon_*.xlsx** - Métricas energéticas y emisiones
- **resultados_*.xlsx** - Fitness, generaciones, estadísticas evolutivas
- **estudio_consumo_*.ipynb** - Análisis comparativo completo


## 📚 Publicaciones y Referencias

Ver [memoria completa del PFG](MEMORIA_PFG_JAVIER_LUQUE_2025.pdf) para:
- **Revisión exhaustiva** del estado del arte
- **Metodología detallada** y justificación experimental
- **Análisis estadístico completo** de los 2.880 experimentos
- **Discusión** con literatura previa sobre green computing

---

⭐ **Si este proyecto te resulta útil para tu investigación, ¡no olvides darle una estrella y citarlo!**

📧 **Contacto:** fjluquehdez@correo.ugr.es
