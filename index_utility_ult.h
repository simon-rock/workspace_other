#ifndef _INDEX_UTILITY_H_
#define _INDEX_UTILITY_H_
//1. firstly we divided a degree into 50 parts.So we divided all the world into about 360*50*180*50 parts.
//2. map all polygons to this world and get the grids each graph represented.
//3. Those polygons which don't have same grid will been remove from the group.
//4. Finally, we get some groups include ids of polygon which are close to first polygon in this group.
//The count of groups is equal to the number of polygons.(mostly less, because there are some polygons which are far from all other polygons )

#define COUNT_P_DEG 50      // divided a degree into COUNT_P_DEG parts, determine degree of precision
#define COUNT_P_DEG_M 20
#define Y_MAX 90            
#define Y_MIN -90           
#define X_MAX 180           
#define X_MIN -180          
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <vector>
#include <set>
#include <map>
using std::map;
using std::vector;
using std::set;
using boost::unordered_set;
template<typename U = int>
struct TT
{
    void print_test_normal(int i){std::cout << " normal print_test --" << i << std::endl;}
    template<typename T>
    static int calculate_index(T x, T y){std::cout << "calculate_index --" << x << std::endl;return 0;}
    template<typename T>
    static void print_test(int i)
    {
        T t=T();
        //int i = t.ISUP;
        //std::cout << "print_test --" << t << std::endl;
    }
    U _v;
};

namespace index_utility
{
    typedef unsigned int index_type;
    #define invalid index_type(-1)

    //template<unsigned int C, unsigned int XMAX, unsigned int XMIN, unsigned int YMAX, unsigned int YMIN>
    template<unsigned int C, int XMAX, int XMIN, int YMAX, int YMIN>
    class _constant
    {
    public:
        enum
        {
            XMAXCOUNT = (XMAX-XMIN)*C + 1,
            XREVISE = 0 - XMIN,        //  correct coordinate
            YREVISE = 0 - YMIN,        //  correct coordinate
            YCOUNT = C*((YMAX-XMIN)*C + 1),
            TOTAL = (YMAX + YREVISE)*C*XMAXCOUNT + (XMAX + XREVISE)*C,
            MCOUNT_P_DEG = C,
            MXMAX = XMAX,
            MXMIN = XMIN,
            MYMAX = YMAX,
            MYMIN = YMIN
        };
        
    };
    typedef _constant<COUNT_P_DEG, X_MAX, X_MIN, Y_MAX, Y_MIN> Global_L;
    typedef _constant<COUNT_P_DEG_M, X_MAX, X_MIN, Y_MAX, Y_MIN> Global_M;

    template<typename Global = Global_L>
    struct calculate_utility
    {
        // map coordinate to index of world
        // warning :  X_MIN < x < X_MAX, Y_MIN < y < Y_MAX
        template<typename T>
        static index_type calculate_index(T x, T y)
        {   
            // check point 
            //assert(Global.MXMIN < x);
            //assert(x < Global.MXMAX);
            //assert(Global.MYMIN < y);
            //assert(y < Global.MYMAX);
            //if (y == Global.MYMAX || x == Global.MXMAX)
            //{
            //    T ny = y == Global.MYMAX? Global.MYMIN:y;
            //    T nx = x == Global.MXMAX? Global.MXMIN:x;
            //    return index_type((ny + G.YREVISE)*COUNT_P_DEG)*G.XMAXCOUNT + index_type((nx + G.XREVISE)*COUNT_P_DEG);
            //}
            return index_type((y + Global::YREVISE)*Global::MCOUNT_P_DEG)*Global::XMAXCOUNT + index_type((x + Global::XREVISE)*Global::MCOUNT_P_DEG);
        }

        static index_type get_x(index_type i)
        {
            return i%Global::XMAXCOUNT;
        }
        static index_type get_y(index_type i)
        {
            return i/Global::XMAXCOUNT;
        }
        static index_type make_index(index_type x, index_type y)
        {
            return y*Global::XMAXCOUNT + x;
        }
        // rect in the world
        //      |--------_end
        //      |           |
        //  _begin----------|
        class index_rect
        {
        public:
            template<typename F, typename S, int B>
            struct side
            {
                typedef F forder;
                typedef S sorder;
                enum{
                    ISUP = B
                };
            };

