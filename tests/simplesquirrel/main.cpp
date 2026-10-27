#include <simplesquirrel/simplesquirrel.hpp>
#include <iostream>

struct Counter {
    int value = 0;
    void add(int amount) { value += amount; }
    int read() const { return value; }
};

int main() {
    try {
        ssq::VM vm(1024);
        auto cls = vm.addClass("Counter", ssq::Class::Ctor<Counter()>());
        cls.addFunc("add", &Counter::add);
        cls.addFunc("read", &Counter::read);
        cls.addFunc("twice", [](Counter* counter) { return counter->value * 2; });
        vm.addFunc("increment", [](int value) { return value + 1; });
        int offset = 3;
        vm.addFunc("capturing", [offset](int value) { return value + offset; });
        vm.run(vm.compileSource(R"(
            local counter = Counter();
            counter.add(7);
            if (counter.read() != 7 || counter.twice() != 14) throw "member binding";
            if (increment(4) != 5 || capturing(4) != 7) throw "function binding";
        )"));
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
