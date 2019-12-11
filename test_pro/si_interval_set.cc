#include <functional>
#include <iostream>
#include <boost/icl/interval_set.hpp>
using namespace std;
#include "include/interval_set.h"
void test_boost(){
    typedef boost::icl::interval_set<unsigned int> set_t;
    typedef set_t::interval_type ival;
    set_t outages;
    outages.insert(ival::closed(1,1));
    outages.insert(ival::closed(7,10));
    outages.insert(ival::closed(10,12));
    outages.insert(ival::open(90,120));
    for(set_t::iterator it = outages.begin(); it != outages.end(); it++){
        std::cout << it->lower() << ", " << it->upper() << "\n";
    }
}
void test_ceph(){
    /* error
    interval_set<int> set1;
    set1.insert(1,3);
    set1.insert(2,3);
    cout << set1 << endl;
    */
    interval_set<int> set1;
    cout << "---------" << endl; 
    set1.insert(7,10);
    set1.insert(9,2);
    cout << set1 << " want : [7.10]"<< endl;
    cout << "---------" << endl; 
    set1.insert(9,2);
    cout << set1 << " want : [7.10]"<< endl;
    
    interval_set<int> set2;
    cout << "---------" << endl; 
    set2.insert(9,2);
    set2.insert(7,10);
    cout << set1 << " want : [7.10]"<< endl;

    interval_set<int> set3;
    cout << "---------" << endl;
    set3.insert(7,10);
    set3.erase(9,2);
    //set2.subtract(10,12);
    cout << set1 << " want : [7.2][11,6]"<< endl;
}
int main(){
    test_ceph();
    // test_boost();
    return 0;
}
