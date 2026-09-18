#include "check.hpp"
#include <manmenmi/core/runtime.hpp>
#include <manmenmi/window/window.hpp>
#include <manmenmi/graphics/graphics.hpp>
#include <type_traits>

int main() {
    static_assert(std::is_abstract_v<manmenmi::Runtime>);
    static_assert(std::is_abstract_v<manmenmi::window::Window>);
    static_assert(std::is_abstract_v<manmenmi::graphics::Device>);
    static_assert(std::is_abstract_v<manmenmi::graphics::Backend>);
    static_assert(std::has_virtual_destructor_v<manmenmi::Runtime>);
    static_assert(std::has_virtual_destructor_v<manmenmi::window::Window>);
    static_assert(std::has_virtual_destructor_v<manmenmi::graphics::Device>);
    const manmenmi::window::Descriptor descriptor{"future neutral client", {1280, 720}};
    CHECK(descriptor.logical_size.width == 1280);
    CHECK(descriptor.logical_size.height == 720);
    return checks::finish();
}