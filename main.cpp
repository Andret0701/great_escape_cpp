#include <algorithm>
#include <chrono>
#include <deque>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

#define UNREACHABLE -1
#define UNVISITED -1

using namespace std;

struct Vector2
{
    int x;
    int y;

    Vector2() : Vector2(0, 0) {}
    Vector2(int x, int y)
    {
        this->x = x;
        this->y = y;
    }

    Vector2 operator+(const Vector2 &other) const
    {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator-(const Vector2 &other) const
    {
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 operator-() const { return Vector2(-x, -y); }
};

enum class Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Grid
{
public:
    Grid(int width, int height)
    {
        this->width = width;
        this->height = height;
        this->blocked_paths = new int[width * height * width * height]{0};
        this->visited = new int[width * height];
        this->queue = new Vector2[width * height];
    }
    ~Grid()
    {
        delete[] blocked_paths;
        delete[] visited;
        delete[] queue;
    }

    bool is_inside(Vector2 pos)
    {
        return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
    }

    int get_index(Vector2 pos) { return pos.x + pos.y * width; }

    bool is_blocked(Vector2 pos1, Vector2 pos2)
    {
        // will crash if pos1 or pos2 are outside the grid
        return blocked_paths[pos1.x + pos1.y * width + pos2.x * width * height +
                             pos2.y * width * width * height] != 0;
    }

    int get_blocked(Vector2 pos1, Vector2 pos2)
    {
        // will crash if pos1 or pos2 are outside the grid
        return blocked_paths[pos1.x + pos1.y * width + pos2.x * width * height +
                             pos2.y * width * width * height];
    }

    void set_blocked(Vector2 pos1, Vector2 pos2, bool blocked)
    {
        // Exploit symmetry
        blocked_paths[pos1.x + pos1.y * width + pos2.x * width * height +
                      pos2.y * width * width * height] = blocked;
        blocked_paths[pos2.x + pos2.y * width + pos1.x * width * height +
                      pos1.y * width * width * height] = blocked;
    }

    void set_blocked(Vector2 pos1, Vector2 pos2, int blocked)
    {
        // Exploit symmetry
        blocked_paths[pos1.x + pos1.y * width + pos2.x * width * height +
                      pos2.y * width * width * height] = blocked;
        blocked_paths[pos2.x + pos2.y * width + pos1.x * width * height +
                      pos1.y * width * width * height] = blocked;
    }

    int get_distance(Vector2 pos, Direction dir)
    {
        if (!is_inside(pos))
            return UNREACHABLE;

        write_index = 0;
        read_index = 0;

        fill_n(visited, width * height, UNVISITED);

        visited[get_index(pos)] = 0;

        // queue
        queue[write_index++] = pos;

        while (write_index > read_index)
        {
            Vector2 current = queue[read_index++];

            switch (dir)
            {
            case Direction::UP:
                if (current.y == 0)
                    return visited[get_index(current)];
                break;
            case Direction::DOWN:
                if (current.y == height - 1)
                    return visited[get_index(current)];
                break;
            case Direction::LEFT:
                if (current.x == 0)
                    return visited[get_index(current)];
                break;
            case Direction::RIGHT:
                if (current.x == width - 1)
                    return visited[get_index(current)];
                break;
            }

            int next_distance = visited[get_index(current)] + 1;

            Vector2 next = Vector2(current.x, current.y - 1);
            int next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED &&
                !is_blocked(current, next))
            {
                visited[next_index] = next_distance;
                queue[write_index++] = next;
            }

            next = Vector2(current.x, current.y + 1);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED &&
                !is_blocked(current, next))
            {
                visited[next_index] = next_distance;
                queue[write_index++] = next;
            }

            next = Vector2(current.x - 1, current.y);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED &&
                !is_blocked(current, next))
            {
                visited[next_index] = next_distance;
                queue[write_index++] = next;
            }

            next = Vector2(current.x + 1, current.y);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED &&
                !is_blocked(current, next))
            {
                visited[next_index] = next_distance;
                queue[write_index++] = next;
            }
        }

        return UNREACHABLE;
    }

    Direction get_direction_to(Vector2 pos, Direction dir)
    {
        write_index = 0;
        read_index = 0;

        fill_n(visited, width * height, UNVISITED);

        visited[get_index(pos)] = 0;

        Vector2 next = Vector2(pos.x, pos.y - 1);
        int next_index = get_index(next);
        if (is_inside(next) && visited[next_index] == UNVISITED &&
            !is_blocked(pos, next))
        {
            visited[next_index] = (int)Direction::UP;
            queue[write_index++] = next;
        }

        next = Vector2(pos.x, pos.y + 1);
        next_index = get_index(next);
        if (is_inside(next) && visited[next_index] == UNVISITED &&
            !is_blocked(pos, next))
        {
            visited[next_index] = (int)Direction::DOWN;
            queue[write_index++] = next;
        }

        next = Vector2(pos.x - 1, pos.y);
        next_index = get_index(next);
        if (is_inside(next) && visited[next_index] == UNVISITED &&
            !is_blocked(pos, next))
        {
            visited[next_index] = (int)Direction::LEFT;
            queue[write_index++] = next;
        }

        next = Vector2(pos.x + 1, pos.y);
        next_index = get_index(next);
        if (is_inside(next) && visited[next_index] == UNVISITED &&
            !is_blocked(pos, next))
        {
            visited[next_index] = (int)Direction::RIGHT;
            queue[write_index++] = next;
        }

        while (write_index > read_index)
        {
            Vector2 current = queue[read_index++];
            int parent = visited[get_index(current)];

            switch (dir)
            {
            case Direction::UP:
                if (current.y == 0)
                {
                    int value = visited[get_index(current)];
                    return (Direction)value;
                }
                break;
            case Direction::DOWN:
                if (current.y == height - 1)
                {
                    int value = visited[get_index(current)];
                    return (Direction)value;
                }
                break;
            case Direction::LEFT:
                if (current.x == 0)
                {
                    int value = visited[get_index(current)];
                    return (Direction)value;
                }
                break;
            case Direction::RIGHT:
                if (current.x == width - 1)
                {
                    int value = visited[get_index(current)];
                    return (Direction)value;
                }
                break;
            }

            Vector2 next = Vector2(current.x, current.y - 1);
            int next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED &&
                !is_blocked(current, next))
            {
                visited[next_index] = parent;
                queue[write_index++] = next;
            }

            next = Vector2(current.x, current.y + 1);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED &&
                !is_blocked(current, next))
            {
                visited[next_index] = parent;
                queue[write_index++] = next;
            }

            next = Vector2(current.x - 1, current.y);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED &&
                !is_blocked(current, next))
            {
                visited[next_index] = parent;
                queue[write_index++] = next;
            }

            next = Vector2(current.x + 1, current.y);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED &&
                !is_blocked(current, next))
            {
                visited[next_index] = parent;
                queue[write_index++] = next;
            }
        }

        return dir;
    }

