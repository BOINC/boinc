#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iostream>

using std::string;
using std::vector;

class pretty_printer {
    string json_string_ = "";
    int decimal_;

    int cur_spacing_;
    const int spacing_;

public:
    explicit pretty_printer(const int& decimal_spots = 2, const int& spacing = 3);
    ~pretty_printer() = default;
    pretty_printer(const pretty_printer& printer) = default;

    template <typename K, typename V>
    explicit pretty_printer(std::map<K, V>& map, const int& decimal_spots = 2, const int& spacing = 3) : decimal_(decimal_spots), cur_spacing_(spacing), spacing_(spacing) {
        const string prettified = inner_prettify(map);
        this->json_string_ = prettified.substr(4, prettified.size() - 8);
    }
    
    string prettify(const bool& console_print = false);
    string raw() const { return this->json_string_; }

    int insert(const pretty_printer& printer);

    int insert(const string& key, const string& value);

    int insert(const string& key, const char* value) { return this->insert(key, string(value)); }

    int insert(const string& key, const char value);

    int insert(const string& key, const pretty_printer& printer) { return this->insert(key, string("###{" + printer.raw() + "}###")); }

    int insert(const string& key, const double& value);

    int insert(const string& key, const unsigned long long& value);
    
    int insert(const string& key, const long long& value);

    int insert(const string& key, const long& value);

    int insert(const string& key, const int& value);

    int insert(const string& key, const bool& value);

    int insert(const string& key);

    int clear();

    int change_spacing(const int& factor = 1) noexcept {
        this->cur_spacing_ += this->spacing_ * factor;

        if (this->cur_spacing_ < 0) {
            this->cur_spacing_ = 0;
            return 1;
        }

        return 0;
    }

    int change_decimal(const int& decimal) noexcept {
        this->decimal_ = decimal;
        return 0;
    }

private:
    template <typename T>
    static string inner_prettify(vector<T> inputs) {
        std::ostringstream result;
        result << "###[";

        for (auto input : inputs) {
            result << inner_prettify(input) << ",";
        }

        const auto result_string = result.str();

        if (result_string.size() < 2) {
            return "";
        }

        return result_string.substr(0, result_string.size() - 1) + "]###";
    }

    template <typename K, typename V>
    static string inner_prettify(std::map<K, V> map) {
        auto result = std::make_unique<pretty_printer>();

        for (auto& element : map) {
            result->insert(inner_prettify(element.first), inner_prettify(element.second));
        }

        std::ostringstream final;
        final << "###{" << result->json_string_ << "}###";

        return final.str();
    }

    template<typename T>
    static T inner_prettify(std::unique_ptr<T>& value) {
        return value.get();
    }

    template<typename T>
    static T inner_prettify(const T& value) noexcept {
        return value;
    }

    static string inner_prettify(const pretty_printer& printer) {
        return "{" + printer.raw() + "}";
    }

public:
    template <typename T>
    static string prettify(vector<T> inputs) {
        return inner_prettify(inputs);
    }

    template <typename K, typename V>
    int insert(const string& key, const std::map<K, V>& map) {
        this->insert(key, inner_prettify(map));

        return 0;
    }

    template <typename T>
    int insert(const string& key, const vector<T>& vec) {
        this->insert(key, inner_prettify(vec));

        return 0;
    }


    template <typename K, typename V>
    static string prettify(std::map<K, V>& map) {
        return pretty_printer(map).raw();
    }

    template<typename T>
    static T prettify(const T& value) noexcept {
        return value;
    }
};
