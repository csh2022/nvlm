#ifdef DATABASE
#include "database.h"

using namespace std;

void MyDataBase::disconnect() {
	if (sql)
		mysql_close(sql);
	sql = nullptr;
	res = nullptr;
	connect_flag = false;
	cout << "disconnect success!" << endl;
}

MyDataBase::MyDataBase() {
	sql = mysql_init(nullptr);
	if (!sql)
		cout << "init error!" << endl;
	connect_flag = false;
}

MyDataBase::MyDataBase(MYSQL *mysql) {
	sql = mysql_init(mysql);
	if (!sql)
		cout << "init error!" << endl;
	connect_flag = false;
}

MyDataBase::~MyDataBase() {
	if (connect_flag)
		disconnect();
	sql = nullptr;
	mysql_free_result(res);
	res = nullptr;
}

void MyDataBase::connect(const string host, const string user, const string password, const string database,
							unsigned int port, const char *unix_socket, unsigned long client_flag) {
	if (!sql) {
		cout << "sql has not been initialized!" << endl;
		return;
	}
	if (!mysql_real_connect(sql, host.c_str(), user.c_str(), password.c_str(), database.c_str(),
							port, unix_socket, client_flag)) {
		cout << "connect error!" << endl;
		return;
	}
	// cout << "connect success!" << endl;
}

void MyDataBase::showdb() {
	string str = "show databases";
	if (mysql_query(sql, str.c_str())) {
		cout << "show databases error!" << endl;
		return;
	}
	showres();
}

void MyDataBase::createdb(const string &database) {
	string str = "create database if not exists " + database;
	if (mysql_query(sql, str.c_str())) {
		cout << "create database error!" << endl;
		return;
	}
	cout << "create database success!" << endl;
}

void MyDataBase::usedb(const string &database) {
	string str = "use " + database;
	if (mysql_query(sql, str.c_str())) {
		cout << "use database error!" << endl;
		return;
	}
	cout << "use " << database << " now!" << endl;
}

void MyDataBase::deletedb(const string &database) {
	string str = "drop database " + database;
	if (mysql_query(sql, str.c_str())) {
		cout << "delete database error!" << endl;
		return;
	}
	cout << database << " has been deleted!" << endl;
}

void MyDataBase::showtb() {
	string str = "show tables";
	if (mysql_query(sql, str.c_str())) {
		cout << "show tables error!" << endl;
		return;
	}
	showres();
}

void MyDataBase::createtb(const string &table, const string &elements) {
	string str = "create table " + table + "(" + elements + ")";
	if (mysql_query(sql, str.c_str())) {
		cerr << "create table error!" << endl;
		return;
	}
	cout << "create table success!" << endl;
}

vector<vector<string>> MyDataBase::selectitem(const string &table, const string &value) {
	string str = "select " + value + " from " + table;
	cout << str << endl;
	if (mysql_query(sql, str.c_str())) {
		cout << "select error!" << endl;
		return {};
	}

	vector<vector<string>> ret;
	res = mysql_use_result(sql);
	while ( (row = mysql_fetch_row(res)) != nullptr ) {
		int i = 0;
		vector<string> temp;
		while (i < mysql_num_fields(res))
			temp.push_back(row[i++]);
		ret.push_back(temp);
	}
	mysql_free_result(res);
	res = nullptr;
	return ret;
}

vector<vector<string>> MyDataBase::selectitem(const string &table, const string &value, const string &limits) {
	string str = "select " + value + " from " + table + " where " + limits;
	cout << str << endl;
	if (mysql_query(sql, str.c_str())) {
		cout << "select error!" << endl;
		return {};
	}

	vector<vector<string>> ret;
	res = mysql_use_result(sql);
	while ( (row = mysql_fetch_row(res)) != nullptr ) {
		int i = 0;
		vector<string> temp;
		while (i < mysql_num_fields(res))
			temp.push_back(row[i++]);
		ret.push_back(temp);
	}
	mysql_free_result(res);
	res = nullptr;
	return ret;
}

void MyDataBase::showres() {
	res = mysql_use_result(sql);
	cout << "****************The result is:****************" << endl;
	while ( (row = mysql_fetch_row(res)) != nullptr ) {
		int i = 0;
		while (i < mysql_num_fields(res))
			cout << row[i++] << "\t";
		cout << endl;
	}
	cout << "**********************************************" << endl;
	mysql_free_result(res);
	res = nullptr;
}

void MyDataBase::insertitem(const string &table, const string &value) {
	string str = "insert into " + table + " values (" + value + ")";
	cout << str << endl;
	if (mysql_query(sql, str.c_str())) {
		cout << "insert error!" << endl;
		return;
	}
	// cout << "insert success!" << endl;
}

void MyDataBase::insertitem(const string &table, const string &value, const string &col) {
	string str = "insert into " + table + " (" + col + ") values (" + value + ")";
	cout << str << endl;
	if (mysql_query(sql, str.c_str())) {
		cout << "insert error!" << endl;
		return;
	}
	cout << "insert success!" << endl;
}

void MyDataBase::deleteitem(const string &table, const string &value) {
	string str = "delete from " + table + " where " + value;
	cout << str << endl;
	if (mysql_query(sql, str.c_str())) {
		cout << "delete error!" << endl;
		return;
	}
	cout << "delete success!" << endl;
}

void MyDataBase::updateitem(const string &table, const string &value, const string &limits) {
	string str = "update " + table + " set " + value + " where " + limits;
	cout << str << endl;
	if (mysql_query(sql, str.c_str())) {
		cout << "delete error!" << endl;
		return;
	}
	cout << "update success!" << endl;
}

void MyDataBase::query(const string &command) {
	if (mysql_query(sql, command.c_str())) {
		cout << "query error!" << endl;
		return;
	}
	cout << "query success!" << endl;
}

#endif