            index_rect(index_type begin, index_type end)
            {
                _begin_x = get_x(begin);
                _begin_y = get_y(begin);
                _end_x = get_x(end);
                _end_y = get_y(end);
                _begin = make_index(_begin_x<_end_x?_begin_x:_end_x, _begin_y<_end_y?_begin_y:_end_y);
                _end = make_index(_begin_x>_end_x?_begin_x:_end_x, _begin_y>_end_y?_begin_y:_end_y);
                _begin_x = get_x(_begin);
                _begin_y = get_y(_begin);
                _end_x = get_x(_end);
                _end_y = get_y(_end);
            }
            template<typename T >
            index_rect(T xl, T yl, T xh, T yh)
            {
                _begin = calculate_index(xl<xh?xl:xh, yl<yh?yl:yh);
                _end = calculate_index(xl>xh?xl:xh, yl>yh?yl:yh);
                _begin_x = get_x(_begin);
                _begin_y = get_y(_begin);
                _end_x = get_x(_end);
                _end_y = get_y(_end);
            }
            // line traverse
            class line
            {
            public:
                line(index_rect* p):_p(p){}
                int add(index_type& curr)
                {
                    if(_p == NULL) return -1;
                    if (get_x(curr) < _p->_end_x)
                    {
                        ++curr;
                    }
                    else curr = invalid;
                    return 0;
                }
                int sub(index_type& curr)
                {
                    if(_p == NULL) return -1;
                    if (get_x(curr) > _p->_begin_x)
                    {
                        --curr;
                    }
                    else curr = invalid;
                    return 0;
                }
                index_rect* _p;
            };
            // column traverse
            class column
            {
            public:
                column(index_rect* p):_p(p){}
                int add(index_type& curr)
                {
                    if(_p == NULL) return -1;
                    if(get_y(curr) < _p->_end_y)
                    {
                        curr = (get_y(curr)+1)*Global::XMAXCOUNT + get_x(curr);
                    }
                    else curr = invalid;
                    return 0;
                }
                int sub(index_type& curr)
                {
                    if(_p == NULL) return -1;
                    if(get_y(curr) > _p->_begin_y)
                    {
                        curr = (get_y(curr)-1)*Global::XMAXCOUNT + get_x(curr);
                    }
                    else curr = invalid;
                    return 0;
                }
                index_rect* _p;
            };
            class all
            {
            public:
                all(index_rect* p):_p(p){}
                int add(index_type& curr)
                {
                    if(_p == NULL) return -1;
                    if (get_x(curr) < _p->_end_x)
                    {
                        ++curr;
                    }
                    else if(get_y(curr) < _p->_end_y)
                    {
                        curr = (get_y(curr)+1)*Global::XMAXCOUNT + _p->_begin_x;
                    }
                    else curr = invalid;
                    return 0;
                }
                int sub(index_type& curr)
                {
                    if(_p == NULL) return -1;
                    if (get_x(curr) > _p->_begin_x)
                    {
                        --curr;
                    }
                    else if(get_y(curr) > _p->_begin_y)
                    {
                        curr = (get_y(curr)-1)*Global::XMAXCOUNT + _p->_end_x;
                    }
                    else curr = invalid;
                    return 0;
                }
                index_rect* _p;
            };
            template<typename T>
            class iterator
            {
            public:
                iterator(index_rect* p = NULL, bool first = true):type(p), curr(invalid)
                {
                    if (p != NULL)
                        if(first)
                            curr = p->_begin;
                        else
                            curr = p->_end;
                }
                template<typename U>
                iterator(iterator<U>& c):type(NULL), curr(invalid)
                {
                    curr = c.get();
                    type._p = c.get_op()._p;
                }
                template<typename U>
                iterator operator = (iterator<U>& c){type._p = c.get_op()._p; curr = c.get(); return *this;}
                iterator operator ++ (int)
                {
                    iterator tmp = *this;
                    type.add(curr);
                    return tmp;
                }
                iterator operator ++ ()
                {
                    type.add(curr);
                    return *this;
                }
                iterator operator -- (int)
                {
                    iterator tmp = *this;
                    type.sub(curr);
                    return tmp;
                }
                iterator operator -- ()
                {
                    type.sub(curr);
                    return *this;
                }
                index_type operator* ()
                {
                    return curr;
                }
                bool operator == (iterator&r)
                {
                    return curr == r.curr;
                }
                bool operator != (iterator&r)
                {
                    return !(*this==r);
                }
                index_type get(){return curr;}
                T get_op(){return type;}
            private:
                T type;
                index_type curr;
            };
            template<typename T>
            iterator<T> begin_impl()
            {
                iterator<T> item(this);
                return item;
            }
            template<typename T>
            iterator<T> last_impl()
            {
                iterator<T> item(this, false);
                return item;
            }
            template<typename T>
            iterator<T> end_impl(){return iterator<T>();}

