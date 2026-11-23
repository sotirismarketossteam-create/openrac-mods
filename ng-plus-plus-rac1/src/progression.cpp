#include "progression.hpp"

#include <algorithm>
#include <limits>

namespace ngpp::rac1 {
namespace {
constexpr std::uint8_t save_version = 1;
constexpr std::uint8_t save_magic[4] = {'N', 'G', 'R', '1'};

std::size_t index_of(Weapon weapon) {
    const auto index = static_cast<std::size_t>(weapon);
    return index < weapon_count ? index : 0;
}

void append_u32(std::vector<std::uint8_t>& out, std::uint32_t value) {
    out.push_back(static_cast<std::uint8_t>(value));
    out.push_back(static_cast<std::uint8_t>(value >> 8));
    out.push_back(static_cast<std::uint8_t>(value >> 16));
    out.push_back(static_cast<std::uint8_t>(value >> 24));
}

bool read_u32(const std::vector<std::uint8_t>& in, std::size_t& at, std::uint32_t& value) {
    if (at + 4 > in.size()) return false;
    value = static_cast<std::uint32_t>(in[at])
          | (static_cast<std::uint32_t>(in[at + 1]) << 8)
          | (static_cast<std::uint32_t>(in[at + 2]) << 16)
          | (static_cast<std::uint32_t>(in[at + 3]) << 24);
    at += 4;
    return true;
}
} // namespace

Progression::Progression() {
    for (auto& weapon : progress_) weapon = WeaponProgress{};
}

const WeaponProgress& Progression::get(Weapon weapon) const {
    return progress_[index_of(weapon)];
}

std::uint32_t Progression::requirement(Weapon weapon, std::uint8_t current_level,
                                       const ProgressionSettings& settings) {
    const std::uint32_t base = std::max<std::uint32_t>(
        1, settings.xp_per_level[index_of(weapon)]);
    if (!settings.increasing_curve) return base;

    const std::uint64_t scaled = static_cast<std::uint64_t>(base)
                               * std::max<std::uint8_t>(1, current_level);
    return static_cast<std::uint32_t>(std::min<std::uint64_t>(
        scaled, std::numeric_limits<std::uint32_t>::max()));
}

bool Progression::award_kill(Weapon weapon, std::uint32_t xp,
                             const ProgressionSettings& settings) {
    if (!settings.enabled || xp == 0) return false;

    auto& state = progress_[index_of(weapon)];
    const auto cap = std::clamp<std::uint8_t>(settings.level_cap, 1, 10);
    if (state.level >= cap) {
        state.level = cap;
        state.xp = 0;
        return false;
    }

    const std::uint64_t total = static_cast<std::uint64_t>(state.xp) + xp;
    state.xp = static_cast<std::uint32_t>(std::min<std::uint64_t>(
        total, std::numeric_limits<std::uint32_t>::max()));

    bool leveled = false;
    while (state.level < cap) {
        const auto needed = requirement(weapon, state.level, settings);
        if (state.xp < needed) break;
        state.xp -= needed;
        ++state.level;
        leveled = true;
    }
    if (state.level >= cap) state.xp = 0;
    return leveled;
}

std::vector<std::uint8_t> Progression::serialize() const {
    std::vector<std::uint8_t> out;
    out.reserve(6 + weapon_count * 5);
    out.insert(out.end(), std::begin(save_magic), std::end(save_magic));
    out.push_back(save_version);
    out.push_back(static_cast<std::uint8_t>(weapon_count));
    for (const auto& weapon : progress_) {
        append_u32(out, weapon.xp);
        out.push_back(std::clamp<std::uint8_t>(weapon.level, 1, 10));
    }
    return out;
}

bool Progression::deserialize(const std::vector<std::uint8_t>& bytes) {
    constexpr std::size_t expected_size = 6 + weapon_count * 5;
    if (bytes.size() != expected_size
        || !std::equal(std::begin(save_magic), std::end(save_magic), bytes.begin())
        || bytes[4] != save_version || bytes[5] != weapon_count) {
        return false;
    }

    auto loaded = progress_;
    std::size_t at = 6;
    for (auto& weapon : loaded) {
        std::uint32_t xp = 0;
        if (!read_u32(bytes, at, xp) || at >= bytes.size()) return false;
        const auto level = bytes[at++];
        if (level < 1 || level > 10) return false;
        weapon.xp = xp;
        weapon.level = level;
    }
    progress_ = loaded;
    return true;
}

} // namespace ngpp::rac1
