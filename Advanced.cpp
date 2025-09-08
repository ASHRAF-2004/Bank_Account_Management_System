#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <ctime>     // for time/ctime
#include <fstream>   // for binary file operations
#include <cstring>   // for strncpy
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <limits>
#include <regex>
#ifdef _WIN32
#include <windows.h>
#undef max
#include <io.h>
#include <fcntl.h>
#endif
using namespace std;

const string DATA_FILE = "accounts.dat";
const string LOG_FILE  = "logs.dat";

void printCentered(const std::string& s);
void printCenteredInline(const string& s);

string formatAccNo(int acc) {
    stringstream ss;
    ss << setw(4) << setfill('0') << acc;
    return ss.str();
}

string formatPin(int pin) {
    stringstream ss;
    ss << setw(4) << setfill('0') << pin;
    return ss.str();
}

bool isDigits(const string& s) {
    return !s.empty() && all_of(s.begin(), s.end(), ::isdigit);
}

bool isAlphaSpace(const string& s) {
    return !s.empty() && all_of(s.begin(), s.end(), [](char c){ return isalpha(c) || c == ' '; });
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(' ');
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(' ');
    return s.substr(start, end - start + 1);
}

long long readNumber(const string& prompt, size_t minDigits = 1) {
    string input;
    while (true) {
        printCenteredInline(prompt);
        getline(cin, input);
        if (isDigits(input) && input.size() >= minDigits) return stoll(input);
        printCentered("Invalid input.");
    }
}

string readName(const string& prompt, size_t minLen = 1) {
    string input;
    while (true) {
        printCenteredInline(prompt);
        getline(cin, input);
        string t = trim(input);
        size_t letters = count_if(t.begin(), t.end(), ::isalpha);
        if (letters >= minLen && isAlphaSpace(t)) return t;
        printCentered("Invalid input.");
    }
}

int readPin(const string& prompt) {
    string input;
    while (true) {
        printCenteredInline(prompt);
        getline(cin, input);
        if (isDigits(input) && input.size() == 4) return stoi(input);
        printCentered("PIN must be exactly 4 digits.");
    }
}

string readPassport(const string& prompt) {
    static const regex pattern("^[A-Z0-9]{6,9}$");
    string input;
    while (true) {
        printCenteredInline(prompt);
        getline(cin, input);
        string cleaned;
        cleaned.reserve(input.size());
        for (char c : input) {
            if (!isspace(static_cast<unsigned char>(c))) {
                cleaned.push_back(toupper(static_cast<unsigned char>(c)));
            }
        }
        if (cleaned.empty()) {
            printCentered("Please enter your passport number.");
        } else if (!regex_match(cleaned, pattern)) {
            printCentered("Passport number must be 6-9 letters/digits, no spaces or symbols.");
        } else {
            return cleaned;
        }
    }
}

struct AccountRecord {
    int accNo;
    char name[100];
    char ic[50];
    char gender;
    char typeCS[10];
    int pin;
    long long balance;
};


// ---------- Console helpers ----------
int getConsoleWidth() {
#ifdef _WIN32
    // Works on Windows. If you compile elsewhere, it falls back to 120.
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        if (width > 0) return width;
    }
#endif
    return 120; // fallback
}

// printCentered function (example)␊
// On non-Windows platforms the previous implementation attempted to call
// Windows API functions unconditionally, which breaks compilation.  We now
// use the portable getConsoleWidth() helper instead.
void printCentered(const std::string& s) {
    int consoleWidth = getConsoleWidth();
    int padding = (consoleWidth - static_cast<int>(s.length())) / 2;
    if (padding < 0) padding = 0;

    std::cout << std::string(padding, ' ') << s << "\n";
}


// NEW: inline prompt version
void printCenteredInline(const string& s) {
    int width = getConsoleWidth();
    int n = (int)s.size();
    if (n >= width) { cout << s; return; }   // no '\n'
    int left = (width - n) / 2;
    if (left < 0) left = 0; // safety: don't go negative
    cout << string(left, ' ') << s;          // no '\n'
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void maximizeConsole() {
#ifdef _WIN32
    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_MAXIMIZE);
#endif
}

void setBlueBackgroundWindows() {
#ifdef _WIN32
    system("Color 2"); // E = bright yellow (close to golden) // 6 = golden yellow text, black background // 6F = golden text on white background
#endif
}

