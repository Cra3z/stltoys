#include <iostream>
#include <stltoys/vector.h>
#include <stltoys/array.h>
#include <gtest/gtest.h>

static_assert(std::ranges::contiguous_range<ccat::vector<int>>);

class test_vector : public testing::Test {};

TEST_F(test_vector, constructors) {
	ccat::array arr{1, 2, 3, 4, 5, 6, 7, 8};
	ccat::vector vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	ccat::vector<int> vec0(3, 10);
	ccat::vector vec1{10, 10, 10};
	EXPECT_EQ(vec0, vec1);
	ccat::vector vec2{ccat::from_range, arr};
	EXPECT_EQ(vec2, (ccat::vector{1, 2, 3, 4, 5, 6, 7, 8}));
}

TEST_F(test_vector, insert) {
	ccat::vector vec{1, 2, 3};
	vec.insert(std::ranges::next(std::ranges::begin(vec)), 0);
	EXPECT_EQ(vec, (ccat::vector{1, 0, 2, 3}));
	vec.insert(std::ranges::next(std::ranges::begin(vec)), 3, 6);
	EXPECT_EQ(vec, (ccat::vector{1, 6, 6, 6, 0, 2, 3}));
	vec.insert(std::ranges::end(vec), {7, 8, 9});
	EXPECT_EQ(vec, (ccat::vector{1, 6, 6, 6, 0, 2, 3, 7, 8, 9}));
	vec.push_back(114);
	EXPECT_EQ(vec, (ccat::vector{1, 6, 6, 6, 0, 2, 3, 7, 8, 9, 114}));
	int i = 514;
	vec.push_back(i);
	EXPECT_EQ(vec, (ccat::vector{1, 6, 6, 6, 0, 2, 3, 7, 8, 9, 114, 514}));
}

TEST_F(test_vector, erase) {
	ccat::vector vec{1, 2, 3, 5, 6, 7, 8};
	vec.erase(
		std::ranges::next(std::ranges::begin(vec), 2),
		std::ranges::next(std::ranges::begin(vec), 5)
	);
	EXPECT_EQ(vec, (ccat::vector{1, 2, 7, 8}));

	ccat::vector vec2{1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	vec2.erase(
		std::ranges::next(std::ranges::begin(vec2), 4),
		std::ranges::prev(std::ranges::end(vec2), 5)
	);
	EXPECT_EQ(vec2, (ccat::vector{1, 2, 3, 4, 5, 6, 7, 8, 9}));

	erase_if(vec2, [](int i) ->bool {
		return i % 2 == 0;
	});
	EXPECT_EQ(vec2, (ccat::vector{1, 3, 5, 7, 9}));
}

TEST_F(test_vector, foreach) {
	ccat::vector vec{1, 2, 3, 4, 5, 6, 7, 8};
	char sep[3]{'\0', ' ', '\0'};
	for (const auto& i : vec) {
		std::cout << sep << i;
		if (sep[0] == '\0') [[unlikely]] sep[0] = ',';
	}
	std::cout << '\n';
	sep[0] = '\0';

	for (auto it = std::ranges::rbegin(vec); it != std::ranges::rend(vec); ++it) {
		std::cout << sep << *it;
		if (sep[0] == '\0') [[unlikely]] sep[0] = ',';
	}
	std::cout << '\n';
	sep[0] = '\0';

	std::ranges::for_each(vec, [](int& i) {
		i *= 2;
	});
	EXPECT_EQ(vec, (ccat::vector{2, 4, 6, 8, 10, 12, 14, 16}));
}

TEST_F(test_vector, assign) {
	ccat::vector vec1{1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27};
	ccat::vector vec2{1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26};
	vec1 = vec2;
	EXPECT_EQ(vec1, vec2);
	vec2 = {4, 5, 6, 7, 8, 9};
	EXPECT_EQ(vec2, (ccat::vector{4, 5, 6, 7, 8, 9}));
}

TEST_F(test_vector, ranges_and_views) {
	ccat::vector vec{1, 2, 3, 4, 5, 6, 7, 8, 9};
	for (const auto& i :
		vec | std::views::filter([] (int i) { return i % 2 == 0;})
			| std::views::transform([] (int i) {return i * 2;})
	) {
		std::cout << i << ' ';
	}
}

auto main(int argc, char* argv[]) ->int {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}