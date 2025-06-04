/**
 * @file sphere_sbx.cpp
 * @brief GA real-codificado con SBX + mutación polinómica sobre Sphere
 * compilar: c++ sphere_sbx.cpp -I../eo/src -std=c++17 -L./lib/ -leo -leoutils -o sphere_sbx
 * ejecutar: ./sphere_sbx -p <poblacion> -c <cruce> -m <mutacion> -i <id>
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

// ----------------------------------------------------
// Individuo: vector de reales con fitness a maximizar
struct Sphere : public EO<eoMaximizingFitness>
{
    std::vector<double> x;
    Sphere() {}
    Sphere(size_t n) : x(n, 0.0) {}
    void printOn(std::ostream &os) const override
    {
        for (double v : x)
            os << v << " ";
    }
    void readFrom(std::istream &is) override
    {
        for (auto &v : x)
            is >> v;
    }
};

// ----------------------------------------------------
// Evaluador real de Sphere: suma de cuadrados escalada a [0,1]
struct SphereFunction : public eoEvalFunc<Sphere>
{
    static constexpr double LOW = -5.12;
    static constexpr double UP = 5.12;
    static constexpr size_t N = 1024;
    static constexpr double FMAX = N * UP * UP;
    void operator()(Sphere &ind) override
    {
        double raw = 0.0;
        for (double v : ind.x)
            raw += v * v;
        double scaled = (1.0 - raw / FMAX);
        ind.fitness(scaled);
    }
};

// ----------------------------------------------------
// Inicializador: vectores uniformes en [LOW,UP]
struct SphereInit : public eoInit<Sphere>
{
    void operator()(Sphere &ind) override
    {
        ind.x.resize(SphereFunction::N);
        for (double &v : ind.x)
        {
            v = rng.uniform(SphereFunction::LOW, SphereFunction::UP);
        }
    }
};

// ----------------------------------------------------
// SBX Crossover
struct SBXCrossover : public eoQuadOp<Sphere>
{
    double eta;
    SBXCrossover(double _eta) : eta(_eta) {}
    bool operator()(Sphere &a, Sphere &b) override
    {
        const size_t n = a.x.size();
        for (size_t i = 0; i < n; ++i)
        {
            double u = rng.uniform();
            double beta = (u <= 0.5)
                              ? pow(2.0 * u, 1.0 / (eta + 1.0))
                              : pow(1.0 / (2.0 * (1.0 - u)), 1.0 / (eta + 1.0));
            double c1 = 0.5 * ((1 + beta) * a.x[i] + (1 - beta) * b.x[i]);
            double c2 = 0.5 * ((1 - beta) * a.x[i] + (1 + beta) * b.x[i]);
            a.x[i] = std::min(std::max(c1, SphereFunction::LOW), SphereFunction::UP);
            b.x[i] = std::min(std::max(c2, SphereFunction::LOW), SphereFunction::UP);
        }
        return true;
    }
};

// ----------------------------------------------------
// Mutación polinómica
struct PolyMutation : public eoMonOp<Sphere>
{
    double pm, eta;
    PolyMutation(double _pm, double _eta) : pm(_pm), eta(_eta) {}
    bool operator()(Sphere &ind) override
    {
        bool mutated = false;
        const size_t n = ind.x.size();
        for (size_t i = 0; i < n; ++i)
        {
            if (rng.uniform() < pm)
            {
                double u = rng.uniform();
                double delta = (u < 0.5)
                                   ? pow(2.0 * u, 1.0 / (eta + 1.0)) - 1.0
                                   : 1.0 - pow(2.0 * (1.0 - u), 1.0 / (eta + 1.0));
                double v = ind.x[i] + delta * (SphereFunction::UP - SphereFunction::LOW);
                ind.x[i] = std::min(std::max(v, SphereFunction::LOW), SphereFunction::UP);
                mutated = true;
            }
        }
        return mutated;
    }
};

// ----------------------------------------------------
// Obtener fecha y hora actual formateada
std::string getCurrentDateTime()
{
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&nowTime), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

// ----------------------------------------------------
int main(int argc, char **argv)
{
    size_t popSize = 1024;
    double pc = 0.8, pm = 0.1;
    int id = 1;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            popSize = std::stoul(argv[++i]);
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc)
            pc = std::stod(argv[++i]);
        else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc)
            pm = std::stod(argv[++i]);
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
            id = std::atoi(argv[++i]);
    }

    SphereInit init;
    SphereFunction eval;
    SBXCrossover xover(20.0);
    PolyMutation mutate(pm, 20.0);
    eoDetTournamentSelect<Sphere> select(2);

    // Población inicial
    eoPop<Sphere> pop;
    pop.reserve(popSize);
    for (size_t i = 0; i < popSize; ++i)
    {
        Sphere ind;
        init(ind);
        eval(ind);
        pop.push_back(ind);
    }

    // Fitness inicial máximo
    double initMax = double(pop[0].fitness());
    for (auto &ind : pop)
        initMax = std::max(initMax, double(ind.fitness()));
    double bestFit = initMax;
    size_t genBest = 0;

    // CSV
    std::ofstream csv("sphere_results.csv", std::ios::app);
    if (csv.tellp() == 0)
        csv << "fecha_hora,framework,tamanio_individuo,poblacion,cruce,mutacion,generacion,fitness_inicial,variacion_fitness,fitness_maximo,generacion_mejor,tiempo_transcurrido,motivo_parada,ubicacion_ejecucion\n";

    // Obtener fecha y hora actual
    std::string dateTime = getCurrentDateTime();

    // Bucle principal
    auto t0 = std::chrono::steady_clock::now();
    const int maxTime = 120;
    std::string stop = "timeout";
    size_t gen = 0;
    while (true)
    {
        auto dt = std::chrono::duration_cast<std::chrono::seconds>(
                      std::chrono::steady_clock::now() - t0)
                      .count();
        if (dt >= maxTime)
        {
            stop = "timeout";
            break;
        }
        ++gen;
        eoPop<Sphere> offspring;
        offspring.reserve(popSize);
        while (offspring.size() < popSize)
        {
            Sphere p1 = select(pop);
            Sphere p2 = select(pop);
            if (rng.uniform() < pc)
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
        for (auto &ind : pop)
        {
            double f = double(ind.fitness());
            if (f > bestFit)
            {
                bestFit = f;
                genBest = gen;
                if (bestFit >= 100.0)
                {
                    stop = "convergence";
                    break;
                }
            }
        }
        if (bestFit >= 100.0)
            break;
    }

    auto t1 = std::chrono::steady_clock::now();
    double timeSec = std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count();
    double var = bestFit - initMax;
    char host[256];
    gethostname(host, sizeof(host));

    // Escritura en CSV
    csv << dateTime << ","          // fecha_hora
        << "Paradiseo" << ","       // framework
        << SphereFunction::N << "," // tamanio_individuo
        << popSize << ","           // poblacion
        << pc << ","                // cruce
        << pm << ","                // mutacion
        << gen << ","               // generacion
        << initMax << ","           // fitness_inicial
        << var << ","               // variacion_fitness
        << bestFit << ","           // fitness_maximo
        << genBest << ","           // generacion_mejor
        << timeSec << ","           // tiempo_transcurrido
        << stop << ","              // motivo_parada
        << host << "\n";            // ubicacion_ejecucion

    csv.close();
    return 0;
}