// Print the centered LOGIN screen
void renderLoginScreen() {
    setBlueBackgroundWindows();
    clearScreen();

    const char* BANNER = R"(
 /$$$$$$$                      /$$                /$$$$$$                        /$$                            
| $$__  $$                    | $$               /$$__  $$                      | $$                            
| $$  \ $$  /$$$$$$  /$$$$$$$ | $$   /$$        | $$  \__/ /$$   /$$  /$$$$$$$ /$$$$$$    /$$$$$$  /$$$$$$/$$$$ 
| $$$$$$$  |____  $$| $$__  $$| $$  /$$/        |  $$$$$$ | $$  | $$ /$$_____/|_  $$_/   /$$__  $$| $$_  $$_  $$
| $$__  $$  /$$$$$$$| $$  \ $$| $$$$$$/          \____  $$| $$  | $$|  $$$$$$   | $$    | $$$$$$$$| $$ \ $$ \ $$
| $$  \ $$ /$$__  $$| $$  | $$| $$_  $$          /$$  \ $$| $$  | $$ \____  $$  | $$ /$$| $$_____/| $$ | $$ | $$
| $$$$$$$/|  $$$$$$$| $$  | $$| $$ \  $$        |  $$$$$$/|  $$$$$$$ /$$$$$$$/  |  $$$$/|  $$$$$$$| $$ | $$ | $$
|_______/  \_______/|__/  |__/|__/  \__/         \______/  \____  $$|_______/    \___/   \_______/|__/ |__/ |__/
                                                           /$$  | $$                                            
                                                          |  $$$$$$/                                            
                                                           \______/                                             

)";


    const char* MADE_BY = "Made by ASHRAF ALI HUSSEIN AL-SALOUL";
    const char* LINE = "********************************";
    const char* TITLE = "********** LOGIN || PANEL **********";
    const char* OPT1 = "*  Press 1 For ADMIN Login     *";
    const char* OPT2 = "*  Press 2 For STAFF Login     *";
    const char* OPT3 = "*  Press 3 For ATM/CDM Service *";
    const char* OPT4 = "*  Press 4 To Exit             *";

    // print banner centered, line by line
    {
        string s = BANNER;
        size_t start = 0;
        while (true) {
            size_t pos = s.find('\n', start);
            string line = (pos == string::npos) ? s.substr(start) : s.substr(start, pos - start);
            printCentered(line);
            if (pos == string::npos) break;
            start = pos + 1;
        }
    }
    printCentered("");
    printCentered(MADE_BY);
    printCentered("");
    printCentered(LINE);
    printCentered(TITLE);
    printCentered(LINE);
    printCentered(OPT1);
    printCentered(OPT2);
    printCentered(OPT3);
    printCentered(OPT4);
    printCentered(LINE);
    printCentered("");
}


// ======================= Log nodes (singly linked) =======================
struct LogNode {
    string text;
    LogNode* next;
    LogNode(const string& t) : text(t), next(NULL) {}
};

// Keep logs of deleted accounts
struct DeletedLogEntry {
    int accNo;
    LogNode* logs;           // head of the logs list
    DeletedLogEntry* next;
    DeletedLogEntry(int a, LogNode* h) : accNo(a), logs(h), next(NULL) {}
};

// ======================= Account (OOP) =======================
class Account {
public:
    int accNo;
    string name;
    string ic;      // passport or ID number
    char gender;      // 'M' or 'F'
    string typeCS;    // "Current" or "Savings"
    int pin;
    long long balance;
    LogNode* logHead; // singly linked list of logs

    Account(int a, const string& nm, const string& c, char g, const string& t, int p, long long b)
        : accNo(a), name(nm), ic(c), gender(g), typeCS(t), pin(p), balance(b), logHead(NULL) {
    }

    void addLog(const string& msg) {
        // append to preserve chronological order
        LogNode* n = new LogNode(msg);
        if (!logHead) {
            logHead = n;
            return;
        }
        LogNode* cur = logHead;
        while (cur->next) cur = cur->next;
        cur->next = n;
    }

    void printBrief() const {
        string gStr = (gender == 'M') ? "Male" : "Female";
        printCentered("Account No: " + formatAccNo(accNo) +
            "; Name: " + name +
            "; Gender: " + gStr +
            "; Balance: RM " + to_string(balance));
    }

    void printFull() const {
        string gStr = (gender == 'M') ? "Male" : "Female";
        printCentered("Account No: " + formatAccNo(accNo) +
            "; Name: " + name +
            "; Passport No: " + ic +
            "; Gender: " + gStr +
            "; Type: " + typeCS +
            "; PIN: " + formatPin(pin) +
            "; Balance: RM " + to_string(balance));
    }
};

// ======================= List Node for accounts =======================
struct Node {
    Account* data; // store pointer so we can move logs easily when deleting
    Node* next;
    Node(Account* acc) : data(acc), next(NULL) {}
};

// ======================= Bank (singly linked list + deleted logs) =======================
class Bank {
private:
    Node* head;                 // active accounts
    DeletedLogEntry* delHead;   // deleted accounts' logs

    void addToList(Account* acc) {
        Node* node = new Node(acc);
        node->next = head;
        head = node;
    }

public:
    Bank() : head(NULL), delHead(NULL) {}

    ~Bank() {
        // free active accounts + logs
        Node* cur = head;
        while (cur) {
            Node* nxt = cur->next;
            freeLogs(cur->data->logHead);
            delete cur->data;
            delete cur;
            cur = nxt;
        }
        // free deleted logs list
        DeletedLogEntry* d = delHead;
        while (d) {
            DeletedLogEntry* dn = d->next;
            freeLogs(d->logs);
            delete d;
            d = dn;
        }
    }

    void freeLogs(LogNode* h) {
        while (h) {
            LogNode* t = h;
            h = h->next;
            delete t;
        }
    }

    int findMaxAccNo() const {
        int mx = 0;
        Node* cur = head;
        while (cur) {
            if (cur->data->accNo > mx) mx = cur->data->accNo;
            cur = cur->next;
        }
        DeletedLogEntry* d = delHead;
        while (d) {
            if (d->accNo > mx) mx = d->accNo;
            d = d->next;
        }
        return mx;
    }

    Node* findNode(int accNo) const {
        Node* cur = head;
        while (cur) {
            if (cur->data->accNo == accNo) return cur;
            cur = cur->next;
        }
        return NULL;
    }

    bool accountExists(int accNo) const {
        return findNode(accNo) != NULL; // reuse your existing findNode
    }

    // generate sequential account numbers starting at 1
    int generateAccNo() const {
        return findMaxAccNo() + 1;
    }


