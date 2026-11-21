#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <iomanip>
using namespace std;
// Custom exception for banking errors
class BankException : public runtime_error {
public: 
    explicit BankException(const string& msg) : runtime_error(msg) {}
};

class InsufficientFundsException : public BankException {
public:
    explicit InsufficientFundsException(double requested, double available)
        : BankException("Insufficient funds: requested " + to_string(requested) +
                        ", available " + to_string(available)) {}
};

// Base class
class Account {
private:
    static int nextId;
    int accountNumber;
    string holderName;
    double balance;   // encapsulated: only accessible through methods

protected:
    void setBalance(double amount) { balance = amount; }

public:
    Account(const string& name, double initialDeposit)
        : accountNumber(nextId++), holderName(name), balance(0.0) {
        if (initialDeposit < 0)
            throw BankException("Initial deposit cannot be negative");
        balance = initialDeposit;
    }

    virtual ~Account() {}

    int getAccountNumber() const { return accountNumber; }
    string getHolderName() const { return holderName; }
    double getBalance() const { return balance; }

    void deposit(double amount) {
        if (amount <= 0)
            throw BankException("Deposit amount must be positive");
        balance += amount;
    }

    // Polymorphic withdrawal: each account type defines its own rules
    virtual void withdraw(double amount) {
        if (amount <= 0)
            throw BankException("Withdrawal amount must be positive");
        if (amount > balance)
            throw InsufficientFundsException(amount, balance);
        balance -= amount;
    }

    // Polymorphic monthly processing
    virtual void monthlyUpdate() = 0;

    virtual string getAccountType() const = 0;

    virtual void display() const {
        cout << fixed << setprecision(2)
                  << "[" << accountNumber << "] " << getAccountType()
                  << " | Holder: " << holderName
                  << " | Balance: " << balance << "\n";
    }
};

int Account::nextId = 1001;

// Derived class 1: Savings account with interest and no overdraft
class SavingsAccount : public Account {
private:
    double interestRate;   // annual, e.g. 0.04 for 4%
    double minimumBalance;

public:
    SavingsAccount(const string& name, double deposit, double rate, double minBal)
        : Account(name, deposit), interestRate(rate), minimumBalance(minBal) {}

    void withdraw(double amount) override {
        if (amount <= 0)
            throw BankException("Withdrawal amount must be positive");
        if (getBalance() - amount < minimumBalance)
            throw BankException("Withdrawal would breach minimum balance of " +
                                to_string(minimumBalance));
        Account::withdraw(amount);
    }

    void monthlyUpdate() override {
        double interest = getBalance() * interestRate / 12.0;
        deposit(interest);
    }

    string getAccountType() const override { return "Savings"; }
};

// Derived class 2: Current account with overdraft facility
class CurrentAccount : public Account {
private:
    double overdraftLimit;

public:
    CurrentAccount(const std::string& name, double deposit, double overdraft)
        : Account(name, deposit), overdraftLimit(overdraft) {}

    void withdraw(double amount) override {
        if (amount <= 0)
            throw BankException("Withdrawal amount must be positive");
        if (getBalance() - amount < -overdraftLimit)
            throw InsufficientFundsException(amount, getBalance() + overdraftLimit);
        // Bypass base check so overdraft is allowed
        setBalance(getBalance() - amount);
    }

   void monthlyUpdate() override {
    if (getBalance() < 0) {

        double fee = 25.0;

        double newBalance = getBalance() - fee;

        if (newBalance >= -overdraftLimit) {
            setBalance(newBalance);
            cout << "Overdraft fee deducted: " << fee << endl;
        }
        else {
            cout << "Overdraft fee skipped. "
                 << "It would exceed the overdraft limit.\n";
        }
    }
}

    string getAccountType() const override { return "Current"; }
};

// Bank manages a collection of accounts polymorphically
class Bank {
private:
    vector<unique_ptr<Account>> accounts;

    Account* find(int accountNumber) {
        for (auto& acc : accounts)
            if (acc->getAccountNumber() == accountNumber)
                return acc.get();
        throw BankException("Account " + to_string(accountNumber) + " not found");
    }

public:
    void addAccount(unique_ptr<Account> acc) {
        accounts.push_back(move(acc));
    }

    void transfer(int fromNo, int toNo, double amount) {
        Account* from = find(fromNo);
        Account* to = find(toNo);
        from->withdraw(amount);
        to->deposit(amount);
    }

    void runMonthEnd() {
        for (auto& acc : accounts)
            acc->monthlyUpdate();   // polymorphic dispatch
    }

    void showAll() const {
        for (const auto& acc : accounts)
            acc->display();
    }

    Account& getAccount(int accountNumber) { return *find(accountNumber); }
};

// Simple menu-driven interface
int main() {
    Bank bank;
    bank.addAccount(make_unique<SavingsAccount>("Asha", 5000.0, 0.04, 1000.0));
    bank.addAccount(make_unique<CurrentAccount>("Ravi", 2000.0,1500.0));

    int choice;
    while (true) {
        cout << "\n1. Show accounts  2. Deposit  3. Withdraw  "
                     "4. Transfer  5. Month-end  0. Exit\nChoice: ";
        cin >> choice;
        if (choice == 0) break;

        try {
            if (choice == 1) {
                bank.showAll();
            } else if (choice == 2 || choice == 3) {
                int no; double amt;
                cout << "Account number: "; cin >> no;
                cout << "Amount: "; cin >> amt;
                if (choice == 2) bank.getAccount(no).deposit(amt);
                else             bank.getAccount(no).withdraw(amt);
                cout << "Done.\n";
            } else if (choice == 4) {
                int from, to; double amt;
                cout << "From account: "; cin >> from;
                cout << "To account: ";   cin >> to;
                cout << "Amount: ";       cin >> amt;
                bank.transfer(from, to, amt);
                cout << "Transfer complete.\n";
            } else if (choice == 5) {
                bank.runMonthEnd();
                cout << "Month-end processing done.\n";
            } else {
                cout << "Invalid choice.\n";
            }
        } catch (const InsufficientFundsException& e) {
            cout << "Funds error: " << e.what() << "\n";
        } catch (const BankException& e) {
            cout << "Bank error: " << e.what() << "\n";
        }
    }
    cout << "Goodbye!\n";
    return 0;
}