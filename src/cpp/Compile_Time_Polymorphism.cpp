#include <iostream>
#include <pthread.h>
#include <thread>
#include <vector>

class Shutdown {
public:
    virtual void shutdown() = 0;
    virtual ~Shutdown() = default;
};

// Base template class using CRTP
template <typename Derived>
class MotorController : public Shutdown{
public:
    int x;
    int y;
    MotorController() : x(0), y(0) {}
    MotorController(int input1, int input2) : x(input1), y(input2) {}

    void update() {
        // Direct compile-time call; no vtable lookup
        static_cast<Derived*>(this)->updateImpl();
    }
};

class EtherCatMotor : public MotorController<EtherCatMotor>{
public:
    EtherCatMotor() : MotorController<EtherCatMotor>(0,0) {}
    EtherCatMotor(int input1, int input2) : MotorController<EtherCatMotor>(input1, input1) {}
    void updateImpl() {
        // High-frequency hardware writes
        std::cout << "EtherCatMotor" << std::endl;
        std::cout << x+y << std::endl;
    }

    void shutdown() override {
        std::cout << "EtherCatMotor Shutdown started" << std::endl;
    }
};

class OtherMotor : public MotorController<OtherMotor> {
public:
    OtherMotor() : MotorController<OtherMotor>(0,0) {}
    OtherMotor(int input1, int input2) : MotorController<OtherMotor>(input1, input2) {}

    void updateImpl() {
        std::cout << "OtherMotor" << std::endl;
        std::cout << x*y << std::endl;
    }

    void shutdown() final override {
        std::cout << "OtherMotor Shutdown started" << std::endl;
    }
};

template <typename T>
void update (MotorController<T>& base) {
    base.update();
}

// std::vector<Shutdown*> motor = {new EtherCatMotor, new OtherMotor};

int main() {
    EtherCatMotor e(3,3);
    OtherMotor o(4,4);
    update(e);
    update(o);

    std::vector<Shutdown*> motor = {&e, &o};

    for (auto&& m : motor) {
        std::thread worker(&Shutdown::shutdown, m);
        worker.join();
    }
}
