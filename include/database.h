#ifdef DATABASE

#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;

class MyDataBase
{
private:
	MYSQL *sql = nullptr;
	MYSQL_RES *res = nullptr;
	MYSQL_ROW row;
	bool connect_flag;
	void showres();
public:
	MyDataBase();
	MyDataBase(MYSQL *mysql);
	~MyDataBase();
	void connect(const string host, const string user, const string password,
					const string database = "mysql", unsigned int port = 0,
					const char *unix_socket = nullptr, unsigned long client_flag = 0);
	void disconnect();
	void showdb();
	void createdb(const string &database);
	void usedb(const string &database);
	void deletedb(const string &database);
	void showtb();
	void createtb(const string &table, const string &elements);
	vector<vector<string>> selectitem(const string &table, const string &value);
	vector<vector<string>> selectitem(const string &table, const string &value, const string &limits);
	void insertitem(const string &table, const string &value);
	void insertitem(const string &table, const string &value, const string &col);
	void deleteitem(const string &table, const string &value);
	void updateitem(const string &table, const string &value, const string &limits);
	void query(const string &command);
};

#endif