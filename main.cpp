#include <iostream>
#include <cstring>
#include <fstream>
#include <string>

// Custom map implementation using sorted array
template<typename K, typename V, int MAXN = 10000>
class Map {
private:
    struct Node {
        K key;
        V value;
        bool valid;
        Node() : valid(false) {}
    };
    Node data[MAXN];
    int size;

public:
    Map() : size(0) {}

    V* find(const K& key) {
        for (int i = 0; i < size; i++) {
            if (data[i].valid && data[i].key == key) {
                return &data[i].value;
            }
        }
        return nullptr;
    }

    const V* find(const K& key) const {
        for (int i = 0; i < size; i++) {
            if (data[i].valid && data[i].key == key) {
                return &data[i].value;
            }
        }
        return nullptr;
    }

    bool insert(const K& key, const V& value) {
        V* existing = find(key);
        if (existing) return false;
        if (size >= MAXN) return false;
        data[size].key = key;
        data[size].value = value;
        data[size].valid = true;
        size++;
        return true;
    }

    bool erase(const K& key) {
        for (int i = 0; i < size; i++) {
            if (data[i].valid && data[i].key == key) {
                data[i].valid = false;
                return true;
            }
        }
        return false;
    }

    void clear() {
        size = 0;
    }
};

// Custom vector implementation
template<typename T, int MAXN = 100000>
class Vector {
private:
    T data[MAXN];
    int size;

public:
    Vector() : size(0) {}

    void push_back(const T& value) {
        if (size < MAXN) {
            data[size++] = value;
        }
    }

    T& operator[](int index) {
        return data[index];
    }

    const T& operator[](int index) const {
        return data[index];
    }

    int getSize() const {
        return size;
    }

    void clear() {
        size = 0;
    }

    T* begin() { return data; }
    T* end() { return data + size; }
};

// User structure
struct User {
    char username[25];
    char password[35];
    char name[35];
    char mailAddr[35];
    int privilege;

    User() {
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        memset(name, 0, sizeof(name));
        memset(mailAddr, 0, sizeof(mailAddr));
        privilege = 0;
    }
};

// Train structure
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

    Train() {
        memset(trainID, 0, sizeof(trainID));
        stationNum = 0;
        seatNum = 0;
        released = false;
    }
};

// Order structure
struct Order {
    char username[25];
    char trainID[25];
    int num;
    int totalPrice;
    char fromStation[35];
    char toStation[35];
    int startMonth, startDay, startHour, startMin;
    int endMonth, endDay, endHour, endMin;
    int status; // 0: success, 1: pending, 2: refunded
    int trainStartMonth, trainStartDay; // Date when train departs from starting station
    int fromIndex, toIndex; // Station indices

    Order() {
        memset(username, 0, sizeof(username));
        memset(trainID, 0, sizeof(trainID));
        num = 0;
        totalPrice = 0;
        status = 0;
    }
};

// Ticket availability structure
struct TrainTickets {
    int seats[100][100]; // seats[day][segment]

    TrainTickets() {
        memset(seats, 0, sizeof(seats));
    }
};

// Global data
Map<std::string, User> users;
Map<std::string, Train> trains;
Map<std::string, TrainTickets> trainTickets;
Map<std::string, bool> loggedIn;
Vector<Order> orders;
Vector<Order> pendingQueue;

int userCount = 0;

// Helper functions
void parseTokens(const std::string& line, Vector<std::string>& tokens) {
    tokens.clear();
    std::string current;
    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] == ' ' || line[i] == '\n') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += line[i];
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
}

void parseByDelimiter(const std::string& str, char delim, Vector<std::string>& result) {
    result.clear();
    std::string current;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == delim) {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else {
            current += str[i];
        }
    }
    if (!current.empty()) {
        result.push_back(current);
    }
}

int dateToDay(int month, int day) {
    // June 1 = day 0, August 31 = day 91
    if (month == 6) return day - 1;
    if (month == 7) return 30 + day - 1;
    if (month == 8) return 61 + day - 1;
    return 0;
}

void dayToDate(int day, int& month, int& day_out) {
    if (day < 30) {
        month = 6;
        day_out = day + 1;
    } else if (day < 61) {
        month = 7;
        day_out = day - 30 + 1;
    } else {
        month = 8;
        day_out = day - 61 + 1;
    }
}

