#include "include/mempool.h"
#include <map>
#include <iostream>
#include <sstream>
using namespace std;

template <typename T, typename T2>
void EXPECT_EQ(const T &a , const T2 &b){
    stringstream ss;
    ss << a << " <--> " << b << "-----" << ((a==b)?"true":"false");
    if (a!=b)
        cout << ss.str() << endl;
}
template <typename T, typename T2>
void EXPECT_GE(const T &a , const T2 &b){
    stringstream ss;
    ss << a << " <--> " << b << "-----" <<( (a>=b)?"true":"false");
    if (a<b)
        cout << ss.str() << endl;
}
template <typename T, typename T2>
void EXPECT_NE(const T &a , const T2 &b){
    stringstream ss;
    //ss << a << " <--> " << b << "-----" <<( (a!=b)?"true":"false");
    ss <<( (a!=b)?"true":"false");
    if (a==b)
        cout << ss.str() << endl;
}
void check_usage(mempool::pool_index_t ix)
{
  mempool::pool_t *pool = &mempool::get_pool(ix);
  mempool::stats_t total;
  map<std::string,mempool::stats_t> m;
  pool->get_stats(&total, &m);
  size_t usage = pool->allocated_bytes();
  size_t u_items = pool->allocated_items();
  size_t sum = 0;
  for (auto& p : m) {
      std::cout << "check_usage detail " << p.first << "->" << p.second.bytes << std::endl;
      sum += p.second.bytes;
  }
  if (sum != usage) {
      //ceph::TableFormatter jf;
      //pool->dump(jf);
    //jf.flush(std::cout);
    //cout << jf.str();
      std::cout << "check_usage error" << std::endl;
  }
  else{
      std::cout << "check_usage ok(" << sum << "/" << usage<< " | " << u_items<< ")"<<std::endl;
  }
      
  EXPECT_EQ(sum, usage);
}

template<typename A, typename B>
void eq_elements(const A& a, const B& b)
{
  auto lhs = a.begin();
  auto rhs = b.begin();
  while (lhs != a.end()) {
    EXPECT_EQ(*lhs,*rhs);
    lhs++;
    rhs++;
  }
  if (rhs == b.end()){
      //std::cout << "eq_elements ok" << std::endl;
  }
  else
      std::cout << "eq_elements error" << std::endl;
  //EXPECT_EQ(rhs,b.end());
}

template<typename A, typename B>
void eq_pairs(const A& a, const B& b)
{
  auto lhs = a.begin();
  auto rhs = b.begin();
  while (lhs != a.end()) {
    EXPECT_EQ(lhs->first,rhs->first);
    EXPECT_EQ(lhs->second,rhs->second);
    lhs++;
    rhs++;
  }
  if (rhs == b.end()){
      //std::cout << "eq_pairs ok" << std::endl;
  }
  else
      std::cout << "eq_pairs error" << std::endl;
  //EXPECT_EQ(rhs,b.end());
}

#define MAKE_INSERTER(inserter)				\
  template<typename A,typename B>			\
void do_##inserter(A& a, B& b, int count, int base) {	\
  for (int i = 0; i < count; ++i) {			\
    a.inserter(base + i);				\
    b.inserter(base + i);				\
  }							\
}

MAKE_INSERTER(push_back);
MAKE_INSERTER(insert);

template<typename A,typename B>
void do_insert_key(A& a, B& b, int count, int base)
{
  for (int i = 0; i < count; ++i) {
    a.insert(make_pair(base+i,base+i));
    b.insert(make_pair(base+i,base+i));
    check_usage(mempool::osd::id);
  }
}

