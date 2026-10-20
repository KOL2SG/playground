#include <cstring>
#include <algorithm> // For std::swap
#include <iostream>

class myString {
private:
    char* myChar;
    size_t count;

public:
    myString(const char* s = "") : count(strlen(s)), myChar(new char[(strlen(s))+1]) {
        std:memcpy(myChar, s, count+1);
    }
    myString(const myString& other) : count(other.count), myChar(new char[other.count+1]) {
        std:memcpy(myChar, other.myChar, count+1);
    }
    myString& operator= (const myString& other) {
        if (this == &other) {
            return *this;
        }
        count = other.count;
        char* new_data = new char[count+1];
        std::memcpy(new_data, other.myChar, count+1);
        myChar = nullptr;
        myChar = new_data;
        return *this;
    }
    ~myString() {
        delete[] myChar;
    }
};

class MyDerivedString : public myString {
public:
    MyDerivedString(const char* s = "") : myString(s) {}
    MyDerivedString(const MyDerivedString& other) : myString(other) {}
};

int main() {
    myString str1("Hello");
    myString str2 = str1;
    myString str3;
    str3 = str2;
    myString str4(str3);
    myString str5 = "Zed";
    MyDerivedString str6("World");
    MyDerivedString str7 = str6;
    MyDerivedString str8;
    str8 = str7;
    MyDerivedString str9("Zed");
    MyDerivedString str10 = "World";
    MyDerivedString str11(str10);
    str11 = str9;
}
