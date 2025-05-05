#include <cstdint>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <algorithm>
#include "include/input_info.hpp"
#include "include/utils.hpp"

using ClientName = std::string;
std::queue<std::string> client_queue;

void print_stat_tables(std::vector<utils::Table>& tables) {
    for (size_t i = 0; i < tables.size(); i++) {
        std::tm tmp_time = {};
        uint64_t seconds = tables[i].get_seconds();
        tmp_time.tm_hour = seconds / 3600;
        tmp_time.tm_min = (seconds % 3600) / 60;
        std::cout << (i + 1) << " " << tables[i].get_money() << " " 
                  << std::put_time(&tmp_time, "%H:%M") << std::endl;
    }
}

void event1(input::Event& event,
            std::vector<utils::Table>& tables,
            std::unordered_map<ClientName, utils::ClientStatus>& clients,
            input::InputInfo& inputInfo) {
    ClientName client_name = event.str;
    std::time_t clubOpeningTime = std::mktime(&inputInfo.OpenTime);
    std::time_t clubClosingTime = std::mktime(&inputInfo.CloseTime);
    std::time_t clientArrivalTime = std::mktime(&event.EventTime);

    if (clientArrivalTime < clubOpeningTime ||
        clientArrivalTime > clubClosingTime) {
        // Error, bad time
        std::cout << input::Event(event.EventTime, 13, "NotOpenYet")  << std::endl;
        return;
    } 

    if (clients.find(client_name) != clients.end()) {
        // Error, client is already in club
        std::cout << input::Event(event.EventTime, 13, "YouShallNotPass") << std::endl;
        return;
    }

    clients[std::move(client_name)] = utils::ClientStatus();            
}

void event2(input::Event& event,
    std::vector<utils::Table>& tables,
    std::unordered_map<ClientName, utils::ClientStatus>& clients,
    input::InputInfo& inputInfo) {
    ClientName client_name = event.str;

    if (clients.find(client_name) == clients.end()) {
        // Error, client isn't in the club
        std::cout << input::Event(event.EventTime, 13, "ClientUnknown") << std::endl;
        return;
    }

    int desired_table_id = *event.table_id;

    /*
    We are sure that desired_table_id - 1 fits in range 0 ... tables.size() - 1
    Otherwise, an appropriate error would have arised during parsing stage
    */
    if (tables[desired_table_id - 1].get_acquired()) {
        std::cout << input::Event(event.EventTime, 13, "PlaceIsBusy") << std::endl;
        return;
    }

    std::optional<int> maybe_current_table_id = clients[client_name].get_maybe_table_id();
    std::optional<std::tm> maybe_start_sitting_time = clients[client_name].get_maybe_start_sitting_time();

    if (maybe_current_table_id) {
        tables[(*maybe_current_table_id - 1)].unset_acquired(); // release table

        std::time_t t1 = std::mktime(&maybe_start_sitting_time.value());
        std::time_t t2 = std::mktime(&event.EventTime);
        uint64_t diff = t2 - t1;
        tables[(*maybe_current_table_id - 1)].add_seconds(diff);
        tables[(*maybe_current_table_id - 1)].add_money(inputInfo.price * std::ceil(diff / 3600.0)); 
    }

    // start new sitting 
    clients[client_name].set_table_id(desired_table_id);
    clients[client_name].set_start_sitting_time(event.EventTime); 

    tables[desired_table_id - 1].set_acquired();
    tables[desired_table_id - 1].set_client_name(client_name);
}

void event3(input::Event& event,
    std::vector<utils::Table>& tables,
    std::unordered_map<ClientName, utils::ClientStatus>& clients,
    input::InputInfo& inputInfo) {
    ClientName client_name = event.str;

    bool has_empty_table = false;
    for (size_t i = 0; i < tables.size(); i++) {
        if (!tables[i].get_acquired()) {
            has_empty_table = true;break;
        }
    }
    if (has_empty_table) {
        std::cout << input::Event(event.EventTime, 13, "ICanWaitNoLonger!") << std::endl;
        return;
    }
    if (client_queue.size() == tables.size()) {
        std::cout << input::Event(event.EventTime, 11, client_name) << std::endl;
        return;
    }
    client_queue.push(client_name);
}

