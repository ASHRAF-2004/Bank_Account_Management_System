# Bank Account Management System Report

## Overview
The Bank Account Management System is a console-based C++ program that demonstrates how typical banking operations can be built from scratch. It targets classroom exercises or small simulations where one wants to study account management, transaction logging, and basic security checks.

Accounts live in memory as a singly linked list. Two binary files keep data persistent between runs:

- `accounts.dat` – stores account number, holder name, passport/ID, gender, account type, PIN, and balance.
- `logs.dat` – holds a capped list of recent activity for every account. When an account is removed its log history is moved into `deleted_logs.dat`.

A lightweight logger adds time-stamped messages to each account. Old entries are truncated so the newest events remain available.

## Data Structures and Constants
The program relies on a few small structures to keep track of accounts and their history:

```cpp
struct LogNode {
    string text;
    LogNode* next;
};

class Account {
    int accNo;
    string name;
    string ic;
    char gender;
    string typeCS;
    int pin;
    long long balance;
    LogNode* logHead;
    // ... member functions ...
};
```
- **LogNode** forms a singly linked list of timestamped messages for each account.
- **Account** stores customer details, the current balance, and the head of its log list.
- A separate `Node` type links multiple `Account` objects together inside the `Bank` class.

Two constants influence all monetary operations:

```cpp
const long long MIN_BAL = 500;
const long long DENOM   = 10;
```
`MIN_BAL` is the minimum balance allowed after a withdrawal, and `DENOM` forces cash amounts to be in multiples of ten.

## File Storage
Account records and logs are written to binary files so the system survives program restarts.
- `accounts.dat` begins with a small header and then repeated `AccountRecord` structures containing fixed-width fields.
- `logs.dat` stores each account number followed by the count of messages and the variable-length strings themselves.
On startup the program reads both files and reconstructs the in-memory lists. A failed write returns an error code and the affected transaction is rolled back so memory and disk stay in sync.

## Account-Level Validations
Every account instance enforces simple rules before money moves. The deposit and withdrawal helpers are shown below:

```cpp
bool deposit(long long amount) {
    if (amount <= 0 || (DENOM > 1 && amount % DENOM != 0)) return false;
    balance += amount;
    return true;
}

// returns: 1 ok, -1 insufficient, -3 bad amount
int withdraw(long long amount) {
    if (amount <= 0 || (DENOM > 1 && amount % DENOM != 0)) return -3;
    if (balance - amount < MIN_BAL) return -1;
    balance -= amount;
    return 1;
}
```
The code rejects non‑positive amounts and any value that is not a multiple of the configured `DENOM`. Withdrawals also ensure the balance never drops below `MIN_BAL`.

## Core Operations with Code
Below are the main public functions provided by the `Bank` class. Each one returns an integer status code and writes a descriptive log entry.

### 1. `deposit`
```cpp
int deposit(int accNo, int pin, long long amount) {
    Node* n = findNode(accNo);
    if (!n) return 0;
    if (!n->data->verifyPin(pin)) {
        addLogCapped(n->data, timestamp("Deposit failed: bad PIN"));
        return -2;
    }
    long long before = n->data->getBalance();
    if (!n->data->deposit(amount)) {
        addLogCapped(n->data, timestamp("Deposit failed: invalid amount"));
        return -1;
    }
    addLogCapped(n->data, timestamp("Deposit +RM " + to_string(amount) +
        ", before=RM " + to_string(before) +
        ", after=RM " + to_string(n->data->getBalance())));
    if (!saveToFile(DATA_FILE)) {
        n->data->withdraw(amount);
        addLogCapped(n->data, timestamp("Deposit failed: storage error"));
        return -4;
    }
    return 1;
}
```
- **0**: account not found
- **-2**: PIN mismatch
- **-1**: invalid amount
- **-4**: saving to disk failed (deposit is rolled back)

