import ec.*;
import ec.util.*;
import ec.vector.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.net.InetAddress;

public class RosenbrockStatistics extends Statistics {
    private int generation;
    private double initialMaxFitness;
    private double maxFitnessReached;
    private int bestGeneration;
    private String stopReason = "MaxGenReached";
    private long startTime;
    private long executionTime;
    private double crossoverProb;
    private double mutationProb;
    private boolean initialEvaluationDone = false;

    @Override
    public void setup(final EvolutionState state, final Parameter base) {
        super.setup(state, base);
        startTime = System.currentTimeMillis();
        // leer parámetros de probabilidad
        crossoverProb = state.parameters.getDouble(
            new Parameter("pop.subpop.0.species.pipe.source.0.prob"), null, 0.0);
        mutationProb = state.parameters.getDouble(
            new Parameter("pop.subpop.0.species.mutation-prob"), null, 0.0);
    }

    @Override
    public void postEvaluationStatistics(final EvolutionState state) {
        super.postEvaluationStatistics(state);
        generation = state.generation;
        double best = getMaxFitness(state);

        // Capturar el fitness inicial después de la primera evaluación
        // Solo lo hacemos una vez, en la generación 0
        if (generation == 0 && !initialEvaluationDone) {
            initialMaxFitness = best;
            maxFitnessReached = initialMaxFitness;
            bestGeneration = 0;
            initialEvaluationDone = true;
            
            // Log para verificar
            state.output.message("POBLACIÓN INICIAL - Fitness máximo (0-100): " + 
                               String.format("%.6f", initialMaxFitness) + 
                               " - (0-1): " + String.format("%.8f", initialMaxFitness/100.0));
        }

        // Solo actualizamos el mejor global si es mejor que lo que ya tenemos
        if (best > maxFitnessReached) {
            maxFitnessReached = best;
            bestGeneration    = generation;
        }

        executionTime = (System.currentTimeMillis() - startTime)/1000;

        boolean terminate = false;
        // convergencia
        if (best >= getMaxPossibleFitness(state)) {
            stopReason = "Convergence";
            terminate  = true;
        }
        // timeout
        else if (state.evaluator instanceof TimeConstrainedEvaluator) {
            if (((TimeConstrainedEvaluator)state.evaluator).timeLimitReached) {
                stopReason = "Timeout";
                terminate  = true;
            }
        }
        // tope de generaciones (si lo hubiera)
        else if (generation >= state.numGenerations - 1) {
            stopReason = "MaxGenReached";
        }

        if (terminate) 
            state.finish(EvolutionState.R_SUCCESS);
    }
    
    // No usamos este método ya que no existe en la versión de ECJ que estás usando
    // public void postInitialEvaluationStatistics(final EvolutionState state) { ... }

    @Override
    public void finalStatistics(final EvolutionState state, final int result) {
        super.finalStatistics(state, result);
        try {
            FileWriter writer = new FileWriter("rosenbrock_resultados.csv", true);
            File file = new File("rosenbrock_resultados.csv");
            boolean isNew = file.length() == 0;
            if (isNew) {
                writer.write("fecha_hora,framework,tamanio_individuo,poblacion,cruce,mutacion,"
                           + "generacion,fitness_inicial,variacion_fitness,fitness_maximo,"
                           + "tiempo_transcurrido,motivo_parada,ubicacion_ejecucion\n");
            }
            String ts = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss")
                            .format(new Date());
            String host = InetAddress.getLocalHost().getHostName();
            double variation = maxFitnessReached - initialMaxFitness;
            
            // Convertir fitness de porcentaje (0-100) a tanto por 1 (0-1)
            // Asegurarse de que los valores estén en el rango [0, 1]
            double initialMaxFitnessProp = Math.max(0.0, Math.min(1.0, initialMaxFitness / 100.0));
            double maxFitnessReachedProp = Math.max(0.0, Math.min(1.0, maxFitnessReached / 100.0));
            double variationProp = Math.max(0.0, Math.min(1.0, variation / 100.0));
            
            // Imprimir los valores antes de guardarlos para verificar
            state.output.message("======= RESULTADOS FINALES =======");
            state.output.message("Generaciones ejecutadas: " + generation);
            state.output.message("Fitness inicial (porcentaje): " + String.format("%.4f", initialMaxFitness));
            state.output.message("Fitness máximo (porcentaje): " + String.format("%.4f", maxFitnessReached));
            state.output.message("Fitness inicial (0-1): " + String.format("%.6f", initialMaxFitnessProp));
            state.output.message("Fitness máximo (0-1): " + String.format("%.6f", maxFitnessReachedProp));
            state.output.message("Tiempo de ejecución (ms): " + executionTime);
            state.output.message("Motivo de parada: " + stopReason);
            
            writer.write(String.format(Locale.US,
                "%s,ECJ,%d,%d,%.6f,%.6f,%d,%.6f,%.6f,%.6f,%d,%s,%s\n",
                ts,
                RosenbrockProblem.N,
                state.population.subpops.get(0).individuals.size(),
                crossoverProb, mutationProb,
                generation, initialMaxFitnessProp,
                variationProp, maxFitnessReachedProp,
                executionTime, stopReason, host
            ));
            writer.close();
        } catch (IOException e) {
            state.output.warning("No se pudo escribir CSV: " + e);
        }
        System.exit(0);
    }

    private double getMaxFitness(final EvolutionState state) {
        double m = Double.NEGATIVE_INFINITY;
        for (Subpopulation sp: state.population.subpops)
            for (Individual ind: sp.individuals)
                if (ind.evaluated)
                    m = Math.max(m, ind.fitness.fitness());
        return m;
    }

    private double getMaxPossibleFitness(final EvolutionState state) {
        return 100.0;  // escala al 100%
    }
}