void list_context()
{
  for (unsigned i = 1; i < 10; ++i) {
      cout << i << "------" << endl;
    list<int> a;
    mempool::osd::list<int> b,c;
    eq_elements(a,b);
    do_push_back(a,b,i,i);
    eq_elements(a,b);
    c.swap(b);
    eq_elements(a,c);
    a.erase(a.begin());
    c.erase(c.begin());
    eq_elements(a,c);
    a.clear();
    b.clear();
    c.clear();
    do_push_back(a,b,i,i);
    c.splice(c.begin(),b,b.begin(),b.end());

    mempool::stats_t total;
    map<std::string,mempool::stats_t> by_type;
    mempool::get_pool(mempool::osd::id).get_stats(&total, &by_type);
    EXPECT_GE(mempool::osd::allocated_bytes(), i * 4u);
    EXPECT_EQ(mempool::osd::allocated_items(), i);

    eq_elements(a,c);
    check_usage(mempool::osd::id);
    a.clear();
    b.clear();
    c.clear();
  }
}

void vector_context(){
    check_usage(mempool::osd::id);
    EXPECT_EQ(mempool::osd::allocated_bytes(), 0u);
    EXPECT_EQ(mempool::osd::allocated_items(), 0u);
    for (unsigned i = 0; i < 10; ++i) {
        cout << i << "----------" << sizeof(int)<< endl;
        vector<int> a;
        mempool::osd::vector<int> b,c;
        eq_elements(a,b);
        do_push_back(a,b,i,i);
        eq_elements(a,b);
        check_usage(mempool::osd::id);

        mempool::stats_t total;
        map<std::string,mempool::stats_t> by_type;
        mempool::get_pool(mempool::osd::id).get_stats(&total, &by_type);
        EXPECT_GE(mempool::osd::allocated_bytes(), i * 4u);
        EXPECT_GE(mempool::osd::allocated_items(), i);
        
        c.swap(b);
        eq_elements(a,c);
        check_usage(mempool::osd::id);
        a.clear();
        b.clear();
        c.clear();
    }
}

void set_context(){
  for (int i = 0; i < 10; ++i) {
    set<int> a;
    mempool::osd::set<int> b;
    do_insert(a,b,i,i);
    eq_elements(a,b);
    check_usage(mempool::osd::id);
  }

  for (int i = 1; i < 10; ++i) {
    set<int> a;
    mempool::osd::set<int> b;
    do_insert(a,b,i,0);
    EXPECT_NE(a.find(i/2),a.end());
    EXPECT_NE(b.find(i/2),b.end());
    a.erase(a.find(i/2));
    b.erase(b.find(i/2));
    eq_elements(a,b);
    check_usage(mempool::osd::id);
  }
}

//
struct obj {
  MEMPOOL_CLASS_HELPERS();
  int a;
  int b;
  obj() : a(1), b(1) {}
  explicit obj(int _a) : a(_a), b(2) {}
  obj(int _a,int _b) : a(_a), b(_b) {}
  friend inline bool operator<(const obj& l, const obj& r) {
    return l.a < r.a;
  }
};
MEMPOOL_DEFINE_OBJECT_FACTORY(obj, obj, osdmap);
void test_factory(){
   obj *o1 = new obj();
   obj *o2 = new obj(10);
   obj *o3 = new obj(20,30);
   check_usage(mempool::osdmap::id);
   EXPECT_NE(o1,nullptr);
   EXPECT_EQ(o1->a,1);
   EXPECT_EQ(o1->b,1);
   EXPECT_EQ(o2->a,10);
   EXPECT_EQ(o2->b,2);
   EXPECT_EQ(o3->a,20);
   EXPECT_EQ(o3->b,30);

   delete o1;
   delete o2;
   delete o3;
   check_usage(mempool::osdmap::id);
}
//
void test_vector(){
    {
        mempool::osd::vector<int> v;
        v.push_back(1);
        v.push_back(2);
        check_usage(mempool::osd::id);
    }
    {
        mempool::osdmap::vector<obj> v;
        v.push_back(obj());
        v.push_back(obj(1));
        check_usage(mempool::osdmap::id);
    }
    check_usage(mempool::osd::id);
    check_usage(mempool::osdmap::id);
}
void testset()
{
    mempool::osd::set<int> set_int;
    set_int.insert(1);
    set_int.insert(2);
    mempool::osdmap::set<obj> set_obj;
    set_obj.insert(obj());
    set_obj.insert(obj(1));
    set_obj.insert(obj(1, 2));
    check_usage(mempool::osd::id);
    check_usage(mempool::osdmap::id);
}

