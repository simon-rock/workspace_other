#include <string>
#include <iostream>
#include <fstream>
//#include <boost/program_options/variables_map.hpp>
//#include <boost/program_options/parsers.hpp>
#include <boost/program_options.hpp>
//#include <boost/scoped_ptr.hpp>
#include <boost/optional.hpp>

using namespace std;
namespace po = boost::program_options;

// Custom Analytic Function
std::pair<std::string, std::string> at_option_parser(std::string const& s)
{
    cout << __func__ << " : " << s << endl;
    return std::pair<std::string, std::string>();
    if ('@' == s[0])
        {

            return make_pair(std::string("config"), s.substr(1));
        }
    else
        {

            return std::pair<std::string, std::string>();
        }
}

template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{

    copy(v.begin(), v.end(), ostream_iterator<T>(os, " -> "));
    return os;
}

int main(int argc, char** argv){

    string str;
    short  num;
    vector<string> vec;

    string default_str;
    short  default_num;

    //
    po::options_description opts("options");
    opts.add_options()
        ("string", po::value<string>(&str), "string")
        ("num", po::value<short>(&num), "number")
        ("vector", po::value<vector<string> >(&vec)->multitoken(), "vector");
    // ("email", value<vector<string>>()->composing()->notifier(&your_function), "email")
    
    po::options_description default_opts("default options");
    default_opts.add_options()
        ("help,h", "produce help message")
        ("dstring", po::value<string>(&default_str)->default_value("default string"), "default string")
        ("dnum", po::value<short>(&default_num)->default_value(80), "default number");

    string config_file;
    po::options_description config_opts("config options");
    config_opts.add_options()
        ("config", po::value<string>(&config_file)->default_value("config.conf"),
         "set config file, specified with '@name' too");
    
    string mode;
    po::options_description pos_opts("pos options");
    pos_opts.add_options()
        ("mode", po::value<string>(&mode)->default_value("no mode"), "mode");
    po::positional_options_description p;
    p.add("mode", 1).add("vector", -1);

    
    int level;
    po::options_description syn_opts("default options");
    syn_opts.add_options()
        ("verbose", po::value<int>(&level)->implicit_value(0), "verbosity level");
    
    po::options_description all("All options");
    all.add(opts).add(default_opts).add(config_opts).add(pos_opts).add(syn_opts);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all).extra_parser(::at_option_parser).run(), vm);
    // po::store(po::command_line_parser(argc, argv).options(all).positional(p).allow_unregistered().run(), vm);
    // po::store(po::command_line_parser(argc, argv).options(all).positional(p).run(), vm);
    

    //
    if (vm.count("help")){
        cout << all << endl;
        return 0;
    }

    if (vm.count("config")){
        cout << "load config " << endl;
        string conf_name = vm["config"].as<string>();  // default config.conf
        ifstream ini_file(conf_name.c_str());  
        po::store(po::parse_config_file(ini_file, all, true), vm);
    }
    boost::program_options::notify(vm);



    if (vm.count("num")){
        std::cout << "set num : " << vm["num"].as<short>()  << std::endl; // type must same to declare!!
    }

    if (vm.count("dnum")){
        std::cout << "set dnum : " << vm["dnum"].as<short>() << std::endl; // type must same to declare!!
    }
    
    {
        cout << "str : " << str << endl;
        cout << "num : " << num << endl;
        cout << "vector : "  << vec << endl;
        /*
        for (int i = 0; i < vec.size(); i++)
            cout << "\t" << vec[i] << endl;
        */
        cout << "dstring : " << default_str << endl;
        cout << "dnum : " << default_num << endl;
        cout << "config : " << config_file << endl;
        cout << "mode : " << mode << endl;
    }
    return 0;
}

// ref : 
// g++ -o main tests_po.cpp -lboost_program_options
// https://blog.csdn.net/morning_color/article/details/50241987  
// http://ju.outofmemory.cn/entry/106428
// http://wrran.com/blog/2017/02/21/howto/boost/program-options/

// config.conf
// num=71
// str="test"
// dnum=87
// mode=tm
