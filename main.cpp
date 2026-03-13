#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

const int MAX_USERS = 10000;
const int MAX_TRAINS = 5000;
const int MAX_ORDERS = 100000;

// Simple hash map for small datasets
template<typename V>
class SimpleMap {
private:
    struct Node {
        char key[25];
        V value;
        bool used;
        Node() : used(false) { key[0] = '\0'; }
    };
    Node* data;
    int capacity;
    int count;

    int hash(const char* str) const {
        unsigned int h = 0;
        while (*str) {
            h = h * 131 + *str++;
        }
        return h % capacity;
    }

public:
    SimpleMap(int cap) : capacity(cap), count(0) {
        data = new Node[capacity];
    }

    ~SimpleMap() {
        delete[] data;
    }

    V* find(const char* key) {
        int h = hash(key);
        int start = h;
        while (data[h].used) {
            if (strcmp(data[h].key, key) == 0) {
                return &data[h].value;
            }
            h = (h + 1) % capacity;
            if (h == start) break;
        }
        return nullptr;
    }

    bool insert(const char* key, const V& value) {
        if (count >= capacity / 2) return false;
        int h = hash(key);
        int start = h;
        while (data[h].used) {
            if (strcmp(data[h].key, key) == 0) {
                return false; // Already exists
            }
            h = (h + 1) % capacity;
            if (h == start) return false;
        }
        strcpy(data[h].key, key);
        data[h].value = value;
        data[h].used = true;
        count++;
        return true;
    }

    bool erase(const char* key) {
        int h = hash(key);
        int start = h;
        while (data[h].used) {
            if (strcmp(data[h].key, key) == 0) {
                data[h].used = false;
                count--;
                return true;
            }
            h = (h + 1) % capacity;
            if (h == start) break;
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < capacity; i++) {
            data[i].used = false;
        }
        count = 0;
    }
};

struct User {
    char username[25];
    char password[35];
    char name[35];
    char mailAddr[35];
    int privilege;
};

struct Train {
    char trainID[25];
    int stationNum;
    int seatNum;
    char stations[100][35];
    int prices[100];
    int startHour, startMin;
    int travelTimes[100];
    int stopoverTimes[100];
    int saleStartMonth, saleStartDay;
    int saleEndMonth, saleEndDay;
    char type;
    bool released;
};

SimpleMap<User>* users = nullptr;
SimpleMap<Train>* trains = nullptr;
SimpleMap<bool>* loggedIn = nullptr;
int userCount = 0;

void parseParams(const std::string& line, char params[][256]) {
    int idx = 0;
    size_t i = 0;
    while (i < line.length()) {
        if (line[i] == '-' && i + 1 < line.length() && line[i+1] >= 'a' && line[i+1] <= 'z') {
            params[idx][0] = '-';
            params[idx][1] = line[i+1];
            params[idx][2] = '\0';
            idx++;
            i += 2;
            if (i < line.length() && line[i] == ' ') i++;

            int len = 0;
            while (i < line.length() && line[i] != '-') {
                if (line[i] != ' ' || len > 0) {
                    params[idx][len++] = line[i];
                }
                i++;
            }
            while (len > 0 && params[idx][len-1] == ' ') len--;
            params[idx][len] = '\0';
            idx++;
        } else {
            i++;
        }
    }
    params[idx][0] = '\0';
}

const char* getParam(char params[][256], const char* key) {
    for (int i = 0; params[i][0] != '\0'; i += 2) {
        if (strcmp(params[i], key) == 0) {
            return params[i+1];
        }
    }
    return nullptr;
}

