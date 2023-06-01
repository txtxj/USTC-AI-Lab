#include <iostream>
#include <bitset>
#include <cmath>
#include <queue>
#include <fstream>
#include <chrono>
#include <unordered_set>

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
#ifdef DIJKSTRA
		return 0;
#else
		if (h > 0) { return h; }
#ifdef UNADMISSIBLE
		/* Find all connected blocks and the size of each connected block. */
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
			group_counter[find_group(group_id[index])] += board[index];
		}
		h = 0;
		for (int i = 1; i <= id_counter; i++)
		{
			/* The minimum number of steps required for each connected block is
			 * the size of the connected block divided by 3 with the same parity as the size of the connected block. */
			h += (int)ceil(group_counter[i] / 3.0) + (((int)ceil(group_counter[i] / 3.0) ^ group_counter[i]) & 1);
		}
		return h;
#else
		/* The minimum number of steps required is the size of the map divided by 3
		 * with the same parity as the size of the map. */
		return h = ceil(board.count() / 3.0);
#endif
#endif
	}

	int operation_count(int index) const
	{
		int i = index / n;
		int j = index % n;
		int ret = 0;

		if (i > 0 && j > 0)
		{
			ret += operation[index * 4] + operation[index * 4 + 1] + operation[index * 4 + 2];
		}

		if (i > 0 && j < n - 1)
		{
			index++;
			ret += operation[index * 4] + operation[index * 4 + 1] + operation[index * 4 + 3];
			index--;
		}

		if (i < n - 1 && j > 0)
		{
			index += n;
			ret += operation[index * 4] + operation[index * 4 + 2] + operation[index * 4 + 3];
			index -= n;
		}

		if (i < n - 1 && j < n - 1)
		{
			index += n + 1;
			ret += operation[index * 4 + 1] + operation[index * 4 + 2] + operation[index * 4 + 3];
			index -= n + 1;
		}

		return ret;
	}

	int find_first() const
	{
		return board._Find_first();
	}

	std::string to_string() const
	{
		return board.to_string();
	}

	bool operator<(const node& other) const
	{
		return depth + heuristic() > other.depth + other.heuristic() ||
			(depth + heuristic() == other.depth + other.heuristic() && heuristic() > other.heuristic());
	}

	bool operator==(const node& other) const
	{
		return (board ^ other.board).none();
	}

	friend std::ostream& operator<<(std::ostream& o, const node& p)
	{
		o << p.depth << std::endl;
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				if (p.operation[(i * n + j) * 4 + 0])
				{
					o << i << ", " << j << ", 2" << std::endl;
				}
				if (p.operation[(i * n + j) * 4 + 1])
				{
					o << i << ", " << j - 1 << ", 1" << std::endl;
				}
				if (p.operation[(i * n + j) * 4 + 2])
				{
					o << i - 1 << ", " << j << ", 3" << std::endl;
				}
				if (p.operation[(i * n + j) * 4 + 3])
				{
					o << i - 1 << ", " << j - 1 << ", 4" << std::endl;
				}
			}
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
	std::unordered_set<std::string> close_list;
	open_list.push(starter);
	pop_count = 0;

	while (!open_list.empty())
	{
		node current_node = open_list.top();
		open_list.pop();
		pop_count++;

		auto old = close_list.find(current_node.to_string());

		if (old != close_list.end())
		{
			continue;
		}
		close_list.insert(current_node.to_string());

		if (current_node.depth + current_node.heuristic() > max_depth) { continue; }

		if (current_node.board.none()) { return current_node; }

		int index = current_node.find_first();
		int i = index / n;
		int j = index % n;
		current_node.h = 0;
		current_node.depth += 1;

		/* Check if this number has been flipped over 1 times. */

		int flip_counter = current_node.operation_count(index);

		if (flip_counter > 1) { continue; }

		/* Handle the 6 operations at index, and push them into open_list. */

		current_node.board.flip(index);

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

		if (i == n - 1 && j < n - 1)
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

		if (i == n - 1 && j > 0)
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
	}

	/* Never reach unless max_depth is too small. 25 is enough :). */
	return {};
}

int main()
{
	std::ios::sync_with_stdio(false);

	for (int k = 0; k <= 9; k++)
	{
		std::string input_file = "../A star/input/input" + std::to_string(k) + ".txt";
		std::string output_file = "../A star/output/output" + std::to_string(k) + ".txt";
		std::ifstream in;
		in.open(input_file);
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

		std::cout << "Test case " << k << ": n = " << n << ", depth = " << ans.depth << ", pop count = " << pop_count << ", time = " << (double)(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den * std::chrono::milliseconds::period::den << " ms" << std::endl;

		std::ofstream out;
		out.open(output_file, std::ios::ate | std::ios::out);
		out << ans;
		out.flush();
		out.close();
	}

	return 0;
}