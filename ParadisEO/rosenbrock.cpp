/**
 * @file rosenbrock_sbx.cpp
 * compilar: c++ rosenbrock.cpp -I../eo/src -std=c++17 -L./lib/ -leo -leoutils -o rosenbrock
 * ejecutar: ./rosenbrock -p <poblacion> -c <cruce> -m <mutacion> -i <id>
 */

#include <eo>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cfloat>
#include <cmath>

using namespace std;

// ----------------------------------------------------
// Configuración del problema
// ----------------------------------------------------
static constexpr double LOWER_BOUND = -5.12;
static constexpr double UPPER_BOUND = 5.12;
static constexpr size_t INDIVIDUAL_SIZE = 1024;
static constexpr double WORST_CASE_VALUE = 1e10;
static constexpr int MAX_TIME_SECONDS = 120;
static constexpr int MAX_GENERATIONS = 1000000;
static constexpr double F_MAX = INDIVIDUAL_SIZE * 40000.0;  
// ----------------------------------------------------
// Individuo: vector de reales con fitness a maximizar
struct Rosenbrock : public EO<eoMaximizingFitness>
{
    vector<double> x;
    
    double raw_value;
    
    Rosenbrock() : raw_value(DBL_MAX) {}
    
    Rosenbrock(size_t n) : x(n, 0.0), raw_value(DBL_MAX) {}
    
    void printOn(ostream &os) const override
    {
        for (double v : x)
            os << v << " ";
        os << " raw=" << raw_value;
    }
    
    void readFrom(istream &is) override
    {
        for (auto &v : x)
            is >> v;
    }
};

// ----------------------------------------------------
// Variables globales para estadísticas
// ----------------------------------------------------
struct GlobalStats {
    double initial_fitness = 0.0;
    double best_fitness = 0.0;
    size_t gen_best_fitness = 0;
    double best_raw_value = DBL_MAX;
    double worst_raw_value = 0.0;
    string termination_cause = "timeout";
} stats;

// ----------------------------------------------------
// Evaluador de Rosenbrock: normalizado entre 0 y 1 (para maximización)
struct RosenbrockFunction : public eoEvalFunc<Rosenbrock>
{
    void operator()(Rosenbrock &ind) override
    {
        // Calcular el valor real de Rosenbrock (a minimizar)
        double raw = 0.0;
        for (size_t i = 0; i < ind.x.size() - 1; ++i) {
            raw += 100.0 * pow(ind.x[i+1] - pow(ind.x[i], 2), 2) + 
                   pow(ind.x[i] - 1.0, 2);
        }
        
        // Limitar valores extremos para evitar problemas numéricos
        if (raw > WORST_CASE_VALUE) {
            raw = WORST_CASE_VALUE;
        }
        
        // Guardar el valor bruto para referencia
        ind.raw_value = raw;
        
        // Actualizar el peor valor visto (para normalización)
        if (raw > stats.worst_raw_value) {
            stats.worst_raw_value = raw;
        }
        
        // Actualizar mejor valor
        if (raw < stats.best_raw_value) {
            stats.best_raw_value = raw;
        }
        
        double normalized_fitness = (1.0 - (raw / F_MAX));
        
        // Asegurar que el fitness esté en el rango [0,1]
        normalized_fitness = max(0.0, min(1.0, normalized_fitness));
        
        ind.fitness(normalized_fitness);
    }
};

// ----------------------------------------------------
// Inicializador de vectores aleatorios 
struct RosenbrockInit : public eoInit<Rosenbrock>
{
    void operator()(Rosenbrock &ind) override
    {
        ind.x.resize(INDIVIDUAL_SIZE);
        
        // Distribución uniforme en todo el rango
	for (double &v : ind.x) {
		v = rng.uniform(LOWER_BOUND, UPPER_BOUND);
	}
    }
};

// ----------------------------------------------------
// SBX Crossover 
struct SafeSBXCrossover : public eoQuadOp<Rosenbrock>
{
    double eta;
    SafeSBXCrossover(double _eta) : eta(_eta) {}
    
