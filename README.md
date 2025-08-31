Bank Account Management System (C++ OOP)
📌 Description

A Bank Account Management System implemented in C++ using Object-Oriented Programming (OOP) and singly linked lists.
The project has two versions:

Basic.cpp → Simple linked list version with add, display, search, deposit, withdraw, and delete operations.

Advanced.cpp → Full-featured system with Admin/Staff/ATM/CDM panels, PIN authentication, account types, transaction logs, file storage (accounts.dat, logs.dat), and error handling.

🚀 Features

Add, display, search, and delete accounts

Deposit and withdraw with validation

Transfer money between accounts

PIN management (set/change PIN)

Mini statement (last N transactions)

Admin & Staff login panels

ATM/CDM simulation (deposit, withdraw, balance inquiry)

Persistent storage in binary files (accounts.dat, logs.dat)

🛠️ Concepts Used

Object-Oriented Programming (classes, encapsulation, constructors, destructors)

Data Structures (singly linked list for accounts, linked list for logs)

Dynamic Memory Management (new/delete, destructor cleanup)

File Handling (binary read/write to save accounts and logs)

Error Handling (invalid inputs, insufficient balance, wrong PIN)

Menu-Driven Interface

📂 Project Files
BankSystem/

│── Basic.cpp       # Simple linked list implementation

│── Advanced.cpp    # Full-featured bank system with logs & panels

│── accounts.dat    # Binary file storing account details

│── logs.dat        # Binary file storing account logs

│── README.md       # Project documentation


▶️ How to Compile & Run

Basic Version
g++ Basic.cpp -o bank_basic

./bank_basic

Advanced Version

g++ Advanced.cpp -o bank_advanced

./bank_advanced

📷 Sample Menu (Basic.cpp)

--- MENU ---
1. Add Account
2. Display Accounts
3. Search Account
4. Deposit
5. Withdraw
6. Delete Account
7. Exit

📷 Sample Menu (Advanced.cpp)

********** LOGIN || PANEL **********
1. Admin Login
2. Staff Login
3. ATM/CDM Service
4. Exit
