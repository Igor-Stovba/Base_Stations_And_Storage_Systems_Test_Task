#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <cstdint>
#include <string>
#include <cassert>
#include <optional>
#include <ctime>

namespace utils {

    class Table {
        int table_id_;
        std::string client_name_;

        /*
        Whether this table is busy by someone
        If so, person has client_name_
        */
        bool acquired_ = false;
        uint64_t earned_money_ = 0;
        uint64_t seconds_ = 0; // How long this table was acquired

    public:
        void set_table_id(int id) {
            // is called only once during init
            table_id_ = id;
        }

        bool get_acquired() const noexcept {
            return acquired_;
        }

        void set_acquired() noexcept {
            assert(acquired_ == false);
            acquired_ = true;
        }

        void unset_acquired() noexcept {
            assert(acquired_ == true);
            acquired_ = false;
        }

        void set_client_name(std::string client_name) {
            client_name_ = std::move(client_name);
        }

        std::string get_client_id() const noexcept {
            return client_name_;
        }

        void add_money(uint64_t delta) noexcept {
            earned_money_ += delta;
        }

        uint64_t get_money() const noexcept {
            return earned_money_;
        }

        void add_seconds(uint64_t delta) noexcept {
            seconds_ += delta;
        }

        uint64_t get_seconds() const noexcept {
            return seconds_;
        }

        void clear() noexcept {
            client_name_.clear();
            acquired_ = false;
            earned_money_ = 0;
            seconds_ = 0;
        }

    };


    class ClientStatus {
        std::optional<int> table_id_;
        std::optional<std::tm> start_sitting_time_;

    public:
        std::optional<int> get_maybe_table_id() const noexcept {
            return table_id_;
        }

        void set_table_id(int table_id) noexcept {
            table_id_ = table_id;
        }

        std::optional<std::tm> get_maybe_start_sitting_time() const noexcept {
            return start_sitting_time_;
        }

        void set_start_sitting_time(std::tm time) noexcept {
            start_sitting_time_ = std::move(time);
        }
    };

};

#endif