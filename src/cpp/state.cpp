#include <iostream>
#include <memory>

class MusicPlayer; // Forward declaration

// 1. Abstract State Interface
class PlayerState {
public:
    virtual ~PlayerState() = default;
    virtual void pressPlay(MusicPlayer& player) = 0;
};

// 2. The Context (holds the current state)
class MusicPlayer {
private:
    std::unique_ptr<PlayerState> currentState;

public:
    MusicPlayer(std::unique_ptr<PlayerState> initialState)
        : currentState(std::move(initialState)) {}

    void setState(std::unique_ptr<PlayerState> newState) {
        currentState = std::move(newState);
    }

    void pressPlay() {
        currentState->pressPlay(*this); // Delegate work to the active state
    }
};

// 3. Concrete States
class PlayingState : public PlayerState {
public:
    void pressPlay(MusicPlayer& player) override;
};

class PausedState : public PlayerState {
public:
    void pressPlay(MusicPlayer& player) override {
        std::cout << "Resuming music...\n";
        player.setState(std::make_unique<PlayingState>());
    }
};

void PlayingState::pressPlay(MusicPlayer& player) {
    std::cout << "Pausing music...\n";
    player.setState(std::make_unique<PausedState>());
}

int main() {
    MusicPlayer player(std::make_unique<PausedState>());
    player.pressPlay(); // Output: Resuming music...
    player.pressPlay(); // Output: Pausing music...
    return 0;
}