private:
    int width;
    int height;
    int *blocked_paths;
    int *visited;
    Vector2 *queue;
    int write_index;
    int read_index;
};

struct Wall
{
    Vector2 pos;
    bool horizontal;
    Wall() : Wall(Vector2(0, 0), true) {}
    Wall(Vector2 pos, bool horizontal)
    {
        this->pos = pos;
        this->horizontal = horizontal;
    }
};

enum class BoardState
{
    UNDECIDED,
    WON,
    LOST,
    ILLEGAL
};

int compare_board_states(BoardState state1, BoardState state2)
{
    int state1_score = state1 == BoardState::WON ? 1
                                                 : (state1 == BoardState::LOST ? -1
                                                                               : 0);
    int state2_score = state2 == BoardState::WON ? 1
                                                 : (state2 == BoardState::LOST ? -1
                                                                               : 0);
    if (state1_score != state2_score)
    {
        if (state1_score > state2_score)
            return 1;
        else
            return -1;
    }
    return 0;
}

struct Score
{
    int score;
    int depth;
    BoardState first_place_state;
    BoardState second_place_state;

    Score() : Score(0, BoardState::UNDECIDED) {}
    Score(int score, BoardState state)
    {
        this->score = score;
        this->first_place_state = state;
        this->second_place_state = state;
    }

    Score(int score, BoardState first_place_state, BoardState second_place_state)
    {
        this->score = score;
        this->first_place_state = first_place_state;
        this->second_place_state = second_place_state;
    }

    bool operator<(const Score &other) const
    {
        int first_state_compare = compare_board_states(first_place_state,
                                                       other.first_place_state);

        if (first_state_compare != 0)
            return first_state_compare < 0;

        if (first_place_state != BoardState::WON)
        {
            int second_state_compare = compare_board_states(second_place_state,
                                                            other.second_place_state);

            if (second_state_compare != 0)
                return second_state_compare < 0;
        }

        return score < other.score;
    }

    bool operator>(const Score &other) const
    {
        int first_state_compare = compare_board_states(first_place_state,
                                                       other.first_place_state);

        if (first_state_compare != 0)
            return first_state_compare > 0;

        if (first_place_state != BoardState::WON)
        {
            int second_state_compare = compare_board_states(second_place_state,
                                                            other.second_place_state);

            if (second_state_compare != 0)
                return second_state_compare > 0;
        }

        return score > other.score;
    }

    bool operator==(const Score &other) const
    {
        return score == other.score && first_place_state == other.first_place_state &&
               second_place_state == other.second_place_state;
    }

    bool operator<=(const Score &other) const
    {
        return *this < other || *this == other;
    }

    bool operator>=(const Score &other) const
    {
        return *this > other || *this == other;
    }
};

Score max(Score score1, Score score2)
{
    if (score1 > score2)
        return score1;
    return score2;
}

Score min(Score score1, Score score2)
{
    if (score1 < score2)
        return score1;
    return score2;
}

struct Move
{
    Score score;
    bool is_wall;
    Wall wall;
    int id;
    Vector2 direction;
    Move() : Move(0, Vector2(0, 0)) {}
    Move(int id, Vector2 direction)
    {
        this->is_wall = false;
        this->id = id;
        this->direction = direction;
    }
    Move(int id, Wall wall)
    {
        this->is_wall = true;
        this->id = id;
        this->wall = wall;
    }
};

class MinMovesArray
{
public:
    MinMovesArray(int _size)
    {
        this->_size = _size;
        this->moves = new Move[_size];
        this->num_elements = 0;
        this->max_score = Score(-999999, BoardState::LOST);
        this->max_index = 0;
    }

    ~MinMovesArray() { delete[] moves; }

    int size() { return num_elements; }

