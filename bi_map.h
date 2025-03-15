#pragma once
#include <unordered_map>

template <typename Key, typename Value>
class bi_map {
    std::unordered_map<Key, Value> forward;
    std::unordered_map<Value, Key> backward;

public:
    void insert(const Key key, const Value value)
    {
        forward[key] = value;
        backward[value] = key;
    }

    Value get_value(const Key& key)
    {
        return forward.at(key);
    }
    
    Key get_key(Value value)
    {
        return backward.at(value);
    }

    bool has_value(const Key& key)
    {
        return forward.contains(key);
    } 
    
    bool has_key(Value value)
    {
        return backward.contains(value);
    }
};