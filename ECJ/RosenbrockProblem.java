import ec.*;
import ec.simple.*;
import ec.vector.*;

/**
 * Implementación del problema de Rosenbrock para optimización mediante ECJ.
 * La función de Rosenbrock, también conocida como función del valle de plátano, es una función
 * no convexa utilizada como problema de prueba para algoritmos de optimización.
 * 
 * El óptimo global es un vector de unos (1.0, 1.0, ..., 1.0) con un valor de función de 0.
 */
public class RosenbrockProblem extends Problem implements SimpleProblemForm {
    // Rango típico para Rosenbrock es [-2.048, 2.048], pero mantendremos [-5.12, 5.12] para
    // consistencia con el problema anterior, aunque ajustaremos el F_MAX
    public static final double LOW = -5.12;
    public static final double UP  =  5.12;
    public static final int    N   = 1024;
    
    // Para Rosenbrock, necesitamos un F_MAX más apropiado basado en análisis estadístico
    // Para un individuo de tamaño N, el peor caso puede ser alrededor de N * 40000
    public static final double F_MAX = N * 40000;  
    
    @Override
    public void evaluate(final EvolutionState state,
                         final Individual ind,
                         final int subpopulation,
                         final int threadnum) {
        if (!(ind instanceof DoubleVectorIndividual))
            state.output.fatal("Individual no es DoubleVectorIndividual");
        DoubleVectorIndividual iv = (DoubleVectorIndividual)ind;

        // Cálculo de la función de Rosenbrock: f(x) = Σ[100(x_{i+1} - x_i²)² + (1 - x_i)²]
        double raw = 0.0;
        for (int i = 0; i < iv.genome.length - 1; i++) {
            double term1 = 100 * Math.pow(iv.genome[i+1] - Math.pow(iv.genome[i], 2), 2);
            double term2 = Math.pow(1 - iv.genome[i], 2);
            raw += (term1 + term2);
        }
        
        // Verificar si la evaluación de raw está dando valores válidos
        if (Double.isNaN(raw) || Double.isInfinite(raw)) {
            state.output.warning("¡Valor de fitness no válido detectado! Ajustando a F_MAX.");
            raw = F_MAX;
        }
        
        // Limitar raw a F_MAX para evitar valores negativos en el fitness
        raw = Math.min(raw, F_MAX);
        
        // Escalado a porcentaje (invertido porque queremos maximizar fitness)
        // El valor óptimo de Rosenbrock es 0, por lo que 0 debe mapear a 100%
        double scaled = 0.0;
        if (raw <= F_MAX) {
            scaled = (1.0 - raw / F_MAX) * 100.0;
        }
        
        // Aseguramos que el fitness esté en el rango [0, 100]
        scaled = Math.max(0.0, Math.min(100.0, scaled));
        
        // Imprimir valores para debugging cada 100 generaciones o para la primera generación
        if ((state.generation == 0) && 
            iv.equals(state.population.subpops.get(0).individuals.get(0))) {
            state.output.message("Gen " + state.generation + 
                              " - Raw value: " + String.format("%.4f", raw) + 
                              " - Scaled fitness (0-100): " + String.format("%.6f", scaled) + 
                              " - Scaled (0-1): " + String.format("%.8f", scaled/100.0));
        }

        if (!(iv.fitness instanceof SimpleFitness))
            state.output.fatal("La fitness no es SimpleFitness");
        ((SimpleFitness)iv.fitness).setFitness(state, scaled, raw==0.0);
        iv.evaluated = true;
    }
}