            iterator<all> begin(){return begin_impl<all>();}
            iterator<all> end(){return end_impl<all>();}

            typedef side<column, line, 1> left;
            typedef side<line, column, 1> bottom;
            typedef side<column, line, 0> rigth;
            typedef side<line, column, 0> top;
        private:
            index_rect(){}
            index_type _begin;
            index_type _end;
            index_type _end_x;
            index_type _end_y;
            index_type _begin_x;
            index_type _begin_y;
        };

        template<typename T>
        int get_polygon_index_by_side(index_rect& r, unordered_set<index_type> &l, unordered_set<index_type> &p)
        {
            typename index_rect::template iterator<typename T::forder> fend = r.template end_impl<typename T::forder>();
            for (typename index_rect::template iterator<typename T::forder> item_f = T::ISUP?r.template begin_impl<typename T::forder>():r.template last_impl<typename T::forder>();
            //    item_f != r.end_impl<T::forder>();
                item_f != fend;
                T::ISUP?++item_f:--item_f)
            {
                typename index_rect::template iterator<typename T::sorder> send = r.template end_impl<typename T::sorder>();
                for (typename index_rect::template iterator<typename T::sorder> item_s = item_f;
                    //item_s != r.end_impl<T::sorder>();
                    item_s != send;
                    T::ISUP?++item_s:--item_s)
                {
                    if (l.find(*item_s) != l.end())
                        break;
                    p.insert(*item_s);
                }
            }
            return 0;
        }

        // check continuity of two block
        bool check_continuity(index_type p, index_type n)
        {
            if (p == invalid || n == invalid || p == n)
                return true;
            index_type ret = n > p?n-p:p-n;
            if (ret == 1 || 
                ret == Global::XMAXCOUNT)
                return true;
            else
                return false;
        }
        // if no continuity, will add blocks
        bool add_block(index_type p, index_type n, unordered_set<index_type>& adds)
        {
            if (!check_continuity(p, n))
            {   
                index_type x1 = get_x(p);
                index_type x2 = get_x(n);
                index_type y1 = get_y(p);
                index_type y2 = get_y(n);
                index_rect r(make_index(x1<x2?x1:x2, y1<y2?y1:y2),
                    make_index(x1>x2?x1:x2, y1>y2?y1:y2));
                typename calculate_utility<Global>::index_rect::template iterator<typename calculate_utility<Global>::index_rect::all> item_end = r.end();
                for (typename calculate_utility<Global>::index_rect::template iterator<typename calculate_utility<Global>::index_rect::all> item = r.begin();
                    //item != r.end();
                    item != item_end;
                    ++item)
                    adds.insert(*item);
                return true;
            }
            else return false;
        }

        // map a point list to world and confirm the continuity of output, if not, add some blocks into the polygon
        //  -------           -------
        //  |1 |  |           |1 | 2|
        //  |  | 4|     ==>   |3 | 4|
        //  -------           -------
        // p        :   point list of the polygon
        // l        :   all block indexes of the polygon in the world
        template<typename T>
        void get_index(const vector<T>& p, unordered_set<index_type> &l)
        {
            if (p.size() == 0) return;
            index_type first = calculate_index(p[0].X(), p[0].Y());
            index_type pre = invalid;
            index_type nex = invalid;
            int cc = 0;
            for (typename vector<T>::const_iterator item = p.begin();
                item != p.end();
                ++item)
            {
                nex = calculate_index((*item).X(), (*item).Y());
                l.insert(nex);
                add_block(pre, nex, l);
                pre = nex;
            }
            add_block(nex, first, l);
        }
    };


    // default advanced check function
    template<typename T>
    class check_true
    {
    public:
        bool operator() (T l, T r){return true;}
    };

    template<typename Global = Global_L, typename cal_type = calculate_utility<Global> >
    class world_map
    {
    public:
        //typedef calculate_utility<Global> cal_type;
        world_map(double xl, double yl, double xh, double yh):_xl(xl), _yl(yl), _xh(xh), _yh(yh)
        {
            if( _xl < Global::MXMIN || _xl > Global::MXMAX)
                _xl = Global::MXMIN;
            if( _xh < Global::MXMIN || _xh > Global::MXMAX)
                _xh = Global::MXMAX;
            if( _yl < Global::MYMIN || _yl > Global::MYMAX)
                _yl = Global::MYMIN;
            if( _yh < Global::MYMIN || _yh > Global::MYMAX)
                _yh = Global::MYMAX;
        }
        world_map():_xl(Global::MXMIN), _yl(Global::MYMIN), _xh(Global::MXMAX), _yh(Global::MYMAX) {}
        typedef boost::unordered_map<index_type, set<unsigned int> > map_type;
        typedef boost::unordered_map<index_type, set<unsigned int> > output_type;

