#include "timer.h"

Timer* Timer::_instance = nullptr;

Timer::Timer()
    : _ready(false),
    _time(0),
    _last_time(0),
    _caches(new std::unordered_map<int, node_t*>())
{}

Timer::~Timer()
{
    clear();
    delete _caches;
}

Timer* Timer::instance()
{
    if (_instance == nullptr)
        _instance = new Timer();
    return _instance;
}

void Timer::executeOnce()
{
    if (!_ready)
    {
        _ready = true;
        _last_time = getTime();
        _time = 0;
        py::print("[Core Timer] Start timer's event loop.");
    }
    else
    {
        uint64_t current_time = getTime();
        uint64_t delta_time = current_time - _last_time;
        if (delta_time < TIME_INTERVAL)
            return;

        uint32_t need_iter_num = delta_time / TIME_INTERVAL;
        for (uint32_t i = 0; i < need_iter_num; ++i)
        {
            doExecute();
            uint32_t ct = ++_time;
            if (ct == 0)
                moveList(3, 0);
            else
            {
                uint32_t time = (ct >> TIME_NEAR_SHIFT);
                int mask = TIME_NEAR;
                int i = 0;
                while ((ct & (mask - 1)) == 0)
                {
                    int idx = time & TIME_LEVEL_MASK;
                    if (idx != 0)
                    {
                        moveList(i, idx);
                        break;
                    }
                    mask <<= TIME_LEVEL_SHIFT;
                    time >>= TIME_LEVEL_SHIFT;
                    i++;
                }
            }
        }

        _last_time = current_time - (delta_time % TIME_INTERVAL);
    }
}

void Timer::doExecute()
{
    int idx = _time & TIME_NEAR_MASK;
    std::list<node_t*>* head = &near[idx];
    if (!head->empty())
    {
        std::vector<node*> repeats;
        auto iter = head->cbegin();
        for (; iter != head->cend(); iter++)
        {
            if ((*iter)->callback && !(*iter)->cancel)
            {
                if ((*iter)->repeat)
                    repeats.push_back(*iter);

                try
                {
                    (*iter)->callback(*(*iter)->args, **(*iter)->kwargs);
                }
                catch(py::error_already_set &e)
                {
                    py::module_::import("traceback").attr("print_exception")(e.type(), e.value(), e.trace());
                    continue;
                }
            }
        }
        head->clear();

        for (auto node : repeats)
        {
            node->expire = _time + node->deltaSecond * 1000 / TIME_INTERVAL;
            addNode(node);
        }
    }
}

void Timer::moveList(int level, int idx)
{
    std::list<node_t*>* current_list = &t[level][idx];
    ssize_t size = current_list->size();
    for (int i = 0; i < size; ++i)
    {
        auto front = current_list->front();
        current_list->pop_front();
        addNode(front);
    }
}

void Timer::addNode(node_t* node)
{
    uint64_t delta = node->expire - _time;
    uint64_t expire = _time + delta;
    node->expire = expire;

    if (delta < TIME_NEAR)
        near[expire & TIME_NEAR_MASK].push_back(node);
    else if (delta < (1 << (TIME_NEAR_SHIFT + TIME_LEVEL_SHIFT)))
        t[0][(expire >> TIME_NEAR_SHIFT) & TIME_LEVEL_MASK].push_back(node);
    else if (delta < (1 << (TIME_NEAR_SHIFT + TIME_LEVEL_SHIFT * 2)))
        t[1][expire >> (TIME_NEAR_SHIFT + TIME_LEVEL_SHIFT)].push_back(node);
    else if (delta < (1 << (TIME_NEAR_SHIFT + TIME_LEVEL_SHIFT * 3)))
        t[2][expire >> (TIME_NEAR_SHIFT + TIME_LEVEL_SHIFT * 2)].push_back(node);
    else
        t[3][expire >> (TIME_NEAR_SHIFT + TIME_LEVEL_SHIFT * 3)].push_back(node);

#ifdef DEBUG
    py::print("==== DEBUG CONTENT ====");
    for (int i = 0; i < TIME_NEAR; ++i)
    {
        auto list = near[i];
        if (!list.empty())
            py::print("Near array: ", i);
    }
    for (int i = 0; i < TIME_LAYER; ++i)
    {
        for (int j = 0; j < TIME_LEVEL; ++j)
        {
            auto list = t[i][j];
            if (!list.empty())
                py::print("t array: ", i, j);
        }
    }
#endif
}

/*
* delta 单位是秒，目前没有实现延帧功能
*/
int Timer::addTimer(bool repeat, double delta, py::object callback, py::args args, py::kwargs kwargs)
{
    uint64_t expire = delta * 1000 / TIME_INTERVAL + _time;
    node_t* node = new node_t{getTid(), callback, args, kwargs, expire, delta, false, repeat};
    if (delta <= 0)
    {
        try
        {
            node->callback(*node->args, **node->kwargs);
        }
        catch(const py::error_already_set& e)
        {
            py::module_::import("traceback").attr("print_exception")(e.type(), e.value(), e.trace());
        }
        delete node;  // 因为 catch 不会退出执行
        return -1;
    }

    _caches->insert(std::pair<int, node_t*>(node->t_id, node));
    addNode(node);
    return node->t_id;
}

void Timer::cancelTimer(int id)
{
    auto iter = _caches->find(id);
    if (iter == _caches->end())
        return;
    (*iter).second->cancel = true;
    _caches->erase(iter);
}

void Timer::clear()
{
    std::list<node_t*>* iter = near;
    for (int i = 0; i < TIME_NEAR; i++)
    {
        if (!iter->empty())
            iter->clear();
        iter++;
    }
    for (int i = 0; i < TIME_LAYER; i++)
    {
        iter = t[i];
        for (int j = 0; j < TIME_LEVEL; j++)
        {
            if (!iter->empty())
                iter->clear();
            iter++;
        }
    }
    _caches->clear();
}

uint64_t Timer::getTime()
{
    uint64_t t;
    struct timespec ti;

    clock_gettime(CLOCK_MONOTONIC, &ti);
    t = (uint64_t)ti.tv_sec * 1000;
    t += ti.tv_nsec / 1000000;  // 控制定时器精度, 虽小可调到1ns, 当前为10ms

    return t;
}

int Timer::getTid()
{
    return ++count;
}
