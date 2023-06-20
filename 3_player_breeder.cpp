#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <deque>
#include <chrono>
#include <queue>
#include <random>

#define UNREACHABLE -1
#define UNVISITED -1
#define WON 99999999
#define LOST -99999999

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

    Vector2 operator-() const
    {
        return Vector2(-x, -y);
    }
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

    int get_index(Vector2 pos)
    {
        return pos.x + pos.y * width;
    }

    bool is_blocked(Vector2 pos1, Vector2 pos2)
    {
        // will crash if pos1 or pos2 are outside the grid
        return blocked_paths[pos1.x + pos1.y * width + pos2.x * width * height + pos2.y * width * width * height] != 0;
    }

    int get_blocked(Vector2 pos1, Vector2 pos2)
    {
        // will crash if pos1 or pos2 are outside the grid
        return blocked_paths[pos1.x + pos1.y * width + pos2.x * width * height + pos2.y * width * width * height];
    }

    void set_blocked(Vector2 pos1, Vector2 pos2, bool blocked)
    {
        // Exploit symmetry
        blocked_paths[pos1.x + pos1.y * width + pos2.x * width * height + pos2.y * width * width * height] = blocked;
        blocked_paths[pos2.x + pos2.y * width + pos1.x * width * height + pos1.y * width * width * height] = blocked;
    }

    void set_blocked(Vector2 pos1, Vector2 pos2, int blocked)
    {
        // Exploit symmetry
        blocked_paths[pos1.x + pos1.y * width + pos2.x * width * height + pos2.y * width * width * height] = blocked;
        blocked_paths[pos2.x + pos2.y * width + pos1.x * width * height + pos1.y * width * width * height] = blocked;
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
            if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(current, next))
            {
                visited[next_index] = next_distance;
                queue[write_index++] = next;
            }

            next = Vector2(current.x, current.y + 1);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(current, next))
            {
                visited[next_index] = next_distance;
                queue[write_index++] = next;
            }

            next = Vector2(current.x - 1, current.y);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(current, next))
            {
                visited[next_index] = next_distance;
                queue[write_index++] = next;
            }

            next = Vector2(current.x + 1, current.y);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(current, next))
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
        if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(pos, next))
        {
            visited[next_index] = (int)Direction::UP;
            queue[write_index++] = next;
        }

        next = Vector2(pos.x, pos.y + 1);
        next_index = get_index(next);
        if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(pos, next))
        {
            visited[next_index] = (int)Direction::DOWN;
            queue[write_index++] = next;
        }

        next = Vector2(pos.x - 1, pos.y);
        next_index = get_index(next);
        if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(pos, next))
        {
            visited[next_index] = (int)Direction::LEFT;
            queue[write_index++] = next;
        }

        next = Vector2(pos.x + 1, pos.y);
        next_index = get_index(next);
        if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(pos, next))
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
            if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(current, next))
            {
                visited[next_index] = parent;
                queue[write_index++] = next;
            }

            next = Vector2(current.x, current.y + 1);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(current, next))
            {
                visited[next_index] = parent;
                queue[write_index++] = next;
            }

            next = Vector2(current.x - 1, current.y);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(current, next))
            {
                visited[next_index] = parent;
                queue[write_index++] = next;
            }

            next = Vector2(current.x + 1, current.y);
            next_index = get_index(next);
            if (is_inside(next) && visited[next_index] == UNVISITED && !is_blocked(current, next))
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