### 2. `withdraw`
```cpp
int withdraw(int accNo, int pin, long long amount) {
    Node* n = findNode(accNo);
    if (!n) return 0;
    if (!n->data->verifyPin(pin)) {
        addLogCapped(n->data, timestamp("Withdraw failed: bad PIN"));
        return -2;
    }
    long long before = n->data->getBalance();
    int w = n->data->withdraw(amount);
    if (w == -3) {
        addLogCapped(n->data, timestamp("Withdraw failed: invalid amount"));
        return -3;
    }
    if (w == -1) {
        addLogCapped(n->data, timestamp("Withdraw failed: insufficient funds"));
        return -1;
    }
    addLogCapped(n->data, timestamp("Withdraw -RM " + to_string(amount) +
        ", before=RM " + to_string(before) +
        ", after=RM " + to_string(n->data->getBalance())));
    if (!saveToFile(DATA_FILE)) {
        n->data->deposit(amount);
        addLogCapped(n->data, timestamp("Withdraw failed: storage error"));
        return -4;
    }
    return 1;
}
```
- **0**: account not found
- **-2**: PIN mismatch
- **-3**: invalid amount
- **-1**: insufficient funds
- **-4**: storage error (withdrawal is reversed)

### 3. `transfer`
```cpp
int transfer(int srcAcc, int pin, int dstAcc, long long amount) {
    Node* src = findNode(srcAcc);
    if (!src) return 0;
    if (srcAcc == dstAcc) {
        addLogCapped(src->data, timestamp("Transfer failed: self-transfer"));
        return -5;
    }
    Node* dst = findNode(dstAcc);
    if (!dst) {
        addLogCapped(src->data, timestamp("Transfer failed: destination not found"));
        return -4;
    }
    if (!src->data->verifyPin(pin)) {
        addLogCapped(src->data, timestamp("Transfer failed: bad PIN"));
        return -2;
    }
    long long beforeSrc = src->data->getBalance();
    int w = src->data->withdraw(amount);
    if (w == -3) {
        addLogCapped(src->data, timestamp("Transfer failed: invalid amount"));
        return -3;
    }
    if (w == -1) {
        addLogCapped(src->data, timestamp("Transfer failed: insufficient funds"));
        return -1;
    }
    long long beforeDst = dst->data->getBalance();
    dst->data->deposit(amount);
    addLogCapped(src->data, timestamp("Transfer -RM " + to_string(amount) + " to account " + formatAccNo(dstAcc) +
        ", before=RM " + to_string(beforeSrc) + ", after=RM " + to_string(src->data->getBalance())));
    addLogCapped(dst->data, timestamp("Transfer +RM " + to_string(amount) + " from account " + formatAccNo(srcAcc) +
        ", before=RM " + to_string(beforeDst) + ", after=RM " + to_string(dst->data->getBalance())));
    if (!saveToFile(DATA_FILE)) {
        src->data->deposit(amount);
        dst->data->withdraw(amount);
        addLogCapped(src->data, timestamp("Transfer failed: storage error"));
        addLogCapped(dst->data, timestamp("Transfer failed: storage error"));
        return -6;
    }
    return 1;
}
```
- **0**: source account not found
- **-5**: attempted self-transfer
- **-4**: destination account missing
- **-2**: PIN mismatch
- **-3**: invalid amount
- **-1**: insufficient funds
- **-6**: storage error (both accounts rolled back)

### 4. `changePin`
```cpp
int changePin(int accNo, int oldPin, int newPin) {
    Node* n = findNode(accNo);
    if (!n) return 0;
    if (!n->data->verifyPin(oldPin)) {
        addLogCapped(n->data, timestamp("PIN change failed: bad PIN"));
        return -1;
    }
    int before = n->data->getPin();
    n->data->setPin(newPin);
    addLogCapped(n->data, timestamp("PIN changed"));
    if (!saveToFile(DATA_FILE)) {
        n->data->setPin(before);
        addLogCapped(n->data, timestamp("PIN change failed: storage error"));
        return -3;
    }
    return 1;
}
```
- **0**: account not found
- **-1**: old PIN incorrect
- **-3**: unable to save new PIN

### 5. `getBalance`
```cpp
int getBalance(int accNo, int pin, long long& outBal) const {
    Node* n = findNode(accNo);
    if (!n) return 0;
    if (!n->data->verifyPin(pin)) return -1;
    outBal = n->data->getBalance();
    return 1;
}
```
- **0**: account not found
- **-1**: PIN mismatch

