#include "progression.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

using namespace ngpp::rac1;

int main() {
    Progression p;
    ProgressionSettings s;
    s.enabled = true;
    s.level_cap = 10;
    s.increasing_curve = false;
    s.xp_per_level.fill(10);

    assert(p.get(Weapon::BombGlove).level == 1);
    assert(p.get(Weapon::Blaster).level == 1);

    // A kill levels only the weapon credited for it.
    assert(p.award_kill(Weapon::BombGlove, 10, s));
    assert(p.get(Weapon::BombGlove).level == 2);
    assert(p.get(Weapon::Blaster).level == 1);

    // Each weapon's requirement is independent.
    s.xp_per_level[static_cast<std::size_t>(Weapon::Blaster)] = 25;
    assert(!p.award_kill(Weapon::Blaster, 10, s));
    assert(p.get(Weapon::Blaster).xp == 10);
    assert(p.get(Weapon::BombGlove).xp == 0);

    // Increasing curve scales the requirement by the current level.
    s.increasing_curve = true;
    assert(Progression::requirement(Weapon::BombGlove, 3, s) == 30);
    s.level_cap = 2;
    assert(!p.award_kill(Weapon::BombGlove, 100, s));
    assert(p.get(Weapon::BombGlove).level == 2);
    assert(p.get(Weapon::BombGlove).xp == 0);

    // Save round-trip preserves all independent weapon records.
    const auto bytes = p.serialize();
    Progression restored;
    assert(restored.deserialize(bytes));
    assert(restored.get(Weapon::BombGlove).level == 2);
    assert(restored.get(Weapon::Blaster).xp == 10);

    auto corrupt = bytes;
    corrupt[4] = 99;
    assert(!restored.deserialize(corrupt));
    assert(restored.get(Weapon::Blaster).xp == 10);
}
