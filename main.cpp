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
        this->blocked_paths = new bool[width * height * width * height]{false};
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
        return blocked_paths[pos1.x + pos1.y * width + pos2.x * width * height + pos2.y * width * width * height];
    }

    void set_blocked(Vector2 pos1, Vector2 pos2, bool blocked)
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
    bool *blocked_paths;
    int *visited;
    Vector2 *queue;
    int write_index;
    int read_index;
};

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

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