#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <ctime>     // for time/ctime
#include <fstream>   // for binary file operations
#include <cstring>   // for strncpy
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif
using namespace std;

const string DATA_FILE = "accounts.dat";

struct AccountRecord {
    int accNo;
    char name[100];
    char ic[50];
    char gender;
    char typeCS[10];
    int pin;
    long long balance;
};
// ======================= SQLite3 Integration =======================

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
    string typeCS;    // "C" or "S"
    int pin;
    long long balance;
    LogNode* logHead; // singly linked list of logs

    Account(int a, const string& nm, const string& c, char g, const string& t, int p, long long b)
        : accNo(a), name(nm), ic(c), gender(g), typeCS(t), pin(p), balance(b), logHead(NULL) {
    }

    void addLog(const string& msg) {
        // prepend for simplicity
        LogNode* n = new LogNode(msg);
        n->next = logHead;
        logHead = n;
    }

    void printBrief() const {
        printCentered("Account No: " + to_string(accNo) +
            "; Name: " + name +
            "; Balance: " + to_string(balance));
    }

    void printFull() const {
        printCentered("Account No: " + to_string(accNo) +
            "; Name: " + name +
            "; Passport No: " + ic +
            "; Gender: " + gender +
            "; Type: " + typeCS +
            "; PIN: " + to_string(pin) +
            "; Balance: " + to_string(balance));
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

    // generate a unique 4-digit number [1000..9999]
    int generateAccNo() const {
        // safety: try a bunch of times; in practice collisions are rare
        for (int tries = 0; tries < 10000; ++tries) {
            int n = 1000 + rand() % 9000;
            if (!accountExists(n)) return n;
        }
        // fallback (very unlikely): linear scan 1000..9999
        for (int n = 1000; n <= 9999; ++n) {
            if (!accountExists(n)) return n;
        }
        return -1; // table full or error
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

        outAccNo = accNo;  // Output the generated account number
        cout << "Account added successfully!" << endl;

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
        n->data->addLog(timestamp("Deposit +" + to_string(amount) +
            ", before=" + to_string(before) +
            ", after=" + to_string(n->data->balance)));
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
        n->data->addLog(timestamp("Withdraw -" + to_string(amount) +
            ", before=" + to_string(before) +
            ", after=" + to_string(n->data->balance)));
        saveToFile(DATA_FILE);
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
            + centerFit("BALANCE", W_BAL) + " |";

        // now print with centering
        printCentered(border);
        printCentered(header);
        printCentered(border);

        // rows
        Node* cur = head;
        while (cur) {
            Account* a = cur->data;

            // convert values to strings so we can center everything
            string sAcc = to_string(a->accNo);
            string sGen(1, a->gender);
            string sPin = to_string(a->pin);
            string sBal = to_string(a->balance);

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
    if (!in) return;
    AccountRecord rec;
    while (in.read(reinterpret_cast<char*>(&rec), sizeof(rec))) {
        bank.addAccountFromFile(rec.accNo, rec.name, rec.ic, rec.gender, rec.typeCS, rec.pin, rec.balance);
    }
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
        if (!(cin >> a)) { cin.clear(); cin.ignore(10000, '\n'); continue; }

        if (a == 1) {
            int pin;
            printCenteredInline("Enter Admin PIN: ");
            cin >> pin;
            if (pin == admin_pswd) admin_panel(bank);
            else printCentered("Wrong PIN.");
        }
        else if (a == 2) {
            int pin;
            printCenteredInline("Enter Staff PIN: ");
            cin >> pin;
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
        int b;
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
            string full_name, ic, acc_type;
            char g;
            int pin;
            long long bal;
            printCenteredInline("Enter Customer's Full Name: "); getline(cin, full_name);
            printCenteredInline("Enter Passport No: "); getline(cin, ic);
            printCenteredInline("Enter Gender “Male/Female” (M/F): "); cin >> g; cin.ignore(10000, '\n');
            printCenteredInline("Enter Account Type “Current/Savings” (C/S): "); getline(cin, acc_type);
            printCenteredInline("Enter PIN: "); cin >> pin;
            printCenteredInline("Enter Balance (Min:500): "); cin >> bal;

            int accNoOut = 0;
            if (bal < 500) {
                printCentered("Minimum Balance is 500.");
            }
            else if (bank.addAccount(full_name, ic, g, acc_type, pin, bal, accNoOut)) {
                printCentered("Account created successfully.");
                printCentered("Generated Account Number: " + to_string(accNoOut)); // <-- show it
            }
            else {
                printCentered("Create failed (duplicate or memory).");
            }
        }
        else if (b == 2) {
            int acc;
            printCenteredInline("Enter Account Number to Delete: ");
            cin >> acc;
            if (bank.deleteAccount(acc)) printCentered("Account deleted.");
            else printCentered("Account not found.");
        }
        else if (b == 3) {
            int acc;
            printCenteredInline("Enter Account Number to Search: ");
            cin >> acc;
            if (!bank.printAccount(acc)) printCentered("Account not found.");
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
            int acc, newPIN;
            string newName, newic, newType;
            char newGender;
            printCenteredInline("Enter Account Number: "); cin >> acc; cin.ignore(10000, '\n');
            printCenteredInline("Enter New Name: "); getline(cin, newName);
            printCenteredInline("Enter New Passport No: "); getline(cin, newic);
            printCenteredInline("Enter Gender (M/F): "); cin >> newGender; cin.ignore(10000, '\n');
            printCenteredInline("Enter Account Type (C/S): "); getline(cin, newType);
            printCenteredInline("Enter New PIN: "); cin >> newPIN;

            int r = bank.changeInfo(acc, newName, newic, newGender, newType, newPIN);
            if (r == 1) printCentered("Information changed."); else printCentered("Account not found.");
        }
        else if (b == 6) {
            int acc;
            printCenteredInline("Enter Account Number: ");
            cin >> acc;
            int hasDel = bank.display1(acc);
            bank.display(acc); // will print either active or deleted logs
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
        printCentered("********** STAFF PANEL **********");
        printCentered("1. Check Account Info");
        printCentered("2. Deposit Cash");
        printCentered("3. Withdraw Cash");
        printCentered("4. Check Logs of User");
        printCentered("5. Back to Main Menu");
        printCenteredInline("Enter an Option: ");
        if (!(cin >> c)) { cin.clear(); cin.ignore(10000, '\n'); continue; }

        if (c == 1) {
            int acc; printCenteredInline("Enter Account Number: "); cin >> acc;
            if (!bank.printAccount(acc)) printCentered("User not found.");
        }
        else if (c == 2) {
            int acc, pin; long long amt;
            printCenteredInline("Enter Account: "); cin >> acc;
            printCenteredInline("Enter Account PIN: "); cin >> pin;
            printCenteredInline("Enter Amount to Deposit: "); cin >> amt;
            // BEFORE
            printCentered("\nStatus BEFORE Deposit:");
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
        }
        else if (c == 3) {
            int acc, pin; long long amt;
            printCenteredInline("Enter Account: "); cin >> acc;
            printCenteredInline("Enter Account PIN: "); cin >> pin;
            printCenteredInline("Enter Amount to Withdraw: "); cin >> amt;
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
        }
        else if (c == 4) {
            int acc; printCenteredInline("Enter Account Number: "); cin >> acc;
            int inDeleted = bank.display1(acc);
            bank.display(acc);
            if (!inDeleted) {
                // active logs printed already (or message)
            }
        }
        else if (c == 5) {
            break;
        }
    }
}

// ---------------- ATM / CDM ----------------
void atm_panel(Bank& bank) {
    while (true) {
        int d;
        printCentered("********** ATM / CDM **********");
        printCentered("1. Deposit Cash (CDM)");
        printCentered("2. Withdraw Cash (ATM)");
        printCentered("3. Back to Main Menu");
        printCenteredInline("Enter an Option: ");
        if (!(cin >> d)) { cin.clear(); cin.ignore(10000, '\n'); continue; }

        if (d == 1) {
            // CDM: Deposit Cash
            int acc, pin; long long amt;
            printCenteredInline("Enter Account Number: "); cin >> acc;

            if (!bank.hasAccount(acc)) {
                printCentered("Account not found.");
                continue;
            }

            printCenteredInline("Enter Account PIN: "); cin >> pin;
            printCenteredInline("Enter Amount to Deposit: "); cin >> amt;
            printCentered("Status BEFORE Deposit:");
            bank.printAccount(acc);

            int res = bank.deposit(acc, pin, amt);
            if (res == 1) {
                printCentered("Status AFTER Deposit:");
                bank.printAccount(acc);
                printCentered("Deposit successful.");
            }
            else if (res == 0) {
                printCentered("Account not found.");
            }
            else if (res == -1) {
                printCentered("Invalid amount.");
            }
            else if (res == -2) {
                printCentered("PIN incorrect.");
            }
        }
        else if (d == 2) {
            // ATM: Withdraw Cash
            int acc, pin; long long amt;
            printCenteredInline("Enter Account Number: "); cin >> acc;

            if (!bank.hasAccount(acc)) {
                printCentered("Account not found.");
                continue;
            }

            printCenteredInline("Enter Account PIN: "); cin >> pin;
            printCenteredInline("Enter Amount to Withdraw: "); cin >> amt;

            printCentered("Status BEFORE Withdraw:");
            bank.printAccount(acc);

            int res = bank.withdraw(acc, pin, amt);
            if (res == 1) {
                printCentered("Status AFTER Withdraw:");
                bank.printAccount(acc);
                printCentered("Withdraw successful.");
            }
            else if (res == 0) {
                printCentered("Account not found.");
            }
            else if (res == -1) {
                printCentered("Insufficient funds.");
            }
            else if (res == -2) {
                printCentered("PIN incorrect.");
            }
            else if (res == -3) {
                printCentered("Invalid amount.");
            }
        }
        else if (d == 3) {
            break; // back to main
        }
        else {
            printCentered("Invalid option.");
        }
    }
}