struct Move
{
    int score;
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
        this->max_score = LOST - 1;
        this->max_index = 0;
    }

    ~MinMovesArray()
    {
        delete[] moves;
    }

    int size()
    {
        return num_elements;
    }

    void push(Move move)
    {
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

    Move get(int index)
    {
        return moves[index];
    }
    void sort()
    {
        std::sort(moves, moves + num_elements, [](const Move &a, const Move &b)
                  { return a.score < b.score; });
    }

private:
    int _size;
    Move *moves;
    int num_elements;
    int max_score;
    int max_index;

    void update_max()
    {
        max_score = LOST - 1;
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
        this->min_score = WON + 1;
        this->min_index = 0;
    }

    ~MaxMovesArray()
    {
        delete[] moves;
    }

    int size()
    {
        return num_elements;
    }

    void push(Move move)
    {
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

    Move get(int index)
    {
        return moves[index];
    }

    void sort()
    {
        std::sort(moves, moves + num_elements, [](const Move &a, const Move &b)
                  { return a.score > b.score; });
    }

private:
    int _size;
    Move *moves;
    int num_elements;
    int min_score;
    int min_index;

    void update_min()
    {
        min_score = WON + 1;
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
    Vector2 pos;
    int walls_left;
    Direction end_direction;

    Player() : Player(Direction::UP) {}
    Player(Direction end_direction)
    {
        this->is_alive = true;
        this->pos = Vector2(0, 0);
        this->walls_left = 0;
        this->end_direction = end_direction;
    }
};

struct Score
{
    int score;
    int depth;

    Score() : Score(0, 0) {}
    Score(int score, int depth)
    {
        this->score = score;
        this->depth = depth;
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
    ~Board()
    {
        delete grid;
    }

    void move_player(int id, Vector2 direction)
    {
        if (id < 0 || id >= player_count)
            return;

        players[id].pos = players[id].pos + direction;
        players[id].is_alive = !player_is_at_end(id);
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
            if (players[i].is_alive && grid->get_distance(players[i].pos, players[i].end_direction) == UNREACHABLE)
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
            if (grid->is_blocked(wall.pos, wall.pos + Vector2(0, -1)) || grid->is_blocked(wall.pos + Vector2(1, 0), wall.pos + Vector2(1, -1)))
                return true;

            if (grid->get_blocked(wall.pos, wall.pos + Vector2(1, 0)) == grid->get_blocked(wall.pos + Vector2(0, -1), wall.pos + Vector2(1, -1)) && grid->is_blocked(wall.pos, wall.pos + Vector2(1, 0)))
                return true;
        }
        else
        {
            if (grid->is_blocked(wall.pos, wall.pos + Vector2(-1, 0)) || grid->is_blocked(wall.pos + Vector2(0, 1), wall.pos + Vector2(-1, 1)))
                return true;

            if (grid->get_blocked(wall.pos, wall.pos + Vector2(0, 1)) == grid->get_blocked(wall.pos + Vector2(-1, 0), wall.pos + Vector2(-1, 1)) && grid->is_blocked(wall.pos, wall.pos + Vector2(0, 1)))
                return true;
        }

        return false;
    }

    bool is_wall_distance(Wall wall, int distance)
    {
        if (players[0].is_alive)
        {
            int distance_1 = abs(wall.pos.x - players[0].pos.x) + abs(wall.pos.y - players[0].pos.y);
            if (distance_1 <= distance)
                return true;
        }

        if (players[1].is_alive)
        {
            int distance_2 = abs(wall.pos.x - players[1].pos.x) + abs(wall.pos.y - players[1].pos.y);
            if (distance_2 <= distance)
                return true;
        }

        if (player_count == 3 && players[2].is_alive)
        {
            int distance_3 = abs(wall.pos.x - players[2].pos.x) + abs(wall.pos.y - players[2].pos.y);
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
            grid->set_blocked(wall.pos + Vector2(1, 0), wall.pos + Vector2(1, -1), wall_count);
        }
        else
        {
            grid->set_blocked(wall.pos, wall.pos + Vector2(-1, 0), wall_count);
            grid->set_blocked(wall.pos + Vector2(0, 1), wall.pos + Vector2(-1, 1), wall_count);
        }
    }

    void remove_wall(Wall wall)
    {
        // Place wall
        if (wall.horizontal)
        {
            grid->set_blocked(wall.pos, wall.pos + Vector2(0, -1), false);
            grid->set_blocked(wall.pos + Vector2(1, 0), wall.pos + Vector2(1, -1), false);
        }
        else
        {
            grid->set_blocked(wall.pos, wall.pos + Vector2(-1, 0), false);
            grid->set_blocked(wall.pos + Vector2(0, 1), wall.pos + Vector2(-1, 1), false);
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

    vector<Wall> get_possible_walls()
    {
        vector<Wall> walls;

        // Horizontal walls
        for (int y = 1; y < height; y++)
        {
            for (int x = 0; x < width - 1; x++)
            {
                Wall wall = Wall(Vector2(x, y), true);
                if (can_place_wall(wall))
                    walls.push_back(wall);
            }
        }

        // Vertical walls
        for (int y = 0; y < height - 1; y++)
        {
            for (int x = 1; x < width; x++)
            {
                Wall wall = Wall(Vector2(x, y), false);
                if (can_place_wall(wall))
                    walls.push_back(wall);
            }
        }

        return walls;
    }

    // not used
    vector<Vector2> get_possible_directions(int id)
    {
        vector<Vector2> directions;
        Vector2 pos = players[id].pos;

        Vector2 up = Vector2(0, -1);
        if (grid->is_inside(pos + up) && !grid->is_blocked(pos, pos + up))
            directions.push_back(up);

        Vector2 down = Vector2(0, 1);
        if (grid->is_inside(pos + down) && !grid->is_blocked(pos, pos + down))
            directions.push_back(down);

        Vector2 left = Vector2(-1, 0);
        if (grid->is_inside(pos + left) && !grid->is_blocked(pos, pos + left))
            directions.push_back(left);

        Vector2 right = Vector2(1, 0);
        if (grid->is_inside(pos + right) && !grid->is_blocked(pos, pos + right))
            directions.push_back(right);

        return directions;
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

    vector<Move> get_possible_moves(int my_id, int current_id)
    {
        vector<Move> moves;
        /*
        // Add possible directions
        vector<Vector2> directions = get_possible_directions(id);
        // cerr << "directions: " << directions.size() << endl;
        for (Vector2 direction : directions)
            moves.push_back(Move(id, direction));
        */
        if (players[current_id].walls_left != 0)
        {

            // Add possible walls
            vector<Wall> walls = get_possible_walls();
            // cerr << "walls: " << walls.size() << endl;
            for (Wall wall : walls)
                moves.push_back(Move(current_id, wall));
        }

        moves.push_back(get_best_direction(current_id));
        // score each move
        for (Move &move : moves)
            move.score = score_move(my_id, move);

        return moves;
    }

    // lost is best
    MinMovesArray *get_minimizing_moves(int my_id, int current_id, int breadth)
    {
        Move direction = get_best_direction(current_id);
        direction.score = score_move(my_id, direction);
        if (direction.score == LOST) // || temp_wall_count >= 4)
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
                    if (!is_overlaping(wall) && is_wall_distance(wall, 3)) // 6))
                    {
                        Move wall_move = Move(current_id, wall);
                        wall_move.score = score_move(my_id, wall_move);
                        if (wall_move.score == LOST)
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
                    if (!is_overlaping(wall) && is_wall_distance(wall, 3)) // 6))
                    {
                        Move wall_move = Move(current_id, wall);
                        wall_move.score = score_move(my_id, wall_move);
                        if (wall_move.score == LOST)
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
    MaxMovesArray *get_maximizing_moves(int my_id, int current_id, int breadth)
    {

        Move direction = get_best_direction(current_id);
        direction.score = score_move(my_id, direction);
        if (direction.score == WON) // || temp_wall_count >= 4)
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
                    if (!is_overlaping(wall) && is_wall_distance(wall, 3)) // 6))
                    {
                        Move wall_move = Move(current_id, wall);
                        wall_move.score = score_move(my_id, wall_move);
                        if (wall_move.score == WON)
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
                    if (!is_overlaping(wall) && is_wall_distance(wall, 3)) // 6))
                    {
                        Move wall_move = Move(current_id, wall);
                        wall_move.score = score_move(my_id, wall_move);
                        if (wall_move.score == WON)
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
        if (!players[id].is_alive)
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

    int score_move(int id, Move move)
    {
        if (get_num_alive() == 2)
            return score_move_2_players(id, move);
        else
            return score_move_3_players(id, move);
    }

    int score_move_2_players(int id, Move move)
    {
        do_move(move);
        int current_id = get_next_id(move.id);

        int distance = get_distance(id);
        if (distance == UNREACHABLE)
        {
            undo_move(move);
            return move.id == id ? LOST : WON;
        }

        if (current_id == id)
            distance = max(0, distance - 1);

        if (distance == 0)
        {
            undo_move(move);
            return WON;
        }

        int other_id = get_next_id(id);
        int other_distance = get_distance(other_id);
        if (other_distance == UNREACHABLE)
        {
            undo_move(move);
            return move.id == id ? LOST : WON;
        }

        int walls_left = players[id].walls_left;
        int other_walls_left = players[other_id].walls_left;

        if (other_walls_left == 0 && distance < other_distance)
        {
            undo_move(move);
            return WON;
        }
        else if (walls_left == 0 && distance >= other_distance)
        {
            undo_move(move);
            return LOST;
        }

        if (other_distance <= 1)
        {
            undo_move(move);
            return LOST;
        }

        int distance_score = other_distance - distance;

        int wall_score = walls_left - other_walls_left;

        int distance_weight = data[0];
        int wall_weight = data[1];

        if (turn_count > data[2])
        {
            distance_weight = data[3];
            wall_weight = data[4];
        }
        else if (turn_count > data[5])
        {
            distance_weight = data[6];
            wall_weight = data[7];
        }

        int score = distance_score * distance_weight + wall_score * wall_weight;

        undo_move(move);
        return score;
    }

    int score_move_3_players(int id, Move move)
    {
        do_move(move);
        int current_id = get_next_id(move.id);
        int next_id = get_next_id(current_id);
        int last_id = get_next_id(next_id);

        int current_distance = get_distance(current_id);
        if (current_distance == UNREACHABLE)
        {
            undo_move(move);
            return move.id == id ? LOST : WON;
        }

        int next_distance = get_distance(next_id);
        if (next_distance == UNREACHABLE)
        {
            undo_move(move);
            return move.id == id ? LOST : WON;
        }

        int last_distance = get_distance(last_id);
        if (last_distance == UNREACHABLE)
        {
            undo_move(move);
            return move.id == id ? LOST : WON;
        }

        int current_walls_left = players[current_id].walls_left;
        int next_walls_left = players[next_id].walls_left;
        int last_walls_left = players[last_id].walls_left;

        if (id == current_id)
        {
            // I finish next turn
            if (current_distance <= 1)
            {
                undo_move(move);
                return WON;
            }

            // opponents are out of walls and I'm the closest
            if (next_walls_left == 0 && last_walls_left == 0 && current_distance <= next_distance && current_distance <= last_distance)
            {
                undo_move(move);
                return WON;
            }

            // everybody is out of walls and I finish last
            if (current_walls_left == 0 && next_walls_left == 0 && last_walls_left == 0 && current_distance > next_distance && current_distance > last_distance)
            {
                undo_move(move);
                return LOST;
            }

            // both opponents are in goal
            if (next_distance == 0 && last_distance == 0)
            {
                undo_move(move);
                return LOST;
            }

            int distance_score = next_distance + last_distance - current_distance * 2;
            int wall_score = current_walls_left - next_distance - last_walls_left;

            int distance_weight = data[8];
            int wall_weight = data[9];

            if (turn_count > data[10])
            {
                distance_weight = data[11];
                wall_weight = data[12];
            }
            else if (turn_count > data[13])
            {
                distance_weight = data[14];
                wall_weight = data[15];
            }

            int score = distance_score * distance_weight + wall_score * wall_weight;

            undo_move(move);
            return score;
        }
        else if (id == next_id)
        {
            // I finish next turn
            if (next_distance == 0 || (next_distance == 1 && current_distance >= 2 && current_walls_left == 0))
            {
                undo_move(move);
                return WON;
            }

            // opponents are out of walls and I'm the closest
            if (current_walls_left == 0 && last_walls_left == 0 && next_distance < current_distance && next_distance <= last_distance)
            {
                undo_move(move);
                return WON;
            }

            // everybody is out of walls and I'm not the closest
            if (current_walls_left == 0 && last_walls_left == 0 && next_walls_left == 0 && next_distance >= current_distance && next_distance > last_distance)
            {
                undo_move(move);
                return LOST;
            }

            // both opponents are in goal
            if (current_distance <= 1 && last_distance == 0)
            {
                undo_move(move);
                return LOST;
            }

            int distance_score = current_distance + last_distance - next_distance * 2;
            int wall_score = next_walls_left - current_distance - last_walls_left;

            int distance_weight = data[8];
            int wall_weight = data[9];

            if (turn_count > data[10])
            {
                distance_weight = data[11];
                wall_weight = data[12];
            }
            else if (turn_count > data[13])
            {
                distance_weight = data[14];
                wall_weight = data[15];
            }

            int score = distance_score * distance_weight + wall_score * wall_weight;

            undo_move(move);
            return score;
        }
        else if (id == last_id)
        {

            // I finish next turn
            if (last_distance == 0 || (last_distance == 1 && current_distance >= 2 && next_distance >= 2 && current_walls_left == 0 && next_walls_left == 0))
            {
                undo_move(move);
                return WON;
            }

            // Opponents finishes next turn
            if (current_distance <= 1 && next_distance <= 1)
            {
                undo_move(move);
                return LOST;
            }

            // opponents are out of walls and I'm the closest
            if (current_walls_left == 0 && next_walls_left == 0 && last_distance < current_distance && last_distance < next_distance)
            {
                undo_move(move);
                return WON;
            }

            // everybody is out of walls and I'm not the closest
            if (current_walls_left == 0 && next_walls_left == 0 && last_walls_left == 0 && last_distance >= current_distance && last_distance >= next_distance)
            {
                undo_move(move);
                return LOST;
            }

            int distance_score = current_distance + next_distance - last_distance * 2;
            int wall_score = last_walls_left - current_distance - next_walls_left;

            int distance_weight = data[8];
            int wall_weight = data[9];

            if (turn_count > data[10])
            {
                distance_weight = data[11];
                wall_weight = data[12];
            }
            else if (turn_count > data[13])
            {
                distance_weight = data[14];
                wall_weight = data[15];
            }

            int score = distance_score * distance_weight + wall_score * wall_weight;

            undo_move(move);
            return score;
        }

        undo_move(move);
        return move.id == id ? LOST : WON;
    }

    int score_move_3_players_legacy(int id, Move move)
    {
        do_move(move);
        int current_id = get_next_id(move.id);

        int distance = get_distance(id);
        if (distance == UNREACHABLE)
        {
            undo_move(move);
            return move.id == id ? LOST : WON;
        }
        if (current_id == id)
            distance = max(0, distance - 1);

        if (distance == 0)
        {
            undo_move(move);
            return WON;
        }

        /*1v2 strategy was based on 1v1 with the following modification :
– If an opponent is within 2 distance of the arrival, he’s my target
– If no opponent is within 2 distance of the arrival, my target is the one with the minimal (distance + 3*number of walls left)
– Evaluation function is modified to take into account both opponents’ distances to arrival with equal weight, and my distance to arrival with double weight.*/

        int next_id = get_next_id(id);
        int last_id = get_next_id(next_id);

        int other_distance = get_distance(next_id);
        if (other_distance == UNREACHABLE)
        {
            undo_move(move);
            return move.id == id ? LOST : WON;
        }
        int last_distance = get_distance(last_id);
        if (last_distance == UNREACHABLE)
        {
            undo_move(move);
            return move.id == id ? LOST : WON;
        }

        int walls_left = players[id].walls_left;
        int other_walls_left = players[next_id].walls_left;
        int last_walls_left = players[last_id].walls_left;

        // opponents are out of walls and I'm the closest
        if (other_walls_left == 0 && last_walls_left == 0 && distance < other_distance && distance < last_distance)
        {
            undo_move(move);
            return WON;
        }

        // everybody is out of walls and I'm not the closest
        if (walls_left == 0 && other_walls_left == 0 && last_walls_left == 0 && distance >= other_distance && distance >= last_distance)
        {
            undo_move(move);
            return LOST;
        }

        // opponents finishes next turn
        if (other_distance <= 1 && last_distance <= 1)
        {
            undo_move(move);
            return LOST;
        }

        int distance_score = other_distance + last_distance - distance * 2;
        int wall_score = walls_left - other_walls_left - last_walls_left;

        int distance_weight = 2;
        int wall_weight = 2;

        if (turn_count > 3)
        {
            // distance_weight = 2;
            // wall_weight = 0;
        }

        int score = distance_score * distance_weight + wall_score * wall_weight;

        if (other_distance <= 2)
        {
            score -= 1;
        }

        if (other_distance <= 2)
        {
            score -= 1;
        }

        undo_move(move);
        return score * 2;
    }

    Score score_move(int depth, int breadth, int alpha, int beta, int id, Move move)
    {
        if (depth == 0 || move.score == WON || move.score == LOST)
            return Score(move.score, depth);

        do_move(move);

        int next_id = get_next_id(move.id);
        bool is_maximizing = id == next_id;
        if (is_maximizing)
        {
            MaxMovesArray *moves = get_maximizing_moves(id, next_id, breadth);

            Score best_score = Score(LOST, -1);
            for (int i = 0; i < moves->size(); i++)
            {
                Move new_move = moves->get(i);
                Score score = score_move(depth - 1, breadth, alpha, beta, id, new_move);
                if (score.score > best_score.score || (score.score == best_score.score && score.depth > best_score.depth))
                {
                    best_score = score;

                    alpha = max(alpha, best_score.score);
                    if (beta <= alpha || best_score.score == WON)
                        break;
                }
            }

            delete moves;
            undo_move(move);
            return best_score;
        }
        else
        {
            MinMovesArray *moves = get_minimizing_moves(id, next_id, breadth);

            Score best_score = Score(WON, -1);
            for (int i = 0; i < moves->size(); i++)
            {
                Move new_move = moves->get(i);

                Score score = score_move(depth - 1, breadth, alpha, beta, id, new_move);
                if (score.score < best_score.score || (score.score == best_score.score && score.depth > best_score.depth))
                {
                    best_score = score;
                    beta = min(beta, best_score.score);
                    if (beta <= alpha || best_score.score == LOST)
                        break;
                }
            }

            delete moves;
            undo_move(move);
            return best_score;
        }

        undo_move(move);
        return Score(score_move(id, move), depth);
    }

    Move get_best_move(int depth, int breadth, int id, int *_data)
    {
        data = _data;

        temp_wall_count = 0;
        MaxMovesArray *moves = get_maximizing_moves(id, id, breadth + 1);

        Score best_score = Score(LOST, -1);
        Move best_move = Move(id, Vector2(0, 0));
        for (int i = 0; i < moves->size(); i++)
        {
            Move move = moves->get(i);
            if (move.score == WON)
            {
                delete moves;
                return move;
            }

            Score score = score_move(depth, breadth, LOST, WON, id, move);
            if (score.score > best_score.score || (score.score == best_score.score && score.depth > best_score.depth))
            {
                best_score = score;
                best_move = move;
            }
        }

        delete moves;
        best_move.score = best_score.score;
        return best_move;
    }

    void print_move(Move move)
    {
        if (move.is_wall)
        {
            cout << move.wall.pos.x << " " << move.wall.pos.y << " " << (move.wall.horizontal ? "H" : "V") << endl;
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

                // cout << move.direction.x << " " << move.direction.y << " " << move.is_wall << " " << move.id << " " << move.score << " " << move.wall.horizontal << " " << move.wall.pos.x << " " << move.wall.pos.y << " " << move.is_wall << endl;
                // write a better message
                cerr << "Is wall: " << move.is_wall << " Direction: " << move.direction.x << " " << move.direction.y << " Wall: " << move.wall.horizontal << " " << move.wall.pos.x << " " << move.wall.pos.y << " ID: " << move.id << " Score: " << move.score << endl;
            }
        }
    }

    void debug_move(Move move)
    {
        if (move.is_wall)
        {
            cerr << "Wall move: " << move.wall.pos.x << " " << move.wall.pos.y << " " << (move.wall.horizontal ? "H" : "V") << endl;
        }
        else
        {
            cerr << "Direction move: ";
            if (move.direction.x == 0 && move.direction.y == -1)
                cerr << "UP" << endl;
            else if (move.direction.x == 0 && move.direction.y == 1)
                cerr << "DOWN" << endl;
            else if (move.direction.x == -1 && move.direction.y == 0)
                cerr << "LEFT" << endl;
            else if (move.direction.x == 1 && move.direction.y == 0)
                cerr << "RIGHT" << endl;
        }
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
                if (grid->is_inside(Vector2(x, y - 1)) && grid->is_blocked(Vector2(x, y), Vector2(x, y - 1)))
                    cerr << "---";
                else
                    cerr << "   ";
            }
            cerr << endl;
            for (int x = 0; x < width; x++)
            {
                if (grid->is_inside(Vector2(x - 1, y)) && grid->is_blocked(Vector2(x, y), Vector2(x - 1, y)))
                    cerr << "|";
                else
                    cerr << " ";
                if (players[0].pos.x == x && players[0].pos.y == y)
                    cerr << " 0 ";
                else if (players[1].pos.x == x && players[1].pos.y == y)
                    cerr << " 1 ";
                else if (player_count == 3 && players[2].pos.x == x && players[2].pos.y == y)
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
    int *data;
};

struct Species
{
    int score;
    vector<int> data;

    Species() {}

    Species(vector<int> _data)
    {
        score = 0;
        data = _data;
    }
};

void print_species(Species species)
{
    cerr << "Species - Score: " << species.score << " Data: ";
    for (int i = 0; i < species.data.size(); i++)
        cerr << species.data[i] << ", ";
    cerr << endl;
}

void get_parent(vector<Species> *species, Species *parent)
{
    int total_score = 0;
    for (int i = 0; i < species->size(); i++)
        total_score += (*species)[i].score;

    int target_score = rand() % total_score;
    int current_score = 0;
    for (int i = 0; i < species->size(); i++)
    {
        current_score += (*species)[i].score;
        if (current_score >= target_score)
        {
            *parent = (*species)[i];
            break;
        }
    }
}

main()
{
    int num_species = 10;
    int num_generations = 100000;
    int num_data = 16;

    vector<int> player_2_data = {20, -3, 1, 11, 16, 3, -28, 18};

    // bunch of random data
    vector<Species>
        species;
    for (int i = 0; i < num_species; i++)
    {
        vector<int> data;
        for (int j = 0; j < 8; j++)
            data.push_back(player_2_data[j]);

        for (int j = 8; j < num_data; j++)
            data.push_back(rand() % 50 - 25);

        data[10] = max(1, data[10]);
        data[13] = max(data[10] + 1, data[13]);

        species.push_back(Species(data));
    }

    // genetic algorithm
    for (int i = 0; i < num_generations; i++)
    {
        cerr << "Generation: " << i << endl;

        // 2 player
        for (int j = 0; j < num_species; j++)
        {
            for (int k = 0; k < num_species; k++)
            {
                if (j == k)
                    continue;

                for (int l = 0; l < num_species; l++)
                {
                    if (j == l || k == l)
                        continue;

                    Board board(9, 9, 3);

                    Species *species1 = &species[j];
                    Species *species2 = &species[k];
                    Species *species3 = &species[l];

                    int start_y_1 = rand() % 9;
                    int start_y_2 = rand() % 9;
                    int start_x_3 = rand() % 9;

                    board.update_player(0, Vector2(0, start_y_1), 6);
                    board.update_player(1, Vector2(8, start_y_2), 6);
                    board.update_player(2, Vector2(start_x_3, 0), 6);

                    int id = 2;
                    while (!board.is_finished())
                    {
                        id = board.get_next_id(id);
                        Move move = board.get_best_move(5, 2, id, id == 0 ? &species1->data[0] : (id == 1 ? &species2->data[0] : &species3->data[0]));
                        board.do_move(move);
                    }

                    if (id == 0)
                        species1->score++;
                    else if (id == 1)
                        species2->score++;
                    else
                        species3->score++;
                }
            }
        }

        // square score
        for (int j = 0; j < num_species; j++)
        {
            species[j].score *= species[j].score;
        }

        int total_score = 0;
        for (int j = 0; j < num_species; j++)
            total_score += species[j].score;

        vector<Species> new_species;
        for (int j = 0; j < num_species; j++)
        {
            Species parent1;
            Species parent2;
            get_parent(&species, &parent1);
            get_parent(&species, &parent2);

            vector<int> data;
            for (int k = 0; k < 8; k++)
                data.push_back(parent1.data[k]);

            for (int k = 8; k < num_data; k++)
            {
                if (rand() % 2 == 0)
                    data.push_back(parent1.data[k]);
                else
                    data.push_back(parent2.data[k]);
            }

            // mutation
            for (int k = 8; k < num_data; k++)
            {
                if (rand() % 100 < 12)
                    data[k] += rand() % 10 - 5;
                if (rand() % 100 < 3)
                    data[k] = rand() % 50 - 25;
            }

            data[10] = max(1, data[10]);
            data[13] = max(data[10] + 1, data[13]);

            new_species.push_back(Species(data));
        }

        sort(species.begin(), species.end(), [](const Species &a, const Species &b)
             { return a.score > b.score; });

        print_species(species[0]);

        if (i != num_generations - 1)
            species = new_species;
    }
    cerr << "Final species: " << endl;
    sort(species.begin(), species.end(), [](const Species &a, const Species &b)
         { return a.score > b.score; });

    print_species(species[0]);
}