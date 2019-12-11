#include <stdio.h>
#include <iostream>
#include <list>
#include <../stime.h>
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
typedef unsigned int		uint32_t;
#include "ceph_frag.h"
#include "frag.h"
/*

typedef uint32_t            _frag_t;
#define assert(t) if(t == 0) {cout << "assert failed : " << __func__<< endl;}
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
*/
/*
bool CInode::get_dirfrags_under(frag_t fg, list<CDir*>& ls)
{
  bool all = true;
  std::list<frag_t> fglist;
  dirfragtree.get_leaves_under(fg, fglist);
  for (list<frag_t>::iterator p = fglist.begin(); p != fglist.end(); ++p)
    if (dirfrags.count(*p))
      ls.push_back(dirfrags[*p]);
    else 
      all = false;

  if (all)
    return all;

  fragtree_t tmpdft;
  tmpdft.force_to_leaf(g_ceph_context, fg);
  for (auto &p : dirfrags) {
    tmpdft.force_to_leaf(g_ceph_context, p.first);
    if (fg.contains(p.first) && !dirfragtree.is_leaf(p.first))
      ls.push_back(p.second);
  }

  all = true;
  tmpdft.get_leaves_under(fg, fglist);
  for (const auto &p : fglist) {
    if (!dirfrags.count(p)) {
      all = false;
      break;
    }
  }

  return all;
}
 */