    // --- helpers: keep them near your Bank class code ---

// if too long, cut to fit and add "..." (so columns never explode)
    static string fit(const string& s, int w) {
        if ((int)s.size() <= w) return s;
        if (w <= 3) return s.substr(0, w);          // tiny safety case
        return s.substr(0, w - 3) + "...";
    }

    // center text within width w (using spaces). assumes single‑byte chars.
    static string centerFit(const string& s, int w) {
        string t = fit(s, w);
        int pad = w - (int)t.size();
        int left = pad / 2;
        int right = pad - left;
        return string(left, ' ') + t + string(right, ' ');
    }

    // build a horizontal border line based on all column widths
    static string makeBorder(int wAcc, int wName, int wic, int wGen, int wType, int wPin, int wBal) {
        int total = 1 // leading '|'
            + 2 + wAcc   // space + field
            + 3 + wName  // " | " + field
            + 3 + wic
            + 3 + wGen
            + 3 + wType
            + 3 + wPin
            + 3 + wBal
            + 2;         // space + trailing '|'
        return string(total, '-');
    }

    // helper function to check if account exists
    bool hasAccount(int accNo) const {
        return findNode(accNo) != NULL;
    }


    // 1) Create account (prevent duplicates)
    // Function to add a new account, avoiding duplicates
    bool addAccount(const string& name, const string& passportNo, char gender, const string& accountType, int pin, long long balance, int& outAccNo) {
        // Check if account already exists in memory
        Node* cur = head;
        while (cur) {
            if (cur->data->ic == passportNo) {
                cout << "Account with this passport number already exists!" << endl;
                return false;
            }
            cur = cur->next;
        }

        // Generate account number (same logic as before)
        int accNo = generateAccNo();
        if (accNo == -1) return false;

        // create and link the account in memory
        Account* acc = new Account(accNo, name, passportNo, gender, accountType, pin, balance);
        addToList(acc);

        // log creation and persist
        acc->addLog(timestamp("Account created"));

        outAccNo = accNo;  // Output the generated account number
        printCentered( "Account added successfully!" );
        
        saveToFile(DATA_FILE);
        return true;
    }

    // add an existing account (e.g., from file) directly into the linked list
    void addAccountFromFile(int accNo, const string& name, const string& passportNo,
        char gender, const string& accountType, int pin, long long balance) {
        Account* acc = new Account(accNo, name, passportNo, gender, accountType, pin, balance);
        addToList(acc);
    }

    void saveToFile(const string& filename) const {
        ofstream out(filename, ios::binary | ios::trunc);
        if (!out) return;
        Node* cur = head;
        while (cur) {
            AccountRecord rec{};
            rec.accNo = cur->data->accNo;
            strncpy(rec.name, cur->data->name.c_str(), sizeof(rec.name));
            rec.name[sizeof(rec.name) - 1] = '\0';
            strncpy(rec.ic, cur->data->ic.c_str(), sizeof(rec.ic));
            rec.ic[sizeof(rec.ic) - 1] = '\0';
            rec.gender = cur->data->gender;
            strncpy(rec.typeCS, cur->data->typeCS.c_str(), sizeof(rec.typeCS));
            rec.typeCS[sizeof(rec.typeCS) - 1] = '\0';
            rec.pin = cur->data->pin;
            rec.balance = cur->data->balance;
            out.write(reinterpret_cast<char*>(&rec), sizeof(rec));
            cur = cur->next;
        }
        // also persist logs for active and deleted accounts
        saveLogsToFile(LOG_FILE);
    }



    // 2) Display all
    void displayAll() const {
        if (!head) {
            printCentered("No accounts found.");
            return;
        }
        Node* cur = head;
        while (cur) {
            cur->data->printBrief();
            cur = cur->next;
        }
    }

    // 3) Search account -> print (full)
    bool printAccount(int accNo) const {
        Node* n = findNode(accNo);
        if (!n) return false;
        n->data->printFull();
        return true;
    }

    // PIN check: 1 ok, -1 bad pin, 0 not found
    int checkAccPin(int accNo, int pin) const {
        Node* n = findNode(accNo);
        if (!n) return 0;
        if (n->data->pin != pin) return -1;
        return 1;
    }

    // 4) Deposit
    // returns: 1 ok, 0 not found, -1 bad amount
    int deposit(int accNo, int pin, long long amount) {
        Node* n = findNode(accNo);
        if (!n) return 0;
        if (n->data->pin != pin) return -2; // PIN wrong (extra check)
        if (amount <= 0) return -1;
        long long before = n->data->balance;
        n->data->balance += amount;
        n->data->addLog(timestamp("Deposit +RM " + to_string(amount) +
            ", before=RM " + to_string(before) +
            ", after=RM " + to_string(n->data->balance)));
        saveToFile(DATA_FILE);
        return 1;
    }

    // 5) Withdraw
    // returns: 1 ok, 0 not found, -1 insufficient, -2 bad pin, -3 bad amount
    int withdraw(int accNo, int pin, long long amount) {
        Node* n = findNode(accNo);
        if (!n) return 0;
        if (n->data->pin != pin) return -2;
        if (amount <= 0) return -3;
        if (n->data->balance < amount) return -1;
        long long before = n->data->balance;
        n->data->balance -= amount;
        n->data->addLog(timestamp("Withdraw -RM " + to_string(amount) +
            ", before=RM " + to_string(before) +
            ", after=RM " + to_string(n->data->balance)));
        saveToFile(DATA_FILE);
        return 1;
    }

