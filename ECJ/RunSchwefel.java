import ec.Evolve;

public class RunSchwefel {
    public static void main(String[] args) {
        String[] ecjArgs = { "-file", "schwefel.params" };
        Evolve.main(ecjArgs);
    }
}
