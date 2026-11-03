#include <string>
#include <memory>
#include <utility>
#include <iostream>

class Widget {
public:
    // 1. Explicit Constructor (Prevents accidental implicit conversions)
    explicit Widget(std::string name, int value)
        : name_(std::move(name))  // Pass-by-value + std::move for optimal copy/move performance
        , value_(value)           // Member Initializer List (always in order of declaration)
    {}

    // 2. The Rule of Zero (Let the compiler manage lifecycles automatically)
    // If your class only holds standard members, DO NOT write constructors/destructors yourself.
    // If you MUST manage resources manually, declare "The Rule of Five" explicitly:
    ~Widget() noexcept = default;                          // Destructor
    Widget(const Widget& other) = default;                 // Copy Constructor
    Widget& operator=(const Widget& other) = default;      // Copy Assignment
    Widget(Widget&& other) noexcept = default;            // Move Constructor
    Widget& operator=(Widget&& other) noexcept = default; // Move Assignment

    // 3. Const-Correct Getters (With trailing return type deduction helper if desired)
    // 'const' at the end ensures the function doesn't modify the object's state.
    [[nodiscard]] const std::string& getName() const noexcept {
        return name_;
    }

    // 4. [[nodiscard]] attribute (Tells compiler to warn if the return value is ignored)
    [[nodiscard]] int getValue() const noexcept {
        return value_;
    }

    // 5. Constexpr and noexcept (Specifying compile-time evaluation and zero-exception guarantees)
    [[nodiscard]] constexpr bool isLarge() const noexcept {
        return value_ > 1000;
    }

private:
    // 6. Member variables with trailing underscores (Common standard naming style)
    // 7. Default Member Initializers (Prevents uninitialized memory bugs)
    std::string name_{ "Default" };
    int value_{ 0 };
};