void addTime(int& month, int& day, int& hour, int& min, int minutes) {
    min += minutes;
    hour += min / 60;
    min %= 60;
    day += hour / 24;
    hour %= 24;

    while (true) {
        int daysInMonth = (month == 6 || month == 8) ? 31 : 31;
        if (month == 7) daysInMonth = 31;
        if (month == 6) daysInMonth = 30;

        if (day > daysInMonth) {
            day -= daysInMonth;
            month++;
        } else {
            break;
        }
    }
}

// Command handlers
void handleAddUser(const Map<std::string, std::string>& params) {
    auto c = params.find(std::string("-c"));
    auto u = params.find(std::string("-u"));
    auto p = params.find(std::string("-p"));
    auto n = params.find(std::string("-n"));
    auto m = params.find(std::string("-m"));
    auto g = params.find(std::string("-g"));

    if (!u || !p || !n || !m) {
        std::cout << "-1\n";
        return;
    }

    // Check if user already exists
    if (users.find(*u)) {
        std::cout << "-1\n";
        return;
    }

    // First user
    if (userCount == 0) {
        User newUser;
        strcpy(newUser.username, u->c_str());
        strcpy(newUser.password, p->c_str());
        strcpy(newUser.name, n->c_str());
        strcpy(newUser.mailAddr, m->c_str());
        newUser.privilege = 10;
        users.insert(*u, newUser);
        userCount++;
        std::cout << "0\n";
        return;
    }

    // Not first user
    if (!c || !g) {
        std::cout << "-1\n";
        return;
    }

    User* curUser = users.find(*c);
    if (!curUser) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn.find(*c);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    int newPrivilege = std::stoi(*g);
    if (newPrivilege >= curUser->privilege) {
        std::cout << "-1\n";
        return;
    }

    User newUser;
    strcpy(newUser.username, u->c_str());
    strcpy(newUser.password, p->c_str());
    strcpy(newUser.name, n->c_str());
    strcpy(newUser.mailAddr, m->c_str());
    newUser.privilege = newPrivilege;
    users.insert(*u, newUser);
    userCount++;
    std::cout << "0\n";
}

