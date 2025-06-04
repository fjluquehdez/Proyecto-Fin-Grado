import ec.Evolve;

public class RunRosenbrock {
    public static void main(String[] args) {
        String[] ecjArgs = { "-file", "rosenbrock.params" };
        Evolve.main(ecjArgs);
    }
}
