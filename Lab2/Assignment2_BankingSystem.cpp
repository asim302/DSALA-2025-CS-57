#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
using namespace std;

const int CAN_WITHDRAW = 1;
const int CAN_DEPOSIT = 2;
const int CAN_TRANSFER = 4;
const int VIP_ACCOUNT = 8;

const int TYPE_DEPOSIT = 1;
const int TYPE_WITHDRAWAL = 2;
const int TYPE_TRANSFER = 3;

class Account {
protected:
    int accountId;
    string name;
    double balance;
    unsigned int permissions;
    vector<double> transactions;

public:
    Account() : accountId(0), name(""), balance(0), permissions(0) {}
    Account(int id, string n, double bal, unsigned int perm) 
        : accountId(id), name(n), balance(bal), permissions(perm) {}
    
    virtual ~Account() {}
    
    virtual void deposit(double amount) = 0;
    virtual void withdraw(double amount) = 0;
    virtual void saveToFile(ofstream& file) = 0;
    virtual string getType() const = 0;
    
    bool hasPermission(unsigned int flag) const {
        return (permissions & flag) != 0;
    }
    
    void addTransaction(double amount) {
        transactions.push_back(amount);
    }
    
    void display() const {
        cout << "\n=== Account Details ===" << endl;
        cout << "ID: " << accountId << endl;
        cout << "Name: " << name << endl;
        cout << "Balance: $" << fixed << setprecision(2) << balance << endl;
        cout << "Permissions: " << permissions << endl;
        cout << "Can Withdraw: " << (hasPermission(CAN_WITHDRAW) ? "Yes" : "No") << endl;
        cout << "Can Deposit: " << (hasPermission(CAN_DEPOSIT) ? "Yes" : "No") << endl;
        cout << "Can Transfer: " << (hasPermission(CAN_TRANSFER) ? "Yes" : "No") << endl;
        cout << "VIP: " << (hasPermission(VIP_ACCOUNT) ? "Yes" : "No") << endl;
        cout << "\nTransaction History:" << endl;
        for (size_t i = 0; i < transactions.size(); i++) {
            cout << "  " << (transactions[i] >= 0 ? "+" : "") << transactions[i] << endl;
        }
    }
    
    int getId() const { return accountId; }
    string getName() const { return name; }
    double getBalance() const { return balance; }
    void setBalance(double b) { balance = b; }
    unsigned int getPermissions() const { return permissions; }
    void setPermissions(unsigned int p) { permissions = p; }
    vector<double> getTransactions() const { return transactions; }
    void setTransactions(const vector<double>& t) { transactions = t; }
};

class SavingsAccount : public Account {
private:
    double interestRate;
    
public:
    SavingsAccount() : Account(), interestRate(0.05) {}
    SavingsAccount(int id, string n, double bal, unsigned int perm) 
        : Account(id, n, bal, perm), interestRate(0.05) {}
    
    void deposit(double amount) override {
        if (!hasPermission(CAN_DEPOSIT)) {
            cout << "Deposit not allowed!" << endl;
            return;
        }
        if (amount > 0) {
            balance += amount;
            addTransaction(amount);
            cout << "Deposit successful! New balance: $" << balance << endl;
        } else {
            cout << "Invalid amount!" << endl;
        }
    }
    
    void withdraw(double amount) override {
        if (!hasPermission(CAN_WITHDRAW)) {
            cout << "Withdrawal not allowed!" << endl;
            return;
        }
        if (amount > 0 && amount <= balance) {
            balance -= amount;
            addTransaction(-amount);
            cout << "Withdrawal successful! New balance: $" << balance << endl;
        } else {
            cout << "Invalid amount or insufficient funds!" << endl;
        }
    }
    
    void applyInterest() {
        double interest = balance * interestRate;
        balance += interest;
        addTransaction(interest);
    }
    
    void saveToFile(ofstream& file) override {
        file << "ACCOUNT Savings\n";
        file << accountId << " " << name << " " << balance << " " << permissions << "\n";
        file << "TRANSACTIONS\n";
        for (size_t i = 0; i < transactions.size(); i++) {
            file << transactions[i] << "\n";
        }
        file << "END\n";
    }
    
    string getType() const override { return "Savings"; }
    
    double getInterestRate() const { return interestRate; }
};

class CurrentAccount : public Account {
private:
    double overdraftLimit;
    
public:
    CurrentAccount() : Account(), overdraftLimit(1000) {}
    CurrentAccount(int id, string n, double bal, unsigned int perm) 
        : Account(id, n, bal, perm), overdraftLimit(1000) {}
    