    bool operator()(Rosenbrock &a, Rosenbrock &b) override
    {
        const size_t n = a.x.size();
        for (size_t i = 0; i < n; ++i)
        {
            try {
                // Evitar realizar cálculos que puedan causar desbordamiento
                if (abs(a.x[i] - b.x[i]) < 1e-10) {
                    continue;  // No cambiar genes casi idénticos
                }
                
                double y1, y2;
                if (a.x[i] < b.x[i]) {
                    y1 = a.x[i];
                    y2 = b.x[i];
                } else {
                    y1 = b.x[i];
                    y2 = a.x[i];
                }
                
                // Cálculos SBX con protección contra desbordamiento
                double beta = 1.0 + (2.0 * (y1 - LOWER_BOUND) / (y2 - y1));
                beta = min(beta, 100.0);  // Limitar beta para evitar problemas
                
                double alpha = 2.0 - min(100.0, pow(beta, eta + 1.0));
                
                double u = rng.uniform();
                double beta_q;
                
                if (u <= 1.0 / alpha) {
                    beta_q = pow(u * alpha, 1.0 / (eta + 1.0));
                } else {
                    beta_q = pow(1.0 / (2.0 - u * alpha), 1.0 / (eta + 1.0));
                }
                
                double c1 = 0.5 * ((y1 + y2) - beta_q * (y2 - y1));
                double c2 = 0.5 * ((y1 + y2) + beta_q * (y2 - y1));
                
                // Mantener dentro de límites
                c1 = max(LOWER_BOUND, min(UPPER_BOUND, c1));
                c2 = max(LOWER_BOUND, min(UPPER_BOUND, c2));
                
                // Asignar valores, respetando el orden original
                if (a.x[i] > b.x[i]) {
                    a.x[i] = c2;
                    b.x[i] = c1;
                } else {
                    a.x[i] = c1;
                    b.x[i] = c2;
                }
            } 
            catch (...) {
                // En caso de error, aplicar cruce aritmético simple
                double blend = rng.uniform();
                double tmp_a = a.x[i];
                double tmp_b = b.x[i];
                a.x[i] = blend * tmp_a + (1.0 - blend) * tmp_b;
                b.x[i] = (1.0 - blend) * tmp_a + blend * tmp_b;
                
                // Asegurar que estén dentro de límites
                a.x[i] = max(LOWER_BOUND, min(UPPER_BOUND, a.x[i]));
                b.x[i] = max(LOWER_BOUND, min(UPPER_BOUND, b.x[i]));
            }
        }
        return true;
    }
};

// ----------------------------------------------------
// Mutación estilo DEAP para valores reales
struct RealMutation : public eoMonOp<Rosenbrock>
{
    double p_ind, p_bit, sigma;
    
    RealMutation(double _p_ind, double _p_bit) 
        : p_ind(_p_ind), p_bit(_p_bit) 
    {
        // Factor de escala para la mutación gaussiana
        sigma = (UPPER_BOUND - LOWER_BOUND) * 0.1;
    }
    
    bool operator()(Rosenbrock &ind) override
    {
        bool mutated = false;
        
        // Paso 1: ¿se muta el individuo?
        if (rng.uniform() < p_ind)
        {
            // Paso 2: para cada gen, decide si mutar
            for (size_t i = 0; i < ind.x.size(); ++i)
            {
                if (rng.uniform() < p_bit)
                {
                    // Mutación gaussiana
                    double delta = rng.normal() * sigma;
                    ind.x[i] += delta;
                    
                    // Asegurar que se mantiene dentro de límites
                    ind.x[i] = max(LOWER_BOUND, min(UPPER_BOUND, ind.x[i]));
                    mutated = true;
                }
            }
        }
        return mutated;
    }
};

// ----------------------------------------------------
// Obtener fecha y hora actual formateada
string getCurrentDateTime()
{
    auto now = chrono::system_clock::now();
    auto nowTime = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&nowTime), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

// ----------------------------------------------------
// Función para obtener el nombre del host
string getHostName()
{
    char host[256];
    gethostname(host, sizeof(host));
    return string(host);
}

