import ec.Evolve;
/**
 *
 * @author fjluquehdez
 */
public class RunOneMax {
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // Argumentos para ECJ: -file onemax.params
        String[] ecjArgs = {"-file", "onemax.params"};
        Evolve.main(ecjArgs);      
    }
}
