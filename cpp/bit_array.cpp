// Input
// 100000000 569099406 1607140150 823906344
// 100000000 569099406 1607140150 823906344
// Output
// 31
#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
using namespace std;

void getUnique(unsigned long long N, unsigned long long S, unsigned long long P, unsigned long long Q) {
    unsigned long long sum = 0;
    unsigned long long max_ll = std::numeric_limits<unsigned long long>::max();
    for (int i = 0; i < N; ++i) {
        sum += (P * S) + Q;
        if (sum>max_ll-1) {
            cout << i;
            return;
        }
    }
    cout << N;
}

int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    // int N, S, P, Q;
    // cin >> N >> S >> P >> Q;

    // 1. Record start time
    auto start = std::chrono::steady_clock::now();

    // if ((S==0)||(Q==0)||(P==0)||(N<1))
    if (false)
        cout << 1;
    else {             //100000000 569099406 1607140150 823906344
        // getUnique(N ,S, P, Q);
        getUnique(100000000, 569099406, 1607140150, 823906344);

        // 2. Record end time
        auto end = std::chrono::steady_clock::now();

        // 3. Calculate duration and cast to desired unit (milliseconds)
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // 4. Output the result
        std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;

        // To get the duration in seconds as a floating-point number:
        std::chrono::duration<double> seconds_duration = end - start;
        std::cout << "Time taken: " << seconds_duration.count() << " seconds" << std::endl;
    }
    return 0;
}
