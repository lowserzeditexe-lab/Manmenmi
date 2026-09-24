#include "check.hpp"

#include <manmenmi/gx2/gx2.hpp>

#include <type_traits>

int main() {
    using namespace manmenmi::gx2;

    CHECK(std::is_standard_layout_v<Vertex>);
    CHECK(sizeof(Vertex) == sizeof(float) * 6U);
    return checks::finish();
}
