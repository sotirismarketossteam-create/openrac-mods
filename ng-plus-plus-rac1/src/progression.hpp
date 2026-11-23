#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ngpp::rac1 {

constexpr std::size_t weapon_count = 15;

enum class Weapon : std::uint8_t {
    BombGlove, Devastator, Blaster, Pyrocitor, MineGlove, Walloper,
    TeslaClaw, GloveOfDoom, MorphORay, Ryno, SuckCannon, DecoyGlove,
    Taunter, DroneDevice
};

struct WeaponProgress {
    std::uint32_t xp = 0; // XP earned toward this weapon's next level.
    std::uint8_t level = 1;
};

struct ProgressionSettings {
    bool enabled = true;
    bool increasing_curve = true;
    std::uint8_t level_cap = 10;
    std::array<std::uint32_t, weapon_count> xp_per_level{};
};

class Progression {
public:
    Progression();

    const WeaponProgress& get(Weapon weapon) const;
    const std::array<WeaponProgress, weapon_count>& all() const { return progress_; }

    // Call only after the game has confirmed a player-caused enemy kill and the
    // responsible weapon has been identified. Returns true if one or more levels
    // were gained. Other weapons are never modified.
    bool award_kill(Weapon weapon, std::uint32_t xp, const ProgressionSettings& settings);

    // Stable, versioned save payload suitable for Mod::save_data_set/save_data.
    std::vector<std::uint8_t> serialize() const;
    bool deserialize(const std::vector<std::uint8_t>& bytes);

    static std::uint32_t requirement(Weapon weapon, std::uint8_t current_level,
                                     const ProgressionSettings& settings);

private:
    std::array<WeaponProgress, weapon_count> progress_{};
};

} // namespace ngpp::rac1
