/*
Run.h
Consists of operations working with blocks of data in array called Runs
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
                      std::less<typename std::iterator_traits<RandomAccessIterator>::value_type>(), params);
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

template <class RandomAccessIterator, class Compare>
int getFollowingLessEqual(RandomAccessIterator& start, int len, RandomAccessIterator& min_value, Compare comp)
{
    if (comp(*min_value, *start))
    {
        return 0;
    }
    //Then left is <=
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    int left = 0, right = 1;
    while (right < len && !comp(*min_value, *(start + right)))
    {
        left = right;
        right *= 2;
    }
    if (right > len)
        right = len;

    int med = (left + right) / 2;
    while (right - left > 1)
    {
        if (comp(*(start + med), *min_value))
            left = med;
        else
            right = med;
        med = (left + right) / 2;
    }
   
    return right;
}

template <class RandomAccessIterator, class Compare>
void mergeRunsWithBuffer(Run<RandomAccessIterator>& left, Run<RandomAccessIterator>& right,
                         RandomAccessIterator buffer, Compare comp, EWriteMethods write_type = WM_ERASING_WRITE,
                         int gallop = NO_GALLOPING_MODE)
{
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    for (int i = 0; i < right.start - left.start; i++)
        writeToDestination(buffer[i], left.start[i], write_type);

    RandomAccessIterator dest = left.start;
    RandomAccessIterator left_ptr = buffer;
    RandomAccessIterator right_ptr = right.start;

    int left_in_row = 0;
    int right_in_row = 0;

    while (left_ptr - buffer < left.size && right_ptr - right.start < right.size)
    {
        if (comp(*left_ptr, *right_ptr))
        {
            writeToDestination(*(dest++), *(left_ptr++), write_type);
            left_in_row++;
            right_in_row = 0;
        }
        else
        {
            writeToDestination(*(dest++), *(right_ptr++), write_type);
            right_in_row++;
            left_in_row = 0;
        }
         
        if ((right_in_row >= gallop || left_in_row >= gallop) && gallop != NO_GALLOPING_MODE &&
            left_ptr - buffer < left.size && right_ptr - right.start < right.size)
        {
            RandomAccessIterator* start_gallop = &left_ptr;
            int n_elems_galloping = 0;

            if (right_in_row >= gallop)
            {
                start_gallop = &right_ptr;
                n_elems_galloping = getFollowingLessEqual(right_ptr, right.size - (right_ptr - right.start),
                                                          left_ptr, comp);
                right_in_row = 0;
            }
            else
            {
                start_gallop = &left_ptr;
                n_elems_galloping = getFollowingLessEqual(left_ptr, left.size - (left_ptr - buffer),
                                                          right_ptr, comp);
                left_in_row = 0;
            }

            for (int i = 0; i < n_elems_galloping; i++)
            {
                writeToDestination(*(dest++), *((*start_gallop)++), write_type);
            }
        }
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

/*template <class RandomAccessIterator, class Compare>
void mergeRuns(Run<RandomAccessIterator> left, Run<RandomAccessIterator> right, Compare comp, EWriteMethods write_type = WM_ERASING_WRITE, int gallop = NO_GALLOPING_MODE)
{
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    ValueType* buf = new ValueType[right.start - left.start];
    for (int i = 0; i < right.start - left.start; i++)
        buf[i] = left.start[i];

    mergeRunsWithBuffer(left, right, buf, comp, write_type, gallop);

    delete[] buf;
}

template <class RandomAccessIterator>
void mergeRuns(Run<RandomAccessIterator> left, Run<RandomAccessIterator> right, EWriteMethods write_type = WM_ERASING_WRITE, int gallop = NO_GALLOPING_MODE)
{
    mergeRuns(left, right, std::less_equal<typename std::iterator_traits<RandomAccessIterator>::value_type>(),
              write_type, gallop);
}*/   