    void push(Move move)
    {
        if (move.score.first_place_state == BoardState::ILLEGAL || move.score.second_place_state == BoardState::ILLEGAL)
            return;

        if (num_elements < _size)
        {
            moves[num_elements++] = move;
            if (move.score > max_score)
            {
                max_score = move.score;
                max_index = num_elements - 1;
            }
            return;
        }

        if (move.score < max_score)
        {
            moves[max_index] = move;
            update_max();
        }
    }

    Move get(int index) { return moves[index]; }
    void sort()
    {
        std::sort(moves, moves + num_elements,
                  [](const Move &a, const Move &b)
                  { return a.score < b.score; });
    }

private:
    int _size;
    Move *moves;
    int num_elements;
    Score max_score;
    int max_index;

    void update_max()
    {
        max_score = Score(-999999, BoardState::LOST);
        for (int i = 0; i < num_elements; i++)
        {
            if (moves[i].score > max_score)
            {
                max_score = moves[i].score;
                max_index = i;
            }
        }
    }
};

class MaxMovesArray
{
public:
    MaxMovesArray(int _size)
    {
        this->_size = _size;
        this->moves = new Move[_size];
        this->num_elements = 0;
        this->min_score = Score(999999, BoardState::WON);
        this->min_index = 0;
    }

    ~MaxMovesArray() { delete[] moves; }

    int size() { return num_elements; }

    void push(Move move)
    {
        if (move.score.first_place_state == BoardState::ILLEGAL || move.score.second_place_state == BoardState::ILLEGAL)
            return;

        if (num_elements < _size)
        {
            moves[num_elements++] = move;
            if (move.score < min_score)
            {
                min_score = move.score;
                min_index = num_elements - 1;
            }
            return;
        }

        if (move.score > min_score)
        {
            moves[min_index] = move;
            update_min();
        }
    }

    Move get(int index) { return moves[index]; }

    Move *get_ref(int index) { return &moves[index]; }

    void sort()
    {
        std::sort(moves, moves + num_elements,
                  [](const Move &a, const Move &b)
                  { return a.score > b.score; });
    }

private:
    int _size;
    Move *moves;
    int num_elements;
    Score min_score;
    int min_index;

    void update_min()
    {
        min_score = Score(999999, BoardState::WON);
        for (int i = 0; i < _size; i++)
        {
            if (moves[i].score < min_score)
            {
                min_score = moves[i].score;
                min_index = i;
            }
        }
    }
};

struct Player
{
    bool is_alive;
    bool is_finished;
    Vector2 pos;
    int walls_left;
    Direction end_direction;

    Player() : Player(Direction::UP) {}
    Player(Direction end_direction)
    {
        this->is_alive = true;
        this->is_finished = false;
        this->pos = Vector2(0, 0);
        this->walls_left = 0;
        this->end_direction = end_direction;
    }
};

class Board
{
public:
    Board(int width, int height, int player_count)
    {

        this->width = width;
        this->height = height;
        this->grid = new Grid(width, height);

        player_count = clamp(player_count, 2, 3);
        this->player_count = player_count;
        this->players = new Player[player_count];
        this->players[0] = Player(Direction::RIGHT);
        this->players[1] = Player(Direction::LEFT);
        if (player_count == 3)
            this->players[2] = Player(Direction::DOWN);
    }
    ~Board() { delete grid; }

    void move_player(int id, Vector2 direction)
    {
        if (id < 0 || id >= player_count)
            return;

        players[id].pos = players[id].pos + direction;
        players[id].is_finished = player_is_at_end(id);
    }

    void update_player(int id, Vector2 pos, int walls_left)
    {
        if (id < 0 || id >= player_count)
            return;

        if (pos.x == -1 || pos.y == -1)
            players[id].is_alive = false;

        players[id].pos = pos;
        players[id].walls_left = walls_left;

        if (player_is_at_end(id))
            players[id].is_alive = false;
    }

    bool can_finish()
    {
        for (int i = 0; i < player_count; i++)
        {
            if (players[i].is_alive &&
                grid->get_distance(players[i].pos, players[i].end_direction) ==
                    UNREACHABLE)
                return false;
        }

        return true;
    }

    bool is_finished()
    {
        for (int i = 0; i < player_count; i++)
        {
            if (player_is_at_end(i))
                return true;
        }

        return false;
    }

    bool player_is_at_end(int id)
    {
        // position
        Vector2 pos = players[id].pos;
        switch (players[id].end_direction)
        {
        case Direction::UP:
            return pos.y == 0;
        case Direction::DOWN:
            return pos.y == height - 1;
        case Direction::LEFT:
            return pos.x == 0;
        case Direction::RIGHT:
            return pos.x == width - 1;
        }

        return false;
    }

    bool has_won(int id)
    {
        if (!is_finished())
            return false;
        return players[id].is_alive && player_is_at_end(id);
    }

