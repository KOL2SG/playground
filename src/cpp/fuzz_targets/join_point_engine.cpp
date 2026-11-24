#include <iostream>
#include <math.h>
#include <string>

int join_point(int n1, int n2){
    if (n1==n2)
        return n1;

    std::string str1 = std::to_string(n1);
    std::string str2 = std::to_string(n2);
    int sum1 = n1;
    int sum2 = n2;
    for(auto&& letter : str1){
        sum1 += (letter - '0');
    }
    for(auto&& letter : str2){
        sum2 += (letter - '0');
    }

    return join_point(sum1,sum2);
}

