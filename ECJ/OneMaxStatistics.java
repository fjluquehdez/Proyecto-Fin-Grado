
import ec.*;
import ec.util.*;
import ec.vector.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.net.InetAddress;

public class OneMaxStatistics extends Statistics {

    private int generation;
    private double maxFitnessReached;
    private double initialAvgFitness;
    private String stopReason="Generaciones_máximas_alcanzadas";
    private long startTime;
    private long executionTime;
    private int bestGeneration;
    private double crossoverProb;
    private double mutationProb;

    @Override
    public void setup(final EvolutionState state, final Parameter base) {
        super.setup(state, base);
        startTime = System.currentTimeMillis();

        Parameter p_cross = new Parameter("pop.subpop.0.species.pipe.source.0.prob");
        crossoverProb = state.parameters.getDouble(p_cross, null, 0.0);

        Parameter p_mut = new Parameter("pop.subpop.0.species.mutation-prob");
        mutationProb = state.parameters.getDouble(p_mut, null, 0.0);
    }

    @Override
    public void postEvaluationStatistics(final EvolutionState state) {
        super.postEvaluationStatistics(state);

        generation = state.generation;
        double avgFitness = getAverageFitness(state);
        double bestFitness = getMaxFitness(state);

        if (generation == 0) {
            initialAvgFitness = avgFitness;
        }

        maxFitnessReached = bestFitness;
        executionTime = System.currentTimeMillis() - startTime;

        if (generation == 0 || bestFitness > maxFitnessReached) {
            bestGeneration = generation;
        }

        boolean shouldTerminate = false;

        if (bestFitness >= getMaxPossibleFitness(state)) {
            stopReason = "Fitness_máximo_alcanzado";
            shouldTerminate = true;
        } else if (state.evaluator instanceof TimeConstrainedEvaluator) {
            TimeConstrainedEvaluator evaluator = (TimeConstrainedEvaluator) state.evaluator;
            if (evaluator.timeLimitReached) {
                stopReason = "Tiempo_máximo_alcanzado";
                shouldTerminate = true;
            }
        } else if (generation >= state.numGenerations - 1) {
            stopReason = "Generaciones_máximas_alcanzadas";
        } else {
            stopReason = "En_ejecución";
        }

        if (shouldTerminate) {
            state.finish(EvolutionState.R_SUCCESS);
        }
    }

    @Override
    public void finalStatistics(final EvolutionState state, final int result) {
        super.finalStatistics(state, result);

        try {
            FileWriter writer = new FileWriter("onemax_resultados.csv", true);
            File file = new File("onemax_resultados.csv");
            boolean isNewFile = file.length() == 0;

            if (isNewFile) {
                writer.write("fecha_hora,framework,tamanio_individuo,poblacion,cruce,mutacion,generacion,fitness_inicial,variacion_fitness,fitness_final,tiempo_transcurrido,motivo_parada,ubicacion_ejecucion\n");
            }

            String fechaHora = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date());
            String hostname = InetAddress.getLocalHost().getHostName();

            writer.write(String.format(Locale.US, "%s,%s,%d,%d,%.4f,%.4f,%d,%.4f,%.4f,%.4f,%d,%s,%s\n",

                fechaHora,                                               // fecha y hora
                "ECJ",                                                   // framework
                1024,                                                    // tamaño del individuo
                state.population.subpops.get(0).individuals.size(), 	 // tamaño poblacion
                crossoverProb,                                           // probabilidad de cruce
                mutationProb,                                            // probabilidad de mutacion
                generation,                                              // generacion alcanzada
                initialAvgFitness,                                       // fitness inicial
                (maxFitnessReached - initialAvgFitness),                 // fitness variacion
                maxFitnessReached,                                       // fitness final
                executionTime,                                           // tiempo de ejecucion
                stopReason,                                              // razon de parada
                hostname                                                 // maquina donde se ha ejecutado
            ));

            writer.close();
        } catch (IOException e) {
            state.output.warning("No se pudo escribir el archivo CSV: " + e.getMessage());
        }

        System.exit(0);
    }

    private double getAverageFitness(final EvolutionState state) {
        double totalFitness = 0;
        int count = 0;
        for (Subpopulation subpop : state.population.subpops) {
            for (Individual ind : subpop.individuals) {
                if (ind.evaluated) {
                    totalFitness += ind.fitness.fitness();
                    count++;
                }
            }
        }
        return count > 0 ? totalFitness / count : 0;
    }

    private double getMaxFitness(final EvolutionState state) {
        double max = Double.NEGATIVE_INFINITY;
        for (Subpopulation subpop : state.population.subpops) {
            for (Individual ind : subpop.individuals) {
                if (ind.evaluated && ind.fitness.fitness() > max) {
                    max = ind.fitness.fitness();
                }
            }
        }
        return max;
    }

    private int getMaxPossibleFitness(final EvolutionState state) {
        Individual ind = state.population.subpops.get(0).individuals.get(0);
        if (ind instanceof BitVectorIndividual) {
            return ((BitVectorIndividual) ind).genome.length;
        }
        return Integer.MAX_VALUE;
    }
}
