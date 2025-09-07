#ifndef GAMEBOARDMAC_BENCHMARK_H
#define GAMEBOARDMAC_BENCHMARK_H
#include "GamePlayer.hpp"
#include<cstddef>
#include<thread>

#include "GameTree.hpp"
#include "Othello.hpp"
#include "OthelloSave.hpp"


class Benchmark {
public:
    struct Results {
        PlayerID winner;
        int blackScore;
        int whiteScore;
        bool inverted;
    };

private:
    using Generator = std::function<TreeAI* ()>;

    struct BenchTask {
        bool inverse;
        GamePointer startPos;
    };
    class BenchThread {
    public:
        BenchThread();
        ~BenchThread();
        BenchThread(BenchThread&& other) noexcept;
        BenchThread& operator=(BenchThread&& other) noexcept;
        BenchThread(const BenchThread& thread) = delete;

        void setFighters(const Generator &genFight1, const Generator &genFight2);

        std::string getNames(PlayerID player) const;

        void runGame(bool inverse, GamePointer startPos = GamePointer());
        const std::vector<Results>& waitForCompletion();
    private:
        void run();

    private:
        std::thread m_thread;
        std::queue<BenchTask> m_tasks;
        std::mutex m_mutexQueue;
        std::mutex m_mutexFlow;
        std::condition_variable m_condition;
        bool m_cond;
        std::vector<Results> m_bench;
        bool m_bRunning;

    private:
        TreeAI* m_fight1;
        TreeAI* m_fight2;
    };

public:
    Benchmark(const Generator &genFight1, const Generator &genFight2);

    void runBenchmark(std::ostream& os, size_t nb_games, const WTHFileReader *reader = nullptr, int turn = 0, size_t nbThread = 10);

public:
    Generator gen_fight1;
    Generator gen_fight2;

private:
    std::vector<Results> m_results;
};


#endif //GAMEBOARDMAC_BENCHMARK_H