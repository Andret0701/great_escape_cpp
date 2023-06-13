#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <deque>
#include <chrono>

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
        this->players[0] = Player(Direction::UP);
        this->players[1] = Player(Direction::DOWN);
        if (player_count == 3)
            this->players[2] = Player(Direction::LEFT);
    }
    ~Board()
    {
        delete grid;
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

        // Later: check if wall blocks a player

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
        // Check if wall is overlaping
        if (is_overlaping(wall))
            return;

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

private:
    int width;
    int height;
    int player_count;
    Grid *grid;
    Player *players;
    int wall_count;
};

int main()
{
    // start message
    cout << "start" << endl;
    // make a grid speed test
    Grid grid(9, 9);

    grid.set_blocked(Vector2(3, 0), Vector2(4, 0), true);
    grid.set_blocked(Vector2(3, 1), Vector2(4, 1), true);
    grid.set_blocked(Vector2(3, 2), Vector2(4, 2), true);

    // time test
    // take time
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++)
    {
        grid.get_distance(Vector2(0, 0), Direction::RIGHT);
    }
    // take time
    auto end = chrono::high_resolution_clock::now();
    // print time
    cout << "Time taken by function: "
         << chrono::duration_cast<chrono::nanoseconds>(end - start).count() / 1000000
         << " milliseconds" << endl;
    // and in seconds
    cout << "Time taken by function: " << chrono::duration_cast<chrono::seconds>(end - start).count() << " seconds" << endl;

    // distance test
    cout << grid.get_distance(Vector2(0, 0), Direction::RIGHT) << endl;

    cout << "end" << endl;
}