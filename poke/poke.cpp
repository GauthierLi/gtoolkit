#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <string>

// ANSI 颜色码
const std::string RED   = "\033[31m";
const std::string RESET = "\033[0m";
const std::string CLEAR_SCREEN = "\033[2J\033[H"; // 清屏码

enum class Suit { Hearts, Diamonds, Clubs, Spades };
enum class Rank { Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace };

class Card {
public:
    Card(Suit suit, Rank rank) : suit(suit), rank(rank) {}

    std::string getTopLine() const {
        return "+-------";
    }

    std::string getRankLine() const {
        std::string rankStr = rankToString();
        return "| " + rankColor() + rankStr + std::string(6 - rankStr.size(), ' ') + RESET;
    }

    std::string getSuitLine() const {
        return "|  " + suitColor() + suitToString() + RESET + "    ";
    }

    std::string getBottomLine() const {
        return "+-------";
    }

    void display() const {
        std::cout << getTopLine() << " ";
    }

    void displayRank() const {
        std::cout << getRankLine() << " ";
    }

    void displaySuit() const {
        std::cout << getSuitLine() << " ";
    }

    void displayBottom() const {
        std::cout << getBottomLine() << " ";
    }

private:
    Suit suit;
    Rank rank;

    static std::string getSuitColor(Suit suit) {
        switch (suit) {
            case Suit::Hearts:
            case Suit::Diamonds:
                return RED;
            case Suit::Clubs:
            case Suit::Spades:
                return RESET;
        }
        return RESET;
    }

    std::string suitColor() const {
        return getSuitColor(suit);
    }

    std::string rankColor() const {
        return RESET; // 使用默认颜色显示点数
    }

    std::string suitToString() const {
        switch (suit) {
            case Suit::Hearts:   return "❤";
            case Suit::Diamonds: return "♦";
            case Suit::Clubs:    return "♣";
            case Suit::Spades:   return "♠";
        }
        return "";
    }

    std::string rankToString() const {
        switch (rank) {
            case Rank::Two:   return "2";
            case Rank::Three: return "3";
            case Rank::Four:  return "4";
            case Rank::Five:  return "5";
            case Rank::Six:   return "6";
            case Rank::Seven: return "7";
            case Rank::Eight: return "8";
            case Rank::Nine:  return "9";
            case Rank::Ten:   return "10";
            case Rank::Jack:  return "J";
            case Rank::Queen: return "Q";
            case Rank::King:  return "K";
            case Rank::Ace:   return "A";
        }
        return "";
    }
};

class Deck {
public:
    Deck() {
        for (int s = 0; s < 4; ++s) {
            for (int r = 2; r <= 14; ++r) {
                cards.emplace_back(static_cast<Suit>(s), static_cast<Rank>(r));
            }
        }
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        rng.seed(std::random_device{}());
        availableIndices.resize(cards.size());
        std::iota(availableIndices.begin(), availableIndices.end(), 0); // 初始化为 0, 1, ..., 51

        // 发两张底牌
        dealInitialCards();
        displayCards(); // 初始化时展示手牌和公共牌
    }

    void startRound() {
        if (availableIndices.empty()) {
            std::cout << "No more cards left to deal." << std::endl;
            return;
        }

        // 发公共牌
        int communityCardIndex = getRandomCardIndex();
        communityCards.push_back(cards[communityCardIndex]);

        // 打印所有手牌和公共牌
        displayCards();
    }

private:
    std::vector<Card> cards;
    std::vector<Card> handCards; // 存储所有手牌
    std::vector<Card> communityCards; // 存储所有公共牌
    std::mt19937 rng; // 随机数生成器
    std::vector<int> availableIndices; // 记录尚未抽取的牌的索引

    void dealInitialCards() {
        for (int i = 0; i < 2; ++i) {
            int handCardIndex = getRandomCardIndex();
            handCards.push_back(cards[handCardIndex]);
        }

        for (int i = 0; i < 3; ++i) {
            int communityCardIndex = getRandomCardIndex();
            communityCards.push_back(cards[communityCardIndex]);
        }
    }

    int getRandomCardIndex() {
        std::uniform_int_distribution<int> dist(0, availableIndices.size() - 1);
        int index = dist(rng);
        int cardIndex = availableIndices[index];
        availableIndices.erase(availableIndices.begin() + index);
        return cardIndex;
    }

    void displayCards() const {
        std::cout << CLEAR_SCREEN; // 清屏
        std::cout << "Hand Cards:" << std::endl;
        displayCardArea(handCards);
        std::cout << "Community Cards:" << std::endl;
        displayCardArea(communityCards);
    }

    void displayCardArea(const std::vector<Card>& cards) const {
        if (cards.empty()) return;

        size_t numCards = cards.size();
        
        for (size_t j = 0; j < numCards; ++j) {
            std::cout << cards[j].getTopLine() << " ";
        }
        std::cout << std::endl;
    
    
        for (size_t j = 0; j < numCards; ++j) {
            std::cout << cards[j].getRankLine() << " ";
        }
        std::cout << std::endl;
    
    
        for (size_t j = 0; j < numCards; ++j) {
            std::cout << cards[j].getSuitLine() << " ";
        }
        std::cout << std::endl;
    
    
        for (size_t j = 0; j < numCards; ++j) {
            std::cout << cards[j].getBottomLine() << " ";
        }
        std::cout << std::endl;
        
    }
};

int main() {
    Deck deck;
    std::string input;
    int rounds = 0;

    while (rounds < 2) {
        std::cout << "Press 'r' to deal cards, or 'q' to quit: ";
        std::getline(std::cin, input);

        if (input == "q") {
            break;
        } else if (input == "r") {
            deck.startRound();
            rounds++;
        } else {
            std::cout << "Invalid input. Please enter 'r' or 'q'." << std::endl;
        }
    }

    std::cout << "Game over. Exiting..." << std::endl;
    return 0;
}
