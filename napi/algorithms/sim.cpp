#include "sim.hpp"
#include "hungry.hpp"
#include "cautious.hpp"
#include "inyourface.hpp"
#include "terminator.hpp"
#include "../movement.hpp"
#include "../astar.hpp"
#include "../simulator.hpp"

#include <functional>
#include <unordered_map>

Sim::Sim() : _maxTurns(10000), _maxMillis(100)
{ }

Sim::Sim(uint32_t maxTurns, uint32_t maxMillis) :
    _maxTurns(maxTurns), _maxMillis(maxMillis)
{ }

Metadata Sim::meta()
{
    return {
        "#008800",
        "#FFFFFF",
        "https://www.popoptiq.com/wp-content/uploads/2015/04/Timecop-1994-Movie-Picture-01.jpg",
        "Time Cop",
        "Hi",
        "shades",
        "fat-rattle"
    };
}

void Sim::start()
{
    // Make sure the simulation threads are all awake so that it's not slow on
    // the first move.
    SimThread::wakeAll();
}

Direction Sim::move(GameState &state)
{
    InYourFace inYourFace;
    InYourFace inMyFace(state.mySnake());
    Hungry hungry;
    Cautious cautious;
    Terminator terminator;

    std::vector<Algorithm *> myAlgorithms { &terminator };
    std::vector<Algorithm *> enemyAlgorithms { &hungry, &inMyFace };

    std::vector<Future> futures = simulateFutures(
    	state, _maxTurns, _maxMillis, myAlgorithms, enemyAlgorithms);

    Direction best = bestMove(futures, state);

    return best;
}
