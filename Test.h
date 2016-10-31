
#include "TimSort.h"
#include <algorithm>
#include <string>
#include <ctime>
#include <iostream>

using std::vector;
using std::string;
using std::cout;
using std::cin;

const int N_DIFFERENT_LENS = 10;
const int LENS[N_DIFFERENT_LENS] = {1, 2, 3, 16, 100, 1000, 1024, 10000, 65536, 100000};
const int MAX_INT = 1000000000;
const int N_RANDOM_SORTS = 15;
const int N_RUN_LENS = 5;
const int RUN_LENS[N_RUN_LENS] = {20, 40, 80, 128, 1024};
const int N_RUN_NUMBERS = 6;
const int RUN_NUMBERS[N_RUN_NUMBERS] = {2, 4, 10, 100, 1000, 10000};
const int STRING_LEN = 100;

class Point3D
{
public:
    int x, y, z;

    Point3D():
        x (rand() % MAX_INT),
        y (rand() % MAX_INT),
        z (rand() % MAX_INT)

        {}

    bool operator == (const Point3D& that) const
    {
        return (x == that.x);
    }
};

class ComparePointFunctor
{
public:
    bool operator ()(Point3D& p1, Point3D& p2)
    {
        return (p1.x < p2.x);
    }
};

bool comparePointsFunction(Point3D& p1, Point3D& p2)
{
    return (p1.x > p2.x);
}

void createRandomIntArray(vector<int>& arr, int len)
{
    arr = vector<int>(len);
    
    for (int i = 0; i < len; i++)
        arr[i] = rand() % MAX_INT;
}

void createPartiallySorted(vector<int>& arr, int len)
{
    createRandomIntArray(arr, len);

    for (int k = 0; k < N_RANDOM_SORTS; k++)
    {
        int start = rand() % len;
        int finish = start + rand() % (len - start + 1);

        std::sort(arr.begin() + start, arr.begin() + finish);
    }
}

void createRunConcatenation(vector<int>& arr, int n_runs)
{
    arr.clear();

    for (int i = 0; i < n_runs; i++)
    {
        int run_len = RUN_LENS[rand() % N_RUN_LENS];
        vector<int> run;
        createRandomIntArray(run, run_len);
        std::sort(run.begin(), run.end());
        
        for (size_t j = 0; j < run.size(); j++)
            arr.push_back(run[j]);
    }
}

string createRandomString()
{
    string str;
    for (int i = 0; i < STRING_LEN; i++)
        str += ('a' + rand() % ('z' - 'a' + 1));
    return str;
}

template <class RandomAccessIterator, class Compare>
clock_t getTimSortTime(RandomAccessIterator start, RandomAccessIterator finish, Compare comp)
{
    clock_t start_time = clock();
    timSort(start, finish, comp);
    return clock() - start_time;
}

template <class RandomAccessIterator, class Compare>
clock_t getStdSortTime(RandomAccessIterator start, RandomAccessIterator finish, Compare comp)
{
    clock_t start_time = clock();
    std::sort(start, finish, comp);
    return clock() - start_time;
}


void createRandomStringArray(vector<string>& arr, int len)
{
    arr = vector<string>(len);
    for (int i = 0; i < len; i++)
        arr[i] = createRandomString();
}

template <class Type>
bool isEqualArrays(vector<Type>& arr1, vector<Type>& arr2)
{
    if (arr1.size() != arr2.size())
        return false;

    for (size_t i = 0; i < arr1.size(); i++)
    {
        if (!(arr1[i] == arr2[i]))
            return false;
    }

    return true;
};

template <class Type, class Generator, class Compare>
void testGenerator(Type val_example, vector<int> lens, Generator gen, Compare comp)
{
    cout << "Testing:\n";
    for (size_t len_i = 0; len_i < lens.size(); len_i++)
    {
        vector<Type> arr_tim;
        gen(arr_tim, lens[len_i]);
        vector<Type> arr_std(arr_tim.begin(), arr_tim.end());

        cout << "    For len " << lens[len_i] << ":\n";
        cout << "    TimSortTime: " << getTimSortTime(arr_tim.begin(), arr_tim.end(), comp);
        cout << "    StdSortTime: " << getStdSortTime(arr_std.begin(), arr_std.end(), comp);

        if (isEqualArrays(arr_tim, arr_std))
            cout << "    Test succeeded\n";
        else
            cout << "    Sorry, test failed\n";
    }
}

void testRandomIntegers()
{
    const int INT_EXAMPLE = 2;
    testGenerator(INT_EXAMPLE, vector<int>(LENS, LENS + N_DIFFERENT_LENS), createRandomIntArray, std::less<int>());
}

void testRandomString()
{
    const string STR_EXAMPLE = "abacaba";
    testGenerator(STR_EXAMPLE, vector<int>(LENS, LENS + N_DIFFERENT_LENS - 1), createRandomStringArray, std::less<string>());
}

void createRandomPointsArray(vector<Point3D>& arr, int len)
{
    arr = vector<Point3D>(len);
}

void testRandomPointsFunctor()
{
    const Point3D POINT_EXAMPLE;
    testGenerator(POINT_EXAMPLE, vector<int>(LENS, LENS + N_DIFFERENT_LENS), createRandomPointsArray, ComparePointFunctor());
}

void testRandomPointsFunction()
{
    const Point3D POINT_EXAMPLE;
    testGenerator(POINT_EXAMPLE, vector<int>(LENS, LENS + N_DIFFERENT_LENS), createRandomPointsArray, comparePointsFunction);
}

void testRunConcatenation()
{
    const int INT_EXAMPLE = 2;
    testGenerator(INT_EXAMPLE, vector<int>(RUN_NUMBERS, RUN_NUMBERS + N_RUN_NUMBERS), createRunConcatenation, std::less<int>());
}

void testPartiallySorted()
{
    const int INT_EXAMPLE = 2;
    testGenerator(INT_EXAMPLE, vector<int>(LENS, LENS + N_DIFFERENT_LENS), createPartiallySorted, std::less<int>());
}