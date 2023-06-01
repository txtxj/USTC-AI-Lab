#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>

constexpr int max_size[] = {196, 360, 6};

int n, d, s;
int min_limit, max_limit;
int satisfied = 0;

bool requests[max_size[0]][max_size[1]][max_size[2]] = {};
int shifts[max_size[1]][max_size[2]] = {};
int ans[max_size[1]][max_size[2]] = {};
int duty_count[max_size[0]] = {};
int request_count[max_size[0]] = {};

bool cmp(int x, int y)
{
	/* Priority is given to those who already have less workload
	 * and to those who have fewer remaining requests in case of equal workload. */
	return duty_count[x] == duty_count[y] ? request_count[x] < request_count[y] : duty_count[x] < duty_count[y];
}

bool check()
{
	/* The constraint of not being able to work two consecutive shifts has been satisfied during the search,
	 * Here only need to determine if each aunt is assigned min_limit number of shifts. */
	for (int i = 0; i < n; i++)
	{
		if (duty_count[i] < min_limit)
		{
			return false;
		}
	}
	satisfied = 0;
	for (int i = 0; i < d; i++)
	{
		for (int j = 0; j < s; j++)
		{
			satisfied += requests[shifts[i][j]][i][j];
		}
	}

	std::copy(&shifts[0][0], &shifts[0][0] + max_size[1] * max_size[2], &ans[0][0]);
	return true;
}

bool solve(int dd, int ss);

inline bool try_assign(int nn, int dd, int ss)
{
	shifts[dd][ss] = nn;
	duty_count[nn]++;
	for (int i = 0; i < n; i++)
	{
		request_count[i] -= requests[i][dd][ss];
	}
	if (dd == d - 1 && ss == s - 1)
	{
		bool ret = check();
		shifts[dd][ss] = -1;
		duty_count[nn]--;
		for (int i = 0; i < n; i++)
		{
			request_count[i] += requests[i][dd][ss];
		}
		return ret;
	}
	bool ret = solve(dd + (ss == s - 1), ss == s - 1 ? 0 : ss + 1);
	shifts[dd][ss] = -1;
	duty_count[nn]--;
	for (int i = 0; i < n; i++)
	{
		request_count[i] += requests[i][dd][ss];
	}
	return ret;
}

/* Try to search for assignments starting from the ss time period working state on day dd. */
bool solve(int dd, int ss)
{
	/* Find the value range of the current point. */
	std::vector<int> d_region;
	int last_duty = 0;
	for (int i = 0; i < n; i++)
	{
		if (shifts[(dd + (ss == s - 1)) % d][(ss + 1) % s] != i &&
			shifts[(d + dd - (ss == 0)) % d][(ss + s - 1) % s] != i &&
			duty_count[i] < max_limit)
		{
			d_region.emplace_back(i);
		}
		if (duty_count[i] < min_limit)
		{
			last_duty += min_limit - duty_count[i];
		}
	}

	/* Unable to meet the minimum workload for each individual. */
	if (last_duty > (d - dd) * s - ss)
	{
		return false;
	}

	std::sort(d_region.begin(), d_region.end(), cmp);
	auto iter = d_region.begin();
	auto end = d_region.end();
	/* Arrange first for those who request to work during this time period. */
	while (iter != end)
	{
		int nn = *iter;
		if (requests[nn][dd][ss])
		{
			bool ret = try_assign(nn, dd, ss);
			if (ret)
			{
				return ret;
			}
		}
		iter++;
	}

	iter = d_region.begin();
	end = d_region.end();
	/* Re-arrange for those who have not requested to work during this time period. */
	while (iter != end)
	{
		int nn = *iter;
		if (!requests[nn][dd][ss])
		{
			bool ret = try_assign(nn, dd, ss);
			if (ret)
			{
				return ret;
			}
		}
		iter++;
	}
	return false;
}

inline int read_int(std::istream& in)
{
	char c;
	int x = 0;
	in.get(c);
	while ('0' <= c && c <= '9')
	{
		x = (x << 3) + (x << 1) + (c ^ 48);
		in.get(c);
	}
	return x;
}

int main()
{
	std::ios::sync_with_stdio(false);

	std::string tmp_str;

	for (int k = 0; k <= 9; k++)
	{
		std::string input_file = "../CSP/input/input" + std::to_string(k) + ".txt";
		std::string output_file = "../CSP/output/output" + std::to_string(k) + ".txt";
		std::ifstream in;
		in.open(input_file);

		n = read_int(in);
		d = read_int(in);
		s = read_int(in);
		min_limit = d * s / n;
		max_limit = d * s - min_limit * (n - 1);

		for (int i = 0; i < n; i++)
		{
			request_count[i] = 0;
			for (int j = 0; j < d; j++)
			{
				for (int e = 0; e < s; e++)
				{
					requests[i][j][e] = read_int(in);
					request_count[i] += requests[i][j][e];
					shifts[j][e] = -1;
				}
			}
			duty_count[i] = 0;
		}
		in.close();

		std::ofstream out;
		out.open(output_file, std::ios::ate | std::ios::out);

		auto start = std::chrono::system_clock::now();

		if (solve(0, 0))
		{
			for (int i = 0; i < d; i++)
			{
				for (int j = 0; j < s; j++)
				{
					out << ans[i][j] + 1 << " ";
				}
				out << std::endl;
			}
			out << satisfied << std::endl;
		}
		else
		{
			out << "No valid schedule found." << std::endl;
		}

		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		std::cout << "Test case " << k << ", satisfied = " << satisfied << "/" << d * s << ", time = " << (double)(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << " seconds" << std::endl;

		out.flush();
		out.close();
	}

	return 0;
}