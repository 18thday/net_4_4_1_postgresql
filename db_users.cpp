#include "db_users.h"

#include <iostream>
#include <string>
#include <utility>

#include <pqxx/pqxx>

bool ClientData::empty() const {
    return first_name.empty() && last_name.empty() && email.empty();
}


// public
DBUsers::DBUsers (pqxx::connection&& db_con) : db_conn_(std::move(db_con)) {
    create_tables();
    prepare();
}
DBUsers::DBUsers (std::string host, std::string port, std::string dbname, std::string user, std::string password)
: db_conn_(std::move(InitializeConnection(host, port, dbname, user, password))) {
    create_tables();
    prepare();
}

void DBUsers::create_tables() {
    create_table_client_data();
    create_table_client_phone();
}

int DBUsers::add_client(const std::string& first_name, const std::string& last_name, const std::string& email) {
    pqxx::transaction tr{db_conn_};
    tr.exec_prepared("insert_client", first_name, last_name, email);
    int id = tr.query_value<int>(select_id_by_email(email));
    tr.commit();

    return id;
}

int DBUsers::add_phone_number(int user_id, const std::string& phone) {
    pqxx::transaction tr{db_conn_};
    tr.exec_prepared("insert_phone", std::to_string(user_id), phone);
    int id = tr.query_value<int>(select_id_by_phone(phone));
    tr.commit();

    return id;
}

void DBUsers::change_client_data(int user_id, const ClientData& client_data) {
    if (client_data.first_name.empty() &&
        client_data.last_name.empty() &&
        client_data.email.empty()) {
        return;
    }

    pqxx::transaction tr{db_conn_};

    std::string query = "UPDATE client_data SET ";
    bool is_not_first = false;
    if (!client_data.first_name.empty()) {
        add_one_pair_to_update(query, "first_name", tr.esc(client_data.first_name), is_not_first);
    }
    if (!client_data.last_name.empty()) {
        add_one_pair_to_update(query, "last_name", tr.esc(client_data.last_name), is_not_first);
    }
    if (!client_data.first_name.empty()) {
        add_one_pair_to_update(query, "email", tr.esc(client_data.email), is_not_first);
    }
    query += " WHERE id = " + std::to_string(user_id) + ";";

    tr.exec(query);
    tr.commit();
}

void DBUsers::change_client_data(int user_id, ClientDataType value_type, const std::string& value) {
    pqxx::transaction tr{db_conn_};
    switch(value_type) {
        case ClientDataType::FIRST_NAME:
            tr.exec_prepared("update_client_first_name", std::to_string(user_id), value); break;
        case ClientDataType::LAST_NAME:
            tr.exec_prepared("update_client_last_name", std::to_string(user_id), value); break;
        case ClientDataType::EMAIL:
            tr.exec_prepared("update_client_email", std::to_string(user_id), value); break;
    }
    tr.commit();
}

void DBUsers::delete_phone_number(const std::string& phone) {
    pqxx::transaction tr{db_conn_};
    tr.exec_prepared("delete_phone", phone);
    tr.commit();
}
void DBUsers::delete_phone_number(int user_id) {
    pqxx::transaction tr{db_conn_};
    tr.exec_prepared("delete_phones_by_user_id", std::to_string(user_id));
    tr.commit();
}

void DBUsers::delete_client(int user_id) {
    pqxx::transaction tr{db_conn_};
    tr.exec_prepared("delete_phones_by_user_id", std::to_string(user_id));
    tr.exec_prepared("delete_client", std::to_string(user_id));
    tr.commit();
}

pqxx::internal::result_iteration<int> DBUsers::find_client(const ClientData& client_data) {
    pqxx::transaction tr{db_conn_};
    return tr.query<int>(select_id_by_client_data(client_data));
}

int DBUsers::find_client_by_email (const std::string& email) {
    if (email.empty()) {
        return -1;
    }
    pqxx::transaction tr{db_conn_};
    return tr.query_value<int>(select_id_by_email(email));
}

int DBUsers::find_client_by_phone (const std::string& phone) {
    if (phone.empty()) {
        return -1;
    }
    pqxx::transaction tr{db_conn_};
    return tr.query_value<int>(select_user_id_by_phone(phone));
}

pqxx::internal::result_iteration<std::string, std::string, std::string> DBUsers::find_client_data_by_id (int id) {
    pqxx::transaction tr{db_conn_};
    return tr.query<std::string, std::string, std::string>(select_client_data_by_id(id));
}
pqxx::internal::result_iteration<int, std::string> DBUsers::find_client_phone_by_id (int id) {
    pqxx::transaction tr{db_conn_};
    return tr.query<int, std::string>(select_phone_by_id(id));
}