void event4(input::Event& event,
    std::vector<utils::Table>& tables,
    std::unordered_map<ClientName, utils::ClientStatus>& clients,
    input::InputInfo& inputInfo) {
    ClientName client_name = event.str;

    if (clients.find(client_name) == clients.end()) {
        // Error, client isn't in the club
        std::cout << input::Event(event.EventTime, 13, "ClientUnknown") << std::endl;
        return;
    }
    int current_table_id = *clients[client_name].get_maybe_table_id();
    std::tm start_sitting_time = *clients[client_name].get_maybe_start_sitting_time();

    tables[current_table_id - 1].unset_acquired(); // release table

    std::time_t t1 = std::mktime(&start_sitting_time);
    std::time_t t2 = std::mktime(&event.EventTime);
    uint64_t diff = t2 - t1;
    tables[current_table_id - 1].add_seconds(diff);
    tables[current_table_id - 1].add_money(inputInfo.price * std::ceil(diff / 3600.0));
    
    clients.erase(client_name);

    if (!client_queue.empty()) {
        std::string first_client = std::move(client_queue.front());
        client_queue.pop();

        tables[current_table_id - 1].set_acquired();
        tables[current_table_id - 1].set_client_name(first_client);
        clients[first_client].set_table_id(current_table_id);
        clients[first_client].set_start_sitting_time(event.EventTime);

        std::cout << input::Event(event.EventTime, 12, first_client, current_table_id) << std::endl;
    }
}

void event5(input::Event& event,
    std::vector<utils::Table>& tables,
    std::unordered_map<ClientName, utils::ClientStatus>& clients,
    input::InputInfo& inputInfo) { 
    std::vector<ClientName> remainedClients;

    for (const auto& status: clients) {
        if (status.second.get_maybe_table_id()) 
            remainedClients.push_back(status.first);
    }

    std::sort(std::begin(remainedClients), std::end(remainedClients));
    for (const auto& cname: remainedClients) {
        // process table info and print event td 11
        std::optional<int> maybe_current_table_id = clients[cname].get_maybe_table_id();
        std::optional<std::tm> maybe_start_sitting_time = clients[cname].get_maybe_start_sitting_time();

        if (maybe_current_table_id) {
            tables[(*maybe_current_table_id - 1)].unset_acquired(); // release table
    
            std::time_t t1 = std::mktime(&maybe_start_sitting_time.value());
            std::time_t t2 = std::mktime(&event.EventTime);
            uint64_t diff = t2 - t1;
            tables[(*maybe_current_table_id - 1)].add_seconds(diff);
            tables[(*maybe_current_table_id - 1)].add_money(inputInfo.price * std::ceil(diff / 3600.0)); 
        }

        std::cout << input::Event(event.EventTime, 11, cname) << std::endl;
    }

    // print end time
    std::cout << std::put_time(&inputInfo.CloseTime, "%H:%M") << std::endl;

    // stat tables
    print_stat_tables(tables);

    // Clean up DS in the end of day
    clients.clear();
    for (auto& table: tables) 
        table.clear();
    {
        std::queue<std::string> tmp;
        std::swap(client_queue, tmp);
    }
}

void processEvent(input::Event& event,
                  std::vector<utils::Table>& tables,
                  std::unordered_map<ClientName, utils::ClientStatus>& clients,
                  input::InputInfo& inputInfo
) {
    // print the event (exception with id 5 since it's system)
    if (event.event_id != 5)
        std::cout << event << std::endl;

    // analize event
    switch (event.event_id) {
        case 1: // Client has come
            event1(event, tables, clients, inputInfo);
            break;
        case 2: // Client sat down to specific table
            event2(event, tables, clients, inputInfo);
            break;
        case 3: // Client is waiting
            event3(event, tables, clients, inputInfo);
            break;
        case 4: // Client has left
            event4(event, tables, clients, inputInfo);
            break;
        case 5: // System event
            event5(event, tables, clients, inputInfo);
            break;
        /*
        There is no default case since we are sure our id is correct
        Otherwise, an appropriate error would have arised during parsing stage
        */
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "error: invalid args" << std::endl;
        std::cerr << "example: ./club test_file.txt" << std::endl;
        exit(1);
    }

    std::string filename(argv[1]);
    input::InputInfo inputInfo;

    auto err = input::ParseInput(filename, inputInfo);
    
    #ifdef DEBUG 
        std::cerr << "ParseInput finished\n";
    #endif
    if (err) {
        // Error occured during parsing, should print line
        std::cerr << *err << std::endl;
        exit(2);
    }

    // state info
    std::vector<utils::Table> tables(inputInfo.CountTables);
    for (size_t i = 1; i <= inputInfo.CountTables; i++) {
        tables[i - 1].set_table_id(i);
    }
    std::unordered_map<ClientName, utils::ClientStatus> clients;

    // print start time
    std::cout << std::put_time(&inputInfo.OpenTime, "%H:%M") << std::endl;

    for (auto& event: inputInfo.events) {
        processEvent(event, tables, clients, inputInfo);
    }

    return 0;
}