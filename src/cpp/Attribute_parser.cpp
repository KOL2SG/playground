#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
using namespace std;


int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    std::vector<std::string> listings;
    std::vector<std::string> queries;
    std::string line;
    getline(std::cin, line);  // 4 3

    int pos = line.find(' ');
    std::string numL = line.substr(0, pos);
    std::string numQ = line.substr(pos, line.npos);
    int n = stoi(numL);   // 4
    int q = stoi(numQ);   // 3
    string tag;

    for (int i=0; i<n; i++) {
        getline(std::cin, line);
        if (line.find('/') != line.npos) { // </xxx> ->line.size()-3
            tag.erase(tag.size()-(line.size()-2), line.size()-2);
            // cerr << "lintag: " << tag << endl;
            // cerr << "e size: " << line.size() << endl;
            // line.clear();
        }
        else {
            line.erase(0,1);
            if (line.find(' ') == line.npos)
                line.erase(line.find('>'), 1);
            listings.push_back(tag+line);
            if (line.find(' ') != line.npos) {
                tag += line.substr(0, line.find(' ')) + ".";
            }
            else {
                tag += line.substr(0, line.find('>')) + ".";
            }
            // cerr << "tag: " << tag << endl;
            // cerr << "line: " << line << endl;
        }
    }
    // for (auto&& line : listings) {
    //     cerr << "listings: " << line << endl;
    // }
    for (int i=0; i<q; i++) {
        getline(std::cin, line);
        line.replace(line.find('~'), 1, " ");
        queries.push_back(line);
    }
    // for (auto&& query : queries) {
    //     cerr << "query: " << query << endl;
    // }
    for (auto&& query : queries) { // tag1.tag2.tag3 name
        int list_size = listings.size();
        string header_q = query.substr(0, query.find(' ')); // tag1.tag2.tag3
        for (auto&& list : listings) { // tag2
            string header_l = list.substr(0, list.find(' '));
            if ((header_q.size() == header_l.size()) && (header_q.compare(header_l) == 0)) {
                // cerr << "query: " << query << endl;
                // cerr << "list: " << list << endl;
                header_q = query.substr(query.find(' '), query.npos);
                // cerr << "attrQ: " << header_q << endl;
                if (list.find(header_q) != list.npos) {
                    header_l = list.substr(list.find(header_q), list.npos);
                    // cerr << "attrL: " << header_l << endl;
                    stringstream ss(header_l);
                    for (int i=0; i<2; i++) {
                        getline(ss, line, '"');
                        if (i==1)
                            cout << line << endl;
                    }
                }
                else {
                    cout << "Not Found!" << endl;
                }
                break;
            }
            list_size--;
            // cerr << "list: " << list_size << endl;
            if (list_size == 0) {
                cout << "Not Found!" << endl;
            }
        }
    }

    return 0;
}




TestCase:
10 10
<a value = "GoodVal">
<b value = "BadVal" size = "10">
</b>
<c height = "auto">
<d size = "3">
<e strength = "2">
</e>
</d>
</c>
</a>
a~value
b~value
a.b~size
a.b~value
a.b.c~height
a.c~height
a.d.e~strength
a.c.d.e~strength
d~sze
a.c.d~size

GoodVal
Not Found!
10
BadVal
Not Found!
auto
Not Found!
2
Not Found!
3

GoodVal
Not Found!
10
BadVal
Not Found!
auto
Not Found!
2
Not Found!
3

Test case 2:
6 4
<a>
<b name = "tag_one">
<c name = "tag_two" value = "val_907">
</c>
</b>
</a>
a.b~name
a.b.c~value
a.b.c~src
a.b.c.d~name

tag_one
val_907
Not Found!
Not Found!
