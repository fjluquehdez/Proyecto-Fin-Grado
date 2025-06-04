import ec.*;
import ec.simple.*;
import ec.vector.*;

/**
 * Implementación del problema de Schwefel para optimización mediante ECJ.
 * La función de Schwefel es una función multimodal utilizada como problema de prueba 
 * para algoritmos de optimización.
 * 
 * El óptimo global es x_i = 420.9687 para todas las dimensiones con un valor de función de 0.
 */
public class SchwefelProblem extends Problem implements SimpleProblemForm {
    // El rango recomendado para Schwefel es [-500, 500]
    public static final double LOW = -500.0;
    public static final double UP  =  500.0;
    public static final int    N   = 1024;
    
    // Para Schwefel, F_MAX es aproximadamente 418.9829 * N + N * UP en el peor caso
    public static final double F_MAX = 418.9829 * N + N * UP;
    
    @Override
    public void evaluate(final EvolutionState state,
                         final Individual ind,
                         final int subpopulation,
                         final int threadnum) {
        if (!(ind instanceof DoubleVectorIndividual))
            state.output.fatal("Individual no es DoubleVectorIndividual");
        DoubleVectorIndividual iv = (DoubleVectorIndividual)ind;

        // Cálculo de la función de Schwefel: f(x) = 418.9829 * n - Σ[x_i * sin(sqrt(|x_i|))]
        double raw = 418.9829 * iv.genome.length;
        for (int i = 0; i < iv.genome.length; i++) {
            raw -= iv.genome[i] * Math.sin(Math.sqrt(Math.abs(iv.genome[i])));
        }
        
        // Verificar si la evaluación de raw está dando valores válidos
        if (Double.isNaN(raw) || Double.isInfinite(raw)) {
            state.output.warning("¡Valor de fitness no válido detectado! Ajustando a F_MAX.");
            raw = F_MAX;
        }
        
        // Limitar raw a F_MAX para evitar valores negativos en el fitness
        raw = Math.min(raw, F_MAX);
        
        // Escalado a porcentaje (invertido porque queremos maximizar fitness)
        // El valor óptimo de Schwefel es 0, por lo que 0 debe mapear a 100%
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
