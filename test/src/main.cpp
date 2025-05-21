#include <cassert>
#include <iostream>
#include <ostream>
#include <string>

#include "boost/pfr/io.hpp"
#include "gc.hpp"

struct RGB {
    int r;
    int g;
    int b;
};
// bool operator==(const RGB &self, const RGB &other) {
//     return boost::pfr::structure_tie(self) == boost::pfr::structure_tie(other);
// }

struct ColoredText {
    RGB *color;
    std::string text;
};

int main() {
    GC gc;

    auto text = gc.create<ColoredText>();
    text->text = "Test";

    {
        auto shmext = gc.create<ColoredText>();
    }

    gc();

    // std::cout << typeid(*text).name() << std::endl
    //           << typeid(text).name() << std::endl;

    // std::cout << *((std::bitset<64> *)(&text->color) - 1) << std::endl;

    text->text = "Haiii";
    std::cout << text->text << std::endl;

    *text->color = { 0xFF, 0x53, 0xAB };

    std::cout << boost::pfr::io(*text->color) << std::endl;

    auto color = gc.bind(text->color);
    std::cout << boost::pfr::io(*color) << std::endl;

    // gc();
}
