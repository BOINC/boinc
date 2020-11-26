#include <string>
#include <sstream>
#include <iostream>

#include "pretty_printer.h"

using std::string;

pretty_printer::pretty_printer(const int& decimal_spots, const int& spacing) : decimal_(decimal_spots), cur_spacing_(spacing), spacing_(spacing) {}

string pretty_printer::prettify(const bool& console_print) {
    std::ostringstream result;

    if (console_print) {
        this->cur_spacing_ -= spacing_;
        result << string(this->cur_spacing_, ' ');
    } else {
        result << "{\n" << string(this->cur_spacing_, ' ');
    }

    auto in_string = false;
    auto cur_count = 0;
    
    for (string::size_type i = 0; i < this->json_string_.size(); i++) {
        const auto cursor = this->json_string_.at(i);

        switch (cursor) {
        case '\\':
            result << cursor << this->json_string_.at(++i);
            break;

        case '#':
            cur_count++;

            if (cur_count == 3) {
                // Custom trigger to detect maps and vectors that were saved as strings
                cur_count = 0;

                auto current = result.str();

                result.clear();
                result.str("");

                if (in_string) {
                    // Remove ### and "
                    in_string ? in_string = false : in_string = true;
                    result << current.substr(0, current.size() - 3);

                } else {
                    // Remove ###, skip ", and trigger in_string
                    result << current.substr(0, current.size() - 2);
                    i++;
                }

                break;
            }

            result << cursor;
            break;

        case '"':
            in_string ? in_string = false : in_string = true;

            if (!console_print) {
                result << cursor;
            }

            break;

        case ':':
            result << cursor;

            if (!in_string) {
                result << " ";
            }

            break;

        case '[':
            if (!in_string) {
                this->cur_spacing_ += this->spacing_;

                if (console_print) {
                    result << "\n" << string(this->cur_spacing_, ' ');
                } else {
                    result << "[\n" << string(this->cur_spacing_, ' ');
                }

                break;
            }

            result << cursor;
            break;

        case ']':
            if (!in_string) {
                this->cur_spacing_ -= this->spacing_;
                if (!console_print) {
                    result << "\n" << string(this->cur_spacing_, ' ') << "]";
                }

                break;
            }

            result << cursor;
            break;

        case ',':
            if (!in_string) {
                if (console_print) {
                    result << "\n" << string(this->cur_spacing_, ' ');
                } else {
                    result << ",\n" << string(this->cur_spacing_, ' ');
                }

                break;
            }

            result << cursor;
            break;

        case '{':
            if (!in_string) {
                this->cur_spacing_ += this->spacing_;
                if (console_print) {
                    result << "\n" << string(this->cur_spacing_, ' ');
                } else {
                    result << "{\n" << string(this->cur_spacing_, ' ');
                }

                break;
            }

            result << cursor;
            break;

        case '}':
            if (!in_string) {
                this->cur_spacing_ -= this->spacing_;
                if (!console_print) {
                    result << "\n" << string(this->cur_spacing_, ' ') << "}";
                }

                break;
            }

            result << cursor;
            break;

        default:
            result << cursor;
        }
    }

    auto result_string = result.str();

    // Finalize and restore to start state
    if (console_print) {
        this->cur_spacing_ += spacing_;
    } else {
        result_string += "\n}";
    }

    return result_string;
}

int pretty_printer::insert(const pretty_printer& printer) {
    if (this->json_string_.empty()) {
        this->json_string_ = printer.raw();
    } else {
        this->json_string_ += "," + printer.raw();
    }

    return 0;
}

int pretty_printer::insert(const string& key, const string& value) {
    const auto size = snprintf(nullptr, 0, R"("%s":"%s")", key.c_str(), value.c_str()) + 1;
    auto buf = vector<char>(size);

    sprintf(buf.data(), R"("%s":"%s")", key.c_str(), value.c_str());

    if (!this->json_string_.empty()) {
        this->json_string_ += ",";
    }

    this->json_string_ += buf.data();

    return 0;
}