    void deposit(double amount) override {
        if (!hasPermission(CAN_DEPOSIT)) {
            cout << "Deposit not allowed!" << endl;
            return;
        }
        if (amount > 0) {
            balance += amount;
            addTransaction(amount);
            cout << "Deposit successful! New balance: $" << balance << endl;
        } else {
            cout << "Invalid amount!" << endl;
        }
    }
    
    void withdraw(double amount) override {
        if (!hasPermission(CAN_WITHDRAW)) {
            cout << "Withdrawal not allowed!" << endl;
            return;
        }
        if (amount > 0 && (balance + overdraftLimit) >= amount) {
            balance -= amount;
            addTransaction(-amount);
            cout << "Withdrawal successful! New balance: $" << balance << endl;
        } else {
            cout << "Invalid amount or overdraft limit exceeded!" << endl;
        }
    }
    
    void saveToFile(ofstream& file) override {
        file << "ACCOUNT Current\n";
        file << accountId << " " << name << " " << balance << " " << permissions << "\n";
        file << "TRANSACTIONS\n";
        for (size_t i = 0; i < transactions.size(); i++) {
            file << transactions[i] << "\n";
        }
        file << "END\n";
    }
    
    string getType() const override { return "Current"; }
    
    double getOverdraftLimit() const { return overdraftLimit; }
};

unsigned int encodeTransaction(int type, double amount) {
    unsigned int amountInt = (unsigned int)(amount * 100);
    return (type << 28) | (amountInt & 0x0FFFFFFF);
}

void decodeTransaction(unsigned int encoded, int& type, double& amount) {
    type = (encoded >> 28) & 0x0F;
    unsigned int amountInt = encoded & 0x0FFFFFFF;
    amount = (double)amountInt / 100.0;
}

void saveAllAccounts(const vector<Account*>& accounts, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file for saving!" << endl;
        return;
    }
    
    for (size_t i = 0; i < accounts.size(); i++) {
        accounts[i]->saveToFile(file);
    }
    file.close();
    cout << "All accounts saved successfully!" << endl;
}

void loadAllAccounts(vector<Account*>& accounts, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file for loading!" << endl;
        return;
    }
    
    string line;
    while (getline(file, line)) {
        if (line == "ACCOUNT Savings") {
            int id; string name; double balance; unsigned int perm;
            getline(file, line);
            stringstream ss(line);
            ss >> id >> name >> balance >> perm;
            
            SavingsAccount* acc = new SavingsAccount(id, name, balance, perm);
            
            getline(file, line);
            vector<double> trans;
            while (getline(file, line) && line != "END") {
                stringstream ts(line);
                double t;
                ts >> t;
                trans.push_back(t);
            }
            acc->setTransactions(trans);
            accounts.push_back(acc);
        }
        else if (line == "ACCOUNT Current") {
            int id; string name; double balance; unsigned int perm;
            getline(file, line);
            stringstream ss(line);
            ss >> id >> name >> balance >> perm;
            
            CurrentAccount* acc = new CurrentAccount(id, name, balance, perm);
            
            getline(file, line);
            vector<double> trans;
            while (getline(file, line) && line != "END") {
                stringstream ts(line);
                double t;
                ts >> t;
                trans.push_back(t);
            }
            acc->setTransactions(trans);
            accounts.push_back(acc);
        }
    }
    file.close();
    cout << "All accounts loaded successfully!" << endl;
}

void calculateMonthlySummary(const vector<Account*>& accounts) {
    double monthlyTotals[12] = {0};
    
    for (size_t i = 0; i < accounts.size(); i++) {
        const vector<double>& trans = accounts[i]->getTransactions();
        for (size_t j = 0; j < trans.size(); j++) {
            int month = j % 12;
            monthlyTotals[month] += trans[j];
        }
    }
    
    cout << "\n=== Monthly Summary ===" << endl;
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++) {
        cout << months[i] << ": " << (monthlyTotals[i] >= 0 ? "+" : "") 
             << fixed << setprecision(2) << monthlyTotals[i] << endl;
    }
}

void transferFunds(Account* from, Account* to, double amount) {
    if (!from->hasPermission(CAN_TRANSFER)) {
        cout << "Transfer not allowed from this account!" << endl;
        return;
    }
    if (!to->hasPermission(CAN_DEPOSIT)) {
        cout << "Transfer not allowed to this account!" << endl;
        return;
    }
    if (amount > 0 && from->getBalance() >= amount) {
        from->setBalance(from->getBalance() - amount);
        to->setBalance(to->getBalance() + amount);
        from->addTransaction(-amount);
        to->addTransaction(amount);
        cout << "Transfer successful! $" << amount << " transferred." << endl;
    } else {
        cout << "Transfer failed! Check balance and amount." << endl;
    }
}

