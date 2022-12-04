#pragma once
#include <pybind11/pybind11.h>
#include <list>

namespace py = pybind11;

#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR - 1)

#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_LEVEL_MASK (TIME_LEVEL - 1)

#define TIME_INTERVAL 10  // 第一层单个格子的时间间隔大小，单位为ms
#define TIME_LAYER 4  // 除第一层外，一共有多少层

typedef struct node {
    int t_id;
    py::object callback;
    py::args args;
    py::kwargs kwargs;
    uint64_t expire;
    double deltaSecond;
    bool cancel;
    bool repeat;
} node_t;

class Timer
{
public:
    static Timer *instance();

    ~Timer();
    void executeOnce();
    int addTimer(bool repeat, double delta, py::object callback, py::args args=py::list(), py::kwargs kwargs=py::dict());
    void cancelTimer(int id);
    void clear();

private:
    Timer();

    static Timer *_instance;

    int getTid();
    uint64_t getTime();
    void addNode(node_t*);
    void moveList(int, int);
    void doExecute();

    std::list<node_t*> near[TIME_NEAR];
    std::list<node_t*> t[TIME_LAYER][TIME_LEVEL];
    uint32_t _time;
    uint64_t _last_time;
    bool _ready;

    std::unordered_map<int, node_t*> *_caches;
    uint64_t count = 0;
};