    bool is_overlaping(Wall wall)
    {
        // check if wall overlaps with another wall
        if (wall.horizontal)
        {
            if (grid->is_blocked(wall.pos, wall.pos + Vector2(0, -1)) ||
                grid->is_blocked(wall.pos + Vector2(1, 0), wall.pos + Vector2(1, -1)))
                return true;

            if (grid->get_blocked(wall.pos, wall.pos + Vector2(1, 0)) ==
                    grid->get_blocked(wall.pos + Vector2(0, -1),
                                      wall.pos + Vector2(1, -1)) &&
                grid->is_blocked(wall.pos, wall.pos + Vector2(1, 0)))
                return true;
        }
        else
        {
            if (grid->is_blocked(wall.pos, wall.pos + Vector2(-1, 0)) ||
                grid->is_blocked(wall.pos + Vector2(0, 1), wall.pos + Vector2(-1, 1)))
                return true;

            if (grid->get_blocked(wall.pos, wall.pos + Vector2(0, 1)) ==
                    grid->get_blocked(wall.pos + Vector2(-1, 0),
                                      wall.pos + Vector2(-1, 1)) &&
                grid->is_blocked(wall.pos, wall.pos + Vector2(0, 1)))
                return true;
        }

        return false;
    }

    bool is_wall_distance(Wall wall, int distance)
    {
        if (players[0].is_alive)
        {
            int distance_1 = abs(wall.pos.x - players[0].pos.x) +
                             abs(wall.pos.y - players[0].pos.y);
            if (distance_1 <= distance)
                return true;
        }

        if (players[1].is_alive)
        {
            int distance_2 = abs(wall.pos.x - players[1].pos.x) +
                             abs(wall.pos.y - players[1].pos.y);
            if (distance_2 <= distance)
                return true;
        }

        if (player_count == 3 && players[2].is_alive)
        {
            int distance_3 = abs(wall.pos.x - players[2].pos.x) +
                             abs(wall.pos.y - players[2].pos.y);
            if (distance_3 <= distance)
                return true;
        }

        return false;
    }

    bool can_place_wall(Wall wall)
    {
        // Check if wall is overlaping
        if (is_overlaping(wall))
            return false;

        // Make sure there is a path for each player

        if (wall_count != 0)
        {
            place_wall(wall);
            bool allowed = can_finish();
            remove_wall(wall);
            return allowed;
        }

        return true;
    }

    void place_wall(Wall wall)
    {
        // Check if wall is overlaping
        if (is_overlaping(wall))
            return;

        wall_count++;

        // Place wall
        if (wall.horizontal)
        {
            grid->set_blocked(wall.pos, wall.pos + Vector2(0, -1), wall_count);
            grid->set_blocked(wall.pos + Vector2(1, 0), wall.pos + Vector2(1, -1),
                              wall_count);
        }
        else
        {
            grid->set_blocked(wall.pos, wall.pos + Vector2(-1, 0), wall_count);
            grid->set_blocked(wall.pos + Vector2(0, 1), wall.pos + Vector2(-1, 1),
                              wall_count);
        }
    }

    void remove_wall(Wall wall)
    {
        // Place wall
        if (wall.horizontal)
        {
            grid->set_blocked(wall.pos, wall.pos + Vector2(0, -1), false);
            grid->set_blocked(wall.pos + Vector2(1, 0), wall.pos + Vector2(1, -1),
                              false);
        }
        else
        {
            grid->set_blocked(wall.pos, wall.pos + Vector2(-1, 0), false);
            grid->set_blocked(wall.pos + Vector2(0, 1), wall.pos + Vector2(-1, 1),
                              false);
        }

        wall_count--;
    }

    void do_move(Move move)
    {
        if (move.is_wall)
        {
            temp_wall_count++;
            players[move.id].walls_left--;
            place_wall(move.wall);
        }
        else
        {
            move_player(move.id, move.direction);
        }

        turn_count++;
    }

    void undo_move(Move move)
    {
        if (move.is_wall)
        {
            temp_wall_count--;
            players[move.id].walls_left++;
            remove_wall(move.wall);
        }
        else
        {
            move_player(move.id, -move.direction);
        }

        turn_count--;
    }

    Move get_best_direction(int id)
    {
        Vector2 pos = players[id].pos;
        Direction dir = grid->get_direction_to(pos, players[id].end_direction);
        switch (dir)
        {
        case Direction::UP:
            return Move(id, Vector2(0, -1));
        case Direction::DOWN:
            return Move(id, Vector2(0, 1));
        case Direction::LEFT:
            return Move(id, Vector2(-1, 0));
        case Direction::RIGHT:
            return Move(id, Vector2(1, 0));
        }

        return Move(id, Vector2(0, 0));
    }

    // lost is best
    MinMovesArray *get_minimizing_moves(int my_id, int current_id, int breadth,
                                        bool use_walls)
    {
        Move direction = get_best_direction(current_id);
        direction.score = score_move(my_id, direction);
        int distance = grid->get_distance(players[current_id].pos,
                                          players[current_id].end_direction);
        if (distance <= 1 || (direction.score.first_place_state == BoardState::LOST && direction.score.second_place_state == BoardState::LOST) || !use_walls)
        {
            MinMovesArray *single_move = new MinMovesArray(1);
            single_move->push(direction);
            return single_move;
        }

        MinMovesArray *moves = new MinMovesArray(breadth);
        moves->push(direction);

        if (players[current_id].walls_left != 0)
        {
            // Horizontal walls
            for (int y = 1; y < height; y++)
            {
                for (int x = 0; x < width - 1; x++)
                {
                    Wall wall = Wall(Vector2(x, y), true);
                    if (!is_overlaping(wall) && is_wall_distance(wall, 3))
                    {
                        Move wall_move = Move(current_id, wall);
                        wall_move.score = score_move(my_id, wall_move);
                        if (wall_move.score.first_place_state == BoardState::LOST && wall_move.score.second_place_state == BoardState::LOST)
                        {
                            MinMovesArray *single_move = new MinMovesArray(1);
                            single_move->push(wall_move);
                            delete moves;
                            return single_move;
                        }
                        moves->push(wall_move);
                    }
                }
            }

            // Vertical walls
            for (int y = 0; y < height - 1; y++)
            {
                for (int x = 1; x < width; x++)
                {
                    Wall wall = Wall(Vector2(x, y), false);
                    if (!is_overlaping(wall) && is_wall_distance(wall, 3))
                    {
                        Move wall_move = Move(current_id, wall);
                        wall_move.score = score_move(my_id, wall_move);
                        if (wall_move.score.first_place_state == BoardState::LOST && wall_move.score.second_place_state == BoardState::LOST)
                        {
                            MinMovesArray *single_move = new MinMovesArray(1);
                            single_move->push(wall_move);
                            delete moves;
                            return single_move;
                        }
                    }
                }
            }
        }

        moves->sort();
        return moves;
    }

