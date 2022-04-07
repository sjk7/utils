#ifndef SQLITE_HPP
#define SQLITE_HPP
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <map>
#include <optional>
#include <iostream>
#include <algorithm>
#include <cstring>
#include "../utils/utils.hpp"

struct sqlite {

    using uid_type = uint32_t;

    struct column_t {
        column_t(std::string_view name, int size_hint) : m_name(name) {
            m_values.reserve(size_hint);
        }

        std::string m_name;

        std::vector<std::string> m_values;
        std::vector<uid_type> m_uids;
        const auto& name() const noexcept { return m_name; }
        const auto& values() const noexcept { return m_values; }
    };

    using column_type = column_t;

    using columns_type = std::unordered_map<std::string, column_t,
        utils::strings::string_hash_transparent_ci,
        utils::strings::transparent_string_ci_equal<>>;
    columns_type m_columns;

    const columns_type& columns() const noexcept { return m_columns; }

    struct row_t {
        static constexpr const uint32_t invalid_row_id
            = static_cast<uint32_t>(-1);

        uint32_t id = invalid_row_id;
        // pointing to the values in the columns
        std::vector<std::string_view> values;
        row_t(uint32_t id, const columns_type& cols) {
            values.reserve(cols.size());
            this->id = id;
            size_t i = 0;
            for (const auto& c : cols) {
                const auto& vals = c.second.values();
                const auto& val = vals[i];
                values.push_back(std::string_view(val));
            }
        }
    };

    using rows_t = std::unordered_map<uint32_t, row_t>;
    rows_t m_rows;

    using cols_ptr = std::optional<const column_t*>;
    const cols_ptr column(std::string_view name) const noexcept {
        auto it = m_columns.find(name);
        if (it != m_columns.end()) {
            const column_t* ptr = &(it->second);
            return ptr;
        }
        return nullptr;
    }

    uint32_t uid_from_index(size_t rowIndex) const {
        auto pcol = m_psort_col;
        if (!pcol) {
            auto v = this->m_columns.find("ID");
            assert(v != m_columns.cend());
            const column_t* col = &v->second;
            pcol = (column_t*)col;
        }
        assert(pcol);
        return pcol->m_uids[rowIndex];
    }
    const std::vector<std::string_view>& getRow(size_t rowIndex) const {
        std::vector<std::string> ret;
        assert(!m_rows.empty());
        const auto uid = uid_from_index(rowIndex);
        const auto& rw = m_rows.find(uid);
        assert(rw != m_rows.end());
        auto& vals = rw->second.values;
        return vals;
    }

    private:
    static void errorLogCallback(void* pArg, int iErrCode, const char* zMsg) {
        sqlite* pthis = (sqlite*)pArg;
        if (zMsg) {
            pthis->m_slasterror = zMsg;
        }
        pthis->m_lasterror = iErrCode;
        if (pthis->errors_to_stderr())
            fprintf(stderr, "C++ sqlite error, (%d) %s\n", iErrCode, zMsg);
    }
    enum class modes { none, getting_row_count, selecting, inserting };

    modes m_mode{modes::none};
    size_t m_row_count;

    public:
    sqlite(std::string_view filepath) {
        int x = sqlite3_config(SQLITE_CONFIG_LOG, errorLogCallback, this);
        assert(x == SQLITE_OK);
        (void)x;

        m_lasterror = sqlite3_open(filepath.data(), &m_handle);
        if (m_lasterror == SQLITE_OK) {
            m_path = filepath;

        } else {
            throw std::runtime_error(sqlite3_errstr(m_lasterror));
        }
    }
    ~sqlite() {
        if (m_handle) {
            sqlite3_close(m_handle);
            m_handle = nullptr;
        }
    }
    sqlite(const sqlite&) = delete;
    sqlite& operator=(const sqlite&) = delete;
    bool errors_to_stderr() const noexcept { return m_errs_to_stderr; }
    void errors_to_stderr(bool b) { m_errs_to_stderr = b; }
    const std::string& path() const noexcept { return m_path; }
    static std::string_view error_string(int errcode) noexcept {
        return sqlite3_errstr(errcode);
    }
    const std::string_view last_error_string() const noexcept {
        return m_slasterror;
    }
    int last_error_code() const noexcept { return m_lasterror; }
    static bool string_contains(
        std::string_view haystack, std::string_view needle) {
        return haystack.find(needle) >= 0;
    }

