#include <cstdlib>
#include "app.hpp"

int main() {
    Application app;
    if (!app)
        return EXIT_FAILURE;
    app.redraw();
    while (app.poll_events()) {
    }
    return EXIT_SUCCESS;
}