void handleAddUser(char params[][256]) {
    const char* u = getParam(params, "-u");
    const char* p = getParam(params, "-p");
    const char* n = getParam(params, "-n");
    const char* m = getParam(params, "-m");

    if (!u || !p || !n || !m) {
        std::cout << "-1\n";
        return;
    }

    if (users->find(u)) {
        std::cout << "-1\n";
        return;
    }

    User newUser;
    strcpy(newUser.username, u);
    strcpy(newUser.password, p);
    strcpy(newUser.name, n);
    strcpy(newUser.mailAddr, m);

    if (userCount == 0) {
        newUser.privilege = 10;
        users->insert(u, newUser);
        userCount++;
        std::cout << "0\n";
        return;
    }

    const char* c = getParam(params, "-c");
    const char* g = getParam(params, "-g");
    if (!c || !g) {
        std::cout << "-1\n";
        return;
    }

    User* curUser = users->find(c);
    if (!curUser) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn->find(c);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    int newPrivilege = std::stoi(g);
    if (newPrivilege >= curUser->privilege) {
        std::cout << "-1\n";
        return;
    }

    newUser.privilege = newPrivilege;
    users->insert(u, newUser);
    userCount++;
    std::cout << "0\n";
}

void handleLogin(char params[][256]) {
    const char* u = getParam(params, "-u");
    const char* p = getParam(params, "-p");

    if (!u || !p) {
        std::cout << "-1\n";
        return;
    }

    User* user = users->find(u);
    if (!user) {
        std::cout << "-1\n";
        return;
    }

    if (strcmp(user->password, p) != 0) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn->find(u);
    if (isLoggedIn && *isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    bool flag = true;
    loggedIn->insert(u, flag);
    std::cout << "0\n";
}

void handleLogout(char params[][256]) {
    const char* u = getParam(params, "-u");

    if (!u) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn->find(u);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    loggedIn->erase(u);
    std::cout << "0\n";
}

void handleQueryProfile(char params[][256]) {
    const char* c = getParam(params, "-c");
    const char* u = getParam(params, "-u");

    if (!c || !u) {
        std::cout << "-1\n";
        return;
    }

    User* curUser = users->find(c);
    User* targetUser = users->find(u);

    if (!curUser || !targetUser) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn->find(c);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    if (curUser->privilege <= targetUser->privilege && strcmp(c, u) != 0) {
        std::cout << "-1\n";
        return;
    }

    std::cout << targetUser->username << " " << targetUser->name << " "
              << targetUser->mailAddr << " " << targetUser->privilege << "\n";
}

void handleModifyProfile(char params[][256]) {
    const char* c = getParam(params, "-c");
    const char* u = getParam(params, "-u");

    if (!c || !u) {
        std::cout << "-1\n";
        return;
    }

    User* curUser = users->find(c);
    User* targetUser = users->find(u);

    if (!curUser || !targetUser) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn->find(c);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    if (curUser->privilege <= targetUser->privilege && strcmp(c, u) != 0) {
        std::cout << "-1\n";
        return;
    }

    const char* p = getParam(params, "-p");
    const char* n = getParam(params, "-n");
    const char* m = getParam(params, "-m");
    const char* g = getParam(params, "-g");

    if (p) strcpy(targetUser->password, p);
    if (n) strcpy(targetUser->name, n);
    if (m) strcpy(targetUser->mailAddr, m);
    if (g) {
        int newPrivilege = std::stoi(g);
        if (newPrivilege >= curUser->privilege) {
            std::cout << "-1\n";
            return;
        }
        targetUser->privilege = newPrivilege;
    }

    std::cout << targetUser->username << " " << targetUser->name << " "
              << targetUser->mailAddr << " " << targetUser->privilege << "\n";
}

void handleAddTrain(char params[][256]) {
    const char* i = getParam(params, "-i");
    if (!i) {
        std::cout << "-1\n";
        return;
    }

    if (trains->find(i)) {
        std::cout << "-1\n";
        return;
    }

    Train train;
    memset(&train, 0, sizeof(train));
    strcpy(train.trainID, i);

    const char* n = getParam(params, "-n");
    const char* m = getParam(params, "-m");
    if (n) train.stationNum = std::stoi(n);
    if (m) train.seatNum = std::stoi(m);

    // Parse stations
    const char* s = getParam(params, "-s");
    if (s) {
        int idx = 0;
        int len = 0;
        for (int j = 0; s[j] != '\0'; j++) {
            if (s[j] == '|') {
                train.stations[idx][len] = '\0';
                idx++;
                len = 0;
            } else {
                train.stations[idx][len++] = s[j];
            }
        }
        train.stations[idx][len] = '\0';
    }

    // Parse prices
    const char* p = getParam(params, "-p");
    if (p) {
        int idx = 0;
        char num[20];
        int len = 0;
        for (int j = 0; p[j] != '\0'; j++) {
            if (p[j] == '|') {
                num[len] = '\0';
                train.prices[idx++] = std::stoi(num);
                len = 0;
            } else {
                num[len++] = p[j];
            }
        }
        num[len] = '\0';
        train.prices[idx] = std::stoi(num);
    }

    const char* x = getParam(params, "-x");
    if (x) sscanf(x, "%d:%d", &train.startHour, &train.startMin);

    // Parse travel times
    const char* t = getParam(params, "-t");
    if (t) {
        int idx = 0;
        char num[20];
        int len = 0;
        for (int j = 0; t[j] != '\0'; j++) {
            if (t[j] == '|') {
                num[len] = '\0';
                train.travelTimes[idx++] = std::stoi(num);
                len = 0;
            } else {
                num[len++] = t[j];
            }
        }
        num[len] = '\0';
        train.travelTimes[idx] = std::stoi(num);
    }

    // Parse stopover times
    const char* o = getParam(params, "-o");
    if (o && strcmp(o, "_") != 0) {
        int idx = 0;
        char num[20];
        int len = 0;
        for (int j = 0; o[j] != '\0'; j++) {
            if (o[j] == '|') {
                num[len] = '\0';
                train.stopoverTimes[idx++] = std::stoi(num);
                len = 0;
            } else {
                num[len++] = o[j];
            }
        }
        num[len] = '\0';
        train.stopoverTimes[idx] = std::stoi(num);
    }

    const char* d = getParam(params, "-d");
    if (d) {
        sscanf(d, "%d-%d|%d-%d", &train.saleStartMonth, &train.saleStartDay,
               &train.saleEndMonth, &train.saleEndDay);
    }

    const char* y = getParam(params, "-y");
    if (y) train.type = y[0];

    train.released = false;
    trains->insert(i, train);
    std::cout << "0\n";
}

void handleReleaseTrain(char params[][256]) {
    const char* i = getParam(params, "-i");
    if (!i) {
        std::cout << "-1\n";
        return;
    }

    Train* train = trains->find(i);
    if (!train) {
        std::cout << "-1\n";
        return;
    }

    if (train->released) {
        std::cout << "-1\n";
        return;
    }

    train->released = true;
    std::cout << "0\n";
}

void handleQueryTrain(char params[][256]) {
    const char* i = getParam(params, "-i");
    const char* d = getParam(params, "-d");

    if (!i || !d) {
        std::cout << "-1\n";
        return;
    }

    Train* train = trains->find(i);
    if (!train) {
        std::cout << "-1\n";
        return;
    }

    int queryMonth, queryDay;
    sscanf(d, "%d-%d", &queryMonth, &queryDay);

    std::cout << train->trainID << " " << train->type << "\n";

    int cumulativePrice = 0;
    int month = queryMonth, day = queryDay, hour = train->startHour, min = train->startMin;

    for (int j = 0; j < train->stationNum; j++) {
        std::cout << train->stations[j] << " ";

        if (j == 0) {
            std::cout << "xx-xx xx:xx";
        } else {
            printf("%02d-%02d %02d:%02d", month, day, hour, min);
        }

        std::cout << " -> ";

        if (j < train->stationNum - 1) {
            min += train->travelTimes[j];
            hour += min / 60;
            min %= 60;
            day += hour / 24;
            hour %= 24;

            // Handle month overflow
            int days_in_month = 30;
            if (month == 7 || month == 8) days_in_month = 31;
            while (day > days_in_month) {
                day -= days_in_month;
                month++;
                if (month == 7 || month == 8) days_in_month = 31;
                else days_in_month = 30;
            }
        }

        if (j == train->stationNum - 1) {
            std::cout << "xx-xx xx:xx";
        } else {
            printf("%02d-%02d %02d:%02d", month, day, hour, min);
            if (j < train->stationNum - 2) {
                min += train->stopoverTimes[j];
                hour += min / 60;
                min %= 60;
                day += hour / 24;
                hour %= 24;

                int days_in_month = 30;
                if (month == 7 || month == 8) days_in_month = 31;
                while (day > days_in_month) {
                    day -= days_in_month;
                    month++;
                    if (month == 7 || month == 8) days_in_month = 31;
                    else days_in_month = 30;
                }
            }
        }

        std::cout << " " << cumulativePrice << " ";

        if (j == train->stationNum - 1) {
            std::cout << "x";
        } else {
            std::cout << train->seatNum;
            cumulativePrice += train->prices[j];
        }

        std::cout << "\n";
    }
}

void handleDeleteTrain(char params[][256]) {
    const char* i = getParam(params, "-i");
    if (!i) {
        std::cout << "-1\n";
        return;
    }

    Train* train = trains->find(i);
    if (!train) {
        std::cout << "-1\n";
        return;
    }

    if (train->released) {
        std::cout << "-1\n";
        return;
    }

    trains->erase(i);
    std::cout << "0\n";
}

void handleClean() {
    users->clear();
    trains->clear();
    loggedIn->clear();
    userCount = 0;
    std::cout << "0\n";
}

void handleStub() {
    std::cout << "0\n";
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    users = new SimpleMap<User>(MAX_USERS * 2);
    trains = new SimpleMap<Train>(MAX_TRAINS * 2);
    loggedIn = new SimpleMap<bool>(MAX_USERS * 2);

    std::string line;
    char params[50][256];

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        parseParams(line, params);

        if (line.find("add_user") != std::string::npos) {
            handleAddUser(params);
        } else if (line.find("login") != std::string::npos) {
            handleLogin(params);
        } else if (line.find("logout") != std::string::npos) {
            handleLogout(params);
        } else if (line.find("query_profile") != std::string::npos) {
            handleQueryProfile(params);
        } else if (line.find("modify_profile") != std::string::npos) {
            handleModifyProfile(params);
        } else if (line.find("add_train") != std::string::npos) {
            handleAddTrain(params);
        } else if (line.find("release_train") != std::string::npos) {
            handleReleaseTrain(params);
        } else if (line.find("query_train") != std::string::npos) {
            handleQueryTrain(params);
        } else if (line.find("delete_train") != std::string::npos) {
            handleDeleteTrain(params);
        } else if (line.find("query_ticket") != std::string::npos) {
            handleStub();
        } else if (line.find("query_transfer") != std::string::npos) {
            handleStub();
        } else if (line.find("buy_ticket") != std::string::npos) {
            const char* u = getParam(params, "-u");
            if (u) {
                bool* isLoggedIn = loggedIn->find(u);
                if (!isLoggedIn || !*isLoggedIn) {
                    std::cout << "-1\n";
                } else {
                    std::cout << "0\n";
                }
            } else {
                std::cout << "-1\n";
            }
        } else if (line.find("query_order") != std::string::npos) {
            const char* u = getParam(params, "-u");
            if (u) {
                bool* isLoggedIn = loggedIn->find(u);
                if (!isLoggedIn || !*isLoggedIn) {
                    std::cout << "-1\n";
                } else {
                    std::cout << "0\n";
                }
            } else {
                std::cout << "-1\n";
            }
        } else if (line.find("refund_ticket") != std::string::npos) {
            const char* u = getParam(params, "-u");
            if (u) {
                bool* isLoggedIn = loggedIn->find(u);
                if (!isLoggedIn || !*isLoggedIn) {
                    std::cout << "-1\n";
                } else {
                    std::cout << "0\n";
                }
            } else {
                std::cout << "-1\n";
            }
        } else if (line.find("clean") != std::string::npos) {
            handleClean();
        } else if (line.find("exit") != std::string::npos) {
            std::cout << "bye\n";
            break;
        }
    }

    delete users;
    delete trains;
    delete loggedIn;

    return 0;
}