void handleLogin(const Map<std::string, std::string>& params) {
    auto u = params.find(std::string("-u"));
    auto p = params.find(std::string("-p"));

    if (!u || !p) {
        std::cout << "-1\n";
        return;
    }

    User* user = users.find(*u);
    if (!user) {
        std::cout << "-1\n";
        return;
    }

    if (strcmp(user->password, p->c_str()) != 0) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn.find(*u);
    if (isLoggedIn && *isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    loggedIn.insert(*u, true);
    std::cout << "0\n";
}

void handleLogout(const Map<std::string, std::string>& params) {
    auto u = params.find(std::string("-u"));

    if (!u) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn.find(*u);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    loggedIn.erase(*u);
    std::cout << "0\n";
}

void handleQueryProfile(const Map<std::string, std::string>& params) {
    auto c = params.find(std::string("-c"));
    auto u = params.find(std::string("-u"));

    if (!c || !u) {
        std::cout << "-1\n";
        return;
    }

    User* curUser = users.find(*c);
    User* targetUser = users.find(*u);

    if (!curUser || !targetUser) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn.find(*c);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    if (curUser->privilege <= targetUser->privilege && *c != *u) {
        std::cout << "-1\n";
        return;
    }

    std::cout << targetUser->username << " " << targetUser->name << " "
              << targetUser->mailAddr << " " << targetUser->privilege << "\n";
}

void handleModifyProfile(const Map<std::string, std::string>& params) {
    auto c = params.find(std::string("-c"));
    auto u = params.find(std::string("-u"));

    if (!c || !u) {
        std::cout << "-1\n";
        return;
    }

    User* curUser = users.find(*c);
    User* targetUser = users.find(*u);

    if (!curUser || !targetUser) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn.find(*c);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    if (curUser->privilege <= targetUser->privilege && *c != *u) {
        std::cout << "-1\n";
        return;
    }

    auto p = params.find(std::string("-p"));
    auto n = params.find(std::string("-n"));
    auto m = params.find(std::string("-m"));
    auto g = params.find(std::string("-g"));

    if (p) strcpy(targetUser->password, p->c_str());
    if (n) strcpy(targetUser->name, n->c_str());
    if (m) strcpy(targetUser->mailAddr, m->c_str());
    if (g) {
        int newPrivilege = std::stoi(*g);
        if (newPrivilege >= curUser->privilege) {
            std::cout << "-1\n";
            return;
        }
        targetUser->privilege = newPrivilege;
    }

    std::cout << targetUser->username << " " << targetUser->name << " "
              << targetUser->mailAddr << " " << targetUser->privilege << "\n";
}

void handleAddTrain(const Map<std::string, std::string>& params) {
    auto i = params.find(std::string("-i"));

    if (!i) {
        std::cout << "-1\n";
        return;
    }

    if (trains.find(*i)) {
        std::cout << "-1\n";
        return;
    }

    Train train;
    strcpy(train.trainID, i->c_str());

    auto n = params.find(std::string("-n"));
    auto m = params.find(std::string("-m"));
    auto s = params.find(std::string("-s"));
    auto p = params.find(std::string("-p"));
    auto x = params.find(std::string("-x"));
    auto t = params.find(std::string("-t"));
    auto o = params.find(std::string("-o"));
    auto d = params.find(std::string("-d"));
    auto y = params.find(std::string("-y"));

    if (!n || !m || !s || !p || !x || !t || !o || !d || !y) {
        std::cout << "-1\n";
        return;
    }

    train.stationNum = std::stoi(*n);
    train.seatNum = std::stoi(*m);

    Vector<std::string> stations;
    parseByDelimiter(*s, '|', stations);
    for (int j = 0; j < train.stationNum; j++) {
        strcpy(train.stations[j], stations[j].c_str());
    }

    Vector<std::string> prices;
    parseByDelimiter(*p, '|', prices);
    for (int j = 0; j < train.stationNum - 1; j++) {
        train.prices[j] = std::stoi(prices[j]);
    }

    sscanf(x->c_str(), "%d:%d", &train.startHour, &train.startMin);

    Vector<std::string> travelTimes;
    parseByDelimiter(*t, '|', travelTimes);
    for (int j = 0; j < train.stationNum - 1; j++) {
        train.travelTimes[j] = std::stoi(travelTimes[j]);
    }

    if (*o != "_") {
        Vector<std::string> stopoverTimes;
        parseByDelimiter(*o, '|', stopoverTimes);
        for (int j = 0; j < train.stationNum - 2; j++) {
            train.stopoverTimes[j] = std::stoi(stopoverTimes[j]);
        }
    }

    Vector<std::string> saleDates;
    parseByDelimiter(*d, '|', saleDates);
    sscanf(saleDates[0].c_str(), "%d-%d", &train.saleStartMonth, &train.saleStartDay);
    sscanf(saleDates[1].c_str(), "%d-%d", &train.saleEndMonth, &train.saleEndDay);

    train.type = (*y)[0];
    train.released = false;

    trains.insert(*i, train);
    std::cout << "0\n";
}

void handleReleaseTrain(const Map<std::string, std::string>& params) {
    auto i = params.find(std::string("-i"));

    if (!i) {
        std::cout << "-1\n";
        return;
    }

    Train* train = trains.find(*i);
    if (!train) {
        std::cout << "-1\n";
        return;
    }

    if (train->released) {
        std::cout << "-1\n";
        return;
    }

    train->released = true;

    // Initialize ticket availability
    TrainTickets tickets;
    int startDay = dateToDay(train->saleStartMonth, train->saleStartDay);
    int endDay = dateToDay(train->saleEndMonth, train->saleEndDay);

    for (int day = startDay; day <= endDay; day++) {
        for (int seg = 0; seg < train->stationNum - 1; seg++) {
            tickets.seats[day][seg] = train->seatNum;
        }
    }

    trainTickets.insert(*i, tickets);
    std::cout << "0\n";
}

void handleQueryTrain(const Map<std::string, std::string>& params) {
    auto i = params.find(std::string("-i"));
    auto d = params.find(std::string("-d"));

    if (!i || !d) {
        std::cout << "-1\n";
        return;
    }

    Train* train = trains.find(*i);
    if (!train) {
        std::cout << "-1\n";
        return;
    }

    int queryMonth, queryDay;
    sscanf(d->c_str(), "%d-%d", &queryMonth, &queryDay);
    int queryDayNum = dateToDay(queryMonth, queryDay);

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
            addTime(month, day, hour, min, train->travelTimes[j]);
        }

        if (j == train->stationNum - 1) {
            std::cout << "xx-xx xx:xx";
        } else {
            printf("%02d-%02d %02d:%02d", month, day, hour, min);
            if (j < train->stationNum - 2) {
                addTime(month, day, hour, min, train->stopoverTimes[j]);
            }
        }

        std::cout << " " << cumulativePrice << " ";

        if (j == train->stationNum - 1) {
            std::cout << "x";
        } else {
            TrainTickets* tickets = trainTickets.find(*i);
            if (tickets && train->released) {
                std::cout << tickets->seats[queryDayNum][j];
            } else {
                std::cout << train->seatNum;
            }
            cumulativePrice += train->prices[j];
        }

        std::cout << "\n";
    }
}

