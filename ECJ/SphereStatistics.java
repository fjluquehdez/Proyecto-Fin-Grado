import ec.*;
import ec.util.*;
import ec.vector.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.net.InetAddress;

public class SphereStatistics extends Statistics {
    private int generation;
    private double initialMaxFitness;
    private double maxFitnessReached;
    private int bestGeneration;
    private String stopReason = "MaxGenReached";
    private long startTime;
    private long executionTime;
    private double crossoverProb;
    private double mutationProb;

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

        if (generation == 0) {
            initialMaxFitness = best;
            maxFitnessReached  = best;
            bestGeneration     = 0;
        }

        // actualizar global
        if (best > maxFitnessReached) {
            maxFitnessReached = best;
            bestGeneration    = generation;
        }

        executionTime = System.currentTimeMillis() - startTime;

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

    @Override
    public void finalStatistics(final EvolutionState state, final int result) {
        super.finalStatistics(state, result);
        try {
            FileWriter writer = new FileWriter("sphere_resultados.csv", true);
            File file = new File("sphere_resultados.csv");
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
            double initialMaxFitnessProp = initialMaxFitness / 100.0;
            double maxFitnessReachedProp = maxFitnessReached / 100.0;
            double variationProp = variation / 100.0;
            
            writer.write(String.format(Locale.US,
                "%s,ECJ,%d,%d,%.4f,%.4f,%d,%.4f,%.4f,%.4f,%d,%s,%s\n",
                ts,
                SphereProblem.N,
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
