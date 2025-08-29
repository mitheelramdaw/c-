#include <iostream>
#include <string>
using namespace std;

class Laptop {
    public :
    string model;
    int year;

    void specs(){
        cout<< "Your laptop is a " << model << " made in " << year<< endl;
    }
};



int main() {

    Laptop Ryan_Laptop;

    Ryan_Laptop.model = "Asus XPLS69";
    Ryan_Laptop.year = 2022;

    Ryan_Laptop.specs();

    return 0;
};