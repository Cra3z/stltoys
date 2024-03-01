#include <gtest/gtest.h>
#include <iostream>
#include <stltoys/basic_string.h>

class string_test : public testing::Test {};

TEST_F(string_test, constructor) {
	ccat::string str1{"hello"};
	EXPECT_EQ(str1, "hello");
	ccat::string str2(5, 'a');
	EXPECT_EQ(str2, "aaaaa");
	ccat::string str3{'a', 'b', 'c'};
	EXPECT_EQ(str3, "abc");
}

TEST_F(string_test, insert) {
	ccat::string str{"hello"};
	str.insert(str.size(), "world");
	EXPECT_EQ(str, "helloworld");
	str.insert(5, 1, ' ');
	EXPECT_EQ(str, "hello world");
}

TEST_F(string_test, find) {
	ccat::string str{"hello world hello c++"};
	EXPECT_EQ(str.find("llo"), 2);
	EXPECT_EQ(str.rfind("el", 12), 1);
	EXPECT_EQ(str.find_first_of("ABab"), ccat::string::npos);
	EXPECT_EQ(str.find_first_not_of("hel"), 4);
	EXPECT_EQ(str.find_last_of('o'), 16);
	EXPECT_EQ(str.find_last_not_of(" c+lo"), 13);
}

TEST_F(string_test, resize_reserve_and_shrink_to_fit) {
	ccat::string str(15, '+');
	EXPECT_EQ(str, "+++++++++++++++");
	str.resize(20, '-');
	EXPECT_EQ(str, "+++++++++++++++-----");
	EXPECT_EQ(str.size(), 20);
	EXPECT_EQ(str.capacity(), 32);
	str.shrink_to_fit();
	EXPECT_EQ(str.capacity(), 20);
}

TEST_F(string_test, assign) {
	ccat::string str1{"hello"};
	ccat::string str2{str1};
	EXPECT_EQ(str2, "hello");
	str1.assign("world");
	EXPECT_EQ(str1, "world");
	str2.assign(str1);
	EXPECT_EQ(str2, "world");
	str2.assign(20, 'X');
	EXPECT_EQ(str2, "XXXXXXXXXXXXXXXXXXXX");
}

TEST_F(string_test, sub_string) {
	ccat::string str{"hello world"};
	EXPECT_EQ(str.substr(3, 4), "lo w");
}

TEST_F(string_test, out_string) {
	ccat::string str{"hello world"};
	std::cout << str;
}

auto main(int argc, char* argv[]) ->int {
	testing::InitGoogleTest(&argc, argv);
	
	return RUN_ALL_TESTS();
}