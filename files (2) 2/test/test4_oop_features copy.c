// Tests: class and object; public/private/protected keywords
class Animal {
public:
    int age;

    void setAge(int a) {
        this->age = a;
    }

protected:
    int legs;

private:
    int secretId;

    int hidden() {
        return secretId + legs;
    }
};

// Tests: inheritance
class Dog : public Animal {
public:
    bool loud;

    void bark() {
        this->loud = true;
    }
};

// Tests: inheritance; protected and private keywords
class Cat : protected Animal {
private:
    bool quiet;
};

// Tests: additional scope resolution operator
int Animal::age = 0;

int main() {
    // Tests: class and object; dynamic memory allocation
    Animal *a = new Animal();
    a->setAge(3);
    delete a;

    // Tests: class and object; inheritance
    Dog d;
    d.bark();
    d.setAge(5);

    Cat c;
    return 0;
}