        // add a polygon to world
        // xl,yl,xh,yh      :the bbox of the polygon
        // p                :point list of polygon
        // id               :polygon id or something that can represent the polygon
        template<typename T, typename U>
        void add_polygon(T xl, T yl, T xh, T yh, const vector<U>& p, int id)
        {
            if (xl > _xh || yl > _yh || xh < _xl || yh < _yl)
                return;
            typename cal_type::index_rect r(xl, yl, xh, yh);
            cal_uti.get_index(p, m_pl_index);
#ifndef NDEBUG
            printf("\r[%d][%u][%u]--------",id,p.size(),m_pl_index.size());
#endif  // NDEBUG
            add_to_map(r, id);
            m_pl_index.clear();
        }

        // get groups of polygon relations from world
        void get_relations(output_type &polygon)
        {
            check_true<unsigned int> ct;
            get_relations(polygon, ct);
        }

        // get groups of polygon relations from world
        // polygon      :   output
        // check_func   :   advanced check function to filter polygon
        template<typename F>
        void get_relations(output_type &polygon, F check_func)
        {
            for (map_type::iterator item = m_world.begin();
                item != m_world.end();
                ++item)
            {
                set<unsigned int>::iterator curr;
                set<unsigned int>::iterator item_end = item->second.end();
                for (set<unsigned int>::iterator item_begin = item->second.begin();
                    item_begin != item_end;
                    ++item_begin)
                {
                    curr = item_begin;
                    for (++curr; curr != item_end;++curr)
                    {
                        if(check_func(*item_begin, *curr))
                            polygon[*item_begin].insert(*curr);
                    }
                }
            }
        }
        index_type get_size(){return m_world.size();}
        void release(){m_world.clear();}
    private:
        // get all covered with graphics and insert them to world
        int add_to_map(typename cal_type::index_rect& r, unsigned int pidx)
        {
            unordered_set<index_type> tmp;
            // error test
            //TT<Global> TT1;   // if change to TT<Global_L> TT1, every code will ok
            //std::cout << "------------------------->>> " << typeid(TT1._v).name() << std::endl;
            //TT1.print_test_normal(i);
            //TT1.print_test<int>(i);  error
            //TT1.print_test<typename cal_type::index_rect::left>(i);   error            
            // http://stackoverflow.com/questions/3505713/c-template-compilation-error-expected-primary-expression-before-token
            //http://stackoverflow.com/questions/2105901/how-to-fix-expected-primary-expression-before-error-in-c-template-code

            //TT1.template print_test<int>(i);  ok
            //TT1.template print_test<typename cal_type::index_rect::left>(i);

            // error test end
            //TT1.calculate_index<int>(1,1);
            //calculate_utility<Global> TT2;
            //TT2.calculate_index<int>(0, 0);
            //TT2.get_polygon_index_by_side<typename cal_type::index_rect::left>(r, m_pl_index, tmp);
            cal_uti.template get_polygon_index_by_side<typename cal_type::index_rect::left>(r, m_pl_index, tmp);
            cal_uti.template get_polygon_index_by_side<typename cal_type::index_rect::bottom >(r, m_pl_index, tmp);
            cal_uti.template get_polygon_index_by_side<typename cal_type::index_rect::rigth>(r, m_pl_index, tmp);
            cal_uti.template get_polygon_index_by_side<typename cal_type::index_rect::top>(r, m_pl_index, tmp);
            //for_each(r.begin(), r.end(), print);
            typename cal_type::index_rect::template iterator<typename cal_type::index_rect::all> item_end = r.end();
            for (typename cal_type::index_rect::template iterator<typename cal_type::index_rect::all> item = r.begin();
                //item != r.end();
                item != item_end;
                ++item)
            {
                if (tmp.find(*item) == tmp.end())
                    m_world[*item].insert(pidx);
            }
            return 0;
        }
        unordered_set<unsigned int> m_pl_index;
        map_type m_world;
        double _xl;
        double _yl;
        double _xh;
        double _yh;
        cal_type cal_uti;
    };
}
#endif // _INDEX_UTILITY_H_
