#include<iostream>
#include <fstream>
using namespace std;

class A{
public:
    A(){
        cout << "A()" << endl;
    }
    ~A(){
        cout << "~A()" << endl;
    }
};

class B{
public:
    A a;
    B(){
        cout << "B()" << endl;
    }
    ~B(){
        cout << "~B()" << endl;
    }
};

#include "ta1.h"

using test::ta;

int main(){
    B b;
    ofstream o;
    o.open("");
    if (o){
        o << "afdf1" << endl;
        o.close();
    }else
        cout << "error" << endl;
    cout <<  (1 << 9) << endl;   

    ta ta1;
    ta1.ta_test();
}