    // 5b) Transfer between accounts
    // returns: 1 ok, 0 src not found, -4 dest not found, -1 insufficient, -2 bad pin, -3 bad amount
    int transfer(int srcAcc, int pin, int dstAcc, long long amount) {
        Node* src = findNode(srcAcc);
        if (!src) return 0;
        Node* dst = findNode(dstAcc);
        if (!dst) return -4;
        if (src->data->pin != pin) return -2;
        if (amount <= 0) return -3;
        if (src->data->balance < amount) return -1;
        long long beforeSrc = src->data->balance;
        long long beforeDst = dst->data->balance;
        src->data->balance -= amount;
        dst->data->balance += amount;
        src->data->addLog(timestamp("Transfer -RM " + to_string(amount) + " to account " + formatAccNo(dstAcc) +
            ", before=RM " + to_string(beforeSrc) + ", after=RM " + to_string(src->data->balance)));
        dst->data->addLog(timestamp("Transfer +RM " + to_string(amount) + " from account " + formatAccNo(srcAcc) +
            ", before=RM " + to_string(beforeDst) + ", after=RM " + to_string(dst->data->balance)));
        saveToFile(DATA_FILE);
        return 1;
    }

    // 5c) Change PIN
    // returns: 1 ok, 0 not found, -1 old pin wrong
    int changePin(int accNo, int oldPin, int newPin) {
        Node* n = findNode(accNo);
        if (!n) return 0;
        if (n->data->pin != oldPin) return -1;
        n->data->pin = newPin;
        n->data->addLog(timestamp("PIN changed"));
        saveToFile(DATA_FILE);
        return 1;
    }

    // 5d) Get balance
    // returns: 1 ok, 0 not found, -1 bad pin
    int getBalance(int accNo, int pin, long long& outBal) const {
        Node* n = findNode(accNo);
        if (!n) return 0;
        if (n->data->pin != pin) return -1;
        outBal = n->data->balance;
        return 1;
    }

    // 5e) Mini statement (last N logs)
    // returns: 1 ok, 0 not found, -1 bad pin, -2 no logs
    int miniStatement(int accNo, int pin, int N) const {
        Node* n = findNode(accNo);
        if (!n) return 0;
        if (n->data->pin != pin) return -1;
        vector<string> logs;
        for (LogNode* cur = n->data->logHead; cur; cur = cur->next) logs.push_back(cur->text);
        if (logs.empty()) return -2;
        int start = (int)logs.size() > N ? (int)logs.size() - N : 0;
        for (int i = start; i < (int)logs.size(); ++i) {
            printCentered(logs[i]);
        }
        return 1;
    }

    // 6) Delete account (move its logs to deleted list so we can show later)
    bool deleteAccount(int accNo) {
        if (!head) return false;
        if (head->data->accNo == accNo) {
            Node* t = head; head = head->next;
            // record deletion before removing
            t->data->addLog(timestamp("Account deleted"));
            moveLogsToDeleted(t->data);
            delete t->data;
            delete t;
            saveToFile(DATA_FILE);
            return true;
        }
        Node* prev = head;
        Node* cur = head->next;
        while (cur) {
            if (cur->data->accNo == accNo) {
                prev->next = cur->next;
                cur->data->addLog(timestamp("Account deleted"));
                moveLogsToDeleted(cur->data);
                delete cur->data;
                delete cur;
                saveToFile(DATA_FILE);
                return true;
            }
            prev = cur; cur = cur->next;
        }
        return false;
    }

    // Edit user info
    // returns: 1 ok, 0 not found
    int changeInfo(int accNo, const string& newName, const string& newic,
        char newGender, const string& newTypeCS, int newPIN) {
        Node* n = findNode(accNo);
        if (!n) return 0;
         n->data->name = newName;
        n->data->ic = newic;
        n->data->gender = newGender;
       n->data->typeCS = newTypeCS;
        n->data->pin = newPIN;
        n->data->addLog(timestamp("Info changed"));
        saveToFile(DATA_FILE);
        return 1;
    }

    // For Admin "Show list" (simple)
    // === replace your existing printForAdmin() with this ===
// assumes you have: struct Account{ int accNo; string name, ic; char gender; string typeCS; int pin; long long balance; };
// and a singly linked list: struct Node{ Account* data; Node* next; };  Node* head;

    void printForAdmin() const {
        // column widths (adjust to taste)
        const int W_ACC = 12;
        const int W_NAME = 30;
        const int W_ic = 18;
        const int W_GEN = 6;
        const int W_TYPE = 10;
        const int W_PIN = 8;
        const int W_BAL = 14;

        if (!head) {
            printCentered("No accounts found.\n");
            return;
        }

        string border = makeBorder(W_ACC, W_NAME, W_ic, W_GEN, W_TYPE, W_PIN, W_BAL);

        // header
        string header =
            "| " + centerFit("ACC_Number", W_ACC) + " | "
            + centerFit("NAME", W_NAME) + " | "
            + centerFit("PASSPORT_NO", W_ic) + " | "
            + centerFit("GENDER", W_GEN) + " | "
            + centerFit("TYPE", W_TYPE) + " | "
            + centerFit("PIN", W_PIN) + " | "
            + centerFit("BALANCE (RM)", W_BAL) + " |";

        // now print with centering
        printCentered(border);
        printCentered(header);
        printCentered(border);

        // rows
        Node* cur = head;
        while (cur) {
            Account* a = cur->data;

            // convert values to strings so we can center everything
            string sAcc = formatAccNo(a->accNo);
            string sGen = (a->gender == 'M') ? "Male" : "Female";
            string sPin = formatPin(a->pin);
            string sBal = string("RM ") + to_string(a->balance);

            // build ONE line, then center the whole line
            string row =
                "| " + centerFit(sAcc, W_ACC) + " | "
                + centerFit(a->name, W_NAME) + " | "
                + centerFit(a->ic, W_ic) + " | "
                + centerFit(sGen, W_GEN) + " | "
                + centerFit(a->typeCS, W_TYPE) + " | "
                + centerFit(sPin, W_PIN) + " | "
                + centerFit(sBal, W_BAL) + " |";

            printCentered(row);  // <<-- centered print for the entire row

            cur = cur->next;
        }


        printCentered(border);
        printCentered(""); // extra line at the end
    }