int main() {
    vector<Account*> accounts;
    int choice;
    
    while (true) {
        cout << "\n===== BANKING SYSTEM =====" << endl;
        cout << "1. Create Account" << endl;
        cout << "2. Deposit" << endl;
        cout << "3. Withdraw" << endl;
        cout << "4. Show Account" << endl;
        cout << "5. Save to File" << endl;
        cout << "6. Load from File" << endl;
        cout << "7. Transfer Funds" << endl;
        cout << "8. Monthly Summary" << endl;
        cout << "9. Test Transaction Compression" << endl;
        cout << "10. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        
        if (choice == 1) {
            int id, type; string name; double balance; unsigned int perm = 0;
            cout << "Enter Account ID: "; cin >> id;
            cout << "Enter Name: "; cin >> name;
            cout << "Enter Initial Balance: "; cin >> balance;
            cout << "Account Type (1=Savings, 2=Current): "; cin >> type;
            cout << "Permissions (1=Withdraw, 2=Deposit, 4=Transfer, 8=VIP): " << endl;
            cout << "Enter permission value (e.g., 7 for all): "; cin >> perm;
            
            if (type == 1) {
                accounts.push_back(new SavingsAccount(id, name, balance, perm));
            } else {
                accounts.push_back(new CurrentAccount(id, name, balance, perm));
            }
            cout << "Account created successfully!" << endl;
        }
        else if (choice == 2) {
            int id; double amount;
            cout << "Enter Account ID: "; cin >> id;
            cout << "Enter Amount: "; cin >> amount;
            bool found = false;
            for (size_t i = 0; i < accounts.size(); i++) {
                if (accounts[i]->getId() == id) {
                    accounts[i]->deposit(amount);
                    found = true;
                    break;
                }
            }
            if (!found) cout << "Account not found!" << endl;
        }
        else if (choice == 3) {
            int id; double amount;
            cout << "Enter Account ID: "; cin >> id;
            cout << "Enter Amount: "; cin >> amount;
            bool found = false;
            for (size_t i = 0; i < accounts.size(); i++) {
                if (accounts[i]->getId() == id) {
                    accounts[i]->withdraw(amount);
                    found = true;
                    break;
                }
            }
            if (!found) cout << "Account not found!" << endl;
        }
        else if (choice == 4) {
            int id;
            cout << "Enter Account ID: "; cin >> id;
            bool found = false;
            for (size_t i = 0; i < accounts.size(); i++) {
                if (accounts[i]->getId() == id) {
                    accounts[i]->display();
                    found = true;
                    break;
                }
            }
            if (!found) cout << "Account not found!" << endl;
        }
        else if (choice == 5) {
            saveAllAccounts(accounts, "banking_data.txt");
        }
        else if (choice == 6) {
            for (size_t i = 0; i < accounts.size(); i++) {
                delete accounts[i];
            }
            accounts.clear();
            loadAllAccounts(accounts, "banking_data.txt");
        }
        else if (choice == 7) {
            int fromId, toId; double amount;
            cout << "Enter Source Account ID: "; cin >> fromId;
            cout << "Enter Destination Account ID: "; cin >> toId;
            cout << "Enter Amount: "; cin >> amount;
            Account* from = nullptr; Account* to = nullptr;
            for (size_t i = 0; i < accounts.size(); i++) {
                if (accounts[i]->getId() == fromId) from = accounts[i];
                if (accounts[i]->getId() == toId) to = accounts[i];
            }
            if (from && to) {
                transferFunds(from, to, amount);
            } else {
                cout << "One or both accounts not found!" << endl;
            }
        }
        else if (choice == 8) {
            calculateMonthlySummary(accounts);
        }
        else if (choice == 9) {
            unsigned int encoded = encodeTransaction(TYPE_DEPOSIT, 1500.50);
            int type; double amount;
            decodeTransaction(encoded, type, amount);
            cout << "Encoded: " << encoded << endl;
            cout << "Decoded Type: " << type << ", Amount: " << amount << endl;
        }
        else if (choice == 10) {
            for (size_t i = 0; i < accounts.size(); i++) {
                delete accounts[i];
            }
            accounts.clear();
            cout << "Goodbye!" << endl;
            break;
        }
        else {
            cout << "Invalid choice!" << endl;
        }
    }
    
    return 0;
}
