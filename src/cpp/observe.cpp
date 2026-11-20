#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

// 1. Observer Interface: Any class wanting updates implements this
class Observer {
public:
    virtual ~Observer() = default;
    virtual void onNotify(const std::string& eventData) = 0;
};

// 2. Subject (Publisher): Manages subscribers and sends updates
class NewsAgency {
private:
    std::vector<Observer*> observers;
    std::string latestNews;

public:
    void subscribe(Observer* obs) {
        observers.push_back(obs);
    }

    void unsubscribe(Observer* obs) {
        observers.erase(
            std::remove(observers.begin(), observers.end(), obs),
            observers.end()
        );
    }

    void publishNews(const std::string& news) {
        latestNews = news;
        notifyAll();
    }

private:
    void notifyAll() {
        for (Observer* obs : observers) {
            obs->onNotify(latestNews);
        }
    }
};

// 3. Concrete Observers
class PhoneApp : public Observer {
private:
    std::string userName;
public:
    explicit PhoneApp(std::string name) : userName(std::move(name)) {}

    void onNotify(const std::string& news) override {
        std::cout << "[" << userName << "'s Phone] Breaking News: " << news << "\n";
    }
};

int main() {
    NewsAgency agency;
    PhoneApp aliceApp("Alice");
    PhoneApp bobApp("Bob");

    agency.subscribe(&aliceApp);
    agency.subscribe(&bobApp);

    // Both observers get notified automatically
    agency.publishNews("C++26 standard finalized!");

    agency.unsubscribe(&bobApp);
    agency.publishNews("New C++ compiler released!"); // Only Alice gets this

    return 0;
}
