class Animal {
public:
    int age;
    Animal(int a) {
        this->age = a;
    }
    void speak() {
        age = age;
    }
protected:
    int legs;
private:
    int secretId;
};

class Dog : public Animal {
public:
    Dog(int a) : Animal(a) {}
};

int main() {
    Animal *a = new Animal(3);
    Dog d(4);
    delete a;

    int value = 10;
    int &ref = value;
    ref = 20;

    auto lam = [&ref](int x) {
        return x + ref;
    };

    return lam(1);
}
