#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <deque>
#include <chrono>

#define UNREACHABLE -1
#define UNVISITED -1
#define WON 999999
#define LOST -999999

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
    }

    void update_player(int id, Vector2 pos, int walls_left)
    {
        if (id < 0 || id >= player_count)
            return;

        if (pos.x == -1 || pos.y == -1)
            players[id].is_alive = false;

        players[id].pos = pos;
        players[id].walls_left = walls_left;
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
            if (players[i].is_alive && grid->get_distance(players[i].pos, players[i].end_direction) == 0)
                return true;
        }

        return false;
    }

    bool has_won(int id)
    {
        if (is_finished())
            return false;
        return players[id].is_alive && grid->get_distance(players[id].pos, players[id].end_direction) == 0;
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

    bool can_place_wall(Wall wall)
    {
        // Check if wall is overlaping
        if (is_overlaping(wall))
            return false;

        // Make sure there is a path for each player
        int min_walls_needed = min(width / 2, height / 2);
        if (wall_count >= min_walls_needed)
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
            players[move.id].walls_left--;
            place_wall(move.wall);
        }
        else
        {
            move_player(move.id, move.direction);
        }
    }

    void undo_move(Move move)
    {
        if (move.is_wall)
        {
            players[move.id].walls_left++;
            remove_wall(move.wall);
        }
        else
        {
            move_player(move.id, -move.direction);
        }
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

    vector<Move> get_possible_moves(int id)
    {
        vector<Move> moves;

        // Add possible directions
        vector<Vector2> directions = get_possible_directions(id);
        // cerr << "directions: " << directions.size() << endl;
        for (Vector2 direction : directions)
            moves.push_back(Move(id, direction));

        if (players[id].walls_left == 0)
            return moves;

        // Add possible walls
        vector<Wall> walls = get_possible_walls();
        // cerr << "walls: " << walls.size() << endl;
        for (Wall wall : walls)
            moves.push_back(Move(id, wall));

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

    int score_move(int id, Move move)
    {
        do_move(move);

        int distance = get_distance(id);

        if (distance == 0)
        {
            undo_move(move);
            return WON;
        }

        int shortest_distance = width * height + 1;
        int enemy_walls = 0;
        for (int i = 0; i < player_count; i++)
        {
            if (i != id && players[i].is_alive)
            {
                int other_distance = get_distance(i);
                // cout << "other_distance: " << other_distance << endl;
                if (other_distance < shortest_distance)
                    shortest_distance = other_distance;

                enemy_walls += players[i].walls_left;
            }
        }

        if (shortest_distance == 0)
        {
            undo_move(move);
            return LOST;
        }

        int score = shortest_distance - distance;
        score *= 100;

        score += players[id].walls_left - enemy_walls;

        undo_move(move);
        return score;
    }

    int score_move(int depth, int breadth, int alpha, int beta, int id, Move move)
    {
        // print out move
        /*if (!move.is_wall)
        {
            cerr << "Move " << move.direction.x << " " << move.direction.y;
        }
        else
        {
            cerr << "Wall " << move.wall.pos.x << " " << move.wall.pos.y << " " << (move.wall.horizontal ? "H" : "V");
        }
        cerr << " "
             << "depth:" << depth << " breadth: " << breadth << " alpha: " << alpha << " beta: " << beta << " id: " << id << " move id: " << move.id << endl;
*/
        // cout << "depth: " << depth << " breadth: " << breadth << " alpha: " << alpha << " beta: " << beta << " id: " << id << " move: " << move.is_wall << endl;
        if (depth == 0)
            return score_move(id, move);

        if (is_finished())
        {
            if (has_won(id))
                return WON;
            else
                return LOST;
        }

        do_move(move);
        int next_id = get_next_id(move.id);
        vector<Move> moves = get_possible_moves(next_id);
        // cerr << "moves: " << moves.size() << endl;
        if (id == next_id)
        {
            int best_score = LOST;
            for (Move move : moves)
            {
                int score = score_move(depth - 1, breadth, alpha, beta, id, move);
                best_score = max(best_score, score);

                alpha = max(alpha, best_score);
                if (beta <= alpha)
                    break;
            }

            undo_move(move);
            return best_score;
        }
        else
        {
            int best_score = WON;
            for (Move move : moves)
            {
                int score = score_move(depth - 1, breadth, alpha, beta, id, move);
                best_score = min(best_score, score);

                beta = min(beta, best_score);
                if (beta <= alpha)
                    break;
            }

            undo_move(move);
            return best_score;
        }

        undo_move(move);
        return score_move(id, move);
    }

    Move get_best_move(int depth, int breadth, int id)
    {
        vector<Move> moves = get_possible_moves(id);

        int best_score = LOST;
        Move best_move = Move(id, Vector2(0, 0));
        for (Move move : moves)
        {
            int score = score_move(depth, breadth, LOST, WON, id, move);
            if (score > best_score)
            {
                best_score = score;
                best_move = move;
            }
        }

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
        }
    }

private:
    int width;
    int height;
    int player_count;
    Grid *grid;
    Player *players;
    int wall_count;
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

        // action: LEFT, RIGHT, UP, DOWN or "putX putY putOrientation" to place a wall
        Move move = board.get_best_move(1, 1, my_id);
        board.print_move(move);
    }
}

void test_main()
{
    Board board = Board(9, 9, 2);
    board.update_player(0, Vector2(0, 0), 10);
    board.update_player(1, Vector2(8, 8), 10);

    Move move = board.get_best_move(2, 1, 0);
    board.print_move(move);
}

int main()
{
    test_main();
}