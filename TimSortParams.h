/*
TimSortParams.h
1. Offers class ITimSortParams with pure virtual funcions to set up your TimSort
2. Offers a default ITimSortDefaultParams class
3. Offers enumeration EWhatMerge to control merging processes in TimSort
*/

#pragma once
#define PURE =0

enum EWhatMerge
{
    WM_MERGE_XY,
    WM_MERGE_YZ,
    WM_NO_MERGE 
};

const int MAX_MIN_RUN_LENGTH = 64;
const int NO_GALLOPING_MODE = -1;

class ITimSortParams
{
public:
    virtual int minRun(int n) const PURE;
    virtual bool needMerge(int len_x, int len_y) const PURE; 
    virtual EWhatMerge whatMerge(int len_x, int len_y, int len_z) const PURE;
    virtual int getGallop() const PURE;
};

class IDefaultTimSortParams : public ITimSortParams
{
public:
    virtual int minRun(int n) const
    {
        int flag = 0;
        while (n >= MAX_MIN_RUN_LENGTH)
        {
            flag = flag | (n % 2);
            n /= 2;
        }
        return n + flag;
        //return 2;
    }

    virtual bool needMerge(int len_x, int len_y) const
    {
        return (len_y <= len_x);
    }

    virtual EWhatMerge whatMerge(int len_x, int len_y, int len_z) const
    {
        if (len_z > len_x + len_y && len_y > len_x)
            return WM_NO_MERGE;

        if (len_z <= len_x + len_y)
            return (len_x < len_z) ? WM_MERGE_XY : WM_MERGE_YZ;
        
        if (len_y <= len_x)
            return WM_MERGE_XY;

        return WM_NO_MERGE;
    }

    virtual int getGallop() const
    {
        return NO_GALLOPING_MODE;
    }
};

const IDefaultTimSortParams DEFAULT_PARAMS;