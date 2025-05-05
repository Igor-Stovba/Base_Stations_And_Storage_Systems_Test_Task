#include "include/input_info.hpp"

#include <cctype>
#include <iomanip>
#include <fstream>
#include <ios>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iostream>

namespace input {

    std::ostream& operator<<(std::ostream& os, const Event& event) {
        std::tm tmp = event.EventTime;
        os << std::put_time(&tmp, "%H:%M") << " " << event.event_id 
           << " " << event.str;
        if (event.table_id) {
            os << " " << *event.table_id;
        } 
        return os;
    }

    namespace {
        std::pair<std::tm, std::tm> parse_time_range(const std::string& tm) {
            #ifdef DEBUG 
                std::cerr << "parse_time_range started" << std::endl;
            #endif
            std::istringstream ss(tm);
            std::tm start = {};
            std::tm end = {};

            ss >> std::get_time(&start, "%H:%M");
            if (ss.fail()) 
                throw std::runtime_error("erorr in start time");

            ss >> std::get_time(&end, "%H:%M");
            if (ss.fail()) 
                throw std::runtime_error("error in end time");

            std::string checker;
            if (ss >> checker) 
                throw std::runtime_error("error: extra invalid input");

            return {std::move(start), std::move(end)};
        }

        Event parse_event(const std::string& ev) {
            #ifdef DEBUG 
                std::cerr << "parse_event started" << std::endl;
            #endif
            std::istringstream ss(ev);
            std::tm tm = {};
            int event_id;
            std::string client_name;
            std::optional<int> table_id;

            auto is_valid_name = [](const std::string& s) -> bool {
                for (char ch: s) {
                    if (!(std::islower(ch) || std::isdigit(ch) || ch == '_')) 
                        return false;
                }
                return !s.empty();
            };

            ss >> std::get_time(&tm, "%H:%M");
            if (ss.fail())
                throw std::runtime_error("error: time invalid");

            ss >> event_id;
            if (ss.fail() || event_id < 1 || event_id > 4) 
                throw std::runtime_error("error: event_id invalid");

            ss >> client_name;
            if (ss.fail() || !is_valid_name(client_name)) 
                throw std::runtime_error("error: client_name invalid");

            if (event_id == 2) {
                int tmp_table_id;
                ss >> tmp_table_id;
                if (ss.fail()) 
                    throw std::runtime_error("error: table_id invalid");
                table_id = tmp_table_id;
            } else {
                // check for extra args - invalid
                std::string checker;
                if (ss >> checker) 
                    throw std::runtime_error("error: extra invalid input");
            }

            if (table_id) 
                return {std::move(tm), event_id, std::move(client_name), *table_id};
            return {std::move(tm), event_id, std::move(client_name)};
        }
    }

    std::optional<std::string> ParseInput(const std::string filename, InputInfo& input) {
        /*
        Parses file and validates info
        If it encouters invalid line, method returns this invalid line. 
        Otherwise std::nullopt

        Note: function also generates "system" events which force us to finish day 
              and make appropriate things by processing events
        */
        #ifdef DEBUG 
            std::cerr << "ParseInput started" << std::endl;
        #endif

        std::ifstream file(filename, std::ios_base::in);

        if (file.is_open()) {
            std::string num_tables, open_close_time, cost;
            std::getline(file, num_tables);
            try {
                int CountTables = std::stoi(num_tables.c_str(), nullptr, 10);
                input.CountTables = CountTables;
            } catch (const std::invalid_argument&) {
                return {num_tables};
            }

            std::getline(file, open_close_time);
            try {
                std::pair<std::tm, std::tm> p = parse_time_range(open_close_time);
                input.OpenTime = p.first;
                input.CloseTime = p.second;
            } catch (const std::runtime_error&) {
                return {open_close_time};
            }

            std::getline(file, cost);
            try {
                int Price = std::stoi(cost.c_str(), nullptr, 10);
                input.price = Price;
            } catch (const std::invalid_argument&) {
                return {cost};
            }

            std::string query;
            

            /*
            Is is needed for "system" events
            */
            std::tm prev_time_event;
            std::tm curr_time_event;
            bool is_opened = false;

            while (std::getline(file, query)) {
                try {
                    Event ev = parse_event(query);
                    std::swap(prev_time_event, curr_time_event);
                    curr_time_event = ev.EventTime;

                    if (std::mktime(&curr_time_event) >= std::mktime(&input.CloseTime) &&
                        std::mktime(&prev_time_event) < std::mktime(&input.CloseTime)) {
                        Event close_event = {
                            input.CloseTime,
                            5,
                            "__system__"
                        };
                        input.events.push_back(std::move(close_event));
                    }

                    input.events.push_back(std::move(ev));
                } catch (const std::runtime_error&) {
                    return {query};
                }
            }
            // check if we should add "system" event (such case is possible!)
            if (input.events.back().event_id != 5 &&
                std::mktime(&input.events.back().EventTime) >= std::mktime(&input.OpenTime) &&
                std::mktime(&input.events.back().EventTime) < std::mktime(&input.CloseTime)) {
                Event close_event = {
                    input.CloseTime,
                    5,
                    "__system__"
                };
                input.events.push_back(std::move(close_event));
            }
            
            
        } else {
            throw std::runtime_error("file error");
        }
        return std::nullopt;
    }

}   