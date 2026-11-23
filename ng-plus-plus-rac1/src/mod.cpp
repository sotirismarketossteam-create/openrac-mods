#include "openrac/mod.hpp"

#include "progression.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace {
using ngpp::rac1::Progression;
using ngpp::rac1::ProgressionSettings;
using ngpp::rac1::Weapon;

Progression progression;
ProgressionSettings settings;

constexpr std::array<const char*, ngpp::rac1::weapon_count> xp_keys{{
    "xp_bomb_glove", "xp_devastator", "xp_blaster", "xp_pyrocitor",
    "xp_mine_glove", "xp_walloper", "xp_tesla_claw", "xp_glove_of_doom", "xp_morph_o_ray", "xp_ryno", "xp_suck_cannon", "xp_decoy_glove",
    "xp_taunter", "xp_drone_device"
}};

void read_settings(openrac::Mod& mod) {
    settings.enabled = mod.setting<bool>("weapon_xp");
    settings.level_cap = static_cast<std::uint8_t>(mod.setting<int>("level_cap"));
    settings.increasing_curve = mod.setting<std::string>("xp_linear") != "linear";
    for (std::size_t i = 0; i < xp_keys.size(); ++i) {
        const int configured = mod.setting<int>(xp_keys[i]);
        settings.xp_per_level[i] = configured > 0 ? static_cast<std::uint32_t>(configured) : 100u;
    }
}

void save_progress(openrac::Mod& mod) {
    const auto bytes = progression.serialize();
    if (!mod.save_data_set(bytes)) {
        mod.warn("Could not write weapon progression to this save.");
    }
}

void load(openrac::Mod& mod) {
    if (mod.game() != "rac1") {
        throw openrac::Error("NG++ RAC1 is only for Ratchet & Clank.");
    }
    if (!mod.has_save_data()) {
        throw openrac::Error("NG++ RAC1 requires OpenRAC save-data support (interface 1.16 or newer).");
    }

    read_settings(mod);

    // Load the current save's independent weapon progression, and reload it whenever
    // the player changes save slot or starts another game.
    const auto bytes = mod.save_data();
    if (!bytes.empty() && !progression.deserialize(bytes)) {
        mod.warn("The NG++ save data was not recognized; weapon progression starts fresh.");
    }
    mod.on_save_data([&mod] {
        const auto current = mod.save_data();
        if (current.empty()) {
            progression = Progression{};
        } else if (!progression.deserialize(current)) {
            mod.warn("The loaded NG++ save data was invalid; weapon progression starts fresh.");
            progression = Progression{};
        }
    });

    // Launcher changes take effect without restarting the game.
    mod.on_settings([&mod] { read_settings(mod); });

    mod.info("NG++ RAC1 progression core loaded. Kill-credit integration is not enabled yet.");
}
} // namespace

OPENRAC_MOD_CPP("ng-plus-plus-rac1", "0.1.0", load)
