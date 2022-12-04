#include "timer.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "timer.h"

namespace py = pybind11;

static Timer *timer = Timer::instance();

PYBIND11_MODULE(MyTimer, m)
{
    m.doc() = "A timewheel timer fro python 3.x.";

    m.def("getTimer", []() {
        return timer;
    }, "Get a singleton timer from module.");

    py::class_<Timer>(m, "Timer", py::is_final())
        .def_static("instance", &Timer::instance, "Get a timer singleton instance")
        .def("AddTimer", &Timer::addTimer, "Add a timer.")
        .def("CancelTimer", &Timer::cancelTimer, "Cancel a timer.")
        .def("Clear", &Timer::clear, "Empty all timers.")
        .def("MainLoop", &Timer::executeOnce, "The timer event loop.");
}