    // Logging helpers
    void insert_log(int accNo, const string& msg) {
        Node* n = findNode(accNo);
        if (!n) return;
        n->data->addLog(timestamp(msg));
        // persist logs if used independently of other operations
        saveLogsToFile(LOG_FILE);
    }

    // display1: return 1 if logs in deleted section exist (like your prototype)
    int display1(int accNo) const {
        const DeletedLogEntry* d = findDeleted(accNo);
        return d ? 1 : 0;
    }

    // display: print logs either from active account or from deleted-logs
    void display(int accNo) const {
        Node* n = findNode(accNo);
        if (n) {
            printLogs(n->data->logHead);
            return;
        }
        const DeletedLogEntry* d = findDeleted(accNo);
        if (d) {
            printLogs(d->logs);
            return;
        }
        printCentered("Logs Not Found....!!!");
    }

    void saveLogsToFile(const string& filename) const {
        ofstream out(filename, ios::binary | ios::trunc);
        if (!out) return;
        auto writeList = [&out](int accNo, LogNode* logs) {
            out.write(reinterpret_cast<const char*>(&accNo), sizeof(accNo));
            int count = 0;
            for (LogNode* c = logs; c; c = c->next) ++count;
            out.write(reinterpret_cast<const char*>(&count), sizeof(count));
            LogNode* c = logs;
            while (c) {
                int len = static_cast<int>(c->text.size());
                out.write(reinterpret_cast<const char*>(&len), sizeof(len));
                out.write(c->text.c_str(), len);
                c = c->next;
            }
        };
        Node* cur = head;
        while (cur) {
            writeList(cur->data->accNo, cur->data->logHead);
            cur = cur->next;
        }
        DeletedLogEntry* d = delHead;
        while (d) {
            writeList(d->accNo, d->logs);
            d = d->next;
        }
    }

    void loadLogsFromFile(const string& filename) {
        ifstream in(filename, ios::binary);
        if (!in) return;
        while (true) {
            int accNo;
            if (!in.read(reinterpret_cast<char*>(&accNo), sizeof(accNo))) break;
            int count;
            if (!in.read(reinterpret_cast<char*>(&count), sizeof(count))) break;
            LogNode* h = nullptr;
            LogNode** tail = &h;
            for (int i = 0; i < count; ++i) {
                int len;
                if (!in.read(reinterpret_cast<char*>(&len), sizeof(len))) {
                    freeLogs(h);
                    return;
                }
                string msg(len, '\0');
                if (!in.read(&msg[0], len)) {
                    freeLogs(h);
                    return;
                }
                LogNode* node = new LogNode(msg);
                *tail = node;
                tail = &node->next;
            }
            Node* n = findNode(accNo);
            if (n) {
                n->data->logHead = h;
            } else {
                DeletedLogEntry* e = new DeletedLogEntry(accNo, h);
                e->next = delHead;
                delHead = e;
            }
        }
    }

private:
    string timestamp(const string& msg) const {
        time_t now = time(nullptr);
        char dt[26];
#ifdef _WIN32
        ctime_s(dt, sizeof(dt), &now);
#else
        ctime_r(&now, dt);
#endif
        string t = msg + " at " + string(dt);
        if (!t.empty() && t.back() == '\n') t.pop_back();
        return t;
    }

    void printLogs(LogNode* h) const {
        if (!h) { printCentered("[No logs]"); return; }
        LogNode* cur = h;
        while (cur) {
            printCentered(cur->text);
            cur = cur->next;
        }
    }

    void moveLogsToDeleted(Account* a) {
        // prepend to deleted list (keep logs)
        DeletedLogEntry* e = new DeletedLogEntry(a->accNo, a->logHead);
        e->next = delHead;
        delHead = e;
        // detach logs from account so destructor won't free twice
        a->logHead = NULL;
    }

    const DeletedLogEntry* findDeleted(int accNo) const {
        const DeletedLogEntry* d = delHead;
        while (d) {
            if (d->accNo == accNo) return d;
            d = d->next;
        }
        return NULL;
    }
};

// Function to load accounts from the binary file into the linked list
void loadAccountsFromFile(Bank& bank) {
    ifstream in(DATA_FILE, ios::binary);
    if (in) {
        AccountRecord rec;
        while (in.read(reinterpret_cast<char*>(&rec), sizeof(rec))) {
            bank.addAccountFromFile(rec.accNo, rec.name, rec.ic, rec.gender, rec.typeCS, rec.pin, rec.balance);
        }
    }
    bank.loadLogsFromFile(LOG_FILE);
}



// ======================= Panels (simple loops, no goto) =======================
int admin_pswd = 1111;
int staff_pswd = 2222;

