#include "movement.hpp"
#include "astar.hpp"
#include "snakelib.hpp"

MaybeDirection closestFood(GameState &state)
{
    Snake *me = state.mySnake();
    std::vector<Direction> best;
    bool foundAnything = false;

    for (auto food : state.food())
    {
        std::vector<Direction> myPath = shortestPath(me->head(), food, state);

        if (myPath.empty())
        {
            continue;
        }

        if (myPath.size() >= best.size() && foundAnything)
        {
            continue;
        }

        foundAnything = true;
        best = myPath;
    }

    if (!best.empty())
    {
        return  MaybeDirection{ true, best.at(0) };
    }
    else
    {
        return MaybeDirection{ false, Direction::Left };
    }
}

MaybeDirection bestFood(GameState &state)
{
    Snake *me = state.mySnake();
    Direction bestDirection = Direction::Left;
    uint32_t bestLength;
    bool foundOne = false;

    for (Point food : state.food())
    {
        auto myPath = shortestPath(me->head(), food, state);

        if (myPath.empty())
            continue;

        if (foundOne && myPath.size() >= bestLength)
            continue;

        bool enemyWillWin = false;
        for (Snake *enemy : state.enemies())
        {
            GameState &enemyState = state.perspective(enemy);
            auto enemyPath = shortestPath(enemy->head(), food, enemyState);

            if (enemyPath.empty())
            {
                continue;
            }

            if (enemyPath.size() == myPath.size())
            {
                enemyWillWin = me->length() < enemy->length();
            }
            else
            {
                enemyWillWin = enemyPath.size() < myPath.size();
            }

            if (enemyWillWin)
            {
                break;
            }
        }

        if (!enemyWillWin)
        {
            foundOne = true;
            bestLength = myPath.size();
            bestDirection = myPath.at(0);
        }
    }

    return MaybeDirection{ foundOne, bestDirection };
}

bool is180(Point p, GameState &state)
{
    // If length of snake is 1 then nothing to worry about (... I think... might wanna test that)
    if (state.mySnake()->length() <= 1)
    {
        return false;
    }

    Point neck = state.mySnake()->parts.at(1);

    return neck == p;
}

bool isCellOk(Point p, GameState &state)
{
    return !outOfBounds(p, state)
        && state.map().turnsUntilVacant(p) == 0
        && !is180(p, state);
}

bool isCellSafe(Point p, GameState &state)
{
    return isCellOk(p, state) && !isCloseToEqualOrBiggerSnakeHead(p, state); 
}

bool isCellRisky(Point p, GameState &state)
{
    return isCellOk(p, state) && isCloseToEqualOrBiggerSnakeHead(p, state); 
}

DirectionSet notImmediatelySuicidalMoves(GameState &state)
{
    Point myHead = state.mySnake()->head();
    uint32_t x = myHead.x;
    uint32_t y = myHead.y;
    DirectionSet moves;

    if (isCellOk({x - 1, y}, state))
    {
        moves.push(Direction::Left);
    }
    if (isCellOk({x + 1, y}, state))
    {
        moves.push(Direction::Right);
    }
    if (isCellOk({x, y - 1}, state))
    {
        moves.push(Direction::Up);
    }
    if (isCellOk({x, y + 1}, state))
    {
        moves.push(Direction::Down);
    }

    return moves;
}

DirectionSet safeMoves(GameState &state)
{
    Point myHead = state.mySnake()->head();
    uint32_t x = myHead.x;
    uint32_t y = myHead.y;
    DirectionSet moves;

    if (isCellSafe({x - 1, y}, state))
    {
        moves.push(Direction::Left);
    }
    if (isCellSafe({x + 1, y}, state))
    {
        moves.push(Direction::Right);
    }
    if (isCellSafe({x, y - 1}, state))
    {
        moves.push(Direction::Up);
    }
    if (isCellSafe({x, y + 1}, state))
    {
        moves.push(Direction::Down);
    }

    return moves;
}

DirectionSet riskyMoves(GameState &state)
{
    Point myHead = state.mySnake()->head();
    uint32_t x = myHead.x;
    uint32_t y = myHead.y;
    DirectionSet moves;

    if (isCellRisky({x - 1, y}, state))
    {
        moves.push(Direction::Left);
    }
    if (isCellRisky({x + 1, y}, state))
    {
        moves.push(Direction::Right);
    }
    if (isCellRisky({x, y - 1}, state))
    {
        moves.push(Direction::Up);
    }
    if (isCellRisky({x, y + 1}, state))
    {
        moves.push(Direction::Down);
    }

    return moves;
}

MaybeDirection notImmediatelySuicidal(GameState &state)
{
    DirectionSet moves = notImmediatelySuicidalMoves(state);
    if (moves.empty())
    {
        return MaybeDirection{ false, Direction::Left };
    }
    else
    {
        return MaybeDirection{ true, *moves.begin() };
    }
}

MaybeDirection chaseTail(GameState &state)
{
    Point myHead = state.mySnake()->head();
    Point myTail = state.mySnake()->tail();
    std::vector<Direction> path = shortestPath(myHead, myTail, state);
    if (!path.empty())
    {
        return MaybeDirection { true, path.at(0) };
    }
    else
    {
        return MaybeDirection { false, Direction::Left };
    }
}