void handleDeleteTrain(const Map<std::string, std::string>& params) {
    auto i = params.find(std::string("-i"));

    if (!i) {
        std::cout << "-1\n";
        return;
    }

    Train* train = trains.find(*i);
    if (!train) {
        std::cout << "-1\n";
        return;
    }

    if (train->released) {
        std::cout << "-1\n";
        return;
    }

    trains.erase(*i);
    std::cout << "0\n";
}

void handleQueryTicket(const Map<std::string, std::string>& params) {
    auto s = params.find(std::string("-s"));
    auto t = params.find(std::string("-t"));
    auto d = params.find(std::string("-d"));

    if (!s || !t || !d) {
        std::cout << "0\n";
        return;
    }

    std::cout << "0\n";
}

void handleQueryTransfer(const Map<std::string, std::string>& params) {
    std::cout << "0\n";
}

void handleBuyTicket(const Map<std::string, std::string>& params) {
    auto u = params.find(std::string("-u"));

    if (!u) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn.find(*u);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    std::cout << "0\n";
}

void handleQueryOrder(const Map<std::string, std::string>& params) {
    auto u = params.find(std::string("-u"));

    if (!u) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn.find(*u);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    std::cout << "0\n";
}

void handleRefundTicket(const Map<std::string, std::string>& params) {
    auto u = params.find(std::string("-u"));

    if (!u) {
        std::cout << "-1\n";
        return;
    }

    bool* isLoggedIn = loggedIn.find(*u);
    if (!isLoggedIn || !*isLoggedIn) {
        std::cout << "-1\n";
        return;
    }

    std::cout << "0\n";
}

void handleClean() {
    users.clear();
    trains.clear();
    trainTickets.clear();
    loggedIn.clear();
    orders.clear();
    pendingQueue.clear();
    userCount = 0;
    std::cout << "0\n";
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string line;
    while (std::getline(std::cin, line)) {
        Vector<std::string> tokens;
        parseTokens(line, tokens);

        if (tokens.getSize() == 0) continue;

        std::string cmd = tokens[0];

        // Parse parameters
        Map<std::string, std::string> params;
        for (int i = 1; i < tokens.getSize(); i += 2) {
            if (i + 1 < tokens.getSize()) {
                params.insert(tokens[i], tokens[i + 1]);
            }
        }

        if (cmd == "add_user") {
            handleAddUser(params);
        } else if (cmd == "login") {
            handleLogin(params);
        } else if (cmd == "logout") {
            handleLogout(params);
        } else if (cmd == "query_profile") {
            handleQueryProfile(params);
        } else if (cmd == "modify_profile") {
            handleModifyProfile(params);
        } else if (cmd == "add_train") {
            handleAddTrain(params);
        } else if (cmd == "release_train") {
            handleReleaseTrain(params);
        } else if (cmd == "query_train") {
            handleQueryTrain(params);
        } else if (cmd == "delete_train") {
            handleDeleteTrain(params);
        } else if (cmd == "query_ticket") {
            handleQueryTicket(params);
        } else if (cmd == "query_transfer") {
            handleQueryTransfer(params);
        } else if (cmd == "buy_ticket") {
            handleBuyTicket(params);
        } else if (cmd == "query_order") {
            handleQueryOrder(params);
        } else if (cmd == "refund_ticket") {
            handleRefundTicket(params);
        } else if (cmd == "clean") {
            handleClean();
        } else if (cmd == "exit") {
            std::cout << "bye\n";
            break;
        }
    }

    return 0;
}
