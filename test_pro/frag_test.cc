#include <stdio.h>
#include <iostream>
#include <list>

using namespace std;
template<typename T, typename T2>
class A{
public:
    void print(){
        cout << 2 << endl;
    }
};

//template<template<typename T, typename T2> class X = A>
template<template<typename, typename> class X = A>
class B{
public:
    void print(){
        X<int, int> x;
        x.print();
    }
};

// start frag test
typedef unsigned int		__u32;
#include "ceph_frag.h"
typedef unsigned int		uint32_t;
typedef uint32_t            _frag_t;
#define assert(t) if(t == 0) {cout << "assert failed : " << __func__<< endl;}
/*
inline uint32_t ceph_frag_make(uint32_t b, uint32_t v)
{
	return (b << 24) |
		(v & (0xffffffu << (24-b)) & 0xffffffu);
}
inline uint32_t ceph_frag_make2(uint32_t b, uint32_t v)
{
	return 
		v & (0xffffffu << (24-b));
}
inline uint32_t ceph_frag_make3(uint32_t b, uint32_t v)
{
	return 
		(v & (0xffffffu << (24-b)) & 0xffffffu);
}
static inline uint32_t ceph_frag_bits(uint32_t f)
{
	return f >> 24;
}
static inline uint32_t ceph_frag_value(uint32_t f)
{
	return f & 0xffffffu;
}

static inline uint32_t ceph_frag_make_child(uint32_t f, int by, int i)
{
	int newbits = ceph_frag_bits(f) + by;
	return ceph_frag_make(newbits,
			 ceph_frag_value(f) | (i << (24 - newbits)));
}
*/
class frag_t{
public:
    _frag_t _enc;
    
    frag_t():_enc(0){}
    frag_t(unsigned v, unsigned b) : _enc(ceph_frag_make(b, v)) { }
    frag_t(_frag_t e) : _enc(e) { }
    
    // constructors
    void from_unsigned(unsigned e) { _enc = e; }
    
    // accessors
    unsigned value() const { return ceph_frag_value(_enc); }
    unsigned bits() const { return ceph_frag_bits(_enc); }
    unsigned mask() const { return ceph_frag_mask(_enc); }
    unsigned mask_shift() const { return ceph_frag_mask_shift(_enc); }
    
    operator _frag_t() const { return _enc; }
    
    // tests
    bool contains(unsigned v) const { return ceph_frag_contains_value(_enc, v); }
    bool contains(frag_t sub) const { return ceph_frag_contains_frag(_enc, sub._enc); }
    bool is_root() const { return bits() == 0; }
    frag_t parent() const {
        assert(bits() > 0);
        return frag_t(ceph_frag_parent(_enc));
    }
    
    // splitting
    frag_t make_child(int i, int nb) const {
        assert(i < (1<<nb));
        return frag_t(ceph_frag_make_child(_enc, nb, i));
    }
    void split(int nb, std::list<frag_t>& fragments) const {
        assert(nb > 0);
        unsigned nway = 1 << nb;
        for (unsigned i=0; i<nway; i++) 
            fragments.push_back(make_child(i, nb));
    }

    // binary splitting
    frag_t left_child() const { return frag_t(ceph_frag_left_child(_enc)); }
    frag_t right_child() const { return frag_t(ceph_frag_right_child(_enc)); }
    
    bool is_left() const { return ceph_frag_is_left_child(_enc); }
    bool is_right() const { return ceph_frag_is_right_child(_enc); }
    frag_t get_sibling() const {
        assert(!is_root());
        return frag_t(ceph_frag_sibling(_enc));
    }

    // sequencing
    bool is_leftmost() const { return ceph_frag_is_leftmost(_enc); }
    bool is_rightmost() const { return ceph_frag_is_rightmost(_enc); }
    frag_t next() const {
        assert(!is_rightmost());
        return frag_t(ceph_frag_next(_enc));
    }
    
