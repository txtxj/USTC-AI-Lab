#include <iostream>
#include <bitset>
#include <cmath>
#include <queue>
#include <fstream>
#include <chrono>

constexpr int max_n = 12;

int n;
int max_depth = 25;

struct node
{
private:
    static char group_id[max_n * max_n];
    static char group_counter[max_n * max_n + 1];
    static char group_rename[max_n * max_n + 1];
public:
    std::bitset<max_n * max_n> board;
    std::bitset<max_n * max_n * 4> operation;
    int depth = 0;
    mutable int h = 0;

    node() = default;
    ~node() = default;

    char find_group(char id) const
    {
        return group_rename[id] = (id == group_rename[id]) ? group_rename[id] : find_group(group_rename[id]);
    }

    void unite(char u, char v) const
    {
        group_rename[find_group(u)] = find_group(v);
    }

    int heuristic() const
    {
        if (h > 0) { return h; }
        char id_counter = 0;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                int k = i * n + j;
                group_id[k] = 0;
                if (board[k])
                {
                    group_id[k] = ++id_counter;
                    group_rename[id_counter] = id_counter;
                    group_counter[id_counter] = 0;
                    if (i > 0 && group_id[k - n] > 0)
                    {
                        unite(id_counter, group_id[k - n]);
                    }
                    if (j > 0 && group_id[k - 1] > 0)
                    {
                        unite(id_counter, group_id[k - 1]);
                    }
                    if (i > 0 && j > 0 && group_id[k - n - 1] > 0)
                    {
                        unite(id_counter, group_id[k - n - 1]);
                    }
                    if (i > 0 && j < n - 1 && group_id[k - n + 1] > 0)
                    {
                        unite(id_counter, group_id[k - n + 1]);
                    }
                }
            }
        }
        for (int index = n * n - 1; index >= 0; index--)
        {
            group_counter[group_rename[group_id[index]]] += board[index];
        }
        int res = 0;
        for (int i = 1; i <= id_counter; i++)
        {
            res += ceil(group_counter[i] / 3.0) + (((int)ceil(group_counter[i] / 3.0) ^ group_counter[i]) & 1);
        }
        return h = (int)(ceil((res + ((board.count() ^ res) & 1)) * (8.0 / 9.0) + board.count() / (3.0 * 9.0)));
    }

    bool operator<(const node& other) const
    {
        return depth + heuristic() > other.depth + other.heuristic() ||
            (depth + heuristic() == other.depth + other.heuristic() && heuristic() > other.heuristic());
    }

    friend std::ostream& operator<<(std::ostream& o, node p)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                o << p.board[i * n + j] << " ";
            }
            o << std::endl;
        }
        return o;
    }
};

char node::group_id[max_n * max_n] = {};
char node::group_counter[max_n * max_n + 1] = {};
char node::group_rename[max_n * max_n + 1] = {};

