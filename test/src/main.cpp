#include <cassert>
#include <format>
#include <iostream>
#include <string>
#include <print>

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

// template<>
// struct std::formatter<RGB> : formatter<int> {
//     auto format(const RGB &c, format_context &ctx) const {
//         return std::format_to(ctx.out(), "RGB {{ r: {}, g: {}, b: {} }}", c.r, c.g, c.b);
//     }
// };

int main() {
    GC gc;

    {
        auto text = gc.create<ColoredText>();
        text->text = "Test";

        {
            auto shmext = gc.create<ColoredText>();
        }

        gc();

        text->text = "Haiii";
        std::println("{}", text->text);

        *text->color = { 0xFF, 0x53, 0xAB };
        
        std::cout << boost::pfr::io(*text->color) << std::endl;

        // idk what is causing the "mutation" when using println
        // std::println("{}", *text->color);
        // std::println("{}", *text->color);

        auto color = gc.bind(text->color);
        std::cout << boost::pfr::io(*color) << std::endl;

        // std::cout << color->r << ", "
        //           << color->g << ", "
        //           << color->b
        //           << std::endl;

        // std::println("{}", *color);
    }
    gc();
}
