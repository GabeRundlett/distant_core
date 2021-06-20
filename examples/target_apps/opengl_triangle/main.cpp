#include "app.hpp"

#include <cstdlib>

int main() {
    Application app;
    if (!app) { return EXIT_FAILURE; }
    app.redraw();
    while (app.poll_events()) {}
    return EXIT_SUCCESS;
}