void admin_panel(Bank& bank);
void staff_panel(Bank& bank);
void atm_panel(Bank& bank);

// ======================= Main =======================
int main() {
    srand((unsigned)time(0)); // seed random once

    Bank bank;
    maximizeConsole();  // make window large
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);   // tell Windows console to use UTF-8
#endif

    // Load accounts from the binary file into the linked list
    loadAccountsFromFile(bank);

  




    while (true) {
        renderLoginScreen();   // <-- centered banner + menu

        int a;
        printCenteredInline("Enter Your Choice: ");
        if (!(cin >> a)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (a == 1) {
            int pin = readPin("Enter Admin PIN: ");
            if (pin == admin_pswd) admin_panel(bank);
            else printCentered("Wrong PIN.");
        }
        else if (a == 2) {
            int pin = readPin("Enter Staff PIN: ");
            if (pin == staff_pswd) staff_panel(bank);
            else printCentered("Wrong PIN.");
        }
        else if (a == 3) {
            atm_panel(bank);
        }
        else if (a == 4) {
            printCentered("Bye!");
            break;
        }
    }
    return 0;
}

// ======================= Panel Functions =======================

// ---------------- Admin ----------------
void admin_panel(Bank& bank) {
    while (true) {
        clearScreen();
        int b;
        cout<<endl;
        printCentered("********** ADMIN PANEL **********");
        printCentered("1. Create Account");
        printCentered("2. Delete Account");
        printCentered("3. Search Account");
        printCentered("4. Show All Accounts");
        printCentered("5. Edit Information");
        printCentered("6. Show Logs of Deleted Account");
        printCentered("7. Back to Main Menu");
        printCenteredInline("Enter an Option: ");
        if (!(cin >> b)) { cin.clear(); cin.ignore(10000, '\n'); continue; }
        cin.ignore(10000, '\n'); // for getline after numbers

        if (b == 1) {
            string full_name = readName("Enter Customer's Full Name: ", 4);
            string ic;
            char g, t;
            string acc_type;
            int pin;
            long long bal;
            ic = readPassport("Enter Passport No: ");
            printCenteredInline("Enter Gender “Male/Female” (M/F): "); cin >> g; cin.ignore(10000, '\n');
            g = toupper(g);
            if (g != 'M' && g != 'F') { printCentered("Invalid gender."); continue; }
            printCenteredInline("Enter Account Type “Current/Savings” (C/S): "); cin >> t; cin.ignore(10000, '\n');
            t = toupper(t);
            if (t != 'C' && t != 'S') { printCentered("Invalid account type."); continue; }
            acc_type = (t == 'C') ? "Current" : "Savings";
            pin = readPin("Enter PIN: ");
            do {
                bal = readNumber("Enter Balance (Min:500): RM ");
                if (bal < 500) printCentered("Minimum Balance is 500.");
            } while (bal < 500);

            int accNoOut = 0;
             if (bank.addAccount(full_name, ic, g, acc_type, pin, bal, accNoOut)) {
                printCentered("Account created successfully.");
                printCentered("Generated Account Number: " + formatAccNo(accNoOut));
            }
            else {
                printCentered("Create failed (duplicate or memory).");
            }
            printCenteredInline("Press Enter to return to ADMIN PANEL...");
            cin.get();
        }
            else if (b == 2) {
            int acc = (int)readNumber("Enter Account Number to Delete: ", 4);
            if (bank.deleteAccount(acc)) printCentered("Account deleted.");
            else printCentered("Account not found.");
            printCenteredInline("Press Enter to return to ADMIN PANEL...");
            cin.get();
        }
        else if (b == 3) {
            int acc = (int)readNumber("Enter Account Number to Search: ", 4);
            if (!bank.printAccount(acc)) printCentered("Account not found.");
            printCenteredInline("Press Enter to return to ADMIN PANEL...");
            cin.get();
        }
        else if (b == 4) {
            // Show list of all accounts in a neat table
            bank.printForAdmin();

            // (Optional) pause so the table doesn't scroll away
            cin.ignore(10000, '\n');   // clear leftover newline from previous input
            printCenteredInline("Press Enter to return to ADMIN PANEL...");
            cin.get();
        }

         else if (b == 5) {
            int acc = (int)readNumber("Enter Account Number: ", 4);
            string newName = readName("Enter New Name: ", 4);
            string newic;
            char newGender, newTypeCh;
            string newType;
            newic = readPassport("Enter New Passport No: ");
            printCenteredInline("Enter Gender (M/F): "); cin >> newGender; cin.ignore(10000, '\n');
            newGender = toupper(newGender);
            if (newGender != 'M' && newGender != 'F') { printCentered("Invalid gender."); continue; }
            printCenteredInline("Enter Account Type (C/S): "); cin >> newTypeCh; cin.ignore(10000, '\n');
            newTypeCh = toupper(newTypeCh);
            if (newTypeCh != 'C' && newTypeCh != 'S') { printCentered("Invalid account type."); continue; }
            newType = (newTypeCh == 'C') ? "Current" : "Savings";
            int newPIN = readPin("Enter New PIN: ");

            int r = bank.changeInfo(acc, newName, newic, newGender, newType, newPIN);
            if (r == 1) printCentered("Information changed."); else printCentered("Account not found.");
            printCenteredInline("Press Enter to return to ADMIN PANEL...");
            cin.get();
        }
        else if (b == 6) {
            int acc = (int)readNumber("Enter Account Number: ", 4);
            int hasDel = bank.display1(acc);
            bank.display(acc); // will print either active or deleted logs
            printCenteredInline("Press Enter to return to ADMIN PANEL...");
            cin.get();
            if (!hasDel) {
                // if not in deleted, we already printed active logs (or "[No logs]" or "Logs Not Found")
            }
        }
        else if (b == 7) {
            break;
        }
    }
}

