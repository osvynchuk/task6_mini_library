// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "gtest/gtest.h"
#include "my_algo.h"
#include <exception>

#include <iostream>
#include <fstream>
#include <string>
#include <functional>

using namespace my_algo;

class FileDataProvider : public DataProvider<std::string, int> {
public:
    FileDataProvider(const std::string& fname) : file (fname) {}
    FileDataProvider(const char * fname) : file (fname) {}

    bool hasNext() const override {
        if (file) return true;
        return false;
    }

    std::pair<std::string, int> next () override {
        std::string word;
        if (file) file >> word;
        else throw std::runtime_error("eof");
        return std::make_pair(word, 1);
    }
private:
    std::ifstream file;
};

class WordCountReducer : public Reducer<std::string, int> {
public:
    void reduce (std::map<std::string, std::vector<int>>& data_set) {
        for (auto& entry : data_set) {
            m_reduced_map[entry.first] = entry.second.size();
        }
    }
};


TEST(MyAlgoTest, MapReduceWordCount) {

    FileDataProvider provider1("../task6_mini_library/data/data1.txt");
    FileDataProvider provider2("../task6_mini_library/data/data2.txt");
    FileDataProvider provider3("../task6_mini_library/data/data3.txt");

    auto mergeOp = [] (int lhs, int rhs) {
        return lhs + rhs;
    };

    MapReduce<std::string, int, WordCountReducer, decltype(mergeOp)> map_reduce(
        { &provider1, &provider2, &provider3 }, mergeOp);

    map_reduce.run();

    auto dataset = map_reduce.getResult();
    EXPECT_FALSE(dataset.empty());
    EXPECT_EQ(dataset["a"], 31);
    EXPECT_EQ(dataset["and"], 16);
    EXPECT_EQ(dataset["class"], 29);
    EXPECT_EQ(dataset["member"], 25);
}


class StringRangeDataProvider : public DataProvider<int, std::string> {
public:
    StringRangeDataProvider(std::vector<std::string>::iterator beg, std::vector<std::string>::iterator end)
        : m_curr (beg), m_end(end) {
    }

    bool hasNext() const override {
        if (m_curr != m_end) return true;
        return false;
    }

    std::pair<int, std::string> next () override {
        auto str = *m_curr++;
        return std::make_pair(str.size(), str);
    }
private:
    std::vector<std::string>::iterator m_curr;
    std::vector<std::string>::iterator m_end;
};

class StringSizeReducer : public Reducer<int, std::string> {
public:
    void reduce (std::map<int, std::vector<std::string>>& data_set) {
        for (auto& entry : data_set) {
            m_reduced_map[entry.first] = entry.second.at(0);
        }
    }
};


TEST(MyAlgoTest, MapReduceLargestString) {

    auto mergeOp = [] (std::string& lhs, std::string& rhs) {
        return lhs;
    };

    std::vector<std::string> str_array {"a","ab", "abc", "abcd", "bcda", "abcdefgh", "abcdefgh", "abc", "class", "member", "polymorphism"};
    StringRangeDataProvider str_provider1(std::begin(str_array), std::begin(str_array) + 6);
    StringRangeDataProvider str_provider2(std::begin(str_array) + 6, std::end(str_array));

    MapReduce<int, std::string, StringSizeReducer, decltype(mergeOp)> map_reduce(
    { &str_provider1, &str_provider2 }, mergeOp);

    map_reduce.run();

    auto dataset = map_reduce.getResult();
    EXPECT_FALSE(dataset.empty());
    EXPECT_EQ((--dataset.end())->first, 12);
    EXPECT_EQ((--dataset.end())->second, "polymorphism");
}

class ArrayRangeDataProvider : public DataProvider<int, int> {
public:
    ArrayRangeDataProvider(int* beg, int* end)
        : m_curr (beg), m_end(end) {
    }

    bool hasNext() const override {
        if (m_curr != m_end) return true;
        return false;
    }

    std::pair<int, int> next () override {
        auto ret = std::make_pair(*m_curr, *m_curr);
        m_curr++;
        return ret;
    }
private:
    int* m_curr = nullptr;
    int* m_end = nullptr;
};

class SquareSumReducer : public Reducer<int, int> {
public:
    void reduce (std::map<int, std::vector<int>>& data_set) {
        for (auto& entry : data_set) {
            if (!m_reduced_map.count(entry.first)) m_reduced_map[entry.first] = 0;
            for (auto e : entry.second)
                m_reduced_map[entry.first] += e * e;
        }
    }
};

TEST(MyAlgoTest, MapReduceSquareSum) {

    std::vector<int> int_array {1, 2, 3, 4, 4, 55, 66, 77, 116, 567, 123, 77, 7, 8, 9, 10};
    ArrayRangeDataProvider int_arr_provider(std::begin(int_array).base(), std::begin(int_array).base() + 6);
    ArrayRangeDataProvider int_arr_provider2(std::begin(int_array).base() + 6, std::end(int_array).base());

    auto mergeOp = [] (int lhs, int rhs) {
        return lhs + rhs;
    };

    MapReduce<int, int, SquareSumReducer, decltype(mergeOp)> map_reduce(
        { &int_arr_provider, &int_arr_provider2 }, mergeOp);
    map_reduce.run();

    auto dataset = map_reduce.getResult();
    EXPECT_FALSE(dataset.empty());
    EXPECT_EQ(dataset[4], 32);
    EXPECT_EQ(dataset[77], 11858);
}

TEST(MyAlgoTest, CopyPodTest) {

    std::vector<int> vi {1,2,3,4,5};
    std::vector<int> dest;

    dest.resize(vi.size());

    my_copy(vi.begin(), vi.end(), dest.begin());
    EXPECT_EQ(dest, vi);

    std::list<int> exp_list{1,2,3,4,5};
    std::list<int> dest2;
    dest2.resize(exp_list.size());

    my_copy(exp_list.begin(), exp_list.end(), dest2.begin());
    EXPECT_EQ(dest2, exp_list);
}

TEST(MyAlgoTest, CopyUserObjectsTest) {

    class Foo {
    public:
        Foo (): x(0) {}
        Foo (int xx):x(xx){}
        virtual void foo(){}
        bool operator == (const Foo& rhs) const { return x == rhs.x; }
        int x = 0;
    };

    std::vector<Foo> vf {Foo{1}, Foo{3}, Foo{5}};
    std::vector<Foo> dest;
    dest.resize(vf.size());

    my_copy(vf.begin(), vf.end(), dest.begin());
    EXPECT_EQ(dest, vf);
}

TEST(MyAlgoTest, TransformTest) {
    std::vector<int> vi {1,2,3,4,5,10};
    std::vector<int> dest;
    dest.resize(vi.size());

    my_transform(vi.begin(), vi.end(), dest.begin(), [](int i){ return i * i; });
    EXPECT_EQ(dest, (std::vector<int>{1,4,9,16,25,100}));


    dest.clear();
    dest.resize(vi.size());
    std::list<int> li {2,4,6,8,10,12,14};
    my_transform(vi.begin(), vi.end(), li.begin(), dest.begin(), [](int i, int ii){ return i + ii; });
    EXPECT_EQ(dest, (std::vector<int>{3,6,9,12,15,22}));
}