pqxx::internal::result_iteration<std::string> DBUsers::find_client_phone_by_user_id (int user_id) {
    pqxx::transaction tr{db_conn_};
    return tr.query<std::string>(select_phone_by_user_id(user_id));
}

// private
void DBUsers::create_table_client_data() {
    std::string query = "CREATE TABLE IF NOT EXISTS client_data ("
                        "id SERIAL PRIMARY KEY,"
                        "first_name VARCHAR(40) NOT NULL, "
                        "last_name VARCHAR(40) NOT NULL, "
                        "email VARCHAR(100) UNIQUE NOT NULL"
                        ");";
    pqxx::transaction tr{db_conn_};
    tr.exec(query);
    tr.commit();
}

void DBUsers::create_table_client_phone() {
    std::string query = "CREATE TABLE IF NOT EXISTS client_phone ("
                        "id SERIAL PRIMARY KEY, "
                        "user_id INTEGER NOT NULL REFERENCES client_data(id), "
                        "phone VARCHAR(16) UNIQUE"
                        ");";
    pqxx::transaction tr{db_conn_};
    tr.exec(query);
    tr.commit();
}

void DBUsers::prepare() {
    db_conn_.prepare("insert_client",
                     "INSERT INTO client_data(first_name, last_name, email) VALUES($1, $2, $3);");
    db_conn_.prepare("insert_phone",
                     "INSERT INTO client_phone(user_id, phone) VALUES($1, $2);");

    db_conn_.prepare("update_client_first_name",
                     "UPDATE client_data SET first_name = $2 WHERE id = $1;");
    db_conn_.prepare("update_client_last_name",
                     "UPDATE client_data SET last_name = $2 WHERE id = $1;");
    db_conn_.prepare("update_client_email",
                     "UPDATE client_data SET email = $2 WHERE id = $1;");

    db_conn_.prepare("delete_phone",
                     "DELETE FROM client_phone WHERE phone = $1;");
    db_conn_.prepare("delete_phones_by_user_id",
                     "DELETE FROM client_phone WHERE user_id = $1;");
    db_conn_.prepare("delete_client",
                     "DELETE FROM client_data WHERE id = $1;");
}

void DBUsers::add_one_pair_to_update(std::string& query, const std::string& param, const std::string& value, bool& is_not_first) {
    if (is_not_first) {
        query += ", ";
    }
    query += param + " = '" + value + '\'';
    is_not_first = true;
}

std::string DBUsers::select_client_data_by_id(int user_id) const {
    return "SELECT first_name, last_name, email FROM client_data WHERE id = " + std::to_string(user_id) + ";";
}
std::string DBUsers::select_id_by_email(const std::string& email) const {
    return "SELECT id FROM client_data WHERE email = '" + email + "';";
}
std::string DBUsers::select_id_by_phone(const std::string& phone) const {
    return "SELECT id FROM client_phone WHERE phone = '" + phone + "';";
}
std::string DBUsers::select_user_id_by_phone(const std::string& phone) const {
    return "SELECT user_id FROM client_phone WHERE phone = '" + phone + "';";
}
std::string DBUsers::select_phone_by_user_id(int user_id) const {
    return "SELECT phone FROM client_phone WHERE user_id = " + std::to_string(user_id) + ";";
}
std::string DBUsers::select_phone_by_id(int phone_id) const {
    return "SELECT user_id, phone FROM client_phone WHERE id = " + std::to_string(phone_id) + ";";
}
std::string DBUsers::select_id_by_client_data(const ClientData& client_data) const {
    std::string query = "SELECT id FROM client_data WHERE ";
    bool is_not_first = false;
    if (!client_data.first_name.empty()) {
        query += "first_name = '" + client_data.first_name + '\'';
        is_not_first = true;
    }
    if (!client_data.last_name.empty()) {
        if (is_not_first) {
            query += " AND ";
        }
        query += "last_name = '" + client_data.last_name + '\'';
        is_not_first = true;
    }
    if (!client_data.email.empty()) {
        if (is_not_first) {
            query += " AND ";
        }
        query += "email = '" + client_data.email + '\'';
    }
    return query + ";";
}

pqxx::connection DBUsers::InitializeConnection(const std::string& host,
                                      const std::string& port,
                                      const std::string& dbname,
                                      const std::string& user,
                                      const std::string& password) {
    try {
        pqxx::connection db_conn ("host=" + host +
                                  " port=" + port +
                                  " dbname=" + dbname +
                                  " user=" + user +
                                  " password=" + password);

        std::cout << "Connection established" << std::endl;
        return db_conn;

    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL: " << e.what() << std::endl;
    }
    return {};
}