// ---------------- Staff ----------------
void staff_panel(Bank& bank) {
    while (true) {
        int c;
        cout<<endl;
        printCentered("********** STAFF PANEL **********");
        printCentered("1. Check Account Info");
        printCentered("2. Deposit Cash");
        printCentered("3. Withdraw Cash");
        printCentered("4. Check Logs of User");
        printCentered("5. Back to Main Menu");
        printCenteredInline("Enter an Option: ");
        if (!(cin >> c)) { cin.clear(); cin.ignore(10000, '\n'); continue; }
        cin.ignore(10000, '\n'); // for getline after numbers

         if (c == 1) {
            int acc = (int)readNumber("Enter Account Number: ", 4);
            if (!bank.printAccount(acc)) printCentered("User not found.");
            printCenteredInline("Press Enter to return to STAFF PANEL...");
            cin.get();
        }
        else if (c == 2) {
            int acc = (int)readNumber("Enter Account: ", 4);
            int pin = readPin("Enter Account PIN: ");
            long long amt = readNumber("Enter Amount to Deposit: RM ");
            if (!bank.hasAccount(acc)) { printCentered("Account not found."); continue; }
            // BEFORE
            cout<<endl;
            printCentered("Status BEFORE Deposit:");
            bank.printAccount(acc);
            int res = bank.deposit(acc, pin, amt);
            if (res == 1) {
                printCentered("Status AFTER Deposit:");
                bank.printAccount(acc);
                printCentered("Deposit successful.");
            }
            else if (res == 0) printCentered("Account not found.");
            else if (res == -1) printCentered("Invalid amount.");
            else if (res == -2) printCentered("PIN incorrect.");
            printCenteredInline("Press Enter to return to STAFF PANEL...");
            cin.get();
        }
        else if (c == 3) {
            int acc = (int)readNumber("Enter Account: ", 4);
            int pin = readPin("Enter Account PIN: ");
            long long amt = readNumber("Enter Amount to Withdraw: RM ");
            if (!bank.hasAccount(acc)) { printCentered("Account not found."); continue; }
            // BEFORE
            printCentered("Status BEFORE Withdraw:");
            bank.printAccount(acc);
            int res = bank.withdraw(acc, pin, amt);
            if (res == 1) {
                printCentered("Status AFTER Withdraw:");
                bank.printAccount(acc);
                printCentered("Withdraw successful.");
            }
            else if (res == 0) printCentered("Account not found.");
            else if (res == -1) printCentered("Insufficient funds.");
            else if (res == -2) printCentered("PIN incorrect.");
            else if (res == -3) printCentered("Invalid amount.");
            printCenteredInline("Press Enter to return to STAFF PANEL...");
            cin.get();
        }
        else if (c == 4) {
            int acc = (int)readNumber("Enter Account Number: ", 4);
            int inDeleted = bank.display1(acc);
            bank.display(acc);
            if (!inDeleted) {
                // active logs printed already (or message)
            }
            printCenteredInline("Press Enter to return to STAFF PANEL...");
            cin.get();
        }
        else if (c == 5) {
            break;
        }
    }
}

// ---------------- ATM / CDM ----------------
void atm_service(Bank& bank, int acc, int& pin) {
    while (true) {
        clearScreen();
        int op;
        cout<<endl;
        printCentered("********** ATM SERVICE **********");
        printCentered("1. Withdraw Cash");
        printCentered("2. Check Account Balance");
        printCentered("3. Mini Statement (Last 5 Transactions)");
        printCentered("4. Transfer Money to Another Account");
        printCentered("5. Change PIN");
        printCentered("6. Back to ATM/CDM Menu");
        printCentered("********************************");
        cout<<endl;
        printCenteredInline("Enter an option: ");
        if (!(cin >> op)) { cin.clear(); cin.ignore(10000, '\n'); continue; }

        if (op == 1) {
            long long amt = readNumber("Enter Amount to Withdraw: RM ");
            int res = bank.withdraw(acc, pin, amt);
            if (res == 1) printCentered("Withdraw successful.");
            else if (res == 0) printCentered("Account not found.");
            else if (res == -1) printCentered("Insufficient funds.");
            else if (res == -2) printCentered("PIN incorrect.");
            else if (res == -3) printCentered("Invalid amount.");
        }
        else if (op == 2) {
            long long bal; int r = bank.getBalance(acc, pin, bal);
            if (r == 1) printCentered("Current Balance: RM " + to_string(bal));
            else if (r == 0) printCentered("Account not found.");
            else if (r == -1) printCentered("PIN incorrect.");
        }
        else if (op == 3) {
            int r = bank.miniStatement(acc, pin, 5);
            if (r == 0) printCentered("Account not found.");
            else if (r == -1) printCentered("PIN incorrect.");
            else if (r == -2) printCentered("No transactions.");
        }
         else if (op == 4) {
            int dst = (int)readNumber("Enter Recipient Account Number: ", 4);
            if (!bank.hasAccount(dst)) { printCentered("Recipient account not found."); }
            else {
                long long amt = readNumber("Enter Amount to Transfer: RM ");
                int r = bank.transfer(acc, pin, dst, amt);
                if (r == 1) printCentered("Transfer successful.");
                else if (r == -1) printCentered("Insufficient funds.");
                else if (r == -2) printCentered("PIN incorrect.");
                else if (r == -3) printCentered("Invalid amount.");
            }
        }
         else if (op == 5) {
            int oldp = readPin("Enter Old PIN: ");
            int newp = readPin("Enter New PIN: ");
            int r = bank.changePin(acc, oldp, newp);
            if (r == 1) { printCentered("PIN changed."); pin = newp; }
            else if (r == 0) printCentered("Account not found.");
            else if (r == -1) printCentered("Old PIN incorrect.");
        }
        else if (op == 6) {
            break;
        }
        else {
            printCentered("Invalid option.");
        }
        cin.ignore(10000, '\n');
        printCenteredInline("Press Enter to continue..."); cin.get();
    }
}