node solve(node& starter, int& pop_count)
{
    std::priority_queue<node> open_list;
    open_list.push(starter);
    pop_count = 0;

    while (!open_list.empty())
    {
        node current_node = open_list.top();
        open_list.pop();

        pop_count++;

        if (current_node.depth + current_node.heuristic() > max_depth) { continue; }

        if (current_node.board.none()) { return current_node; }

        int index = current_node.board._Find_first();
        int i = index / n;
        int j = index % n;
        current_node.h = 0;
        current_node.depth += 1;

        // Check if this number has been flipped over 3 times

        int flip_counter = 0;

        if (i > 0 && j > 0)
        {
            flip_counter += current_node.operation[index * 4] + current_node.operation[index * 4 + 1] + current_node.operation[index * 4 + 2];
        }

        if (i > 0 && j < n - 1)
        {
            index++;
            flip_counter += current_node.operation[index * 4] + current_node.operation[index * 4 + 1] + current_node.operation[index * 4 + 3];
            index--;
        }

        if (i < n - 1 && j > 0)
        {
            index += n;
            flip_counter += current_node.operation[index * 4] + current_node.operation[index * 4 + 2] + current_node.operation[index * 4 + 3];
            index -= n;
        }

        if (i < n - 1 && j < n - 1)
        {
            index += n + 1;
            flip_counter += current_node.operation[index * 4 + 1] + current_node.operation[index * 4 + 2] + current_node.operation[index * 4 + 3];
            index -= n + 1;
        }

        if (flip_counter >= 3) { continue; }

        // Handle the 12 operations at index, and push them into open_list

        current_node.board.flip(index);

        if (i > 0 && j > 0)
        {
            if (!current_node.operation[index * 4])
            {
                current_node.operation[index * 4] = true;
                current_node.board.flip(index - 1);
                current_node.board.flip(index - n);
                open_list.push(current_node);
                current_node.board.flip(index - 1);
                current_node.board.flip(index - n);
                current_node.operation[index * 4] = false;
            }
            if (!current_node.operation[index * 4 + 1])
            {
                current_node.operation[index * 4 + 1] = true;
                current_node.board.flip(index - 1);
                current_node.board.flip(index - 1 - n);
                open_list.push(current_node);
                current_node.board.flip(index - 1);
                current_node.board.flip(index - 1 - n);
                current_node.operation[index * 4 + 1] = false;
            }
            if (!current_node.operation[index * 4 + 2])
            {
                current_node.operation[index * 4 + 2] = true;
                current_node.board.flip(index - 1 - n);
                current_node.board.flip(index - n);
                open_list.push(current_node);
                current_node.board.flip(index - 1 - n);
                current_node.board.flip(index - n);
                current_node.operation[index * 4 + 2] = false;
            }
        }

        if (i > 0 && j < n - 1)
        {
            index++;
            if (!current_node.operation[index * 4])
            {
                current_node.operation[index * 4] = true;
                current_node.board.flip(index);
                current_node.board.flip(index - n);
                open_list.push(current_node);
                current_node.board.flip(index);
                current_node.board.flip(index - n);
                current_node.operation[index * 4] = false;
            }
            if (!current_node.operation[index * 4 + 1])
            {
                current_node.operation[index * 4 + 1] = true;
                current_node.board.flip(index);
                current_node.board.flip(index - 1 - n);
                open_list.push(current_node);
                current_node.board.flip(index);
                current_node.board.flip(index - 1 - n);
                current_node.operation[index * 4 + 1] = false;
            }
            if (!current_node.operation[index * 4 + 3])
            {
                current_node.operation[index * 4 + 3] = true;
                current_node.board.flip(index - 1 - n);
                current_node.board.flip(index - n);
                open_list.push(current_node);
                current_node.board.flip(index - 1 - n);
                current_node.board.flip(index - n);
                current_node.operation[index * 4 + 3] = false;
            }
            index--;
        }

        if (i < n - 1 && j > 0)
        {
            index += n;
            if (!current_node.operation[index * 4])
            {
                current_node.operation[index * 4] = true;
                current_node.board.flip(index);
                current_node.board.flip(index - 1);
                open_list.push(current_node);
                current_node.board.flip(index);
                current_node.board.flip(index - 1);
                current_node.operation[index * 4] = false;
            }
            if (!current_node.operation[index * 4 + 2])
            {
                current_node.operation[index * 4 + 2] = true;
                current_node.board.flip(index);
                current_node.board.flip(index - 1 - n);
                open_list.push(current_node);
                current_node.board.flip(index);
                current_node.board.flip(index - 1 - n);
                current_node.operation[index * 4 + 2] = false;
            }
            if (!current_node.operation[index * 4 + 3])
            {
                current_node.operation[index * 4 + 3] = true;
                current_node.board.flip(index - 1);
                current_node.board.flip(index - 1 - n);
                open_list.push(current_node);
                current_node.board.flip(index - 1);
                current_node.board.flip(index - 1 - n);
                current_node.operation[index * 4 + 3] = false;
            }
            index -= n;
        }

        if (i < n - 1 && j < n - 1)
        {
            index += n + 1;
            if (!current_node.operation[index * 4 + 1])
            {
                current_node.operation[index * 4 + 1] = true;
                current_node.board.flip(index);
                current_node.board.flip(index - 1);
                open_list.push(current_node);
                current_node.board.flip(index);
                current_node.board.flip(index - 1);
                current_node.operation[index * 4 + 1] = false;
            }
            if (!current_node.operation[index * 4 + 2])
            {
                current_node.operation[index * 4 + 2] = true;
                current_node.board.flip(index);
                current_node.board.flip(index - n);
                open_list.push(current_node);
                current_node.board.flip(index);
                current_node.board.flip(index - n);
                current_node.operation[index * 4 + 2] = false;
            }
            if (!current_node.operation[index * 4 + 3])
            {
                current_node.operation[index * 4 + 3] = true;
                current_node.board.flip(index - 1);
                current_node.board.flip(index - n);
                open_list.push(current_node);
                current_node.board.flip(index - 1);
                current_node.board.flip(index - n);
                current_node.operation[index * 4 + 3] = false;
            }
            index -= n + 1;
        }
    }

    return {};
}

int main()
{
    std::ios::sync_with_stdio(false);

    char file_name[] = "../A star/input/input0.txt";

    for (int k = 4; k <= 4; k++)
    {
        file_name[21] = '0' + k;
        std::ifstream in;
        in.open(file_name);
        in >> n;

        node starter;
        int tmp = 0;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                in >> tmp;
                starter.board[i * n + j] = tmp;
                starter.operation[(i * n + j) * 4 + 0] = false;
                starter.operation[(i * n + j) * 4 + 1] = false;
                starter.operation[(i * n + j) * 4 + 2] = false;
                starter.operation[(i * n + j) * 4 + 3] = false;
            }
        }

        in.close();

        auto start = std::chrono::system_clock::now();
        int pop_count = 0;
        node ans = solve(starter, pop_count);
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Test case " << k << ": n = " << n << ", depth = " << ans.depth << ", pop count = " << pop_count << ", time = " << (double)(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << " seconds" << std::endl;
    }

    return 0;
}