
#pragma once
#include <assert.h>
#include <cmath>
#include <vector>
#include "Run.h"
#define Block Run

int min(int a, int b) { return (a < b) ? a : b; }

template <class RandomAccessIterator>
class BlockCompare
{
public:
    template <class Compare>
    bool operator ()(Block<RandomAccessIterator> block1, Block<RandomAccessIterator> block2, Compare comp)
    {
        if (*block1.start == *block2.start)
            return comp(*(block1.start + block1.size - 1), *(block2.start + block2.size - 1));
        else
            return comp(*(block1.start), *(block2.start));
    }
};

template <class RandomAccessIterator>
class BlockSwapper
{
public:
    void operator ()(Block<RandomAccessIterator>& block1, Block<RandomAccessIterator>& block2)
    {
        for (int i = 0; i < min(block1.size, block2.size); i++)
            timSortSwap(*(block1.start + i), *(block2.start + i));
    }
};

template <class Type>
class DefaultSwapper
{
public:
    void operator ()(Type& a, Type& b)
    {
        Type t = a;
        a = b;
        b = t;
    }
};

template <class RandomAccessIterator, class Compare, class Swapper>
void selectionSort(RandomAccessIterator start, RandomAccessIterator finish, Compare comp, Swapper swapper)
{
    int arr_size = finish - start;
    
    for (int i = 0; i < arr_size; i++)
    {
        RandomAccessIterator min_iter = start + i;
        
        for (RandomAccessIterator iter = start + i + 1; iter != finish; iter++)
        {
            if (comp(*iter, *min_iter))
                min_iter = iter;
        }

        swapper(*(start + i), *min_iter);
    }
}

template <class RandomAccessIterator>
void getBlockForward(RandomAccessIterator start, int i, int block_len, 
                     Block<RandomAccessIterator>& block, int arr_size)
{
    block.start = start + block_len * i;
    block.size = min(block_len, arr_size - block_len * i);
}

template <class RandomAccessIterator>
void getBlockBackward(RandomAccessIterator finish, int i, int block_len,
                      Block<RandomAccessIterator>& block, int arr_size)
{
    if ((i + 1) * block_len <= arr_size)
    {
        block.start = finish - (i + 1) * block_len;
        block.size = block_len;
    }
    else
    {
        block.start = finish - arr_size;
        block.size = arr_size - block_len * i;
    }
}

template <class RandomAccessIterator>
void divideArrayToBlocks(RandomAccessIterator start, RandomAccessIterator finish,
                         std::vector<Block<RandomAccessIterator>>& blocks)
{
    int arr_size = finish - start;
    int block_len = static_cast<int>(sqrt(arr_size));

    int block_start_index = 0;
    while (block_start_index + block_len <= arr_size)
    {
        Block<RandomAccessIterator> new_block = {start + block_start_index, block_len};
        block_start_index += block_len;
        blocks.push_back(new_block);
    }

    Block<RandomAccessIterator> last_block = {start + block_start_index, arr_size - block_start_index};
    blocks.push_back(last_block);
}

template <class RandomAccessIterator>
void divideArrayToBlocksFromEnd(RandomAccessIterator start, RandomAccessIterator finish,
                                std::vector<Block<RandomAccessIterator>>& blocks, int block_len)
{
    int arr_size = finish - start;

    int block_finish_index = arr_size;
    while (block_finish_index - block_len >= 0)
    {
        Block<RandomAccessIterator> new_block = {start + block_finish_index - block_len, block_len};
        block_finish_index -= block_len;
        blocks.push_back(new_block);
    }

    Block<RandomAccessIterator> last_block = {start, block_finish_index};
    blocks.push_back(last_block);
}

template <class RandomAccessIterator, class CompareBlock, class CompareElem, class Swapper>
void sortBlocks(RandomAccessIterator start, int arr_size, int block_len, int n_blocks,
                CompareBlock comp_block, CompareElem comp_elem, Swapper swap)
{
    for (int i = 0; i < n_blocks - 1; i++)
    {
        Block<RandomAccessIterator> min_block;
        getBlockForward(start, i, block_len, min_block, arr_size);
        Block<RandomAccessIterator> dest = min_block;

        for (int j = i + 1; j < n_blocks; j++)
        {
            Block<RandomAccessIterator> new_block;
            getBlockForward(start, j, block_len, new_block, arr_size);

            if (comp_block(new_block, min_block, comp_elem))
                min_block = new_block;
        }

        swap(dest, min_block);
    }
}

template <class RandomAccessIterator, class Compare>
void inplaceMerge(Run<RandomAccessIterator> left, Run<RandomAccessIterator> right, Compare comp, int gallop = NO_GALLOPING_MODE)
{
    int arr_size = left.size + right.size;
    int forward_len = static_cast<int>(sqrt(arr_size));
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    int min_len = 4 * forward_len;
    if (arr_size <= min_len)
    {
        selectionSort(left.start, right.start + right.size, comp, DefaultSwapper<ValueType>());
        return;
    }

    BlockSwapper<RandomAccessIterator> block_swapper;

    int middle_block_index = 0;

    while (true)
    {
        Block<RandomAccessIterator> block;
        getBlockForward(left.start, middle_block_index, forward_len, block, arr_size);

        if (right.start >= block.start + block.size)
            middle_block_index++;
        else
            break;
    }

    int n_blocks_forward = arr_size / forward_len + 1;
    Block<RandomAccessIterator> remained, buffer, middle;
    getBlockForward(left.start, n_blocks_forward - 1, forward_len, remained, arr_size);
    getBlockForward(left.start, n_blocks_forward - 2, forward_len, buffer, arr_size);
    getBlockForward(left.start, middle_block_index, forward_len, middle, arr_size);

    block_swapper(middle, buffer);
    
    sortBlocks(left.start, arr_size, forward_len, n_blocks_forward - 2, 
               BlockCompare<RandomAccessIterator>(), comp, block_swapper);

    for (int i = 0; i + 1 < n_blocks_forward - 2; i++)
    {
        Block<RandomAccessIterator> left_block, right_block;
        getBlockForward(left.start, i, forward_len, left_block, arr_size);
        getBlockForward(left.start, i + 1, forward_len, right_block, arr_size);

        mergeRunsWithBuffer(left_block, right_block, buffer.start, comp, WM_SWAP_WRITE, gallop);
    }

    RandomAccessIterator finish = right.start + right.size;
    int backward_len = buffer.size + remained.size;

    selectionSort(finish - 2 * backward_len, finish, comp, DefaultSwapper<ValueType>());
    
    int n_backward = (arr_size - backward_len) / backward_len + 1;

    for (int i = 0; i + 1 < n_backward; i++)
    {
        Block<RandomAccessIterator> left_block, right_block;
        getBlockBackward(finish - backward_len, i + 1, backward_len, left_block, arr_size - backward_len);
        getBlockBackward(finish - backward_len, i, backward_len, right_block, arr_size - backward_len);

        mergeRunsWithBuffer(left_block, right_block, finish - backward_len, comp, WM_SWAP_WRITE, gallop);
    }

    selectionSort(finish - backward_len, finish, comp, DefaultSwapper<ValueType>());
} 