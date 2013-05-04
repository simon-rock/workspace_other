#include "calculate_digest.hpp"
#include <iostream>
using namespace std;

int main()
{
	string result;
	cout << "-----md5-----" << endl;
	digest::digt_md5 new_md5;
	new_md5.append_file("hash_data/1234.txt");
	new_md5.append_file("hash_data/readme.txt");
	cout << new_md5.get_digest() << endl;

	digest::calculate_digest<digest::md5> _md5;
	_md5.append_file("hash_data/1234.txt");
	_md5.append_file("hash_data/readme.txt");
	cout << _md5.get_digest() << endl;

	cout << "-----sha1-----" << endl;
	digest::digt_sha1 new_sha1;
	new_sha1.append_file("hash_data/1234.txt");
	new_sha1.append_file("hash_data/readme.txt");
	cout << new_sha1.get_digest() << endl;

	digest::calculate_digest<digest::sha1> _sha1;
	_sha1.append_file("hash_data/1234.txt");
	_sha1.append_file("hash_data/readme.txt");
	cout << _sha1.get_digest() << endl;

	cout << "-----sha256-----" << endl;
	digest::digt_sha256 new_sha256;
	new_sha256.append_file("hash_data/1234.txt");
	new_sha256.append_file("hash_data/readme.txt");
	cout << new_sha256.get_digest() << endl;

	digest::calculate_digest<digest::sha256> _sha256;
	_sha256.append_file("hash_data/1234.txt");
	_sha256.append_file("hash_data/readme.txt");
	cout << _sha256.get_digest() << endl;

	cout << "-----sha512-----" << endl;
	digest::digt_sha512 new_sha512;
	new_sha512.append_file("hash_data/1234.txt");
	new_sha512.append_file("hash_data/readme.txt");
	cout << new_sha512.get_digest() << endl;
//	string ttt = new_sha512.get_digest();
//	cout << ttt.substr(32,32)<< endl;
//	cout << ttt.substr(96) << endl;
	digest::calculate_digest<digest::sha512> _sha512;
	_sha512.append_file("hash_data/1234.txt");
	_sha512.append_file("hash_data/readme.txt");
	cout << _sha512.get_digest() << endl;

	cout << "-----filesize-----" << endl;
	digest::digt_size new_size;
	new_size.append_file("hash_data/1234.txt");
	new_size.append_file("hash_data/readme.txt");
	cout << new_size.get_digest() << endl;
	digest::calculate_digest<digest::size> _size;
	_size.append_file("hash_data/1234.txt");
	_size.append_file("hash_data/readme.txt");
	cout << _size.get_digest() << endl;

	return 0;
}