void test_map()
{
    {
        mempool::osd::map<int,int> v;
        v[1] = 2;
        v[3] = 4;
        check_usage(mempool::osd::id);
    }
    {
        mempool::osdmap::map<int,obj> v;
        v[1] = obj();
        v[2] = obj(2);
        v[3] = obj(2, 3);
        check_usage(mempool::osdmap::id);
    }
}

void test_list()
{
    {
        mempool::osd::list<int> v;
        v.push_back(1);
        v.push_back(2);
        check_usage(mempool::osd::id);                
    }
    {
        mempool::osdmap::list<obj> v;
        v.push_back(obj());
        v.push_back(obj(1));
        check_usage(mempool::osdmap::id);       
    }
    
}
void test_unordered_map()
{
    mempool::osdmap::unordered_map<int,obj> h;
    h[1] = obj();
    h[2] = obj(1);
    check_usage(mempool::osdmap::id);
}

void string_test()
{
    mempool::osdmap::string s;
    s.reserve(100);
    EXPECT_GE(mempool::osdmap::allocated_items(), s.capacity() + 1u); // +1 for zero-byte termination :
    for (size_t i = 0; i < 10; ++i) {
        s += '1';
        s.append(s);
        EXPECT_GE(mempool::osdmap::allocated_items(), s.capacity() + 1u);
    }
    check_usage(mempool::osdmap::id);
}

struct i_type {
  MEMPOOL_CLASS_HELPERS();
  int a;
  int b;
    int c;
  i_type() : a(0) {}
  explicit i_type(int _a) : a(_a) {}
    //  i_type(int _a) : a(_a) {}
  friend inline bool operator<(const i_type& l, const i_type& r) {
    return l.a < r.a;
  }
};
MEMPOOL_DEFINE_OBJECT_FACTORY(i_type, i_type, osd);

void type_size(){
    i_type *o1 = new i_type();
    i_type *o2 = new i_type(10);
    i_type *o3 = new i_type(20);
    check_usage(mempool::osd::id);
    delete o1;
    delete o2;
    delete o3;
    return;
}
void vector_obj_size(){
    mempool::osd::vector<i_type> a;
    check_usage(mempool::osd::id);
    for (int i = 1; i < 10; i++){
        cout << "i = " << i << endl;
        a.push_back((i_type)i);
        check_usage(mempool::osd::id);
    }
    return;
}
void list_obj_size(){
    mempool::osd::list<i_type> a;
    check_usage(mempool::osd::id);
    for (int i = 1; i < 10; i++){
        cout << "i = " << i << endl;
        a.push_back((i_type)i);
        check_usage(mempool::osd::id);
    }
    return;
}
void set_obj_size(){
    mempool::osd::set<i_type> a;
    check_usage(mempool::osd::id);
    for (int i = 1; i < 10; i++){
        cout << "i = " << i << endl;
        a.insert((i_type)i);
        check_usage(mempool::osd::id);
    }
    return;
}
void map_obj_size(){
    mempool::osd::map<i_type,i_type> a;
    check_usage(mempool::osd::id);
    for (int i = 1; i < 10; i++){
        cout << "i = " << i << endl;
        a[(i_type)i] = (i_type)i;
        check_usage(mempool::osd::id);
    }
    return;
}
int main()
{
    mempool::set_debug_mode(true);
    cout << "vector_context" << endl;
    vector_context();
    cout << "list_context" << endl;
    list_context();
    cout << "set_context" << endl;
    set_context();
    cout << "set_factory" << endl;
    test_factory();
    cout << "--i_type--" << endl;
    type_size();
    cout << "--vector--" << endl;
    vector_obj_size();
    cout << "--list--" << endl;
    list_obj_size();
    cout << "--set--" << endl;
    set_obj_size();
    cout << "--map--" << endl;
    map_obj_size();
    return 0;
}
