#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

/**
 * Don't let the machines win. You are humanity's last hope...
 **/

int main()
{
    int width; // the number of cells on the X axis
    cin >> width; cin.ignore();
    int height; // the number of cells on the Y axis
    cin >> height; cin.ignore();

    std::vector<int> nodes;

    for (int i = 0; i < height; i++) {
        string line;
        getline(cin, line); // width characters, each either 0 or .
        cerr << line << endl;
        int posW = 0;
        for (auto letter : line) { //00 0.
            if (letter == '0') {
                nodes.push_back(posW);
                // cerr << posW << endl;
                nodes.push_back(i);
                // cerr << i << endl;
            } else {
                nodes.push_back(-1);
                // cerr << -1 << endl;
                nodes.push_back(-1);
                // cerr << -1 << endl;
            }
            posW++;
        }
        nodes.push_back(-1); // Non-existing neighbour after width
        nodes.push_back(-1); // Non-existing neighbour after width
    }

    // Fill up empty heights below
    for (int i = 0; i < width; i++) {
        nodes.push_back(-1);
        nodes.push_back(-1);
    }

    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;
    // 00 10 ..
    // 01 11 ..
    // .. ..    .. -> -1-1

    // w, w+1, w+2, w+3 ,w*2, w*(2+1)
    int matrix = width*2*height*2;
    int width_vect = width*2+2;
    int width_edge = width_vect+2;
    for (int curr=0; curr<matrix; curr+=2/*curr+=4*/) {
        cerr << "Nodes Value: " << nodes[curr] << endl;
        cerr << "curr " << curr << endl;
        if (curr==width_vect-1 || curr==width_vect-2)
            continue;
           
        cerr << nodes[curr] << endl;
        cout << nodes[curr] << " ";
        cout << nodes[curr+1] << " ";
        cout << nodes[curr+2] << " ";
        cout << nodes[curr+3] << " ";
        cout << nodes[curr+width_vect] << " ";
        cout << nodes[curr+width_vect+1] << " " << endl;
    }

    // Three coordinates: a node, its right neighbor, its bottom neighbor
    // cout << endl;
}