    // parse
    bool parse(const char *s) {
        int pvalue, pbits;
        int r = sscanf(s, "%x/%d", &pvalue, &pbits);
        if (r == 2) {
            *this = frag_t(pvalue, pbits);
            return true;
        }
        return false;
    }
};
inline std::ostream& operator<<(std::ostream& out, const frag_t& hb)
{
  //out << std::hex << hb.value() << std::dec << "/" << hb.bits() << '=';
  unsigned num = hb.bits();
  if (num) {
    unsigned val = hb.value();
    for (unsigned bit = 23; num; num--, bit--) 
      out << ((val & (1<<bit)) ? '1':'0');
  }
  return out << '*';
}

int main(){
    A<int, int> a;
    B<A> b;
    B<> b2;
    a.print();
    b.print();
    b2.print();
    int t1, t2;
    t1 = t2 = 1;
    cout << t1 << t2 << endl;
    /*int cnt = 0;
    for (int i = 0;i<1000;i++){
        if (!(++cnt % 10)){
            cnt = 0;
            cout << " cnt: " << cnt << " i:" << i << endl;
        }
        }*/
    //test 2
    frag_t f = frag_t();
    printf( ".%08llx %i %i\n", (long long unsigned)f, f.value(), f.bits());
    cout << f << endl;
    list<frag_t> l;
    f.split(2,l);
    for (list<frag_t>::iterator it = l.begin(); it != l.end(); ++it){
        printf( ".%08llx %i %i\n", (long long unsigned)*it, (*it).value(), (*it).bits());
        cout << *it << endl;
    }
    list<frag_t> l2;
    frag_t tmp = *(++l.begin());
    tmp.split(3,l2);
    for (list<frag_t>::iterator it = l2.begin(); it != l2.end(); ++it){
        printf( ".%08llx %i %i\n", (long long unsigned)*it, (*it).value(), (*it).bits());
        cout << *it << endl;
    }

    cout << "my test" << endl;
    frag_t f_t2 = frag_t(56623104);
    printf( ".%08llx %i %i\n", (long long unsigned)f_t2, f_t2.value(), f_t2.bits());
    cout << f_t2 << endl;
    cout << f_t2.parent() << endl;
    cout << f_t2.parent().parent() << endl;
    cout << f_t2.parent().parent().parent() << endl;
    
    frag_t f2_t2 = frag_t(33554432);
    printf( ".%08llx %i %i\n", (long long unsigned)f2_t2, f2_t2.value(), f2_t2.bits());
    cout << f2_t2 << endl;
    cout << f2_t2.parent() << endl;
    cout << f2_t2.parent().parent() << endl;
    //uint32_t tmp = ceph_frag_make(3,0);
    /*
    uint32_t tmp = 0;
    uint32_t nb = 3;
    unsigned nway = 1 << nb;
    for (unsigned i=0; i<nway; i++) {
        tmp = ceph_frag_make_child(ceph_frag_make_child(0, 4, 1), nb, i);
        printf( ".%08llx %i %i\n", (long long unsigned)tmp, ceph_frag_value(tmp), ceph_frag_bits(tmp));
        }*/
    /*
    tmp = ceph_frag_make_child(0, 3, 0);
    printf( ".%08llx %i %i\n", (long long unsigned)tmp, ceph_frag_value(tmp), ceph_frag_bits(tmp));
    
    tmp = ceph_frag_make_child(0, 3, 1);
    printf( ".%08llx .%08llx .%08llx %i %i\n", (long long unsigned)tmp, ceph_frag_make2(3,1), ceph_frag_make3(3,1), ceph_frag_value(tmp), ceph_frag_bits(tmp));
    tmp = ceph_frag_make_child(0, 3, 2);
    printf( ".%08llx %i %i\n", (long long unsigned)tmp, ceph_frag_value(tmp), ceph_frag_bits(tmp));
    */
    return 0;
}

    
