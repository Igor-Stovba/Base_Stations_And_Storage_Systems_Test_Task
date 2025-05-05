#include <gtest/gtest.h>
#include "../src/include/input_info.hpp"
#include <sstream>
#include <string>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

std::string make_tmp_file_with_content(std::stringstream& ss) {
    const std::string filename = "input.txt";
    std::ofstream file(filename);
    file << ss.rdbuf();
    file.close();
    return filename;
}

void delete_tmp_file_with_content(const std::string& fname) {
    fs::remove(fname);
}

std::string run_club_simulation(std::stringstream& ss) {
    std::string fname = make_tmp_file_with_content(ss);
    std::string cmd = "../club " + fname;

    testing::internal::CaptureStdout();
    std::system(cmd.c_str());
    std::string output = testing::internal::GetCapturedStdout();

    delete_tmp_file_with_content(fname);
    return output;
}

TEST(ClubTest, GivenTest) {
    std::stringstream ss;
    ss << "3\n"
    << "09:00 19:00\n"
    << "10\n"
    << "08:48 1 client1\n"
    << "09:41 1 client1\n"
    << "09:48 1 client2\n"
    << "09:52 3 client1\n"
    << "09:54 2 client1 1\n"
    << "10:25 2 client2 2\n"
    << "10:58 1 client3\n"
    << "10:59 2 client3 3\n"
    << "11:30 1 client4\n"
    << "11:35 2 client4 2\n"
    << "11:45 3 client4\n"
    << "12:33 4 client1\n"
    << "12:43 4 client2\n"
    << "15:52 4 client4\n";


    std::stringstream expected;
    expected << "09:00\n"
       << "08:48 1 client1\n"
       << "08:48 13 NotOpenYet\n"
       << "09:41 1 client1\n"
       << "09:48 1 client2\n"
       << "09:52 3 client1\n"
       << "09:52 13 ICanWaitNoLonger!\n"
       << "09:54 2 client1 1\n"
       << "10:25 2 client2 2\n"
       << "10:58 1 client3\n"
       << "10:59 2 client3 3\n"
       << "11:30 1 client4\n"
       << "11:35 2 client4 2\n"
       << "11:35 13 PlaceIsBusy\n"
       << "11:45 3 client4\n"
       << "12:33 4 client1\n"
       << "12:33 12 client4 1\n"
       << "12:43 4 client2\n"
       << "15:52 4 client4\n"
       << "19:00 11 client3\n"
       << "19:00\n"
       << "1 70 05:58\n"
       << "2 30 02:18\n"
       << "3 90 08:01\n";

    std::string output = run_club_simulation(ss);
    ASSERT_EQ(expected.str(), output);
}

TEST(ClubTest, DoubleEnterError) {
    std::stringstream ss;
    ss << "2\n10:00 18:00\n15\n"
       << "10:10 1 client1\n"
       << "10:15 1 client1\n"; 

    std::stringstream expected;
    expected << "10:00\n"
    << "10:10 1 client1\n"
    << "10:15 1 client1\n"
    << "10:15 13 YouShallNotPass\n"
    << "18:00\n"
    << "1 0 00:00\n"
    << "2 0 00:00\n";

    std::string output = run_club_simulation(ss);
    ASSERT_EQ(expected.str(), output);
}

TEST(ClubTest, TableBusy) {
    std::stringstream ss;
    ss << "2\n09:00 22:00\n5\n"
       << "09:10 1 user1\n"
       << "09:12 2 user1 1\n"
       << "09:15 1 user2\n"
       << "09:20 2 user2 1\n"; // busy already

    std::stringstream expected;
    expected << "09:00\n"
    << "09:10 1 user1\n"
    << "09:12 2 user1 1\n"
    << "09:15 1 user2\n"
    << "09:20 2 user2 1\n"
    << "09:20 13 PlaceIsBusy\n"
    << "22:00 11 user1\n"
    << "22:00\n"
    << "1 65 12:48\n"
    << "2 0 00:00\n";

    std::string output = run_club_simulation(ss);
    ASSERT_EQ(expected.str(), output);
}


TEST(ClubTest, RoundingMOney) {
    std::stringstream ss;
    ss << "1\n09:00 22:00\n10\n"
       << "09:00 1 tester\n"
       << "09:05 2 tester 1\n"
       << "09:10 4 tester\n";

    std::stringstream expected;
    expected << "09:00\n"
       << "09:00 1 tester\n"
       << "09:05 2 tester 1\n"
       << "09:10 4 tester\n"
       << "22:00\n"
       << "1 10 00:05\n";

    std::string output = run_club_simulation(ss);
    ASSERT_EQ(expected.str(), output);
}