    // won is best
    MaxMovesArray *get_maximizing_moves(int my_id, int current_id, int breadth,
                                        bool use_walls)
    {

        Move direction = get_best_direction(current_id);
        direction.score = score_move(my_id, direction);
        if ((direction.score.first_place_state == BoardState::WON && direction.score.second_place_state == BoardState::WON) || !use_walls)
        {
            MaxMovesArray *single_move = new MaxMovesArray(1);
            single_move->push(direction);
            return single_move;
        }

        MaxMovesArray *moves = new MaxMovesArray(breadth);
        moves->push(direction);

        if (players[current_id].walls_left != 0)
        {
            // Horizontal walls
            for (int y = 1; y < height; y++)
            {
                for (int x = 0; x < width - 1; x++)
                {
                    Wall wall = Wall(Vector2(x, y), true);
                    if (!is_overlaping(wall) && is_wall_distance(wall, 3))
                    {
                        Move wall_move = Move(current_id, wall);
                        wall_move.score = score_move(my_id, wall_move);
                        if (wall_move.score.first_place_state == BoardState::WON && wall_move.score.second_place_state == BoardState::WON)
                        {
                            MaxMovesArray *single_move = new MaxMovesArray(1);
                            single_move->push(wall_move);
                            delete moves;
                            return single_move;
                        }
                        moves->push(wall_move);
                    }
                }
            }

            // Vertical walls
            for (int y = 0; y < height - 1; y++)
            {
                for (int x = 1; x < width; x++)
                {
                    Wall wall = Wall(Vector2(x, y), false);
                    if (!is_overlaping(wall) && is_wall_distance(wall, 3))
                    {
                        Move wall_move = Move(current_id, wall);
                        wall_move.score = score_move(my_id, wall_move);
                        if (wall_move.score.first_place_state == BoardState::WON && wall_move.score.second_place_state == BoardState::WON)
                        {
                            MaxMovesArray *single_move = new MaxMovesArray(1);
                            single_move->push(wall_move);
                            delete moves;
                            return single_move;
                        }
                        moves->push(wall_move);
                    }
                }
            }
        }

        moves->sort();
        return moves;
    }

    int get_distance(int id)
    {
        return grid->get_distance(players[id].pos, players[id].end_direction);
    }

    int get_next_id(int id)
    {
        id = (id + 1) % player_count;
        if (!players[id].is_alive || players[id].is_finished)
            return get_next_id(id);
        return id;
    }

    int get_num_alive()
    {
        int num_alive = 0;
        for (int i = 0; i < player_count; i++)
            num_alive += players[i].is_alive;
        return num_alive;
    }

    int get_num_playing()
    {
        int num_playing = 0;
        for (int i = 0; i < player_count; i++)
            if (players[i].is_alive && !players[i].is_finished)
                num_playing++;
        return num_playing;
    }

    int get_id_finished()
    {
        for (int i = 0; i < player_count; i++)
            if (players[i].is_alive && players[i].is_finished)
                return i;
        return -1;
    }

    Score score_move(int id, Move move)
    {
        if (get_num_alive() == 2)
            return score_move_2_players(id, move);
        else
        {
            int finished_id = get_id_finished();
            if (finished_id != -1)
            {
                Score score = score_move_2_players(id, move);
                score = Score(score.score, BoardState::LOST, score.first_place_state);
                return score;
            }
            else
                return score_move_3_players(id, move);
        }
    }

