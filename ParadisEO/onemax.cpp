/**
 * @file onemax.cpp
 * @author fjluque
 * @brief compilar con > c++ onemax.cpp -I../eo/src -I../edo/src -std=c++17 -L./lib/ -leo -leoutils -o onemax
 * @brief ejecutar con ./onemax -p <tamanio_poblacion> -c <probabilidad_cruce> -i <id>
 * @version 0.1
 * @date 2025-04-03
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <eo>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <unistd.h> // Para gethostname
#include <string>
#include <cstdlib>
#include <fstream> // Para guardar el CSV

//-----------------------------------------------------
// Definición del individuo para OneMax (cadena binaria)
// Hereda de EO con fitness de maximización.
class OneMax : public EO<eoMaximizingFitness>
{
public:
    std::vector<bool> bits{0};

    OneMax() {}
    OneMax(size_t n) : bits(n, false) {}

    // Imprime la cadena (sin espacios)
    void printOn(std::ostream &os) const override
    {
        for (bool b : bits)
        {
            os << b;
        }
    }

    // Lectura del individuo (usando índices para vector<bool>)
    void readFrom(std::istream &is) override
    {
        for (size_t i = 0; i < bits.size(); ++i)
        {
            char c;
            is >> c;
            bits[i] = (c == '1');
        }
    }
};

//-----------------------------------------------------
// Función de evaluación: cuenta el número de 1
class OneMaxEval : public eoEvalFunc<OneMax>
{
public:
    void operator()(OneMax &ind) override
    {
        int count = 0;
        for (bool b : ind.bits)
        {
            if (b)
                count++;
        }
        // Se asigna el fitness (objetivo: maximizar el número de 1)
        ind.fitness(count);
    }
};

//-----------------------------------------------------
// Operador de inicialización: genera una cadena aleatoria
class OneMaxInit : public eoInit<OneMax>
{
public:
    OneMaxInit(size_t n) : n(n) {}

    void operator()(OneMax &ind) override
    {
        ind.bits.resize(n);
        for (size_t i = 0; i < n; ++i)
        {
            // Cada bit se inicializa a true con probabilidad 0.5
            ind.bits[i] = (eo::rng.uniform() < 0.5);
        }
    }

private:
    size_t n;
};

//-----------------------------------------------------
// Operador de cruce de un punto para cadenas binarias
class OnePointCrossover : public eoQuadOp<OneMax>
{
public:
    bool operator()(OneMax &parent1, OneMax &parent2) override
    {
        if (parent1.bits.size() != parent2.bits.size())
            return false;
        size_t n = parent1.bits.size();
        // Se elige un punto de cruce aleatorio
        size_t point = eo::rng.random(n);
        for (size_t i = point; i < n; ++i)
        {
            std::swap(parent1.bits[i], parent2.bits[i]);
        }
        return true;
    }
};

//-----------------------------------------------------
// Operador de mutación: flip de bits con una probabilidad dada
class BitFlipMutation : public eoMonOp<OneMax>
{
public:
    BitFlipMutation(double mutationRate) : mutationRate(mutationRate) {}

    bool operator()(OneMax &ind) override
    {
        bool mutated = false;
        for (size_t i = 0; i < ind.bits.size(); ++i)
        {
            if (eo::rng.uniform() < mutationRate)
            {
                ind.bits[i] = !ind.bits[i];
                mutated = true;
            }
        }
        return mutated;
    }

private:
    double mutationRate;
};

//-----------------------------------------------------
// Función auxiliar para parsear argumentos desde argv
void parseArgs(int argc, char **argv, size_t &popSize, double &pc, int &id)
{
    // Valores por defecto
    popSize = 50;
    pc = 0.7;
    id = 1;
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-p" && i + 1 < argc)
        {
            popSize = std::stoul(argv[++i]);
        }
        else if (arg == "-c" && i + 1 < argc)
        {
            pc = std::stod(argv[++i]);
        }
        else if (arg == "-i" && i + 1 < argc)
        {
            id = std::stod(argv[++i]);
        }
    }
}

//-----------------------------------------------------
int main(int argc, char **argv)
{
    // Parámetros leídos desde argv
    size_t popSize;
    double pc;
    int id;
    parseArgs(argc, argv, popSize, pc, id);

    const size_t nbits = 1024;             // Longitud de la cadena binaria (fitness máximo = nbits)
    const double pm = 0.01;                // Probabilidad de mutación (fija)
    const size_t nGenerationsMax = 100000; // Límite de generaciones

    // Condiciones de parada: fitness == nbits (100%) o timeout de 2 minutos (120 segundos)
    const int timeout_seconds = 120;

    // Datos para la salida final
    std::string stop_reason = "";
    size_t generation_stop = 0;
    int fitness_initial = 0;
    int best_fitness = 0;
    size_t generation_max_fitness = 0;

    // Obtener fecha y hora de inicio
    time_t now_time = time(nullptr);
    char time_str[100];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now_time));
    std::string fecha_hora = time_str;
    std::string framework = "Paradiseo";

    // Obtener el hostname (dónde se ha ejecutado)
    char hostname[1024];
    gethostname(hostname, sizeof(hostname));
    std::string ejecutado_en = hostname;

    // Inicializar la semilla del generador de números aleatorios
    eo::rng.reseed(now_time);

    // Inicialización de la población
    OneMaxInit initializer(nbits);
    eoPop<OneMax> pop;
    for (size_t i = 0; i < popSize; i++)
    {
        OneMax ind(nbits);
        initializer(ind);
        pop.push_back(ind);
    }

    // Evaluar la población inicial
    OneMaxEval eval;
    for (auto &ind : pop)
    {
        eval(ind);
    }

    // Registrar el fitness inicial
    auto bestIt = std::max_element(pop.begin(), pop.end(), [](const OneMax &a, const OneMax &b)
                                   { return a.fitness() < b.fitness(); });
    fitness_initial = bestIt->fitness();
    best_fitness = fitness_initial;
    generation_max_fitness = 0;

    // Iniciar el cronómetro
    auto start = std::chrono::steady_clock::now();

    // Operadores genéticos
    OnePointCrossover crossover;
    BitFlipMutation mutation(pm);
    eoDetTournamentSelect<OneMax> select(2);

    // Bucle principal del algoritmo genético
    size_t gen;
    bool sig = true;
    for (gen = 1; gen <= nGenerationsMax && sig; gen++)
    {
        eoPop<OneMax> newPop;
        while (newPop.size() < popSize)
        {
            // Seleccionar dos padres
            OneMax parent1 = select(pop);
            OneMax parent2 = select(pop);

            // Aplicar cruce con probabilidad pc
            if (eo::rng.uniform() < pc)
            {
                crossover(parent1, parent2);
            }
            // Aplicar mutación
            mutation(parent1);
            mutation(parent2);

            // Evaluar los descendientes
            eval(parent1);
            eval(parent2);

            // Agregar a la nueva población
            newPop.push_back(parent1);
            if (newPop.size() < popSize)
                newPop.push_back(parent2);
        }
        pop = newPop;

        // Evaluar el mejor fitness de la generación actual
        bestIt = std::max_element(pop.begin(), pop.end(), [](const OneMax &a, const OneMax &b)
                                  { return a.fitness() < b.fitness(); });
        int current_best = bestIt->fitness();
        if (current_best > best_fitness)
        {
            best_fitness = current_best;
            generation_max_fitness = gen;
        }

        // Comprobar condiciones de parada
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        generation_stop = gen; // Se actualiza cada generación
        if (current_best >= (int)nbits)
        {
            stop_reason = "Fitness alcanzado";
            sig = false;
        }
        if (elapsed >= timeout_seconds)
        {
            stop_reason = "Timeout de 2 minutos";
            sig = false;
        }
    }

    // Calcular el tiempo total de ejecución
    auto finish = std::chrono::steady_clock::now();
    double tiempo_ejecucion = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() / 1000.0;

    // Calcular la variación de fitness (fitness_final - fitness_inicial)
    int variacion_fitness = best_fitness - fitness_initial;

    // Imprimir la información final
    /*
    std::cout << "================ Resumen de la ejecución ================\n";
    std::cout << "ID: " << id << "\n";
    std::cout << "Fecha y hora: " << fecha_hora << "\n";
    std::cout << "Framework: " << framework << "\n";
    std::cout << "Tamaño de la población: " << popSize << "\n";
    std::cout << "Probabilidad de cruce: " << pc << "\n";
    std::cout << "Probabilidad de mutación: " << pm << "\n";
    std::cout << "Generación alcanzada: " << generation_stop << "\n";
    std::cout << "Fitness inicial: " << fitness_initial << "\n";
    std::cout << "Variación de fitness: " << variacion_fitness << "\n";
    std::cout << "Fitness final: " << best_fitness << "\n";
    std::cout << "Tiempo de ejecución (s): " << tiempo_ejecucion << "\n";
    std::cout << "Generación donde se alcanzó el fitness máximo: " << generation_max_fitness << "\n";
    std::cout << "Fitness máximo alcanzado: " << best_fitness << "\n";
    std::cout << "Motivo de parada: " << (stop_reason.empty() ? "Fin de generaciones" : stop_reason) << "\n";
    std::cout << "Dónde se ha ejecutado: " << ejecutado_en << "\n";
    std::cout << "==========================================================\n";
    */
    // Guardar los resultados en CSV
    std::ofstream csv("onemax_resultados.csv", std::ios::app);
    // Si el archivo está vacío, escribir la cabecera
    if (csv.tellp() == 0)
    {
        csv << "ID,Fecha_Hora,Framework,Tamano_individuo,Tamano_Poblacion,Prob_Cruce,Prob_Mutacion,Gen_Alcanzada,Fitness_Inicial,Variacion_Fitness,Fitness_Final,Tiempo_Ejecucion,Gen_Fitness_Max,Fitness_Max,Motivo_Parada,Donde_Ejecutado\n";
    }
    csv << id << ","
        << fecha_hora << ","
        << framework << ","
        << nbits << ","
        << popSize << ","
        << pc << ","
        << pm << ","
        << generation_stop << ","
        << fitness_initial << ","
        << variacion_fitness << ","
        << best_fitness << ","
        << tiempo_ejecucion << ","
        << generation_max_fitness << ","
        << best_fitness << ","
        << (stop_reason.empty() ? "Fin de generaciones" : stop_reason) << ","
        << ejecutado_en << "\n";
    csv.close();

    return 0;
}
