#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"

#include "progression.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace {
using ngpp::rac1::Progression;
using ngpp::rac1::ProgressionSettings;
using ngpp::rac1::Weapon;

Progression progression;
ProgressionSettings settings;
openrac::Mod* active_mod = nullptr;
std::int32_t equipped_item = -1;

struct PendingHit {
    std::int32_t item = -1;
    std::uint32_t attacker = 0;
    float damage = 0.0f;
};
std::unordered_map<int, PendingHit> pending_hits;

constexpr std::array<const char*, ngpp::rac1::weapon_count> xp_keys{{
    "xp_bomb_glove", "xp_devastator", "xp_blaster", "xp_pyrocitor",
    "xp_mine_glove", "xp_walloper", "xp_tesla_claw", "xp_visibomb",
    "xp_glove_of_doom", "xp_morph_o_ray", "xp_ryno", "xp_suck_cannon",
    "xp_decoy_glove", "xp_taunter", "xp_drone_device"
}};

constexpr std::array<const char*, ngpp::rac1::weapon_count> weapon_names{{
    "Bomb Glove", "Devastator", "Blaster", "Pyrocitor", "Mine Glove",
    "Walloper", "Tesla Claw", "Visibomb Gun", "Glove of Doom",
    "Morph-o-Ray", "R.Y.N.O.", "Suck Cannon", "Decoy Glove",
    "Taunter", "Drone Device"
}};

bool weapon_from_item(std::int32_t item, Weapon& out) {
    using namespace ngpp::rac1;
    switch (item) {
        case 0x0A: out = Weapon::BombGlove; return true;
        case 0x0B: out = Weapon::Devastator; return true;
        case 0x0F: out = Weapon::Blaster; return true;
        case 0x10: out = Weapon::Pyrocitor; return true;
        case 0x11: out = Weapon::MineGlove; return true;
        case 0x12: out = Weapon::Walloper; return true;
        case 0x13: out = Weapon::TeslaClaw; return true;
        case 0x0D: out = Weapon::VisibombGun; return true;
        case 0x14: out = Weapon::GloveOfDoom; return true;
        case 0x15: out = Weapon::MorphORay; return true;
        case 0x17: out = Weapon::Ryno; return true;
        case 0x09: out = Weapon::SuckCannon; return true;
        case 0x19: out = Weapon::DecoyGlove; return true;
        case 0x0E: out = Weapon::Taunter; return true;
        case 0x18: out = Weapon::DroneDevice; return true;
        default: return false;
    }
}

std::size_t weapon_index(Weapon weapon) {
    return static_cast<std::size_t>(weapon);
}

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

std::int32_t read_equipped_item(openrac::Mod& mod) {
    // RAC1's Hero_UpdateEquippedItems hook runs after the game updates the held item.
    // Relocate the executable's hero record for both NTSC-U and PAL before reading it.
    constexpr std::uint32_t hero_record = 0x0013F450u;
    constexpr std::uint32_t hero_item_id_offset = 0x10B8u;
    const auto base = mod.relocate(hero_record);
    if (base == 0) return -1;
    return mod.memory().read<std::int32_t>(base + hero_item_id_offset);
}

void load(openrac::Mod& mod) {
    if (mod.game() != "rac1") {
        throw openrac::Error("NG++ RAC1 is only for Ratchet & Clank.");
    }
    if (!mod.has_save_data()) {
        throw openrac::Error("NG++ RAC1 requires OpenRAC save-data support (interface 1.16 or newer).");
    }

    active_mod = &mod;
    read_settings(mod);

    const auto bytes = mod.save_data();
    if (!bytes.empty() && !progression.deserialize(bytes)) {
        mod.warn("The NG++ save data was not recognized; weapon progression starts fresh.");
    }

    mod.on_save_data([&mod] {
        pending_hits.clear();
        equipped_item = -1;
        const auto current = mod.save_data();
        if (current.empty()) {
            progression = Progression{};
        } else if (!progression.deserialize(current)) {
            mod.warn("The loaded NG++ save data was invalid; weapon progression starts fresh.");
            progression = Progression{};
        }
    });
    mod.on_settings([&mod] { read_settings(mod); });

    // Keep the currently held item in sync, including changes made through quick-select.
    mod.hook<openrac::rac1::Hero_UpdateEquippedItems>([&mod](auto original) {
        original();
        equipped_item = read_equipped_item(mod);
    });

    // Weapon_TakeAmmo is called when a weapon actually fires. RAC1 passes -1 for
    // the item in Ratchet's hand; explicit item IDs are used when the caller knows them.
    mod.hook<openrac::rac1::Weapon_TakeAmmo>([&mod](auto original, std::int32_t item,
                                                    std::int32_t amount) {
        const auto result = original(item, amount);
        if (result != 0) {
            if (item >= 0) equipped_item = item;
            else if (equipped_item < 0) equipped_item = read_equipped_item(mod);
        }
        return result;
    });

    // The hit ring keeps the attacker and damage for each moby hit. Remember the
    // weapon in hand when a hit is read; the death hook consumes that victim's record.
    mod.hook<openrac::rac1::Moby_GetHitMessage>([&mod](auto original, std::uint32_t moby,
                                                        std::int32_t mask, std::int32_t keep) {
        const auto record = original(moby, mask, keep);
        if (record != 0 && settings.enabled) {
            const auto attacker = mod.memory().read<std::uint32_t>(record + 0x20u);
            const auto damage = mod.memory().read<float>(record + 0x2Cu);
            const auto slot = mod.moby_slot_of(moby);
            Weapon hit_weapon{};
            if (equipped_item >= 0 && weapon_from_item(equipped_item, hit_weapon)) {
                if (mod.setting<bool>("damage_scaling")) {
                    const auto level = progression.get(hit_weapon).level;
                    const float multiplier = 1.0f + 0.10f * static_cast<float>(level - 1);
                    mod.memory().write<float>(record + 0x2Cu, damage * multiplier);
                }
                if (slot >= 0 && damage > 0.0f) {
                    pending_hits[slot] = PendingHit{equipped_item, attacker, damage};
                }
            }
        }
        return record;
    });

    mod.hook<openrac::rac1::Moby_RecordDeath>([&mod](auto original, std::uint32_t moby,
                                                      std::int32_t flags, std::int32_t index) {
        const auto slot = mod.moby_slot_of(moby);
        original(moby, flags, index);
        if (slot < 0) return;

        const auto found = pending_hits.find(slot);
        if (found == pending_hits.end()) return;
        const auto hit = found->second;
        pending_hits.erase(found);

        Weapon weapon{};
        if (!settings.enabled || !weapon_from_item(hit.item, weapon)) return;

        // Award only after a player weapon has been identified for the hit.
        // The default kill reward is configurable in the launcher.
        const auto xp = static_cast<std::uint32_t>(mod.setting<int>("xp_per_kill"));
        const bool leveled = progression.award_kill(weapon, xp, settings);
        save_progress(mod);
        if (leveled && mod.setting<bool>("level_notices")) {
            const auto i = weapon_index(weapon);
            const auto notice = std::string(weapon_names[i]) + " reached level "
                              + std::to_string(progression.get(weapon).level) + "!";
            mod.play_notice(notice);
            mod.info(notice);
        }
    });

    mod.info("NG++ RAC1 weapon progression loaded.");
}
} // namespace

OPENRAC_MOD_CPP("ng-plus-plus-rac1", "0.1.0", load)
