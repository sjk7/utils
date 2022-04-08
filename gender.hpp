// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#pragma once
// gender.hpp

#ifdef _MSC_VER
#pragma warning(disable : 4068)
#endif

#include "my_utils.hpp"
#include <array>

namespace limits {
static constexpr int jingle_ad_len = 90; // ignore jingles and the like
static constexpr int max_song_len
    = 60 * 10; // anything longer than this may not be moved and doesn't count
               // as two adjacent item
static constexpr int not_enough_cuts_in_hour = 12;
static constexpr int average_cuts_per_hour = 16;
static constexpr auto max_attempts = 16L * 16L;
static constexpr auto max_swaps = 64;
} // namespace limits

enum class gender_t { assume_female, female, male_female, male, MAX_GENDER };
static const std::array<const char*, 4> genders
    = {"assume_female", "female", "male/female", "male"};

static inline auto gender_from_string(const std::string_view s) {
    constexpr auto ret = gender_t::assume_female;

    std::string supper{s};
    my::utils::strings::to_upper_branchless(supper);
    if (supper == "M") {
        return gender_t::male;
    } else if (supper == "F" || supper == "F(a)") {
        return gender_t::female;
    } else if (supper.find('/') != std::string::npos) {
        return gender_t::male_female;
    }

    return ret;
}

static const std::array<const char*, 4> genders_for_file
    = {"F(a)", "F", "M/F", "M"};
static inline auto string_from_gender(gender_t g) {
    if (g > gender_t::MAX_GENDER) g = gender_t::assume_female;
    return std::basic_string_view<char>(genders_for_file[static_cast<int>(g)]);
}

struct artist_info {

    size_t line_number = 0;
    std::string_view artist;
    int dur = 0;
    gender_t gender = gender_t::assume_female;
    size_t line_number_orig = 0;

    artist_info(const int line_number, const std::string_view artistv,
        const int dur, const gender_t gender)
        : line_number(line_number)
        , artist(artistv)
        , dur(dur)
        , gender(gender)
        , line_number_orig(line_number) {}

    artist_info& operator=(const artist_info& rhs) = default;
    artist_info& operator=(artist_info&&) = default;
    bool operator==(const artist_info& other) const noexcept {
        return (other.artist == artist) && (other.line_number == line_number)
            && (other.line_number_orig == line_number_orig
                && (other.gender == gender) && (dur == other.dur));
    }
    artist_info(const artist_info& rhs) = default;
    artist_info(artist_info&& rhs) = default;

    artist_info() = default;
    ~artist_info() = default;

    [[nodiscard]] std::string_view gender_to_string() const noexcept {
        return genders[static_cast<int>(gender)];
    }

    [[nodiscard]] std::string_view gender_to_string_for_file() const noexcept {
        return ::string_from_gender(gender);
    }

    [[nodiscard]] bool is_empty() const noexcept {
        return artist.empty() && dur == 0 && line_number == 0
            && line_number_orig == 0;
    }
    [[nodiscard]] bool is_male() const noexcept {
        return static_cast<int>(gender) >= static_cast<int>(gender_t::male);
    }
    [[nodiscard]] bool is_song() const noexcept {
        return dur > limits::jingle_ad_len && dur < limits::max_song_len
            && !artist.empty();
    }

    friend void swap(artist_info& lhs, artist_info& rhs) noexcept {
        using std::swap;

        swap(lhs.artist, rhs.artist);
        swap(lhs.dur, rhs.dur);
        swap(lhs.gender, rhs.gender);
        swap(lhs.line_number, rhs.line_number);
        // swap(lhs.line_number_orig, rhs.line_number_orig);
    }

    friend std::ostream& operator<<(std::ostream& os, const artist_info& me) {
        os << "line: " << me.line_number << " artist: " << me.artist
           << ", Gender: " << me.gender_to_string()
           << " [orig line number:" << me.line_number_orig << "]";
        return os;
    }

    [[nodiscard]] std::string to_short_string() const noexcept {
        std::string ret = my::utils::strings::concat("line:", line_number,
            " artist:", artist, " gender:", gender_to_string(), " dur:", dur);
        return ret;
    }
};
