#ifndef INPUT_INFO_HPP_
#define INPUT_INFO_HPP_

#include <cstring>
#include <ctime>
#include <vector>
#include <optional>
#include <string>

namespace input {

    class Event {
    public:
        std::tm EventTime;
        int event_id; 

        /*
            str can be client_name, error_message
        */
        std::string str;
        std::optional<int> table_id;

        Event(std::tm et, int eid, std::string s)
            : EventTime(std::move(et)), event_id(eid), str(std::move(s)) {}

        Event(std::tm et, int eid, std::string s, int tid)
            : Event(std::move(et), eid, std::move(s)) {
                table_id = tid;
            }

        friend std::ostream& operator<<(std::ostream& os, const Event& event);
    };

    class InputInfo {
    public:
        size_t CountTables;
        std::tm OpenTime;
        std::tm CloseTime;
        size_t price;
        std::vector<Event> events;
    };

    /*
    Input parameters: 
        filename - name of file, which stores incoming events and other info
        input - object where we should store parsed data from file

    Return: 
        Optional string which is seen as first line where parsing error occured.
        Empty optional means that no error occured   
    */
    std::optional<std::string> ParseInput(const std::string filename, InputInfo& input);

}


#endif