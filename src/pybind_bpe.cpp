#include "BytePairEncoding.h"
#include "learn.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <string>

namespace py = pybind11;

PYBIND11_MODULE(prunedbpe, m) {
    py::class_<BPE>(m, "BPE")
        .def("save", &BPE::save);
    m.def("learn", &learn);
};
