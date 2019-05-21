#pragma once

// Reads a config file containing lines with format "KEY VALUE".
// KEY and VALUE can be separated by any manner of whitespace.
// Only first two whitespace-separated tokens are read on each line.
// If '#' is anywhere in line, the line is ignored as a comment.
// In the case of repeated keys, the last value is used.

#include <algorithm> // transform, all_of
#include <cctype> // tolower, isdigit
#include <fstream>
#include <sstream> // stringstream
#include <string> // getline, stoul
#include <unordered_map>
#include <variant>

class Config
{
public:
    Config(const std::string& file_path);

    template <typename ValueType>
    ValueType get(const std::string& key) const;

    template <typename ValueType>
    ValueType get(const std::string& key, const ValueType& default_value) const;

private:
    using Variant = std::variant<bool, size_t, std::string>;
    std::unordered_map<std::string, Variant> m_dictionary;

};

inline Config::Config(const std::string& file_path)
{
    std::ifstream input_stream(file_path);
    std::string line;

    while (std::getline(input_stream, line))
    {
        if (line.find('#') != std::string::npos) // line is comment.
        {
            continue;
        }
        std::stringstream line_stream(line);

        std::string key;
        line_stream >> key;
        if (key.empty())
        {
            continue;
        }

        std::string value;
        line_stream >> value;
        if (value.empty())
        {
            continue;
        }

        // check boolean.
        auto lower {value};
        std::transform(std::begin(value), std::end(value), std::begin(lower), ::tolower);
        if (lower == "true")
        {
            m_dictionary[key] = true;
            continue;
        }
        else if (lower == "false")
        {
            m_dictionary[key] = false;
            continue;
        }

        // check size_t.
        if (std::all_of(std::cbegin(value), std::cend(value), ::isdigit))
        {
            m_dictionary[key] = std::stoul(value);
            continue;
        }

        m_dictionary[key] = value;
    }
}

template <typename ValueType>
ValueType Config::get(const std::string& key) const
{
    return std::get<ValueType>(m_dictionary.at(key));
}

template <typename ValueType>
ValueType Config::get(const std::string& key, const ValueType& default_value) const
{
    if (m_dictionary.find(key) == std::cend(m_dictionary))
    {
        return default_value;
    }
    return get<ValueType>(key);
}
