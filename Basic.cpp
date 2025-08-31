#include <iostream>
#include <string>
using namespace std;

// Node structure for each bank account
class Node {
public:
    int accNo;
    string name;
    double balance;
    Node* next;

    Node(int a, string n, double b) {
        accNo = a;
        name = n;
        balance = b;
        next = NULL;
    }
};

// Bank class to manage accounts
class Bank {
private:
    Node* head;

public:
    Bank() { head = NULL; }

    // Add account
    void addAccount(int accNo, string name, double balance) {
        if (findAccount(accNo) != NULL) {
            cout << "Account already exists!" << endl;
            return;
        }
        Node* newNode = new Node(accNo, name, balance);
        newNode->next = head;
        head = newNode;
        cout << "Account added." << endl;
    }

    // Display accounts
    void displayAccounts() {
        if (head == NULL) {
            cout << "No accounts found." << endl;
            return;
        }
        Node* temp = head;
        while (temp != NULL) {
            cout << "Account No: " << temp->accNo
                 << "; Name: " << temp->name
                 << "; Balance: " << temp->balance << endl;
            temp = temp->next;
        }
    }

    // Find account (helper)
    Node* findAccount(int accNo) {
        Node* temp = head;
        while (temp != NULL) {
            if (temp->accNo == accNo)
                return temp;
            temp = temp->next;
        }
        return NULL;
    }

    // Deposit
    void deposit(int accNo, double amount) {
        Node* acc = findAccount(accNo);
        if (acc == NULL) {
            cout << "Account not found." << endl;
            return;
        }
        if (amount <= 0) {
            cout << "Invalid amount." << endl;
            return;
        }
        acc->balance += amount;
        cout << "Deposit successful. New Balance: " << acc->balance << endl;
    }

    // Withdraw
    void withdraw(int accNo, double amount) {
        Node* acc = findAccount(accNo);
        if (acc == NULL) {
            cout << "Account not found." << endl;
            return;
        }
        if (amount <= 0) {
            cout << "Invalid amount." << endl;
            return;
        }
        if (acc->balance < amount) {
            cout << "Insufficient funds." << endl;
            return;
        }
        acc->balance -= amount;
        cout << "Withdrawal successful. New Balance: " << acc->balance << endl;
    }

    // Delete account
    void deleteAccount(int accNo) {
        Node* temp = head;
        Node* prev = NULL;

        while (temp != NULL && temp->accNo != accNo) {
            prev = temp;
            temp = temp->next;
        }

        if (temp == NULL) {
            cout << "Account not found." << endl;
            return;
        }

        if (prev == NULL) {
            head = head->next; // deleting head
        } else {
            prev->next = temp->next;
        }

        delete temp;
        cout << "Account deleted." << endl;
    }

    // Destructor
    ~Bank() {
        Node* temp;
        while (head != NULL) {
            temp = head;
            head = head->next;
            delete temp;
        }
    }
};

// Main menu
int main() {
    Bank bank;
    int choice;

    do {
        cout << "\n--- MENU ---" << endl;
        cout << "1. Add Account" << endl;
        cout << "2. Display Accounts" << endl;
        cout << "3. Search Account" << endl;
        cout << "4. Deposit" << endl;
        cout << "5. Withdraw" << endl;
        cout << "6. Delete Account" << endl;
        cout << "7. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            int accNo;
            string name;
            double bal;
            cout << "Enter account number: ";
            cin >> accNo;
            cin.ignore(); // fix for getline
            cout << "Enter name: ";
            getline(cin, name);
            cout << "Enter initial balance: ";
            cin >> bal;
            bank.addAccount(accNo, name, bal);
        }
        else if (choice == 2) {
            bank.displayAccounts();
        }
        else if (choice == 3) {
            int accNo;
            cout << "Enter account number: ";
            cin >> accNo;
            Node* acc = bank.findAccount(accNo);
            if (acc == NULL)
                cout << "Account not found." << endl;
            else
                cout << "Account No: " << acc->accNo
                     << "; Name: " << acc->name
                     << "; Balance: " << acc->balance << endl;
        }
        else if (choice == 4) {            // Deposit
            int accNo;
            cout << "Enter account number: ";
            cin >> accNo;
            Node* acc = bank.findAccount(accNo);
            if (acc == NULL) {
            cout << "Account not found." << endl;
        } else {
            double amt;
            cout << "Enter deposit amount: ";
            cin >> amt;
            bank.deposit(accNo, amt);
            }
        }

        else if (choice == 5) {
            int accNo;
            cout << "Enter account number: ";
            cin >> accNo;

            Node* acc = bank.findAccount(accNo);
        if (acc == NULL) {
            cout << "Account not found." << endl;
        } else {
            double amt;
            cout << "Enter withdraw amount: ";
            cin >> amt;
            bank.withdraw(accNo, amt);
            }
        }

        else if (choice == 6) {
            int accNo;
            cout << "Enter account number: ";
            cin >> accNo;
            bank.deleteAccount(accNo);
        }
        else if (choice == 7) {
            cout << "Exiting..." << endl;
        }
        else {
            cout << "Please enter 1-7." << endl;
        }
    } while (choice != 7);

    return 0;
}
