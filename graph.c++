#include "generation.c++"
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

class CSV {
    void loop(ofstream &o, vector<string> l){
        for (int i = 0; i < l.size(); ++i) {
            o << l[i];
            if (i != (l.size() - 1)) o << ",";
        }
    };
public:
    vector<vector<string>> rows;
    vector<string> top;
    string name;
    CSV() {
        rows = vector<vector<string>>{};
    }
    CSV(string c_name, vector<string> c_top) {
        rows = vector<vector<string>>{};
        name = c_name;
        top = c_top;
    }
    void make() {
        ofstream out("analysis/"+name+".csv");
        loop(out, top);
        for (int i = 0; i < rows.size(); ++i) {
            out << endl; loop(out, rows[i]);
        }
        out.close();
    }
    void push(vector<string> row) {
        rows.push_back(row);
    }
};

int number(string prompt) {
    cout << prompt << endl;
    int num; cin >> num;
    return num;
}


void one() {
    CSV table("change-in-top-10-over-time", vector<string>{"gen", "change"});
    int g = number("Generations run:"), m = 100;
    if (g == 1) {
        cout << "Cannot observe marginal change over just one item" << endl;
        return;
    }
    for (int i = 2; i < g+1; ++i) {
        unordered_set<int> target{0,15,30,45,60,75,80,85,90,95};
        unordered_set<int> real{};
        auto info = Generational::info_best(i, 100, 10);
        for (auto item : info) {
            real.insert(item.second);
        }
        for (auto item : real) {
            // cout << item << endl;
            if (target.contains(item)) {
                target.erase(target.find(item));
            }
        }
        table.push(vector<string>{
            to_string(i),
            to_string(int(target.size()))
        });
    }
    table.make();
}
void two() {
    CSV table("avg-solve-over-time", vector<string>{"gen", "avg solve"});
    int g = number("Generations run:"), m = 100;
    for (int i = 1; i < g+1; ++i) {
        double average = 0;
        auto info = Generational::info_best(i, m, m);
        for (auto item : info) {
            average += item.first;
        }
        average /= m;
        table.push(vector<string>{
            to_string(i),
            to_string(average)
        });
    }
    table.make();
}
void three() {
    int top = number("Depth of search:");
    CSV table("top-"+to_string(top)+"-solve-over-time", vector<string>{"gen", "avg solve"});
    int g = number("Generations run:"), m = 100;
    for (int i = 1; i < g+1; ++i) {
        double average = 0;
        auto info = Generational::info_best(i, m, top);
        for (auto item : info) {
            average += item.first;
        }
        average /= top;
        table.push(vector<string>{
            to_string(i),
            to_string(average)
        });
    }
    table.make();
}
void four() {
    int top = number("Depth of search:");
    CSV table("bottom-"+to_string(top)+"-solve-over-time", vector<string>{"gen", "avg solve"});
    int g = number("Generations run:"), m = 100;
    for (int i = 1; i < g+1; ++i) {
        double average = 0;
        auto info = Generational::info_best(i, m, top * -1);
        for (auto item : info) {
            average += item.first;
        }
        average /= top;
        table.push(vector<string>{
            to_string(i),
            to_string(average)
        });
    }
    table.make();
}
void five() {
    CSV table("avg-vars-over-time", vector<string>{
        "gen",
        "1 (avg)", "2 (avg)", "3 (avg)",
        "4 (avg)", "5 (avg)", "6 (avg)",
        "7 (avg)", "8 (avg)", "9 (avg)",
    });
    int g = number("Generations run:"), m = 100;
    for (int i = 1; i < g+1; ++i) {
        auto models = Generational::load_gen(i, m);
        unordered_map<int, double> avg_vars{
            {1, 0}, {2, 0}, {3, 0},
            {4, 0}, {5, 0}, {6, 0},
            {7, 0}, {8, 0}, {9, 0},
        };
        for (auto model : models) for (auto var : model.variables)
            avg_vars[var.first] += var.second;
        vector<string> to_push{to_string(i)};
        for (int i = 1; i < 10; ++i) to_push.push_back(to_string(avg_vars[i]/m));
        table.push(to_push);
    }
    table.make();
}
void six() {
    int top = number("Depth of search:");
    CSV table("top-"+to_string(top)+"-avg-vars-over-time", vector<string>{
        "gen",
        "1 (avg)", "2 (avg)", "3 (avg)",
        "4 (avg)", "5 (avg)", "6 (avg)",
        "7 (avg)", "8 (avg)", "9 (avg)",
    });
    int g = number("Generations run:"), m = 100;
    for (int i = 1; i < g+1; ++i) {
        auto models = Generational::load_best(i, m, top);
        unordered_map<int, double> avg_vars{
            {1, 0}, {2, 0}, {3, 0},
            {4, 0}, {5, 0}, {6, 0},
            {7, 0}, {8, 0}, {9, 0},
        };
        for (auto model : models) for (auto var : model.variables)
            avg_vars[var.first] += var.second;
        vector<string> to_push{to_string(i)};
        for (int i = 1; i < 10; ++i) to_push.push_back(to_string(avg_vars[i]/m));
        table.push(to_push);
    }
    table.make();
}
void seven() {
    int bottom = number("Depth of search:");
    CSV table("bottom-"+to_string(bottom)+"-avg-vars-over-time", vector<string>{
        "gen",
        "1 (avg)", "2 (avg)", "3 (avg)",
        "4 (avg)", "5 (avg)", "6 (avg)",
        "7 (avg)", "8 (avg)", "9 (avg)",
    });
    int g = number("Generations run:"), m = 100;
    for (int i = 1; i < g+1; ++i) {
        auto models = Generational::load_best(i, m, bottom * -1);
        unordered_map<int, double> avg_vars{
            {1, 0}, {2, 0}, {3, 0},
            {4, 0}, {5, 0}, {6, 0},
            {7, 0}, {8, 0}, {9, 0},
        };
        for (auto model : models) for (auto var : model.variables)
            avg_vars[var.first] += var.second;
        vector<string> to_push{to_string(i)};
        for (int i = 1; i < 10; ++i) to_push.push_back(to_string(avg_vars[i]/m));
        table.push(to_push);
    }
    table.make();
}

int main() {
    vector<string> types;
    types.push_back("1  ==> NEW TOP MODELS OVER TIME");
    // avg avg solve for all models over all generations
    types.push_back("2  ==> AVERAGE AVERAGE SOLVE OVER TIME");
    types.push_back("3  ==> AVERAGE TOP SOLVES OVER TIME");
    types.push_back("4  ==> AVERAGE BOTTOM SOLVE OVER TIME");
    // avg variables over generations
    types.push_back("5  ==> AVERAGE VARIABLES OVER TIME");
    types.push_back("6  ==> AVERAGE TOP VARIABLES OVER TIME");
    types.push_back("7  ==> AVERAGE BOTTOM VARIABLES OVER TIME");
    cout << "Select analysis type:" << endl;
    for (auto type : types) cout << type << endl;
    int num; cin >> num;
    if (num == 1) one();
    if (num == 2) two();
    if (num == 3) three();
    if (num == 4) four();
    if (num == 5) five();
    if (num == 6) six();
    if (num == 7) seven();
}
