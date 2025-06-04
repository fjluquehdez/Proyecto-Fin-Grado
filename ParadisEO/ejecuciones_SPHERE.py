import subprocess
import time
import csv
from codecarbon import EmissionsTracker
from itertools import product


population_sizes = [2**6, 2**10, 2**14] 
crossover_probs = [0.2, 0.01, 0.8]

# Variable global para el id (debe llegar a 90)
global_id = 1

# Bucle principal: para cada combinación de tamaño de población y probabilidad de cruce
for pop_size, crossover_prob in product(population_sizes, crossover_probs):
    print("\n=======================================")
    print(f"CONFIGURACIÓN: Población={pop_size}, Cruce={crossover_prob}")
    print("=======================================")
    
    # Ejecutar 10 iteraciones para cada configuración
    for _ in range(10):
        print(f"\nEsperando 5 segundos antes de la ejecución con ID {global_id}...")
        time.sleep(5)
        print(f"Ejecutando iteración con ID {global_id} con CodeCarbon...")
        
        # Iniciar el tracker de CodeCarbon
        tracker = EmissionsTracker(
            project_name="OneMax_Experiment",
            output_dir=".",
            log_level="critical",
            save_to_file=True,
            tracking_mode="process"
        )
        tracker.start()
        
        # Construir el comando para ejecutar el programa C++
        cmd = [
            "./sphere_sbx",
            "-p", str(pop_size),
            "-c", str(crossover_prob),
            "-i", str(global_id)
        ]
        subprocess.run(cmd, check=True)
        tracker.stop()
        
        print(f"Iteración con ID {global_id} completada.")
        global_id += 1

print("\n EXPERIMENTO COMPLETO")