void cdm_service(Bank& bank, int acc, int pin) {
    while (true) {
        clearScreen();
        int d;
        cout<<endl;
        printCentered("********** CDM SERVICE **********");
        printCentered("1. Deposit Cash");
        printCentered("2. Check Account Balance");
        printCentered("3. Mini Statement (Last 5 Transactions)");
        printCentered("4. Back to ATM/CDM Menu");
        printCentered("********************************");
        cout<<endl;
        printCenteredInline("Enter an option: ");
        if (!(cin >> d)) { cin.clear(); cin.ignore(10000, '\n'); continue; }

        if (d == 1) {
            while (true) {
                clearScreen();
                int sub;
                cout<<endl;
                printCentered("********** Deposit Cash **********");
                printCentered("1. Deposit to My Account");
                printCentered("2. Deposit to Another Account");
                printCentered("3. Back to CDM Menu");
                printCentered("********************************");
                cout<<endl;
                printCenteredInline("Enter an option: ");
                if (!(cin >> sub)) { cin.clear(); cin.ignore(10000, '\n'); continue; }

                if (sub == 1) {
                    long long amt = readNumber("Enter Amount to Deposit: RM ");
                    int r = bank.deposit(acc, pin, amt);
                    if (r == 1) printCentered("Deposit successful.");
                    else if (r == 0) printCentered("Account not found.");
                    else if (r == -1) printCentered("Invalid amount.");
                    else if (r == -2) printCentered("PIN incorrect.");
                }
                else if (sub == 2) {
                    int dst = (int)readNumber("Enter Recipient Account Number: ", 4);
                    if (!bank.hasAccount(dst)) { printCentered("Recipient account not found."); }
                    else {
                        long long amt = readNumber("Enter Amount to Deposit: RM ");
                        int r = bank.transfer(acc, pin, dst, amt);
                        if (r == 1) printCentered("Deposit successful.");
                        else if (r == -1) printCentered("Insufficient funds.");
                        else if (r == -2) printCentered("PIN incorrect.");
                        else if (r == -3) printCentered("Invalid amount.");
                    }
                }
                else if (sub == 3) {
                    break;
                }
                else {
                    printCentered("Invalid option.");
                }
                cin.ignore(10000, '\n');
                printCenteredInline("Press Enter to continue..."); cin.get();
            }
        }
        else if (d == 2) {
            long long bal; int r = bank.getBalance(acc, pin, bal);
            if (r == 1) printCentered("Current Balance: RM " + to_string(bal));
            else if (r == 0) printCentered("Account not found.");
            else if (r == -1) printCentered("PIN incorrect.");
            cin.ignore(10000, '\n');
            printCenteredInline("Press Enter to continue..."); cin.get();
        }
        else if (d == 3) {
            int r = bank.miniStatement(acc, pin, 5);
            if (r == 0) printCentered("Account not found.");
            else if (r == -1) printCentered("PIN incorrect.");
            else if (r == -2) printCentered("No transactions.");
            cin.ignore(10000, '\n');
            printCenteredInline("Press Enter to continue..."); cin.get();
        }
        else if (d == 4) {
            break;
        }
        else {
            printCentered("Invalid option.");
            cin.ignore(10000, '\n');
            printCenteredInline("Press Enter to continue..."); cin.get();
        }
    }
}

void atm_panel(Bank& bank) {
    while (true) {
        clearScreen();
        int d;
        cout<<endl;
        printCentered("********** ATM / CDM **********");
        printCentered("1. ATM Service");
        printCentered("2. CDM Service");
        printCentered("3. Back to Main Menu");
        cout<<endl;
        printCenteredInline("Enter an Option: ");
        if (!(cin >> d)) { cin.clear(); cin.ignore(10000, '\n'); continue; }

        if (d == 1 || d == 2) {
            int acc = (int)readNumber("Enter Account Number: ", 4);
            if (!bank.hasAccount(acc)) { printCentered("Account not found."); cin.ignore(10000, '\n'); printCenteredInline("Press Enter to continue..."); cin.get(); continue; }
            int pin = readPin("Enter PIN: ");
            int chk = bank.checkAccPin(acc, pin);
            if (chk != 1) { printCentered("PIN incorrect."); cin.ignore(10000, '\n'); printCenteredInline("Press Enter to continue..."); cin.get(); continue; }
            if (d == 1) atm_service(bank, acc, pin);
            else cdm_service(bank, acc, pin);
        }
        else if (d == 3) {
            break;
        }
        else {
            printCentered("Invalid option.");
            cin.ignore(10000, '\n');
            printCenteredInline("Press Enter to continue..."); cin.get();
        }
    }
}

