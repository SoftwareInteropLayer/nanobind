#include <nanobind/nanobind.h>
#include <memory>

namespace nb = nanobind;
using namespace nb::literals;

static int default_constructed = 0, value_constructed = 0, copy_constructed = 0,
           move_constructed = 0, copy_assigned = 0, move_assigned = 0,
           destructed = 0;

struct Struct;
std::unique_ptr<Struct> struct_tmp;

struct Struct {
    int i = 5;

    Struct() { default_constructed++; }
    Struct(int i) : i(i) { value_constructed++; }
    Struct(const Struct &s) : i(s.i) { copy_constructed++; }
    Struct(Struct &&s) : i(s.i) { s.i = 0; move_constructed++; }
    Struct &operator=(const Struct &s) { i = s.i; copy_assigned++; return *this; }
    Struct &operator=(Struct &&s) { std::swap(i, s.i); move_assigned++; return *this; }
    ~Struct() { destructed++; }

    int value() const { return i; }
    void set_value(int value) { i = value; }

    static Struct* create_take() { return new Struct(10); }
    static Struct  create_move() { return Struct(11); }
    static Struct* create_copy() { return struct_tmp.get(); }
    static Struct* create_reference() { return struct_tmp.get(); }
    Struct &self() { return *this; }
};

struct PairStruct {
    Struct s1;
    Struct s2;
};

struct alignas(1024) Big { char data[1024]; };

NB_MODULE(test_classes_ext, m) {
    struct_tmp = std::unique_ptr<Struct>(new Struct(12));

    nb::class_<Struct>(m, "Struct")
        .def(nb::init<>())
        .def(nb::init<int>())
        .def("value", &Struct::value)
        .def("set_value", &Struct::set_value, "value"_a)
        .def("self", &Struct::self)
        .def("none", [](Struct &) -> const Struct * { return nullptr; })
        .def_static("create_move", &Struct::create_move)
        .def_static("create_reference", &Struct::create_reference,
                    nb::rv_policy::reference)
        .def_static("create_copy", &Struct::create_copy,
                    nb::rv_policy::copy)
        .def_static("create_take", &Struct::create_take);

    // nb::class_<PairStruct>(m, "PairStruct")
    //     .def(nb::init<>())
    //     .def_readwrite("s1", &PairStruct::s1)
    //     .def_readwrite("s2", &PairStruct::s2);

    m.def("stats", []{
        nb::dict d;
        d["default_constructed"] = default_constructed;
        d["value_constructed"] = value_constructed;
        d["copy_constructed"] = copy_constructed;
        d["move_constructed"] = move_constructed;
        d["copy_assigned"] = copy_assigned;
        d["move_assigned"] = move_assigned;
        d["destructed"] = destructed;
        return d;
    });

    m.def("reset", []() {
        default_constructed = 0;
        value_constructed = 0;
        copy_constructed = 0;
        move_constructed = 0;
        copy_assigned = 0;
        move_assigned = 0;
        destructed = 0;
    });

    nb::class_<Big>(m, "Big")
        .def(nb::init<>());
}