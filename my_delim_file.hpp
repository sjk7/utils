// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
#ifndef DELIM_FILE_HPP
#define DELIM_FILE_HPP // NOLINT(clang-diagnostic-unused-macros)

#include "my_utils.hpp"
#include <unordered_map>
#include <string_view>
#include <cstdint>
#include <unordered_set>

namespace my {

using string_map_type
    = my::utils::strings::case_insensitive_map<std::string_view,
        std::string_view>;

inline string_map_type make_unique_values(
    const std::vector<std::string_view>& v) {

    string_map_type retval{};
    for (const auto& s : v) {
        retval.insert({s, s});
    }
    return retval;
}

struct DelimitedTextReader {
    using rows_type = std::vector<std::string_view>;
    using uid_type = uint32_t;

    struct column {
        rows_type values;
        std::string name;
        size_t index;
    };
    using columns_type = std::vector<column>;
    using column_keys_type = std::unordered_map<std::string, column*>;

    static inline auto unique_column_values(const column& col) {
        return make_unique_values(col.values);
    }

    inline column* find_column(const std::string& colName) {
        std::string uFind = my::utils::strings::to_upper(colName);
        for (auto& c : m_columns) {
            std::string uFound = my::utils::strings::to_upper(c.name);
            if (uFound.size() == uFind.size()) {
                return &c;
            }
        }
        return nullptr;
    }

    DelimitedTextReader(
        std::string_view filepath, std::string_view delim = "\t")
        : DelimitedTextReader(filepath, delim, false) {}

    private:
    DelimitedTextReader(
        bool dummy, std::string_view filepath, std::string_view delim)
        : m_filepath(filepath), m_delim(delim) {}

    // we call a base constructor here, to make sure the class is fully formed
    // if we fail to parse the file.
    DelimitedTextReader(
        std::string_view filepath, std::string_view delim, bool dummy)
        : DelimitedTextReader(dummy, filepath, delim) {
        (void)dummy;
        const auto parse_result = parse();
        if (parse_result != 0) {
            THROW_ERROR("Error parsing file: ", filepath,
                std::string_view{m_serr}, "Error code: ", parse_result);
        }
    }
    columns_type m_columns;
    column_keys_type column_keys;
    std::string m_sdata;
    std::string m_filepath;
    std::string m_delim;
    std::string m_serr;

    template <typename T>
    void make_columns(const std::vector<T>& fields, size_t lines) {

        for (const auto& field : fields) {
            struct column c {
                {}, std::string(field), m_columns.size()
            };
            auto& col = m_columns.emplace_back(std::move(c));
            col.values.resize(lines - 1);
        }
    }

    // have seen some artist (Celine dion in my case) start with an ETX
    // character or something stupid
    std::string_view sanitize(std::string_view what) {
        static constexpr const char SPACE = ' ';
        const char* ptr = what.data();

        for (const char c : what) {
            if (static_cast<int>(c) < static_cast<int>(SPACE)) {
                ++ptr;
            } else {
                break; // just checking for leading bad chars
            }
        }
        const char* end = what.data() + what.size();
        return std::string_view(ptr, end - ptr);
    }

    int parse() {
        std::fstream f;
        auto ec = utils::file_open_and_read_all(m_filepath, m_sdata);
        if (ec.code() != std::error_code()) {
            throw ec; // NOLINT(misc-throw-by-value-catch-by-reference)
        }

        auto lines = utils::strings::split<std::string_view>(m_sdata, "\r\n");
        if (lines.empty()) {
            lines = utils::strings::split<std::string_view>(m_sdata, "\n");
        }
        size_t line_counter = 0;
        size_t row = 0;

        for (const auto& line : lines) {

            auto fields = utils::strings::split<std::string_view>(line, "\t");
            if (line_counter == 0) {
                make_columns(fields, lines.size());
                ++line_counter;
                continue;
            }

            size_t col = 0;
            int ctr = 0;

            for (const auto& field : fields) {
                auto& c = m_columns.at(col++);
                auto& v = c.values;
                v.at(row) = sanitize(field);
                ++ctr;
            }

            if (fields.size() != m_columns.size()) {
                std::cerr << "Incorrect number of fields"
                          << " at line:" << line_counter << std::endl;
                std::cerr << std::endl
                          << "--------------------------" << std::endl;
            }
            ++row;
            ++line_counter;
        }

        for (auto& col : m_columns) {
            column_keys[col.name] = &col;
        }

        return 0;
    }

    public:
    const columns_type& columns() const noexcept { return m_columns; }
    const std::string& last_error() const noexcept { return m_serr; }
    size_t rowcount() const noexcept {
        if (m_columns.empty()) return 0;
        return m_columns[0].values.size();
    }
    const struct column* column(std::string_view colname) const noexcept {
        auto it = this->column_keys.find(std::string(colname));
        if (it == column_keys.end()) return nullptr;
        return it->second;
    }

    const struct column* column(size_t index) {
        assert(index <= this->columns().size());
        return &this->m_columns[index];
    }

    // get an *approximation* of the length of the row data
    size_t get_row_data_len(
        size_t index, std::string_view delim, bool quoted) const noexcept {
        size_t len = 0;
        for (const auto& col : m_columns) {
            len += col.values[index].size();
        }
        len += m_columns.size() * delim.size();
        if (quoted) {
            len += m_columns.size() * 2;
        }
        return len - delim.size();
    }

    void rowData(std::string& out, size_t row_index, bool clear_out = true,
        std::string_view delim = ",", bool quoted = true, bool escaped = true) {

        if (clear_out) out.clear();
        const auto col_count = m_columns.size();
        if (col_count == 0) return;

        const auto& vals = m_columns[0].values;
        if (vals.empty()) return;
        const auto row_count = vals.size();
        assert(row_index < row_count);

        for (const auto& col : m_columns) {
            if (quoted) {
                out.append("'");
            }
            if (escaped)
                out.append(utils::strings::escape(col.values[row_index]));
            else
                out.append(col.values[row_index]);

            if (quoted) {
                out.append("'");
            }
            if (col.index < col_count - 1) {
                out.append(delim);
            }
        } // for
    }

    // ReSharper disable once CppInconsistentNaming
    void rowData(const bool clear_out, std::stringstream& out,
        const size_t row_index, const std::string_view delim = ",",
        const bool quoted = true, const bool escaped = true) const noexcept {
        if (clear_out) {
            static std::string empty_string;
            out.clear();
            out.str(empty_string);
        }
        const auto col_count = m_columns.size();
        if (col_count == 0) return;

        const auto& vals = m_columns[0].values;
        if (vals.empty()) return;
        const auto row_count = vals.size();
        assert(row_index < row_count);

        for (const auto& col : m_columns) {
            if (quoted) {
                out << "'";
            }
            if (escaped)
                out << utils::strings::escape(col.values[row_index]);
            else
                out << col.values[row_index];

            if (quoted) {
                out << "'";
            }
            if (col.index < col_count - 1) {
                out << delim;
            }
        } // for

        return;
    }
};
} // namespace my

#endif // DELIM_FILE_HPP
