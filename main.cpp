#include <iostream>
#include <unordered_map>
#include <string>
using namespace std;
using namespace std;

int main() {
    unordered_map<string, string> db;

    cout << "Simple Redis-like Store\n";

    string command;
    while (true) {
        cout << "> ";
        cin >> command;

        if (command == "SET") {
            string key, value;
            cin >> key >> value;
            db[key] = value;
            cout << "OK\n";
        } 
        else if (command == "GET") {
            string key;
            cin >> key;
            if (db.count(key)) {
                cout << db[key] << "\n";
            } else {
                cout << "NULL\n";
            }
        } 
        else if (command == "EXIT") {
            break;
        } 
        else {
            cout << "Unknown command\n";
        }
    }

    return 0;
}