// ----------------------------------------------------
int main(int argc, char **argv)
{
    // Parámetros por defecto
    size_t popSize = 1024;
    double crossover_rate = 0.8;
    double mutation_ind_rate = 0.1;
    double mutation_bit_rate = 0.1;
    int run_id = 1;
    
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            popSize = stoul(argv[++i]);
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc)
            crossover_rate = stod(argv[++i]);
        else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc)
            mutation_ind_rate = stod(argv[++i]);
        else if (strcmp(argv[i], "-mb") == 0 && i + 1 < argc)
            mutation_bit_rate = stod(argv[++i]);
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
            run_id = atoi(argv[++i]);
    }
    
    // Inicialización de componentes
    RosenbrockInit init;
    RosenbrockFunction eval;
    SafeSBXCrossover xover(2.0);  
    RealMutation mutate(mutation_ind_rate, mutation_bit_rate);
    eoDetTournamentSelect<Rosenbrock> select(2);
    
    // Inicializar estadísticas
    stats = GlobalStats();
    
    // Nombre del archivo de resultados basado en el host
    string resultsFile = "resultados_rosenbrock_paradiseo_" + getHostName() + ".csv";
    
    // Verificar si el archivo existe para escribir la cabecera
    bool fileExists = ifstream(resultsFile).good();
    
    // Abrir archivo CSV para resultados
    ofstream csv(resultsFile, ios::app);
    if (!fileExists) {
        csv << "population_size,crossover_rate,mutation_individual_rate,"
            << "mutation_bit_rate,run,generations,initial_fitness,"
            << "best_fitness,fitness_variation,time,energy_consumed,"
            << "rosenbrock_value,worst_rosenbrock_seen,hostname\n";
    }
    
    // Población inicial
    eoPop<Rosenbrock> pop;
    pop.reserve(popSize);
    for (size_t i = 0; i < popSize; ++i)
    {
        Rosenbrock ind(INDIVIDUAL_SIZE);
        init(ind);
        eval(ind);
        pop.push_back(ind);
    }
    
    // Fitness inicial máximo
    double initMax = double(pop[0].fitness());
    for (auto &ind : pop)
        initMax = max(initMax, double(ind.fitness()));
    
    // Actualizar estadísticas iniciales
    stats.initial_fitness = initMax;
    stats.best_fitness = initMax;
    
    // Fecha y hora actual para el registro
    string dateTime = getCurrentDateTime();
    
    // Mostrar información de inicio
    cout << "Ejecutando: Población=" << popSize << ", Cruce=" << crossover_rate 
              << ", MutInd=" << mutation_ind_rate << ", MutBit=" << mutation_bit_rate
              << ", Run=" << run_id << endl;
    
    // Bucle principal
    auto t0 = chrono::steady_clock::now();
    string stop = "timeout";
    size_t gen = 0;
    
    while (true)
    {
        auto dt = chrono::duration_cast<chrono::seconds>(
                      chrono::steady_clock::now() - t0)
                      .count();
        
        // Comprobar condiciones de finalización
        if (dt >= MAX_TIME_SECONDS)
        {
            stats.termination_cause = "timeout";
            break;
        }
        if (gen >= MAX_GENERATIONS)
        {
            stats.termination_cause = "max_generations";
            break;
        }
        
        // Actualizar contador de generaciones
        ++gen;
        
        // Para primeras generaciones o cada 50, mostrar información
        if (gen < 3 || gen % 50 == 0) {
            cout << "  Gen " << gen << ": fitness=" << stats.best_fitness
                      << ", raw=" << stats.best_raw_value 
                      << ", worst_seen=" << stats.worst_raw_value << endl;
        }
        
        // Crear nueva generación
        eoPop<Rosenbrock> offspring;
        offspring.reserve(popSize);
        
        while (offspring.size() < popSize)
        {
            Rosenbrock p1 = select(pop);
            Rosenbrock p2 = select(pop);
            
            if (rng.uniform() < crossover_rate)
                xover(p1, p2);
                
            mutate(p1);
            mutate(p2);
            
            eval(p1);
            eval(p2);
            
            offspring.push_back(p1);
            if (offspring.size() < popSize)
                offspring.push_back(p2);
        }
        
        pop = offspring;
        
        // Actualizar estadísticas
        for (auto &ind : pop)
        {
            double f = double(ind.fitness());
            if (f > stats.best_fitness)
            {
                stats.best_fitness = f;
                stats.gen_best_fitness = gen;
                
                // Si llegamos a convergencia perfecta
                if (stats.best_raw_value < 1e-10)
                {
                    stats.termination_cause = "convergence";
                    break;
                }
            }
        }
        
        if (stats.termination_cause == "convergence")
            break;
    }

    auto t1 = chrono::steady_clock::now();
    double timeSec = chrono::duration_cast<chrono::seconds>(t1 - t0).count();
    
    double fitness_variation = stats.best_fitness - stats.initial_fitness;
    
    double emissions = 0.0;
    
    // Mostrar resultados finales
    cout << "Generaciones: " << gen + 1 << " | "
              << "Aptitud inicial=" << stats.initial_fitness << " → "
              << "mejor=" << stats.best_fitness << " | "
              << "Δ=" << fitness_variation << endl;
    cout << "Valor Rosenbrock: mejor=" << stats.best_raw_value << ", "
              << "peor visto=" << stats.worst_raw_value << endl;
    cout << "Tiempo: " << timeSec << "s | "
              << "Parada: " << stats.termination_cause << endl;
    
    // Escribir resultados en CSV
    csv << popSize << ","                  // population_size
        << crossover_rate << ","           // crossover_rate
        << mutation_ind_rate << ","        // mutation_individual_rate
        << mutation_bit_rate << ","        // mutation_bit_rate
        << run_id << ","                   // run
        << gen + 1 << ","                  // generations
        << stats.initial_fitness/100 << ","    // initial_fitness
        << stats.best_fitness/100 << ","       // best_fitness
        << fitness_variation/100 << ","        // fitness_variation
        << timeSec << ","                  // time
        << emissions << ","                // energy_consumed (no disponible)
        << stats.best_raw_value << ","     // rosenbrock_value
        << stats.worst_raw_value << ","    // worst_rosenbrock_seen
        << getHostName() << "\n";          // hostname
        
    csv.close();
    
    return 0;
}
