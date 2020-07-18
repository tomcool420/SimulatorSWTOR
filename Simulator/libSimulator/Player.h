#pragma once
#include "detail/units.h"
#include "types.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <vector>
namespace Simulator {
class Buff;
using BuffPtr = std::unique_ptr<Buff>;
class Ability;
class Target;
using PlayerId = boost::uuids::uuid;
class Player {
  public:
    Player(RawStats rs) : _rawStats(std::move(rs)) { _tag = boost::uuids::random_generator()(); }

    void setBuffs(std::vector<BuffPtr> && buffs) { _buffs = std::move(buffs); };
    StatChanges getCurrentPlayerStats( const Ability &abl, const Target &target) const;
    const RawStats &getRawStats() const { return _rawStats; }
    PlayerId getId() const {return _tag;}
  private:
    RawStats _rawStats;
    std::vector<BuffPtr> _buffs;
    PlayerId _tag;
};
using PlayerPtr = std::shared_ptr<Player>;
} // namespace Simulator
