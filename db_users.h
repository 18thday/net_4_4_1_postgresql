#pragma once

#include <iostream>
#include <string>
#include <utility>

#include <pqxx/pqxx>

enum class ClientDataType {
    FIRST_NAME,
    LAST_NAME,
    EMAIL
};

struct ClientData {
    std::string first_name;
    std::string last_name;
    std::string email;

    bool empty() const;
};

class DBUsers {
public:
    explicit DBUsers (pqxx::connection&& db_con);
    explicit DBUsers (std::string host, std::string port, std::string dbname, std::string user, std::string password);

    void create_tables();

    int add_client(const std::string& first_name, const std::string& last_name, const std::string& email);
    int add_phone_number(int user_id, const std::string& phone);

    void change_client_data(int user_id, const ClientData& client_data);
    void change_client_data(int user_id, ClientDataType value_type, const std::string& value);

    void delete_phone_number(const std::string& phone);
    void delete_phone_number(int user_id);
    void delete_client(int user_id);

    pqxx::internal::result_iteration<int> find_client(const ClientData& client_data);
    int find_client_by_email (const std::string& email);
    int find_client_by_phone (const std::string& phone);

    pqxx::internal::result_iteration<std::string, std::string, std::string>  find_client_data_by_id (int id);
    pqxx::internal::result_iteration<int, std::string> find_client_phone_by_id (int id);
    pqxx::internal::result_iteration<std::string> find_client_phone_by_user_id (int user_id);

private:
    pqxx::connection db_conn_;

    void create_table_client_data();
    void create_table_client_phone();

    void prepare();

    void add_one_pair_to_update(std::string& query, const std::string& param, const std::string& value,
                                bool& is_not_first);

    std::string select_client_data_by_id(int user_id) const;
    std::string select_id_by_email(const std::string& email) const;
    std::string select_id_by_phone(const std::string& phone) const;
    std::string select_user_id_by_phone(const std::string& phone) const;
    std::string select_phone_by_user_id(int user_id) const;
    std::string select_phone_by_id(int phone_id) const ;
    std::string select_id_by_client_data(const ClientData& client_data) const;

    pqxx::connection InitializeConnection(const std::string& host,
                                          const std::string& port,
                                          const std::string& dbname,
                                          const std::string& user,
                                          const std::string& password);
};