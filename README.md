# PyTimer

使用 Pybind11 库实现 C++ 到 Python 层的接口暴露，PyTimer 用时间轮算法实现定时器，预计满足线程安全的要求。

## 依赖

- python 3.x
- Pybind11
- cmake

## 编译

1. 下载 pybind11 库

```bash
git submodule add -b stable ../../pybind/pybind11 libs/pybind11
git submodule update --init
```

2. cmake 编译

```bash
cmake -S . -B build
cmake --build ./build
```
