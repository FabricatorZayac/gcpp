#include <bitset>
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

struct ColoredText {
    RGB *color;
    std::string text;
};

int main() {
    std::cout << "Hello World" << std::endl;

    GC gc;

    auto text = gc.create<ColoredText>();

    std::cout << typeid(*text).name() << std::endl
              << typeid(text).name() << std::endl;

    // std::cout << *((std::bitset<64> *)(&text->color) - 1) << std::endl;

    text->text = "Haiii";
    std::cout << text->text << std::endl;

    *text->color = { 0xFF, 0x53, 0xAB };

    std::cout << boost::pfr::io(*text->color) << std::endl;
}
