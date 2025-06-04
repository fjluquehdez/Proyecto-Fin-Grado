import ec.*;
import ec.simple.*;
import ec.vector.*;

public class SphereProblem extends Problem implements SimpleProblemForm {
    public static final double LOW = -5.12;
    public static final double UP  =  5.12;
    public static final int    N   = 1024;
    public static final double F_MAX = N * UP * UP;  // raw máximo para escala

    @Override
    public void evaluate(final EvolutionState state,
                         final Individual ind,
                         final int subpopulation,
                         final int threadnum) {
        if (!(ind instanceof DoubleVectorIndividual))
            state.output.fatal("Individual no es DoubleVectorIndividual");
        DoubleVectorIndividual iv = (DoubleVectorIndividual)ind;

        // Suma de cuadrados
        double raw = 0.0;
        for (double x : iv.genome)
            raw += x*x;

        // Escalado a porcentaje
        double scaled = (1.0 - raw / F_MAX) * 100.0;

        if (!(iv.fitness instanceof SimpleFitness))
            state.output.fatal("La fitness no es SimpleFitness");
        ((SimpleFitness)iv.fitness).setFitness(state, scaled, raw==0.0);
        iv.evaluated = true;
    }
}