int pretty_printer::insert(const string& key, const char value) {
    const auto size = snprintf(nullptr, 0, R"("%s":"%c")", key.c_str(), value) + 1;
    auto buf = vector<char>(size);

    sprintf(buf.data(), R"("%s":"%c")", key.c_str(), value);

    if (!this->json_string_.empty()) {
        this->json_string_ += ",";
    }

    this->json_string_ += buf.data();

    return 0;
}

int pretty_printer::insert(const string& key, const double& value) {
    const auto size = snprintf(nullptr, 0, "\"%s\":%.*f", key.c_str(), this->decimal_, value) + 1;
    auto buf = vector<char>(size);

    sprintf(buf.data(), "\"%s\":%.*f", key.c_str(), this->decimal_, value);

    if (!this->json_string_.empty()) {
        this->json_string_ += ",";
    }

    this->json_string_ += buf.data();

    return 0;
}

int pretty_printer::insert(const string& key, const unsigned long long& value) {
    const auto size = snprintf(nullptr, 0, "\"%s\":%s", key.c_str(), std::to_string(value).c_str()) + 1;
    auto buf = vector<char>(size);
    
    sprintf(buf.data(), "\"%s\":%s", key.c_str(), std::to_string(value).c_str());

    if (!this->json_string_.empty()) {
        this->json_string_ += ",";
    }

    this->json_string_ += buf.data();

    return 0;
}

int pretty_printer::insert(const string& key, const long long& value) {
    const auto size = snprintf(nullptr, 0, "\"%s\":%s", key.c_str(), std::to_string(value).c_str()) + 1;
    auto buf = vector<char>(size);

    sprintf(buf.data(), "\"%s\":%s", key.c_str(), std::to_string(value).c_str());

    if (!this->json_string_.empty()) {
        this->json_string_ += ",";
    }

    this->json_string_ += buf.data();

    return 0;
}

int pretty_printer::insert(const string& key, const long& value) {
    const auto size = snprintf(nullptr, 0, "\"%s\":%s", key.c_str(), std::to_string(value).c_str()) + 1;
    auto buf = vector<char>(size);
    
    sprintf(buf.data(), "\"%s\":%s", key.c_str(), std::to_string(value).c_str());

    if (!this->json_string_.empty()) {
        this->json_string_ += ",";
    }

    this->json_string_ += buf.data();

    return 0;
}

int pretty_printer::insert(const string& key, const int& value) {
    const auto size = snprintf(nullptr, 0, "\"%s\":%s", key.c_str(), std::to_string(value).c_str()) + 1;
    auto buf = vector<char>(size);
    
    sprintf(buf.data(), "\"%s\":%s", key.c_str(), std::to_string(value).c_str());

    if (!this->json_string_.empty()) {
        this->json_string_ += ",";
    }

    this->json_string_ += buf.data();

    return 0;
}

int pretty_printer::insert(const string& key, const bool& value) {
    const auto size = snprintf(nullptr, 0, "\"%s\":false", key.c_str()) + 1;
    auto buf = vector<char>(size);

    if (value) {
        sprintf(buf.data(), "\"%s\":true", key.c_str());
    } else {
        sprintf(buf.data(), "\"%s\":false", key.c_str());
    }

    if (!this->json_string_.empty()) {
        this->json_string_ += ",";
    }

    this->json_string_ += buf.data();
    return 0;
}

int pretty_printer::insert(const string& key) {
    const auto size = snprintf(nullptr, 0, "\"%s\":null", key.c_str()) + 1;
    auto buf = vector<char>(size);

    sprintf(buf.data(), "\"%s\":null", key.c_str());

    if (!this->json_string_.empty()) {
        this->json_string_ += ",";
    }

    this->json_string_ += buf.data();

    return 0;
}

int pretty_printer::clear() {
    this->json_string_ = "";
    this->cur_spacing_ = this->spacing_;

    return 0;
}