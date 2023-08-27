#include "Zap.h"

int main() {
    Zap::init();

    Zap::Window window = Zap::Window(1000, 600, "Zap Application");
    window.init();
    window.show();

    while (!window.shouldClose()) {
        Zap::Window::pollEvents();
    }

    window.~Window();

    system("pause");
    return 0;
}