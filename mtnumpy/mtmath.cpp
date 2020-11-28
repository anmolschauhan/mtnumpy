#include "mtmath.h"
#include <numeric>

template<typename T>
T agg_helper(T* arr, long long size, long start, long end, T init, std::function<T(T,T)> Op)
{
    if (end > size)
        end = size;
    T res = init;
    while (start < end)
    {
        res = Op(res,  arr[start]);
        start++;
    }
    return res;
}

template<typename T>
T mtagg_helper(T* arr, long long size, int num_threads, T init, std::function<T(T, T)> Op)
{
    auto st = std::chrono::high_resolution_clock::now();
    std::vector<std::future<T>> results;
    long size_per_thread = (long)ceil((double)size / (double)num_threads);
    long start = 0, end = size_per_thread;
    while (start < size)
    {
        results.push_back(std::async(std::launch::async, agg_helper<T>, std::ref(arr), size, start, end, init, Op));
        start += size_per_thread;
        end += size_per_thread;
    }
    T res = init;
    for (auto& i : results)
    {
        res = Op(res, i.get());
    }
    return res;
}

template<typename T>
void agg_helper_other(T* res, T* arr, long long size, T other, long start, long end, std::function<T(T, T)> Op)
{
    if (end > size)
        end = size;
    while (start < end)
    {
        res[start] = Op(arr[start],  other);
        start++;
    }
}

template<typename T>
void mtagg_helper(T* res, T* arr, long long size, T other, int num_threads, std::function<T(T, T)> Op)
{
    auto st = std::chrono::high_resolution_clock::now();
    std::vector<std::future<void>> results;
    long size_per_thread = (long)ceil((double)size / (double)num_threads);
    long start = 0, end = size_per_thread;
    while (start < size)
    {
        results.push_back(std::async(std::launch::async, agg_helper_other<T>, std::ref(res), std::ref(arr), size, other, start, end, Op));
        start += size_per_thread;
        end += size_per_thread;
    }
    for (auto& i : results)
    {
        i.get();
    }
    auto en = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(en - st);
}

template<typename T>
int random_helper(T* arr, long long size, long start, long end)
{
    T lower_bound = 0;
    T upper_bound = 1;
    std::uniform_real_distribution<T> unif(lower_bound, upper_bound);
    std::default_random_engine re;
    if (end > size)
        end = size;
    T res = 0;
    while (start < end)
    {
        arr[start] = unif(re);
        start++;
    }
    return 0;
}

template<typename T>
void mtrandom_helper(T* arr, long long size, int num_threads)
{
    std::vector<std::future<int>> results;
    long size_per_thread = (long)ceil((double)size / (double)num_threads);
    long start = 0, end = size_per_thread;
    while (start < size)
    {
        results.push_back(std::async(std::launch::async, random_helper<T>, std::ref(arr), size, start, end));
        start += size_per_thread;
        end += size_per_thread;
    }
    for (auto& i : results)
    {
        i.get();
    }
}