class frag_adv_test{
public:
    frag_adv_test():frag_cnt(1){
    }
    ~frag_adv_test(){
    }
    void init(int64_t cnt, int split_bit, bool make_complete = true, bool balance = true){
        std::list<frag_t> fglist;
        while (frag_cnt < cnt){
            fglist.clear();
            fragtree.get_leaves_under(frag_t(), fglist);
            auto it = fglist.begin();
            if ((frag_cnt << 3) > cnt && !balance){
              cout << "skip split for "<< *it << " " << it->value() << " " << it->bits() << endl;
              ++it;
            }
            for (; it != fglist.end();++it){
                // cout << *it << " " << it->value() << " " << it->bits() << endl;
                fragtree.split(*it, 3);
            }
            frag_cnt = frag_cnt << 3;
            cout << "current frag_cnt :" << frag_cnt << endl;
        }
        auto it = fglist.begin();
        if (!make_complete){
            cout << "skip "<< *it << " " << it->value() << " " << it->bits() << endl;
            ++it;
            cout << "skip "<< *it << " " << it->value() << " " << it->bits() << endl;
            ++it;            
        }

        std::list<frag_t> fglist_leaves;
        if (make_complete){
            fragtree.get_leaves_under(frag_t(), fglist_leaves);            
        }else{
            for (; it != fglist.end();++it){
                fragtree.get_leaves_under(*it, fglist_leaves);    
            }
        }
        for (auto &p : fglist_leaves)
            dirfrags.insert(p);
        cout << "show all leaves : " << dirfrags.size() << endl;
        //for (auto it = dirfrags.begin(); it != dirfrags.end();++it){
        //    cout << *it << " " << it->value() << " " << it->bits() << endl;
        //}
    }
    bool get_dirfrags_under_old(frag_t fg, list<frag_t>& ls){
        STATISTICSTIMECOUTDEFSINGLETHREAD
        bool all = true;
        std::list<frag_t> fglist;
        fragtree.get_leaves_under(fg, fglist);
        for (list<frag_t>::iterator p = fglist.begin(); p != fglist.end(); ++p)
            if (dirfrags.count(*p))
                //ls.push_back(dirfrags[*p]);
                ls.push_back(*p);
            else 
                all = false;
        
        if (all)
            return all;
        
        fragtree_t tmpdft;
        tmpdft.force_to_leaf(fg);
        for (auto &p : dirfrags) {
            tmpdft.force_to_leaf(p);
            if (fg.contains(p) && !fragtree.is_leaf(p))
                ls.push_back(p);
        }
        
        all = true;
        tmpdft.get_leaves_under(fg, fglist);
        for (const auto &p : fglist) {
            if (!dirfrags.count(p)) {
                all = false;
                break;
            }
        }
        
        return all;
    }
    bool get_dirfrags_under(frag_t fg, list<frag_t>& ls){
        STATISTICSTIMECOUTDEFSINGLETHREAD
        bool all = false;
        if (dirfrags.count(fg)){
            all = true;
            ls.push_back(fg);
            return all;
        }
        uint64_t total = 0;
        for (auto &p : dirfrags){
            //cout << p << endl;
            //if (fg == p){
            //    cout << fg << " = " << p << endl;
            //    ls.push_back(p);
            //    all = true;
            //    break;
            //}
            //else
            if (fg.bits() < p.bits() && fg.contains(p)){
                // cout << fg << " have " << p << endl;
                ls.push_back(p);
                total += 1<<(24-p.bits());
            }
            else if (fg.bits() > p.bits() && p.contains(fg)){
                all = true;
                cout << fg << " belong " << p << endl;
                break;
            }
        }
        if(!all &&
           (1<<(24-fg.bits())) == total)
            all = true;
        // all is not necessary expect when merge dir , so we cannot calc it every time
        /*
        if (!all && ls.size() > 0){
            std::list<frag_t> leaves;
            fragtree.get_leaves_under(fg, leaves);
            uint64_t size = leaves.size();
            leaves.clear();
            for(auto &p : ls){
                fragtree.get_leaves_under(p, leaves);
            }
            if (size == leaves.size())
                all = true;
            else if (size < leaves.size())
                cout << __func__ << " error occurred (" << fg << ") leaves : "
                        << size << "|" << leaves.size() << endl;    
                        }*/
        return all;
    }
    void fragset_t_simplify_test(){
        STATISTICSTIMECOUTDEFSINGLETHREAD
        fragset_t tmp;
        std::set<frag_t>::iterator p = dirfrags.begin();
        while (p != dirfrags.end()) {
            // for (auto &p : dirfrags){
            tmp.insert(*p);
            dirfrags.erase(p++);
            if (p != dirfrags.end())
                ++p;
        }
        p = dirfrags.begin();
        while (p != dirfrags.end()) {
            // for (auto &p : dirfrags){
            tmp.insert(*p);
            dirfrags.erase(p++);
        }
        cout << "after simplify : " << tmp.get().size() << endl;
        for (auto &it : tmp.get()){
            cout << it << " ";
        }
    }
    void fragset_t_simplify_test_adv(){
        STATISTICSTIMECOUTDEFSINGLETHREAD
        fragset_t tmp;
        std::set<frag_t>::iterator p = dirfrags.begin();
        while (p != dirfrags.end()) {
            // for (auto &p : dirfrags){
            tmp.insert_raw(*p);
            dirfrags.erase(p++);
        }
        /*
        for (auto &it : tmp.get()){
            cout << it << " ";
            }*/
        cout << endl;
        tmp.simplify_adv();
        cout << "after simplify : " << tmp.get().size() << endl;
        for (auto &it : tmp.get()){
            cout << it << " ";
        }
        cout << endl;
    }
private:
    fragtree_t fragtree;
    int64_t frag_cnt;
    set<frag_t> dirfrags;
};

