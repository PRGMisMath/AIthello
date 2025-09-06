#include "Benchmark.hpp"

#include "GameBoardManager.hpp"
#include "Random.hpp"

Benchmark::BenchThread::BenchThread() :
    m_bench(), m_tasks(), m_bRunning(true), m_mutexQueue(), m_mutexFlow(), m_thread(), m_cond(), m_fight1(), m_fight2()
{
    m_thread = std::thread(&Benchmark::BenchThread::run, this);
}

Benchmark::BenchThread::~BenchThread() {
    delete m_fight1;
    delete m_fight2;
}

Benchmark::BenchThread::BenchThread(BenchThread &&other) noexcept :
    m_thread(std::move(other.m_thread)), m_bench(std::move(other.m_bench)), m_tasks(std::move(other.m_tasks)),
    m_bRunning(other.m_bRunning), m_mutexQueue(), m_mutexFlow(), m_cond(other.m_cond),
    m_fight1(other.m_fight1), m_fight2(other.m_fight2)
{
}

Benchmark::BenchThread & Benchmark::BenchThread::operator=(BenchThread &&other) noexcept {
    if (this != &other) {
        m_thread = std::move(other.m_thread);
        m_bRunning = other.m_bRunning;
        m_tasks = std::move(other.m_tasks);
        m_bench = std::move(other.m_bench);
        m_cond = other.m_cond;
    }
    return *this;
}

void Benchmark::BenchThread::setFighters(const Generator &genFight1, const Generator &genFight2) {
    m_fight1 = genFight1(m_tree1);
    m_fight2 = genFight2(m_tree2);
    m_fight1->wait_time = 0;
    m_fight2->wait_time = 0;
}

std::string Benchmark::BenchThread::getNames(PlayerID player) const {
    return (player == PlayerID::Noir) ? m_fight1->name : m_fight2->name;
}

void Benchmark::BenchThread::runGame(bool inverse, GamePointer startPos) {
    m_mutexQueue.lock();
    m_tasks.push({inverse, startPos});
    m_mutexQueue.unlock();

    std::lock_guard<std::mutex> lock(m_mutexFlow);
    m_cond = true;
    m_condition.notify_one();
}

const std::vector<Benchmark::Results> & Benchmark::BenchThread::waitForCompletion() {
    m_bRunning = false;

    {
        std::lock_guard<std::mutex> lock(m_mutexFlow);
        m_cond = true;
        m_condition.notify_one();
    }

    m_thread.join();

    return m_bench;
}

void Benchmark::BenchThread::run() {
    Othello othello;
    GameBoardManager gbman{ &othello, nullptr, nullptr };
    while (m_bRunning) {
        std::unique_lock<std::mutex> lock(m_mutexFlow);
        m_condition.wait(lock, [&] { return m_cond; });
        m_cond = false;

        m_mutexQueue.lock();
        while (!m_tasks.empty()) {
            auto task = m_tasks.front();
            m_tasks.pop();
            m_mutexQueue.unlock();

            task.startPos.readTo(&othello);
            gbman.reset((task.inverse) ? m_fight2 : m_fight1, (task.inverse) ? m_fight1 : m_fight2);

            while (!gbman.isFinish()) {
                gbman.playTurn();
            }

            m_bench.push_back({
                othello.winner(),
                othello.getScore(PlayerID::Noir),
                othello.getScore(PlayerID::Blanc),
                task.inverse
            });

            m_mutexQueue.lock();
        }
        m_mutexQueue.unlock();
    }
}

Benchmark::Benchmark(const Generator &genFight1, const Generator &genFight2) :
    gen_fight1(genFight1), gen_fight2(genFight2)
{
}

void Benchmark::runBenchmark(std::ostream& os, size_t nb_games, const WTHFileReader *reader, int turn, size_t nbThread) {
    // Benchmark
    std::vector<BenchThread> threads{ nbThread };

    for (size_t i = 0; i < nbThread; i++) {
        threads[i].setFighters(gen_fight1, gen_fight2);
    }
    int lenwth = 0;
    if (reader != nullptr) {
        lenwth = reader->getGames().size() - 1;
    }
    auto unif_gen = rd::src.uniform_int_distributor(0,lenwth);
    auto half_gen = rd::src.bernoulli_distributor(0.5);

    while (nb_games-- != 0) {
        GamePointer pointer ;
        if (reader != nullptr) {
            pointer = GamePointer(&(reader->getGames()[unif_gen.next()].gameSave)) + turn;
        }
        threads[nb_games % nbThread].runGame(half_gen.next(), pointer);
    }

    m_results.clear();
    for (size_t i = 0; i < nbThread; i++) {
        const auto& tres = threads[i].waitForCompletion();
        m_results.insert(m_results.end(), tres.begin(), tres.end());
    }

    // Bilan

    os << "--- Analyse du benchmark ---\n"
       << " - Parties joues : " << m_results.size() << "\n"
       << " - Tour de depart : " << turn << "\n";
    int victory[2][2] = {{0,0},{0,0}}; // [winner][inverted?]
    int score[2][2] = {{0,0},{0,0}}; // [winner][inverted?]
    int draws = 0;

    for (const Results& result : m_results) {
        PlayerID corrWinner = result.inverted ? !result.winner : result.winner;
        score[*((result.inverted) ? PlayerID::Blanc : PlayerID::Noir)][result.inverted] += result.blackScore;
        score[*((result.inverted) ? PlayerID::Noir : PlayerID::Blanc)][result.inverted] += result.whiteScore;
        if (result.winner == PlayerID::Nobody)
            ++draws;
        else {
            ++victory[*corrWinner][*result.winner];
        }
    }

    std::string nameFigth1 = threads[0].getNames(PlayerID::Noir);
    std::string nameFigth2 = threads[0].getNames(PlayerID::Blanc);
    os << " - Victoire de " << nameFigth1 << " (1) : " << victory[0][0] + victory[0][1]
       << " (" << victory[0][0] << " en Noir et " << victory[0][1] << " en Blanc)\n";
    os << " - Victoire de " << nameFigth2 << " (2) : " << victory[1][0] + victory[1][1]
       << " (" << victory[1][1] << " en Noir et " << victory[1][0] << " en Blanc)\n";
    os << " - Egalite : " << draws << "\n";
    os << " - % Victoire Noire : "
       << ((float)victory[0][0] + (float)victory[1][0]) / ((float)m_results.size() - (float)draws + 1e-8f) * 100.0f
       << "\n";
    os << " - Score de " << nameFigth1 << " (1) : " << score[0][0] + score[0][1]
       << " (" << score[0][0] << " en Noir et " << score[0][1] << " en Blanc)\n";
    os << " - Score de " << nameFigth2 << " (2) : " << score[1][0] + score[1][1]
       << " (" << score[1][1] << " en Noir et " << score[1][0] << " en Blanc)\n";

}
