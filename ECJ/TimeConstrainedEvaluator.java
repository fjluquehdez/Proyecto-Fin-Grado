import ec.*;
import ec.simple.*;
import ec.util.*;

public class TimeConstrainedEvaluator extends SimpleEvaluator {
    private static final long serialVersionUID = 1L;

    public static final long MAX_TIME = 120000; // 2 minutos
    private long startTime;
    public boolean timeLimitReached = false;

    @Override
    public void setup(final EvolutionState state, final Parameter base) {
        super.setup(state, base);
        startTime = System.currentTimeMillis();
    }

    @Override
    public void evaluatePopulation(final EvolutionState state) {
        if (timeLimitReached) return;
        super.evaluatePopulation(state);
        long currentTime = System.currentTimeMillis();
        if (currentTime - startTime > MAX_TIME) {
            timeLimitReached = true;
        }
    }
}
