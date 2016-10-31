/*
Run.h
Consists o operations working with blocks of data in array called Runs
1. Offers template class Run
2. Allows dividing your array into runs using ITimSortParams
3. Allows merging two runs with usual merge
4. Offers timSortSwap function
5. Offers merge methods with different types: swapping elements or just erasing those in buffer
*/

#pragma once
#include <vector>
#include <iterator>
#include <stack>
#include <functional>
#include "TimSortParams.h"

enum EWriteMethods
{
    WM_SWAP_WRITE,   
    WM_ERASING_WRITE
};

template <class Type>
void timSortSwap(Type& a, Type& b)
{
    Type c(a);
    a = b;
    b = c;
}

template <class RandomAccessIterator>
struct Run
{
    RandomAccessIterator start;
    int size;
};

template <class RandomAccessIterator, class Compare>
void divideArrayToRuns(RandomAccessIterator start, RandomAccessIterator finish, 
                       std::vector<Run<RandomAccessIterator>>& runs,
                       Compare comp, const ITimSortParams& params = DEFAULT_PARAMS)
{
    RandomAccessIterator curr_start = start;
    RandomAccessIterator curr_finish = start + 1;
    int min_run = params.minRun(finish - start);

    while (curr_start != finish)
    {
        if (curr_finish == finish)
        {
            Run<RandomAccessIterator> new_run = {curr_start, finish - curr_start};
            runs.push_back(new_run);
            break;
        }

        if (comp(*curr_start, *curr_finish))
        {
            while (curr_finish != finish && comp(*(curr_finish - 1), *curr_finish))
                curr_finish++;
        }
        else
        {
            while (curr_finish != finish && comp(*curr_finish, *(curr_finish - 1)))
                curr_finish++;
            reverseArrayPart(curr_start, curr_finish);
        }

        while (curr_finish != finish && curr_finish - curr_start < min_run)
            curr_finish++;

        Run<RandomAccessIterator> new_run = {curr_start, curr_finish - curr_start};
        insertionSort(curr_start, curr_finish, comp);
        runs.push_back(new_run);
        curr_start = curr_finish;

        if (curr_finish != finish)
            curr_finish++;
        else
            break;
    }
}

template <class RandomAccessIterator>
void divideArrayToRuns(RandomAccessIterator start, RandomAccessIterator finish, 
                       std::vector<Run<RandomAccessIterator>>& runs, const ITimSortParams& params = DEFAULT_PARAMS)
{
    divideArrayToRuns(start, finish, runs,
                      std::less_equal<std::iterator_traits<RandomAccessIterator>::value_type>(), params);
}

template <class Type>
void writeToDestination(Type& dest, Type& src, EWriteMethods write_type)
{
    if (write_type == WM_SWAP_WRITE)
    {
        Type temp = src;
        src = dest;
        dest = temp;
    }
    else
        dest = src;
}

template <class RandomAccessIterator, class BufferIterator, class Compare>
void mergeRunsWithBuffer(Run<RandomAccessIterator> left, Run<RandomAccessIterator> right,
                         BufferIterator buffer, Compare comp, EWriteMethods write_type = WM_ERASING_WRITE)
{
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    for (int i = 0; i < right.start - left.start; i++)
        writeToDestination(buffer[i], left.start[i], write_type);

    RandomAccessIterator dest = left.start;
    BufferIterator left_ptr = buffer;
    RandomAccessIterator right_ptr = right.start;

    while (left_ptr - buffer < left.size && right_ptr - right.start < right.size)
    {
        writeToDestination(*(dest++), (comp(*left_ptr, *right_ptr)) ? *(left_ptr++) : *(right_ptr++), write_type);
    }

    if (left_ptr - buffer == left.size)
    {
        while (right_ptr - right.start < right.size)
            writeToDestination(*(dest++), *(right_ptr++), write_type);
    }
    else
    {
        while (left_ptr - buffer < left.size)
            writeToDestination(*(dest++), *(left_ptr++), write_type);
    }
}

template <class RandomAccessIterator, class Compare>
void mergeRuns(Run<RandomAccessIterator> left, Run<RandomAccessIterator> right, Compare comp, EWriteMethods write_type = WM_ERASING_WRITE)
{
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    ValueType* buf = new ValueType[right.start - left.start];
    for (int i = 0; i < right.start - left.start; i++)
        buf[i] = left.start[i];

    mergeRunsWithBuffer(left, right, buf, comp, write_type);

    delete[] buf;
}

template <class RandomAccessIterator>
void mergeRuns(Run<RandomAccessIterator> left, Run<RandomAccessIterator> right, EWriteMethods write_type = WM_ERASING_WRITE)
{
    mergeRuns(left, right, std::less_equal<typename std::iterator_traits<RandomAccessIterator>::value_type>(),
              write_type);
}   