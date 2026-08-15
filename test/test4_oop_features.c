// test4_oop_features.c
// Covers: classes and objects, inheritance, the public / private / protected
//         keywords, this, new and delete, scope resolution.

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

class Dog : public Animal {
public:
    bool loud;

    void bark() {
        this->loud = true;
    }
};

class Cat : protected Animal {
private:
    bool quiet;
};

int Animal::age = 0;

int main() {
    Animal *a = new Animal();
    a->setAge(3);
    delete a;

    Dog d;
    d.bark();
    d.setAge(5);

    Cat c;
    return 0;
}