void frag_adv_v2(){
    frag_adv_test ft;
    ft.init(500, 3, true, false);
    ft.fragset_t_simplify_test();
    frag_adv_test ft_adv;
    ft_adv.init(500, 3, true, false);
    ft_adv.fragset_t_simplify_test_adv();
    // 32768 173994322 -> 63933us  2721
    // 4096  2397699 -> 7545us     317
    // 512   34037 ->  800         42
    // 
}
void frag_adv_v1(){
    frag_adv_test ft;
    ft.init(2000, 3, false);
    list<frag_t> case_list;
    case_list.push_back(frag_t(16773120, 12));// case 1 111111111111* 16773120 12     // last leaves
    case_list.push_back(frag_t(16744448, 9));// case 2 111111111* 16744448 9         // parent of last leaves
    case_list.push_back(frag_t(16515072, 6));// case 3 111111* 16515072 6
    case_list.push_back(frag_t(0, 12));      // case 4 000000000000* 0 12            // no exist
    case_list.push_back(frag_t(4096, 12));   // case 5 000000000001* 4096 12         // no exist
    case_list.push_back(frag_t());           // case 6 root
    case_list.push_back(frag_t(0, 6));       // case 7 
    
    
    list<frag_t> ls;
    bool ret = false;
    frag_t case_cur = frag_t();
    for (auto &p : case_list){
        case_cur = p;ls.clear();
        ret = ft.get_dirfrags_under(case_cur, ls);
        cout << case_cur << " ret : " << ret << " size : " << ls.size() << endl;
    }
    for (auto &p : case_list){
        case_cur = p;ls.clear();
        ret = ft.get_dirfrags_under_old(case_cur, ls);
        cout << case_cur << " ret : " << ret << " size : " << ls.size() << endl;
    }
}
void frag_adv(){
    cout << "--frag_adv--" << endl;
    list<frag_t> dirfrags;
    // create fragtree
    fragtree_t fragtree;
    frag_t root = frag_t();
    fragtree.split(root, 3);
    std::list<frag_t> fglist_lv1;
    fragtree.get_leaves_under(root, fglist_lv1);
    auto it = fglist_lv1.begin();
    ++it;
    for (; it != fglist_lv1.end();++it){
        cout << *it << endl;
        fragtree.split(*it, 3);
    }
    fragtree.get_leaves_under(root, dirfrags);
    cout << "--frag_check--" << endl;
    frag_t cf = frag_t(262144, 6);  // 000001* 262144 6
    frag_t cf2 = frag_t(2097152, 6);// 001000* 2097152 6
    frag_t cf3 = frag_t(0, 3);      // 000* 0 3
    frag_t cf4 = frag_t(2097152, 3);// 001* 2097152 3
    frag_t inf = cf4;
    for (auto &p : dirfrags){
        cout << p << endl;
        if (inf == p){
            cout << inf << " = " << p << endl;
            break;
        }
        else if (inf.bits() < p.bits() && inf.contains(p))
            cout << inf << " have " << p << endl;
        else if (inf.bits() > p.bits() && p.contains(inf)){
            cout << inf << " belong " << p << endl;
            break;
        }
    }
}
int frag_test() {
    cout << "start func : " << __func__ << endl;
    fragtree_t fragtree;
    frag_t root = frag_t();
    cout << "=>split from leaf : " << root << endl;
    fragtree.split(root, 3);   
    // fragtree.split(root, 3);  // error, root must be leaf
    cout << fragtree << endl;
    // test diri->get_dirfrags_under(basefrag, srcfrags);   // get dir under the frag
    //      |__ dirfragtree.get_leaves_under(fg, fglist);
    cout << "=>split from all leaves " << endl;
    std::list<frag_t> fglist_lv1;
    fragtree.get_leaves_under(root, fglist_lv1);
    frag_t lv1_1 = *(fglist_lv1.begin());
    for (auto it = fglist_lv1.begin(); it != fglist_lv1.end();++it){
        cout << "spliting " << *it << endl;
        fragtree.split(*it, 3);
    }
    cout << "=>current tree : " << fragtree << endl;
    cout << "=>get leaves of " << lv1_1 << endl;
    std::list<frag_t> fglist_lv2;
    fragtree.get_leaves_under(lv1_1, fglist_lv2);
    //fragtree.get_leaves_under(frag_t(2097152,3), fglist_lv2); 
    frag_t lv2_1 = *fglist_lv2.begin();
    for (auto it = fglist_lv2.begin(); it != fglist_lv2.end();++it){
        cout << *it << " " << it->value() << " " << it->bits()<< endl;
    }
    
    cout << "=>get leaves of " << lv2_1<< endl;
    std::list<frag_t> fglist_lv3;
    fragtree.get_leaves_under(lv2_1, fglist_lv3);
    for (auto it = fglist_lv3.begin(); it != fglist_lv3.end();++it){
        cout << *it << " " << it->value() << " " << it->bits()<< endl;
    }

    // importent opt
    cout << "=>test operator []" << frag_t(2097152,3) << " ==> " << fragtree[frag_t(2097152,3).value()] << endl;
    cout << "=>test operator []" << frag_t() << " ==> " << fragtree[frag_t().value()] << endl;
    cout << "=>test operator []" << frag_t(0,12) << " ==> " << fragtree[frag_t(0,12).value()] << endl;
    cout << "=>test get_sibling" << frag_t(2097152,3) << " ==> " << frag_t(2097152,3).get_sibling() << " " << frag_t(2097152,3).parent() << endl;
    //cout << "=>test get_sibling" << frag_t() << " ==> " << frag_t().get_sibling() << endl;
    cout << "=>test get_sibling" << frag_t(0,12) << " ==> " << frag_t(0,12).get_sibling() << " " << frag_t(0,12).parent() << endl;
    cout << "=>test get_sibling" << frag_t(4096,12) << " ==> " << frag_t(4096,12).get_sibling() << " " << frag_t(4096,12).parent() << endl;
    // will get all leaf node
    // cout << "get leaf : " << root<< endl;
    // std::list<frag_t> fglist_all_ori;
    // fragtree.get_leaves_under(root, fglist_all_ori);
    // for (auto it = fglist_all_ori.begin(); it != fglist_all_ori.end();++it){
    //    cout << *it << endl;
    // }
    cout << "=>tmptree force_to_leaf : " << lv1_1<< endl;
    fragtree_t tmpdft;
    tmpdft.force_to_leaf(lv1_1);
    cout << tmpdft << endl;
    std::list<frag_t> fglist_all;
    tmpdft.get_leaves_under(frag_t(), fglist_all);
    for (auto it = fglist_all.begin(); it != fglist_all.end();++it)
        cout << *it << " " << it->value() << " " << it->bits()<< endl;
    fglist_all.clear();
    cout << "=>tmptree and then force_to_leaf : " << lv2_1 << "," << frag_t(262144,6) << "," << frag_t(2097152,6) << endl;
    tmpdft.force_to_leaf(lv2_1);
    tmpdft.force_to_leaf(frag_t(262144,6));
    tmpdft.force_to_leaf(frag_t(2097152,6));
    cout << tmpdft << " " << fglist_all.size() << endl;
    tmpdft.get_leaves_under(frag_t(), fglist_all);
    for (auto it = fglist_all.begin(); it != fglist_all.end();++it)
        cout << *it << " " << it->value() << " " << it->bits()<< endl;

    cout << "=>frag contain test " << lv2_1<< endl;
    if (lv1_1.contains(lv2_1))
        cout << " 1v1 have 2_1" << endl;
    return 0;
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
    // test null
    cout << "---- test null" << endl;
    frag_t f = frag_t();
    printf( ".%08llx %i %i\n", (long long unsigned)f, f.value(), f.bits());
    frag_t f2 = frag_t(2,3);
    printf( ".%08llx %i %i\n", (long long unsigned)f2, f2.value(), f2.bits());
    cout << f << endl;
    // test splite 2
    cout << "---- test split 2 for " << f << endl;
    list<frag_t> l;
    f.split(2,l);
    for (list<frag_t>::iterator it = l.begin(); it != l.end(); ++it){
        printf( ".%08llx %i %i\n", (long long unsigned)*it, (*it).value(), (*it).bits());
        cout << *it << endl;
    }
    // test splite 3
    frag_t tmp = *(++l.begin());
    cout << "test split 3 for " << tmp << endl;
    list<frag_t> l2;
    tmp.split(3,l2);
    for (list<frag_t>::iterator it = l2.begin(); it != l2.end(); ++it){
        printf( ".%08llx %i %i\n", (long long unsigned)*it, (*it).value(), (*it).bits());
        cout << *it << endl;
    }

    // test get parent
    cout << "---- test get parent for sp. frag" << endl;
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
    
    frag_test();
    //2frag_adv();
    //frag_adv_v1();
    frag_adv_v2();
    return 0;
}

    
