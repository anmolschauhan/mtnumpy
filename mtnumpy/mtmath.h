#pragma once
#include <vector>
#include <math.h>
#include <future>
#include <random>
#include <chrono>
#include <thread>
#include <iostream>


template<typename T> 
T agg_helper(T* arr, long long size, long start, long end, T init, std::function<T(T, T)> Op);

template<typename T>
T mtagg_helper(T* arr, long long size, int num_threads, T init, std::function<T(T, T)> Op);

template<typename T>
void agg_helper_other(T* res, T* arr, long long size, T other, long start, long end, std::function<T(T, T)> Op);

template<typename T>
void mtagg_helper(T* res, T* arr, long long size, T other, int num_threads, std::function<T(T, T)> Op);

template<typename T>
void agg_helper_self_and_other(T* res, T* arr1, T* arr2, long long size, long start, long end, std::function<T(T, T)> Op);

template<typename T>
void mtagg_helper1(T* res, T* arr1, T* arr2, long long size, int num_threads, std::function<T(T, T)> Op);

template<typename T>
int random_helper(T* arr, long long size, long start, long end);

template<typename T>
void mtrandom_helper(T* arr, long long size, int num_threads);

