#include <iostream>
#include <vector>

#include <Windows.h>
#pragma execution_character_set("utf-8")

#include "db_users.h"

void PrintClients (DBUsers& db, const std::vector<int> user_ids) {
    for (int id : user_ids) {
        auto [first_name, last_name, email] = *(db.find_client_data_by_id(id).begin());
        std::cout << id << ": " << first_name << " " << last_name << " " << email << std::endl;
    }
}

void PrintClientPhones (DBUsers& db, const std::vector<int> phone_ids) {
    for (int id : phone_ids) {
        auto [user_id, phone] = *(db.find_client_phone_by_id(id).begin());
        std::cout << id << ": " << user_id << " " << phone << " " << std::endl;
    }
}

int main() {
    // setlocale(LC_ALL, "Russian");
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);

    DBUsers db_users("localhost", "5432", "db_net_users", "postgres", "password");

    std::vector<int> db_users_clients;
    {
        std::cout << "DB add 3 clients:" << std::endl;

        int id = db_users.add_client("Sergey", "Ivanov", "s.ivanov@yandex.ru");
        db_users_clients.push_back(id);
        id = db_users.add_client("Alexey", "Fedorov", "a.fedorov@mail.ru");
        db_users_clients.push_back(id);
        id = db_users.add_client("Valeriy", "Upakov", "v.upakov@mail.ru");
        db_users_clients.push_back(id);

        PrintClients(db_users, db_users_clients);
    }
    std::vector<int> db_users_phones;
    {
        std::cout << std::endl << "DB add phone numbers for clients:" << std::endl;
        auto iter_id = db_users_clients.begin();

        int id = db_users.add_phone_number(*iter_id, "+79912807456");
        db_users_phones.push_back(id);
        ++iter_id;
        id = db_users.add_phone_number(*iter_id, "+79921408456");
        db_users_phones.push_back(id);
        ++iter_id;
        id = db_users.add_phone_number(*iter_id, "+79930001001");
        db_users_phones.push_back(id);
        id = db_users.add_phone_number(*iter_id, "+79930001002");
        db_users_phones.push_back(id);

        PrintClientPhones(db_users, db_users_phones);
    }
    {
        auto iter_id = db_users_clients.begin();
        std::string deleted_phone = "+79930001002";
        std::cout << std::endl << "DB delete phone for client:"  << *iter_id
                               << " and for number: " << deleted_phone <<std::endl;

        db_users.delete_phone_number(*iter_id);
        db_users.delete_phone_number(deleted_phone);
        PrintClientPhones(db_users, db_users_phones);
    }
    {
        auto iter_id = db_users_clients.begin() + 1;
        db_users.delete_client(*iter_id);

        std::cout << std::endl << "DB clients after delete client with id:"  << *iter_id << std::endl;
        PrintClients(db_users, db_users_clients);
        std::cout << std::endl << "DB phones after delete client with id:"  << *iter_id << std::endl;
        PrintClientPhones(db_users, db_users_phones);
    }
    {
        auto iter_id = db_users_clients.begin();
        db_users.change_client_data(*iter_id, ClientDataType::FIRST_NAME, "Sergio");
        db_users.change_client_data(*iter_id, ClientDataType::LAST_NAME, "Kozlov");
        db_users.change_client_data(*iter_id, ClientDataType::EMAIL, "s.kozlov@gmail.com");

        std::cout << std::endl << "DB clients after change client with id:"  << *iter_id << std::endl;
        PrintClients(db_users, db_users_clients);
    }
    {
        auto iter_id = db_users_clients.begin();
        ClientData client_data("Sergey", "Ivanov", "s.ivanov@gmail.com");
        db_users.change_client_data(*iter_id, client_data);

        std::cout << std::endl << "DB clients after change client with id:"  << *iter_id << std::endl;
        PrintClients(db_users, db_users_clients);
    }
    {
        std::string search_email = "v.upakov@mail.ru";
        auto id = db_users.find_client_by_email(search_email);
        std::cout << std::endl << "User with email: " << search_email << " has id = " << id;

        std::string search_phone = "+79930001001";
        id = db_users.find_client_by_phone(search_phone);
        std::cout << std::endl << "User with phone: " << search_phone << " has id = " << id;

        ClientData client_data("Valeriy", "Upakov", {});
        auto res_it = db_users.find_client(client_data);
        std::cout << std::endl << client_data.first_name << " " << client_data.last_name;
        for (auto [idx] : res_it) {
            std::cout << " was found, user_id = " << idx << std::endl;
        }


        id = std::get<0>(*res_it.begin());
        std::cout << std::endl << "find_client_data_by_id : "  << id << std::endl;
        auto res_client = db_users.find_client_data_by_id(id);
        for (auto [f_name, l_name, mail] : res_client) {
            std::cout << f_name << " " << l_name << " " << mail << std::endl;
        }
        std::cout << std::endl << "find_client_phone_by_user_id : "  << id << std::endl;
        auto res_phone = db_users.find_client_phone_by_user_id(id);
        for (auto [phone] : res_phone) {
            std::cout << phone << std::endl;
        }
    }
    return 0;
}
