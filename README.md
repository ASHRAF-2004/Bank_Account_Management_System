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
Below are the main public functions provided by the `Bank` class. Each one is shown with its implementation followed by a detailed explanation of how it works and what error conditions it reports.

### 1. `addAccount`
```cpp
bool addAccount(const string& name, const string& passportNo,
                char gender, const string& accountType,
                int pin, long long balance, int& outAccNo) {
    Node* cur = head;
    while (cur) {
        if (cur->data->getIC() == passportNo) {
            cout << "Account with this passport number already exists!" << endl;
            return false;
        }
        cur = cur->next;
    }
    int accNo = generateAccNo();
    if (accNo == -1) return false;
    Account* acc = new Account(accNo, name, passportNo, gender, accountType, pin, balance);
    addToList(acc);
    addLogCapped(acc, timestamp("Account created"));
    outAccNo = accNo;
    printCentered("Account added successfully!");
    if (!saveToFile(DATA_FILE)) return false;
    return true;
}
```

What it does:

- Rejects duplicates by checking passport numbers through the linked list.
- Generates a new account number sequentially.
- Allocates an `Account` object and pushes it to the head of the singly linked list.
- Logs “Account created” and persists the data to `accounts.dat` and the log file.

*Returns `true` on success.* The function first checks for duplicate passport numbers and aborts if a matching account already exists. A new sequential account number is generated, the account is linked into the list, and a creation log entry is added. If writing to disk fails, the function returns `false` and the in‑memory account remains for the current session only.

### 2. `deposit`
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
What it does:
- Looks up the account by number and aborts if missing.
- Verifies the provided PIN before modifying funds.
- Calls the account's `deposit` helper to validate the amount and update the balance.
- Adds a detailed log entry noting the old and new balance.
- Persists the change to disk; if saving fails the deposit is rolled back and a storage error is logged.

Return codes:

- **1** – deposit completed and transaction saved.
- **0** – the supplied account number does not exist.
- **-2** – the PIN did not match; a "bad PIN" entry is logged.
- **-1** – amount was non‑positive or not a multiple of `DENOM`; the attempt is logged and ignored.
- **-4** – writing to the binary file failed; the code rolls back the in‑memory deposit using `withdraw(amount)` and logs the storage error.

### 3. `withdraw`
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
What it does:
- Searches for the account and rejects the request if it doesn't exist.
- Confirms the PIN then invokes the account's `withdraw` helper to enforce denomination and minimum balance rules.
- On success it logs the withdrawal with before/after balances.
- Saves the new state to disk, rolling back and logging an error if persistence fails.

Return codes:

- **1** – withdrawal completed and saved.
- **0** – account number does not exist.
- **-2** – PIN mismatch; failure logged.
- **-3** – invalid amount (non‑positive or wrong denomination).
- **-1** – insufficient funds after enforcing `MIN_BAL`.
- **-4** – disk write failed; the withdrawn amount is re‑deposited and an error log recorded.

### 4. `transfer`
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
What it does:
- Finds both source and destination accounts, refusing self-transfers or missing accounts.
- Validates the source PIN and ensures sufficient funds and a valid amount.
- Withdraws from the source and deposits into the destination, logging both sides.
- Attempts to save; if writing fails both balances are restored and an error is logged.

Return codes:

- **1** – transfer completed and recorded for both accounts.
- **0** – source account not found.
- **-5** – attempt to transfer to the same account; ignored and logged.
- **-4** – destination account does not exist.
- **-2** – source PIN incorrect.
- **-3** – invalid amount.
- **-1** – insufficient funds in the source account.
- **-6** – saving to disk failed; both accounts are rolled back to their previous balances and an error is logged.

### 5. `changePin`
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
What it does:
- Locates the account and confirms the old PIN.
- Updates the PIN and records the change in the log.
- Persists to disk, reverting to the previous PIN and logging a storage error if saving fails.

Return codes:

- **1** – PIN updated and written to disk.
- **0** – account number not found.
- **-1** – provided old PIN was incorrect.
- **-3** – disk write failed; the original PIN is restored and a log entry notes the storage error.

### 6. `getBalance`
```cpp
int getBalance(int accNo, int pin, long long& outBal) const {
    Node* n = findNode(accNo);
    if (!n) return 0;
    if (!n->data->verifyPin(pin)) return -1;
    outBal = n->data->getBalance();
    return 1;
}
```
What it does:
- Retrieves the account node by number.
- Checks the PIN and outputs the current balance into `outBal`.

Return codes:

- **1** – balance retrieved and stored in `outBal`.
- **0** – account not found.
- **-1** – PIN mismatch.

### 7. `miniStatement`
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
What it does:
- Finds the account and verifies the PIN.
- Gathers up to `N` recent log entries and prints them centered on the console.

Return codes:

- **1** – the last `N` log entries were printed.
- **0** – account number not found.
- **-1** – PIN mismatch.
- **-2** – the account has no transaction logs.

### 8. `deleteAccount`
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
What it does:
- Removes the matching node from the list, archiving its log history.
- Deletes the account object to free memory.
- Saves the updated account list to disk and reports success or failure.

Return value:

- **true** – the account was removed, its logs archived, and changes written to disk.
- **false** – the account number was not found or saving to disk failed.

### 9. `changeInfo`
```cpp
int changeInfo(int accNo, const string& newName, const string& newic,
               char newGender, const string& newTypeCS, int newPIN) {
    Node* n = findNode(accNo);
    if (!n) return 0;
    for (Node* c = head; c; c = c->next) {
        if (c != n && c->data->getIC() == newic) {
            printCentered("Passport already in use.");
            addLogCapped(n->data, timestamp("Info change failed: duplicate passport"));
            return -2;
        }
    }
    n->data->setName(newName);
    n->data->setIC(newic);
    n->data->setGender(newGender);
    n->data->setType(newTypeCS);
    n->data->setPin(newPIN);
    addLogCapped(n->data, timestamp("Info changed"));
    if (!saveToFile(DATA_FILE)) {
        printCentered("Storage error.");
        addLogCapped(n->data, timestamp("Info change failed: storage error"));
        return -3;
    }
    return 1;
}
```
What it does:
- Looks up the account and ensures no other account uses the new passport/ID number.
- Updates all mutable fields (name, passport, gender, account type, PIN) and logs the modification.
- Saves the record, logging and returning an error if persistence fails.

Return codes:

- **1** – information updated and persisted.
- **0** – account number not found.
- **-2** – another account already uses the provided passport/ID number.
- **-3** – saving to disk failed after changes; a log entry describes the storage error.

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
