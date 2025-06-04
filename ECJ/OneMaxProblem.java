import ec.*;
import ec.simple.*;
import ec.vector.*;

public class OneMaxProblem extends Problem implements SimpleProblemForm {
    @Override
    public void evaluate(final EvolutionState state, final Individual ind, final int subpopulation, final int threadnum) {
        if (!(ind instanceof BitVectorIndividual))
            state.output.fatal("El individuo no es de tipo BitVectorIndividual");

        BitVectorIndividual individual = (BitVectorIndividual) ind;
        int sum = 0;
        for (boolean gene : individual.genome) {
            if (gene) sum++;
        }

        if (!(individual.fitness instanceof SimpleFitness))
            state.output.fatal("La fitness no es de tipo SimpleFitness");

        ((SimpleFitness) individual.fitness).setFitness(state, sum, sum == individual.genome.length);
        individual.evaluated = true;
    }

    @Override
    public void describe(final EvolutionState state, final Individual ind, final int subpopulation, final int threadnum, final int log) {
        state.output.println("Individuo: " + ind.toString(), log);
    }
}
