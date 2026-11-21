#include <iostream>
#include <string>
using namespace std;

// Standalone helper function
void printDivider() {
    cout << "----------------------------------------\n";
}

class ShoppingCart {
private:
    string owner;
    double* prices;             // Dynamic memory pointer
    int itemCount;
    static int activeCarts;     // Static data member

public:
    // Parameterized constructor: allocates dynamic memory
    ShoppingCart(const string& name, int count) : owner(name), itemCount(count) {
        prices = new double[itemCount];
        for (int i = 0; i < itemCount; i++) prices[i] = 0.0;
        activeCarts++;
    }

    // Copy constructor: deep copy of dynamic array
    ShoppingCart(const ShoppingCart& other) : owner(other.owner), itemCount(other.itemCount) {
        prices = new double[itemCount];
        for (int i = 0; i < itemCount; i++) prices[i] = other.prices[i];
        activeCarts++;
    }

    // Destructor: frees dynamic memory
    ~ShoppingCart() {
        delete[] prices;
        activeCarts--;
    }

    // Encapsulated setter and member function
    void setPrice(int index, double price) {
        if (index >= 0 && index < itemCount && price >= 0.0)
            prices[index] = price;
    }

    double totalBill() const {
        double sum = 0.0;
        for (int i = 0; i < itemCount; i++) sum += prices[i];
        return sum;
    }

    // Operator overloading (+): merges two shopping carts into a new cart
    ShoppingCart operator+(const ShoppingCart& other) const {
        ShoppingCart merged(owner + " & " + other.owner, itemCount + other.itemCount);
        for (int i = 0; i < itemCount; i++)
            merged.prices[i] = prices[i];
        for (int i = 0; i < other.itemCount; i++)
            merged.prices[itemCount + i] = other.prices[i];
        return merged;
    }

    // Static member function
    static int getActiveCarts() {
        return activeCarts;
    }

    // Friend functions
    friend double applyDiscount(const ShoppingCart& c, double percent);
    friend ostream& operator<<(ostream& out, const ShoppingCart& c);
};

// Static member initialization
int ShoppingCart::activeCarts = 0;

// Friend function: accesses private prices and itemCount directly
double applyDiscount(const ShoppingCart& c, double percent) {
    double total = 0.0;
    for (int i = 0; i < c.itemCount; i++) total += c.prices[i];
    return total * (1.0 - percent / 100.0);
}

// Overloaded << operator to print ShoppingCart object
ostream& operator<<(ostream& out, const ShoppingCart& c) {
    out << "Cart(" << c.owner << ") | Items: ";
    for (int i = 0; i < c.itemCount; i++) out << c.prices[i] << " ";
    out << "| Total: " << c.totalBill();
    return out;
}

int main() {
    printDivider();
    cout << "Active carts at start: " << ShoppingCart::getActiveCarts() << "\n";

    ShoppingCart c1("Leon.S Kennedy", 2);
    c1.setPrice(0, 250.0);
    c1.setPrice(1, 150.0);

    ShoppingCart c2("Ada Wong", 2);
    c2.setPrice(0, 100.0);
    c2.setPrice(1, 300.0);

    ShoppingCart combined = c1 + c2;    // Uses overloaded +

    cout << c1 << "\n";
    cout << c2 << "\n";
    cout << combined << "\n";
    cout << "Combined after 10% discount: " << applyDiscount(combined, 10.0) << "\n";
    cout << "Active carts now: " << ShoppingCart::getActiveCarts() << "\n";
    printDivider();

    return 0;
}