    Score score_move_2_players(int id, Move move)
    {
        int current_id = get_next_id(move.id);
        int next_id = get_next_id(current_id);
        do_move(move);

        int current_distance = get_distance(current_id);
        if (current_distance == UNREACHABLE)
        {
            undo_move(move);
            return Score(0, BoardState::ILLEGAL);
        }

        int next_distance = get_distance(next_id);
        if (next_distance == UNREACHABLE)
        {
            undo_move(move);
            return Score(0, BoardState::ILLEGAL);
        }

        BoardState current_state = BoardState::UNDECIDED;
        int score = 0;

        int current_walls_left = players[current_id].walls_left;
        int next_walls_left = players[next_id].walls_left;

        if (current_id == id)
        {
            if (current_distance <= 1)
            {
                current_state = BoardState::WON;
            }
            else if (next_walls_left == 0 && current_distance <= next_distance)
            {
                current_state = BoardState::WON;
            }
            else if (current_walls_left == 0 && current_distance > next_distance)
            {
                current_state = BoardState::LOST;
            }
            else if (next_distance == 0)
            {
                current_state = BoardState::LOST;
            }

            int distance_score = next_distance - current_distance;
            int wall_score = current_walls_left - next_walls_left;

            score = distance_score + wall_score + (current_walls_left == 0 ? -2 : 0);
        }
        else if (next_id == id)
        {
            if (next_distance == 0)
            {
                current_state = BoardState::WON;
            }
            else if (current_walls_left == 0 && next_distance < current_distance)
            {
                current_state = BoardState::WON;
            }
            else if (next_walls_left == 0 && next_distance >= current_distance)
            {
                current_state = BoardState::LOST;
            }
            else if (current_distance <= 1)
            {
                current_state = BoardState::LOST;
            }

            int distance_score = current_distance - next_distance;
            int wall_score = next_walls_left - current_walls_left;

            score = distance_score + wall_score + (next_walls_left == 0 ? -2 : 0);
        }
        else
        {
            cerr << "ERROR" << endl;
        }
        undo_move(move);
        return Score(score, current_state);
    }

    Score score_move_3_players(int id, Move move)
    {
        int current_id = get_next_id(move.id);
        int next_id = get_next_id(current_id);
        int last_id = get_next_id(next_id);

        do_move(move);

        BoardState first_place_state = BoardState::UNDECIDED;
        BoardState second_place_state = BoardState::UNDECIDED;
        int score = 0;
        int current_distance = get_distance(current_id);
        if (current_distance == UNREACHABLE)
        {
            undo_move(move);
            return Score(0, BoardState::ILLEGAL);
        }

        int next_distance = get_distance(next_id);
        if (next_distance == UNREACHABLE)
        {
            undo_move(move);
            return Score(0, BoardState::ILLEGAL);
        }

        int last_distance = get_distance(last_id);
        if (last_distance == UNREACHABLE)
        {
            undo_move(move);
            return Score(0, BoardState::ILLEGAL);
        }

        int current_walls_left = players[current_id].walls_left;
        int next_walls_left = players[next_id].walls_left;
        int last_walls_left = players[last_id].walls_left;

        int distance_weight = 3;
        int wall_weight = 4;
        if (id == current_id)
        {
            // I finish next turn
            if (current_distance <= 1)
            {
                first_place_state = BoardState::WON;
            }

            // opponents are out of walls and I'm the closest
            else if (next_walls_left == 0 && last_walls_left == 0 &&
                     current_distance <= next_distance &&
                     current_distance <= last_distance)
            {
                first_place_state = BoardState::WON;
            }
            // one opponent is in goal
            else if (next_distance == 0)
            {
                undo_move(move);
                players[next_id].is_alive = false;
                Score score = score_move_2_players(id, move);
                players[next_id].is_alive = true;
                return Score(score.score, BoardState::LOST, score.first_place_state);
            }
            else if (last_distance == 0)
            {
                undo_move(move);
                players[last_id].is_alive = false;
                Score score = score_move_2_players(id, move);
                players[last_id].is_alive = true;
                return Score(score.score, BoardState::LOST, score.first_place_state);
            }
            // im out of walls and the next can finish next turn
            else if (current_walls_left == 0 && next_distance <= 1)
            {
                undo_move(move);
                players[next_id].is_alive = false;
                Score score = score_move_2_players(id, move);
                players[next_id].is_alive = true;
                return Score(score.score, BoardState::LOST, score.first_place_state);
            }
            // I and the next is out of walls and the last can finish next turn
            else if (current_walls_left == 0 && next_walls_left == 0 &&
                     last_distance <= 1)
            {
                undo_move(move);
                players[last_id].is_alive = false;
                Score score = score_move_2_players(id, move);
                players[last_id].is_alive = true;
                return Score(score.score, BoardState::LOST, score.first_place_state);
            }

            int distance_score = next_distance + last_distance - current_distance * 2;
            int wall_score = 2 * current_walls_left - next_distance - last_walls_left;

            score = distance_score * distance_weight + wall_score * wall_weight;
        }
        else if (id == next_id)
        {
            // I finish next turn
            if (next_distance == 0 || (next_distance == 1 && current_distance >= 2 &&
                                       current_walls_left == 0))
            {
                first_place_state = BoardState::WON;
            }

            // opponents are out of walls and I'm the closest
            else if (current_walls_left == 0 && last_walls_left == 0 &&
                     next_distance < current_distance && next_distance <= last_distance)
            {
                first_place_state = BoardState::WON;
            }
            // one opponent is in goal
            else if (current_distance <= 1)
            {
                undo_move(move);
                players[current_id].is_alive = false;
                Score score = score_move_2_players(id, move);
                players[current_id].is_alive = true;
                return Score(score.score, BoardState::LOST, score.first_place_state);
            }
            else if (last_distance == 0)
            {
                undo_move(move);
                players[last_id].is_alive = false;
                Score score = score_move_2_players(id, move);
                players[last_id].is_alive = true;
                return Score(score.score, BoardState::LOST, score.first_place_state);
            }
            // I and the current is out of walls and the last can finish next turn
            else if (current_walls_left == 0 && next_walls_left == 0 &&
                     last_distance <= 1)
            {
                undo_move(move);
                players[last_id].is_alive = false;
                Score score = score_move_2_players(id, move);
                players[last_id].is_alive = true;
                return Score(score.score, BoardState::LOST, score.first_place_state);
            }

            int distance_score = current_distance + last_distance - next_distance * 2;
            int wall_score = 2 * next_walls_left - current_distance - last_walls_left;

            score = distance_score * distance_weight + wall_score * wall_weight;
        }
        else if (id == last_id)
        {

            // I finish next turn
            if (last_distance == 0 ||
                (last_distance == 1 && current_distance >= 2 && next_distance >= 2 &&
                 current_walls_left == 0 && next_walls_left == 0))
            {
                first_place_state = BoardState::WON;
            }
            // opponents are out of walls and I'm the closest
            else if (current_walls_left == 0 && next_walls_left == 0 &&
                     last_distance < current_distance && last_distance < next_distance)
            {
                first_place_state = BoardState::WON;
            }
            // Opponents finishes next turn
            else if (current_distance <= 1)
            {
                undo_move(move);
                players[current_id].is_alive = false;
                Score score = score_move_2_players(id, move);
                players[current_id].is_alive = true;
                return Score(score.score, BoardState::LOST, score.first_place_state);
            }
            else if (next_distance <= 1 && current_walls_left == 0)
            {
                undo_move(move);
                players[next_id].is_alive = false;
                Score score = score_move_2_players(id, move);
                players[next_id].is_alive = true;
                return Score(score.score, BoardState::LOST, score.first_place_state);
            }

            int distance_score = current_distance + next_distance - last_distance * 2;
            int wall_score = 2 * last_walls_left - current_walls_left - next_distance;

            score = distance_score * distance_weight + wall_score * wall_weight;
        }

        undo_move(move);
        return Score(score, first_place_state, second_place_state);
    }

