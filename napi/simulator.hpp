#pragma once

#include "snakelib.hpp"
#include "timing.hpp"
#include <thread>
#include <chrono>

#define THREAD_COUNT 4

// Sleep this amount of time each iteration of worker thread when in sleep mode.
#define SLEEP_MODE_DELAY_MILLIS 10

// Amount of time with no work before worker thread enters sleep mode.
#define SECONDS_OF_NO_WORK_UNTIL_SLEEP 10.0

enum class TerminationReason
{
    Loss, MaxTurns, OutOfTime, Unknown
};

struct AlgorithmPair
{
    Algorithm *myAlgorithm;
    Algorithm *enemyAlgorithm;
};

struct AlgorithmBranch
{
    AlgorithmPair pair;
    MaybeDirection firstMove;
};

struct Future
{
    std::unordered_map<std::string, uint32_t> obituaries;
    std::unordered_map<std::string, std::vector<uint32_t>> foodsEaten;
    Algorithm *algorithm;
    TerminationReason terminationReason;
    Direction move;
    uint32_t turns;
};

struct DirectionScore
{
    Direction direction;
    int score;
};

class Simulation
{
public:
    Simulation(
        AlgorithmBranch branch,
        GameState &initialState,
        uint32_t maxTurns,
        uint32_t simNumber);

    bool next();
    Future result() { return _result; }
    uint32_t simNumber() { return _simNumber; }


private:
    Direction getMyMove(GameState &state)
    {
        if (_turn == 1 && _branch.firstMove.hasValue)
        {
            return _branch.firstMove.value;
        }
        else
        {
            return _branch.pair.myAlgorithm->move(state);
        }
    }

    void updateObituaries(GameState &newState, GameState &oldState)
    {
        for (auto pair : oldState.snakes())
        {
            Snake *snake = pair.second;
            auto newIter = newState.snakes().find(snake->id);
            if (newIter == newState.snakes().end())
            {
                _result.obituaries[snake->id] = _turn;
            }
        }
    }

    void updateFoodsEaten(GameState &newState, GameState &oldState)
    {
        for (Point food : oldState.food())
        {
            Snake *inThatCellNow = newState.map().getSnake(food);
            if (inThatCellNow != nullptr)
            {
                auto iter = _result.foodsEaten.find(inThatCellNow->id);
                if (iter == _result.foodsEaten.end())
                {
                    _result.foodsEaten[inThatCellNow->id] = {};
                }
                _result.foodsEaten[inThatCellNow->id].push_back(_turn);
            }
        }
    }

    AlgorithmBranch _branch;
    GameState &_initialState;
    uint32_t _maxTurns;
    uint32_t _simNumber;
    uint32_t _turn;
    Future _result;
    std::unique_ptr<GameState> _newestState;
};

std::vector<Future> runSimulationBranches(
    std::vector<AlgorithmBranch> &branches,
    GameState &initialState,
    uint32_t maxTurns);

std::vector<Future> runSimulations(
    std::vector<AlgorithmPair> algorithmPairs,
    GameState &initialState,
    uint32_t maxTurns,
    DirectionSet firstMoves);

std::vector<Future> simulateFutures(
    GameState &initialState,
    uint32_t maxTurns,
    std::vector<Algorithm *> myAlgorithms,
    std::vector<Algorithm *> enemyAlgorithms);

Direction bestMove(std::vector<Future> &futures, GameState &state);

struct SimParams
{
    std::vector<AlgorithmBranch> branches;
    std::unique_ptr<GameState> state;
    uint32_t maxTurns;
};

class SimThread
{
public:
    SimThread();
    void startWork(SimParams params)
    void spin();
    std::vector<Future> &result();
    bool done();
    void kill();
    void join();
    void wakeUp();
    void sleep();

    static std::array<SimThread, THREAD_COUNT> instances;
    static void stopAll();
    static void wakeAll();

private:
    std::vector<Future> _result;
    SimParams _params;
    volatile bool _hasWork;
    volatile bool _quit;
    std::thread _thread;
    Clock::time_point _timeOfLastWork;
    volatile bool _sleeping;
};