    int exec(std::string_view sql) {
        char* errmsg{nullptr};

        if (sql.find("INSERT") == 0) {
            // inserting data.
            this->m_mode = modes::inserting;
            m_lasterror
                = sqlite3_exec(m_handle, sql.data(), callback, this, &errmsg);
        } else if (sql.find("SELECT COUNT") == 0) {
            this->m_mode = modes::getting_row_count;
            m_lasterror
                = sqlite3_exec(m_handle, sql.data(), callback, this, &errmsg);
        } else if (sql.find("SELECT") == 0) {
            this->m_mode = modes::selecting;
            this->m_row_index = 0;
            this->m_rows.clear();
            m_columns.clear();
            m_slasterror.clear();
            m_lasterror = sqlite3_exec(
                m_handle, sql.data(), select_callback, this, &errmsg);
        } else {
            m_lasterror
                = sqlite3_exec(m_handle, sql.data(), callback, this, &errmsg);
        }

        if (m_slasterror.empty()) set_last_error(&errmsg);
        this->m_mode = modes::none;
        return m_lasterror;
    }

    size_t row_count(std::string_view table) {
        std::string sql{"SELECT COUNT(*) FROM "};
        sql += table;
        m_mode = modes::getting_row_count;
        auto ret = this->exec(sql);
        if (ret != SQLITE_OK) {
            return 0;
        }

        return m_row_count;
    }
    template <typename CB> int exec(std::string_view sql, CB&& cb) {
        /*/
// int exec(sqlite3 *,                                      // An open
database
//       const char *sql,                                // SQL to be
evaluated
//       int (*callback)(void *, int, char **, char **), // Callback
function
//       void *,                                         // 1st argument to
//       callback char **errmsg                                   // Error
msg
//       written here
//);
/*/
        char* errmsg{nullptr};
        m_lasterror = sqlite3_exec(m_handle, sql.data(), cb, this, &errmsg);
        set_last_error(&errmsg);

        return m_lasterror;
    }

    auto last_insert_id() const noexcept {
        return sqlite3_last_insert_rowid(m_handle);
    }
    inline static int callback(
        void* vp, int count, char** data1, char** data2) {
        auto* pthis = (sqlite*)vp;
        std::cout << "pthis: " << pthis << std::endl;
        std::cout << "count: " << count << std::endl;
        std::cout << "data1: " << *data1 << std::endl;
        std::cout << "data2: " << *data2 << std::endl << std::endl;
        if (pthis->m_mode == modes::getting_row_count) {
            pthis->m_row_count = strtoul(*data1, nullptr, 10);
        }
        return SQLITE_OK;
    }
    size_t m_row_index = 0;
    size_t m_uid_col = 0;
    static inline int select_callback(
        void* vp, int count, char** data, char** columns) {
        int idx;
        auto* pthis = (sqlite*)vp;
        assert(pthis->m_mode == modes::selecting);

        auto& cols = pthis->m_columns;
        uint32_t this_row_uid = row_t::invalid_row_id;
        if (pthis->m_row_index == 0) {
            for (idx = 0; idx < count; idx++) {
                std::string_view cname(columns[idx]);
                std::string_view cvalue(data[idx]);
                auto it = cols.find(cname);
                if (it == cols.end()) {
                    cols.insert({std::string(cname),
                        column_t(cname, pthis->m_row_count)});
                }
                if (cname == "ID") {
                    this_row_uid = std::atol(data[idx]);
                    pthis->m_uid_col = idx;
                }
            }

            if (this_row_uid == row_t::invalid_row_id) {
                pthis->m_slasterror = "There must be a column with a name of "
                                      "ID in the database";
                pthis->m_lasterror = -1000;
                return -1000;
            }
        }

        this_row_uid = std::atol(data[pthis->m_uid_col]);
        assert(this_row_uid != row_t::invalid_row_id);
        for (idx = 0; idx < count; idx++) {
            std::string_view cname(columns[idx]);
            std::string_view cvalue(data[idx]);
            auto& col = cols.find(cname)->second;
            col.m_values.push_back(std::string(cvalue));
            col.m_uids.push_back(this_row_uid);
        }

        pthis->m_rows.insert({this_row_uid, row_t(this_row_uid, cols)});
        ++(pthis->m_row_index);
        return 0;
    }

    std::string_view version_string() const { return sqlite3_version; }

    column_t* m_psort_col = nullptr;
    void sort(const column_t* col) {
        auto it = m_columns.find(col->name());
        assert(it != m_columns.end());
        m_psort_col = &it->second;

        std::stable_sort(
            m_psort_col->m_values.begin(), m_psort_col->m_values.end());
    }

    private:
    int m_lasterror{SQLITE_OK};
    std::string m_slasterror;
    sqlite3* m_handle = nullptr;
    std::string m_path;
    bool m_errs_to_stderr{true};
    void set_last_error(char** sqlerr) {
        if (*sqlerr) {
            m_slasterror = *sqlerr;
            sqlite3_free(*sqlerr);
        } else {
            m_slasterror.clear();
        }
    }
};

#endif // SQLITE_HPP
