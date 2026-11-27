#include <iostream>
#include <vector>
#include <functional>

class Button {
private:
    std::vector<std::function<void()>> clickListeners;

public:
    void onClick(std::function<void()> callback) {
        clickListeners.push_back(std::move(callback));
    }

    void press() {
        for (const auto& listener : clickListeners) {
            listener();
        }
    }
};

int main() {
    Button submitBtn;

    // Subscribe using simple lambdas without creating extra classes
    submitBtn.onClick([]() { std::cout << "Validating form...\n"; });
    submitBtn.onClick([]() { std::cout << "Sending data to server...\n"; });

    submitBtn.press();
    return 0;
}
