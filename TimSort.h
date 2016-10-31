#pragma once
#include "Run.h"
#include "TimSortParams.h"
#include "InplaceMerge.h"

#define PURE =0

using std::stack;

template <class RandomAccessIterator>
void reverseArrayPart(RandomAccessIterator start, RandomAccessIterator finish)
{
    finish--;

    while (start < finish)
    {
        timSortSwap(*start, *finish);
        start++;
        finish--;
    }
}

template <class RandomAccessIterator>
void insert(RandomAccessIterator dest, RandomAccessIterator src)
{
    timSortSwap(*dest, *src);
    RandomAccessIterator right = src;
    while (right > dest + 1)
    {
        timSortSwap(*(right - 1), *right);
        right--;
    }
}

template <class RandomAccessIterator, class Compare>
void insertionSort(RandomAccessIterator start, RandomAccessIterator finish, Compare comp)
{
    int sorted_part_size = 1;
    RandomAccessIterator curr = start + 1;
    while (curr != finish && comp(*(curr - 1), *curr))
    {
        curr++;
        sorted_part_size++;
    }

    while (curr != finish)
    {
        RandomAccessIterator to_insert = start;
        while (comp(*to_insert, *curr))
            to_insert++;
        insert(to_insert, curr);
        curr++;
    }
}

template <class RandomAccessIterator>
void insertionSort(RandomAccessIterator start, RandomAccessIterator finish)
{
    insertionSort(start, finish, std::less_equal<std::iterator_traits<RandomAccessIterator>::value_type>());
}

template <class Run>
void popTo(stack<Run>& runs, Run& x)
{
    x = runs.top();
    runs.pop();
}

template <class Run, class Compare>
void replaceWithMerged(stack<Run>& runs, Run& left, Run& right, Compare comp)
{
    inplaceMerge(left, right, comp);
    left.size += right.size;
    runs.push(left);
}

template <class RandomAccessIterator, class Compare>
void timSort(RandomAccessIterator start, RandomAccessIterator finish,
             Compare comp, const ITimSortParams& params = DEFAULT_PARAMS)
{
    vector<Run<RandomAccessIterator>> runs;
    divideArrayToRuns(start, finish, runs, comp, params);
    if (runs.size() < 2)
        return;

    stack<Run<RandomAccessIterator>> run_stack;
    run_stack.push(runs[0]);
    for (size_t i = 1; i < runs.size(); i++)
    {
        run_stack.push(runs[i]);
        Run<RandomAccessIterator> x, y, z;

        popTo(run_stack, x);
        popTo(run_stack, y);

        while (!run_stack.empty())
        {
            popTo(run_stack, z); 
            EWhatMerge merge_type = params.whatMerge(x.size, y.size, z.size);
            
            switch (merge_type)
            {
                case WM_MERGE_XY:
                    inplaceMerge(y, x, comp);
                    y.size += x.size;
                    x = y;
                    y = z;
                    break;
                case WM_MERGE_YZ:
                    inplaceMerge(z, y, comp);
                    z.size += y.size;
                    y = z;
                    break;
            }

            if (merge_type == WM_NO_MERGE)
            {
                run_stack.push(z);
                break;
            }
        }

        if (params.needMerge(x.size, y.size))
            replaceWithMerged(run_stack, y, x, comp);
        else
        {
            run_stack.push(y);
            run_stack.push(x);
        }
    }

    while (run_stack.size() > 1)
    {
        Run<RandomAccessIterator> x, y;
        popTo(run_stack, x);
        popTo(run_stack, y);
        replaceWithMerged(run_stack, y, x, comp);
    }
}

template <class RandomAccessIterator>
void timSort(RandomAccessIterator start, RandomAccessIterator finish, 
             const ITimSortParams& params = DEFAULT_PARAMS)
{
    timSort(start, finish, 
            std::less_equal<typename std::iterator_traits<RandomAccessIterator>::value_type>(), params);
}