    Score score_move(int depth, int breadth, Score alpha, Score beta, int id,
                     Move move)
    {
        if (temp_wall_count > 4)
            depth += 1;

        if (depth == 0 || move.score.first_place_state == BoardState::WON || (move.score.first_place_state == BoardState::LOST && move.score.second_place_state != BoardState::UNDECIDED))
        {
            Score score = move.score;
            score.depth = depth;
            return score;
        }

        do_move(move);

        int next_id = get_next_id(move.id);
        bool is_maximizing = id == next_id;
        if (is_maximizing)
        {
            MaxMovesArray *moves =
                get_maximizing_moves(id, next_id, breadth, temp_wall_count <= 4);

            Score best_score = Score(-999999, BoardState::LOST);
            for (int i = 0; i < moves->size(); i++)
            {
                Move new_move = moves->get(i);
                Score score = score_move(depth - 1, breadth, alpha, beta, id, new_move);
                if (score > best_score)
                {
                    best_score = score;

                    alpha = max(alpha, best_score);
                    if (beta <= alpha || best_score.first_place_state == BoardState::WON)
                        break;
                }
            }

            delete moves;
            undo_move(move);
            return best_score;
        }
        else
        {
            MinMovesArray *moves =
                get_minimizing_moves(id, next_id, breadth, temp_wall_count <= 4);

            Score best_score = Score(999999, BoardState::WON);
            for (int i = 0; i < moves->size(); i++)
            {
                Move new_move = moves->get(i);

                Score score = score_move(depth - 1, breadth, alpha, beta, id, new_move);
                if (score < best_score)
                {
                    best_score = score;
                    beta = min(beta, best_score);
                    if (beta <= alpha || (best_score.first_place_state == BoardState::LOST && best_score.second_place_state == BoardState::LOST))
                        break;
                }
            }

            delete moves;
            undo_move(move);
            return best_score;
        }

        undo_move(move);
        return Score(0, BoardState::ILLEGAL);
    }

    Move get_best_move(int depth, int breadth, int time_micro, int id)
    {
        auto start_time = chrono::high_resolution_clock::now();

        temp_wall_count = 0;
        MaxMovesArray *moves = get_maximizing_moves(id, id, 20, true);
        Score best_score = Score(-999999, BoardState::LOST);
        Move best_move = Move(id, Vector2(0, 0));

        Score alpha = Score(-999999, BoardState::LOST);
        for (int i = 0; i < moves->size(); i++)
        {
            Move move = moves->get(i);

            if (move.score.first_place_state == BoardState::WON)
            {
                delete moves;
                return move;
            }
            Score score = score_move(depth, breadth, alpha, Score(999999, BoardState::WON), id, move);
            move.score = score;
            // debug_move(move);
            if (score > best_score)
            {
                best_score = score;
                best_move = move;
            }

            alpha = max(alpha, best_score);
            if (alpha.first_place_state == BoardState::WON)
                break;

            auto end_time = chrono::high_resolution_clock::now();
            if (chrono::duration_cast<chrono::microseconds>(end_time - start_time)
                    .count() > time_micro)
            {
                cerr << "Time out: " << chrono::duration_cast<chrono::microseconds>(end_time - start_time).count()
                     << endl;
                cerr << "Checked " << (i + 1) << " moves" << endl;
                break;
            }
        }

        delete moves;
        best_move.score = best_score;

        // if (best_move.score == LOST)
        //     return get_best_direction(id);

        return best_move;
    }