### 6. `miniStatement`
```cpp
int miniStatement(int accNo, int pin, int N) const {
    Node* n = findNode(accNo);
    if (!n) return 0;
    if (!n->data->verifyPin(pin)) return -1;
    vector<string> logs;
    for (LogNode* cur = n->data->getLogHead(); cur; cur = cur->next) logs.push_back(cur->text);
    if (logs.empty()) return -2;
    int start = (int)logs.size() > N ? (int)logs.size() - N : 0;
    for (int i = start; i < (int)logs.size(); ++i) {
        printCentered(logs[i]);
    }
    return 1;
}
```
- **0**: account not found
- **-1**: PIN mismatch
- **-2**: no transactions recorded

### 7. `deleteAccount`
```cpp
bool deleteAccount(int accNo) {
    if (!head) return false;
    if (head->data->getAccNo() == accNo) {
        Node* t = head; head = head->next;
        addLogCapped(t->data, timestamp("Account deleted"));
        moveLogsToDeleted(t->data);
        delete t->data;
        delete t;
        return saveToFile(DATA_FILE);
    }
    Node* prev = head;
    Node* cur = head->next;
    while (cur) {
        if (cur->data->getAccNo() == accNo) {
            prev->next = cur->next;
            addLogCapped(cur->data, timestamp("Account deleted"));
            moveLogsToDeleted(cur->data);
            delete cur->data;
            delete cur;
            return saveToFile(DATA_FILE);
        }
        prev = cur; cur = cur->next;
    }
    return false;
}
```
  - Returns `true` if the account is removed and data saved. `false` means the account was missing or saving failed.

## Program Flow and Menus
The `main` function seeds the random generator, loads data from disk, and shows a top‑level menu with three service panels:

- **Administrator panel** – create, list, search, edit, or delete accounts.
- **ATM panel** – deposit, withdraw, transfer, check balance, change PIN, or print a mini statement.
- **CDM panel** – quick deposits and balance inquiries.

Each panel is a loop that reads an option number, asks for any required information, and calls one of the `Bank` methods above. Prompts and messages are centered on the console using `printCentered`.

## Example Session
1. Start the program and choose option `1` for the administrator panel.
2. Create an account by entering the holder’s name, passport number, gender, account type, a four‑digit PIN, and an initial balance.
3. Return to the main menu and choose `3` for the ATM service.
4. Select the deposit option, enter the new account number and PIN, then provide the amount (must be a multiple of `DENOM`).
5. The system prints the updated balance and appends a log entry such as "Deposit +RM 100, before=RM 500, after=RM 600".
6. Performing a withdrawal or transfer follows the same pattern: enter account numbers, PINs, and amounts. Errors (bad PIN, insufficient funds, etc.) are reported and logged.

## Error Handling Summary
- **Account not found** – the supplied account number does not exist.
- **PIN mismatch** – entered PIN does not match the stored one.
- **Invalid amount** – deposit or withdrawal amount is non-positive or not a multiple of `DENOM`.
- **Insufficient funds** – withdrawal or transfer would drop the balance below `MIN_BAL`.
- **Self-transfer** – source and destination accounts are identical.
- **Destination not found** – transfer target account is missing.
- **Storage error** – data could not be written to disk; operations are rolled back.
- **No transactions** – requested mini statement but no logs are available.

All failures are logged with timestamps. When storage errors occur, the code reverses in-memory changes so file and memory states remain consistent.

## Running the Program
1. **Compile** (needs a C++17 compiler such as `g++`):
   ```bash
   g++ -std=c++17 bank_system.cpp -o bank_system
   ```
2. **Run** the executable:
   ```bash
   ./bank_system
   ```
3. **Use the menus**:
   - *Administrator panel*: create, search, list, edit, or delete accounts.
   - *ATM service*: deposit, withdraw, transfer, change PIN, or print mini statement.
   - *CDM service*: quick deposits and balance inquiries.
   Input is menu-driven; enter the number shown, then supply any requested details (account number, PIN, amount, etc.).

## Conclusion
The Bank Account Management System showcases how core banking features can be implemented with linked lists, binary file storage, and defensive programming. Detailed logs, explicit return codes, and transaction rollbacks make the system suitable for studying error handling in financial software. Its modular design invites further enhancements such as encryption, networking, or graphical interfaces.
