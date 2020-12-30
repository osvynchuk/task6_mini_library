#ifndef MY_ALGO_H
#define MY_ALGO_H

#include <type_traits>
#include <iterator>
#include <cstring>
#include <vector>
#include <map>
#include <list>
#include <future>


namespace my_algo {

template <typename InIter, typename OutIter,
          std::enable_if_t<std::is_pod<typename std::iterator_traits<InIter>::value_type>::value, int> = 0>
OutIter copy_impl(InIter first, InIter last, OutIter dest_first, std::random_access_iterator_tag, std::random_access_iterator_tag) {
    auto count = last - first;
    std::memcpy(&*dest_first, &*first, count * sizeof(typename std::iterator_traits<InIter>::value_type) );
    return dest_first + count;
}

template <typename InIter, typename OutIter>
OutIter copy_impl(InIter first, InIter last, OutIter dest_first, std::forward_iterator_tag, std::forward_iterator_tag) {
    while (first != last) {
        *dest_first = *first;
        ++first;
        ++dest_first;
    }
    return dest_first;
}

template <typename InIter, typename OutIter>
OutIter my_copy (InIter first, InIter last, OutIter dest_first) {
    using  category_in = typename std::iterator_traits<InIter>::iterator_category ;
    using category_out = typename std::iterator_traits<OutIter>::iterator_category ;
    return copy_impl(first, last, dest_first, category_in(), category_out());
}

template <typename InIter, typename OutIter, typename UnaryOperation >
OutIter my_transform (InIter first, InIter last, OutIter dest_first, UnaryOperation unary_op) {
    while (first != last) {
        *dest_first = unary_op(*first);
        ++first;
        ++dest_first;
    }
    return dest_first;
}

template <typename InIter1, typename InIter2, typename OutIter, typename BinaryOp>
OutIter my_transform (InIter1 first1, InIter1 last1, InIter2 first2, OutIter dest_first, BinaryOp binary_op) {
    while (first1 != last1) {
        *dest_first = binary_op(*first1, *first2);
        ++first1;
        ++first2;
        ++dest_first;
    }
    return dest_first;
}


template <typename Key, typename Val>
class DataProvider {
public:
    virtual bool hasNext() const = 0;
    virtual std::pair<Key, Val> next () = 0;
    virtual ~DataProvider(){}
};

template <typename Key, typename Val>
class Mapper {
public:
    Mapper (DataProvider<Key, Val>& provider) : m_provider(provider) {
    }

    void map () {
        while (m_provider.hasNext()) {
            coll.push_back(m_provider.next());
        }
    }

    auto& getEntries () {
        return coll;
    }
private:
    DataProvider<Key, Val>& m_provider;
    std::vector<std::pair<Key, Val>> coll;
};

template <typename Key, typename Val>
class Shuffler {
public:
    Shuffler (Mapper<Key, Val>& mapper) : m_mapper(mapper) {
    }

    void run () {
        auto& entries = m_mapper.getEntries();

        for (auto& entry : entries) {
            m_mapof_sets[entry.first].push_back(entry.second);
        }
    }

    auto& getDataSet () {
        return m_mapof_sets;
    }
private:
    Mapper<Key, Val>& m_mapper;
    std::map<Key, std::vector<Val>> m_mapof_sets;
};

template <typename Key, typename Val>
class Reducer {
public:
    virtual void reduce (std::map<Key, std::vector<Val>>& data_set) = 0;

    auto& getDataSet () {
        return m_reduced_map;
    }
    virtual ~Reducer(){}
protected:
    std::map<Key, Val> m_reduced_map;
};


template <typename Key, typename Val, typename Reducer, typename mergeOp>
class MapReduce {
public:

    MapReduce(const std::list<DataProvider<Key, Val>*>& providers, mergeOp merge_op) : m_providers(providers), m_merge_op(merge_op) {
    }

    void run () {

        auto reduceTask = [] (DataProvider<Key, Val>* provider_p) {
            Mapper<Key, Val> mapper(*provider_p);
            mapper.map();

            Shuffler<Key, Val> shuffler(mapper);
            shuffler.run();

            Reducer reducer;
            reducer.reduce(shuffler.getDataSet());

            return reducer.getDataSet();
        };

        auto combineTask = [this] (std::future<std::map<Key, Val>>& f) {
            for (auto& entry : f.get()) {
                if (m_result_set.count(entry.first))
                    m_result_set[entry.first] = m_merge_op(m_result_set[entry.first], entry.second);
                else
                    m_result_set[entry.first] = entry.second;
            }
        };

        while (!m_providers.empty()) {
            auto f1 = std::async(std::launch::async, reduceTask, m_providers.back());
            m_providers.pop_back();

            std::future<std::map<Key, Val>> f2;
            if (!m_providers.empty()) {
                f2 = std::async(std::launch::async, reduceTask, m_providers.back());
                m_providers.pop_back();
            }

            f1.wait();
            if (f2.valid()) f2.wait();

            combineTask(f1);
            if (f2.valid()) combineTask(f2);
        }
    }

    auto& getResult () {
        return m_result_set;
    }

private:
    std::list<DataProvider<Key, Val>*> m_providers;
    std::map<Key, Val> m_result_set;
    mergeOp m_merge_op;
};

}



#endif // MY_ALGO_H