    void print_move(Move move)
    {
        if (move.is_wall)
        {
            cout << move.wall.pos.x << " " << move.wall.pos.y << " "
                 << (move.wall.horizontal ? "H" : "V") << endl;
        }
        else
        {
            if (move.direction.x == 0 && move.direction.y == -1)
                cout << "UP" << endl;
            else if (move.direction.x == 0 && move.direction.y == 1)
                cout << "DOWN" << endl;
            else if (move.direction.x == -1 && move.direction.y == 0)
                cout << "LEFT" << endl;
            else if (move.direction.x == 1 && move.direction.y == 0)
                cout << "RIGHT" << endl;
            else
            {
                cout << "ERROR I HAVE LOST" << endl;

                // cout << move.direction.x << " " << move.direction.y << " " <<
                // move.is_wall << " " << move.id << " " << move.score << " " <<
                // move.wall.horizontal << " " << move.wall.pos.x << " " <<
                // move.wall.pos.y << " " << move.is_wall << endl; write a better
                // message
                cerr << "Is wall: " << move.is_wall
                     << " Direction: " << move.direction.x << " " << move.direction.y
                     << " Wall: " << move.wall.horizontal << " " << move.wall.pos.x
                     << " " << move.wall.pos.y << " ID: " << move.id << endl;
            }
        }
    }

    void debug_move(Move move)
    {
        if (move.is_wall)
        {
            cerr << "Wall: " << move.wall.pos.x << " " << move.wall.pos.y << " "
                 << (move.wall.horizontal ? "H" : "V");
        }
        else
        {
            if (move.direction.x == 0 && move.direction.y == -1)
                cerr << "UP";
            else if (move.direction.x == 0 && move.direction.y == 1)
                cerr << "DOWN";
            else if (move.direction.x == -1 && move.direction.y == 0)
                cerr << "LEFT";
            else if (move.direction.x == 1 && move.direction.y == 0)
                cerr << "RIGHT";
        }

        cerr << " ID: " << move.id << " Score: ";

        if (move.score.first_place_state == BoardState::WON)
            cerr << "WON ";
        else if (move.score.first_place_state == BoardState::UNDECIDED)
            cerr << "UNDECIDED ";
        else if (move.score.first_place_state == BoardState::LOST)
            cerr << "LOST ";

        if (move.score.second_place_state == BoardState::WON)
            cerr << "WON ";
        else if (move.score.second_place_state == BoardState::UNDECIDED)
            cerr << "UNDECIDED ";
        else if (move.score.second_place_state == BoardState::LOST)
            cerr << "LOST ";

        cerr << move.score.score << endl;
    }

    void print_board()
    {
        cerr << "Board:" << endl;
        for (int y = 0; y < height; y++)
        {
            cerr << " ";
            for (int x = 0; x < width; x++)
            {
                cerr << " ";
                if (grid->is_inside(Vector2(x, y - 1)) &&
                    grid->is_blocked(Vector2(x, y), Vector2(x, y - 1)))
                    cerr << "---";
                else
                    cerr << "   ";
            }
            cerr << endl;
            for (int x = 0; x < width; x++)
            {
                if (grid->is_inside(Vector2(x - 1, y)) &&
                    grid->is_blocked(Vector2(x, y), Vector2(x - 1, y)))
                    cerr << "|";
                else
                    cerr << " ";
                if (players[0].pos.x == x && players[0].pos.y == y)
                    cerr << " 0 ";
                else if (players[1].pos.x == x && players[1].pos.y == y)
                    cerr << " 1 ";
                else if (player_count == 3 && players[2].pos.x == x &&
                         players[2].pos.y == y)
                    cerr << " 2 ";
                else
                    cerr << "   ";
            }
            cerr << endl;
        }
    }

private:
    int width;
    int height;
    int player_count;
    Grid *grid;
    Player *players;
    int turn_count = 0;
    int wall_count;

    int temp_wall_count;
    int data[4] = {1, 1, 1, 1};
};

void coding_game_main()
{
    int w;            // width of the board
    int h;            // height of the board
    int player_count; // number of players (2 or 3)
    int my_id;        // id of my player (0 = 1st player, 1 = 2nd player, ...)
    cin >> w >> h >> player_count >> my_id;
    cin.ignore();

    Board board = Board(w, h, player_count);

    // game loop
    while (1)
    {
        for (int i = 0; i < player_count; i++)
        {
            int x;          // x-coordinate of the player
            int y;          // y-coordinate of the player
            int walls_left; // number of walls available for the player
            cin >> x >> y >> walls_left;
            cin.ignore();
            board.update_player(i, Vector2(x, y), walls_left);
        }

        int wall_count; // number of walls on the board
        cin >> wall_count;
        cin.ignore();
        for (int i = 0; i < wall_count; i++)
        {
            int wall_x;              // x-coordinate of the wall
            int wall_y;              // y-coordinate of the wall
            string wall_orientation; // wall orientation ('H' or 'V')
            cin >> wall_x >> wall_y >> wall_orientation;
            cin.ignore();
            board.place_wall(Wall(Vector2(wall_x, wall_y), wall_orientation == "H"));
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        // action: LEFT, RIGHT, UP, DOWN or "putX putY putOrientation" to place a
        // wall
        // 0,08 sec
        int micros = 90000;

        Move move = board.get_num_alive() == 2
                        ? board.get_best_move(8, 2, micros, my_id)
                        : board.get_best_move(6, 2, micros, my_id);
        cerr << board.get_num_alive() << endl;
        // board.print_board();
        // cerr << "Move: " << move.score << endl;
        board.print_move(move);
    }
}

int main() { coding_game_main(); }