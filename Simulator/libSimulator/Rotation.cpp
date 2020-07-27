#include "Rotation.h"
#include "Ability.h"
#include "detail/helpers.h"
#include "detail/log.h"
#include "detail/names.h"

namespace Simulator {
std::optional<Second> Rotation::getNextEventTime() {
    if (_target->getCurrentHealth() <= HealthPoints(0.0))
        return std::nullopt;

    if (_currentAbility) {
        auto &&info = _currentAbility->getInfo();
        switch (info.type) {
        case AbilityCastType::Instant:
            CHECK(false, "instant casts should never cause a future rotational event");
            break;
        case AbilityCastType::OffGCD:
            CHECK(false, "Off GCD abilities should never cause a future rotational event");
            break;
        case AbilityCastType::Cast:
            return _abilityStartTime + _abilityCastTickTime;
        case AbilityCastType::Channeled:
            return _abilityStartTime + _currentTick * _abilityCastTickTime;
        }
    }

    return std::nullopt;
}
std::optional<Second> SetRotation::getNextEventTime() {
    auto time = Rotation::getNextEventTime();
    if (time)
        return time;
    if (_idCounter < _ids.size()) {
        return getNextFreeGCD();
    } else if (++_iterationCount < _Repeats) {
        _idCounter = 0;
        return getNextFreeGCD();
    }
    return std::nullopt;
}
AbilityId SetRotation::getNextAbility() {
    CHECK(_idCounter < _ids.size());
    return _ids[_idCounter++];
}
enum class TargetType {
    Rotation,
    Target,
    Source,
};
struct NextEvent {
    TargetType type;
    Second time;
    int targetCount{0};
};
void Rotation::doRotation() {
    CHECK(_target);

    auto getEvents = [&]() {
        std::vector<NextEvent> events;
        if (auto nextSourceEvent = getSource()->getNextEventTime()) {
            events.push_back(NextEvent{TargetType::Source, *nextSourceEvent});
        }
        if (auto nextTargetEvent = _target->getNextEventTime()) {
            events.push_back(NextEvent{TargetType::Target, *nextTargetEvent});
        }
        if (auto nextRotationEvent = getNextEventTime()) {
            events.push_back(NextEvent{TargetType::Rotation, *nextRotationEvent});
        }
        std::sort(events.begin(), events.end(), [](const NextEvent &a, const NextEvent &b) { return a.time < b.time; });
        return events;
    };

    auto ne = getEvents();
    while (ne.size() > 0) {
        auto &&event = ne[0];
        if (event.type == TargetType::Source) {
            getSource()->applyEventsAtTime(event.time + Second(1e-7));
        } else if (event.type == TargetType::Target) {
            _target->applyEventsAtTime(event.time + Second(1e-7));
        } else if (event.type == TargetType::Rotation) {
            resolveEventsUpToTime(event.time + Second(1e-7), _target);
        }
        ne = getEvents();
    }
}

void Rotation::resolveEventsUpToTime(const Second &time, const TargetPtr &target) {
    if (_currentAbility) {
        auto &&info = _currentAbility->getInfo();
        switch (info.type) {
        case AbilityCastType::Cast: {
            auto endCastTime = _abilityStartTime + _abilityCastTickTime;
            CHECK(endCastTime < time + Second(1e-5));
            auto fs = getAllFinalStats(*_currentAbility, getSource(), target);
            auto hits = getHits(*_currentAbility, fs, target);
            applyDamageToTarget(hits, getSource(), target, endCastTime);
            auto abilityCooldown = _currentAbility->getCooldownIsAffectedByAlacrity()
                                       ? _currentAbility->getCooldown() / (1.0 + _abilityAlacrityAmount)
                                       : _currentAbility->getCooldown();
            getSource()->setAbilityCooldown(_currentAbility->getId(), abilityCooldown + endCastTime);
            _currentAbility->onAbilityHitTarget(hits, getSource(), target, endCastTime);
            _currentAbility->onAbilityEnd(getSource(), target, endCastTime);
            _source->spendEnergy(_currentAbility->getInfo().energyCost, endCastTime);
            _currentAbility = nullptr;
        } break;
        case AbilityCastType::Channeled: {
            auto currentTickTime = _abilityStartTime + _currentTick * _abilityCastTickTime;
            CHECK(_abilityStartTime + _abilityCastTickTime < time + Second(1e-5));

            auto fs = getAllFinalStats(*_currentAbility, getSource(), target);
            auto hits = getHits(*_currentAbility, fs, target);
            applyDamageToTarget(hits, getSource(), target, currentTickTime);
            _currentAbility->onAbilityHitTarget(hits, getSource(), target, currentTickTime);
            _source->spendEnergy(_currentAbility->getInfo().energyCost, currentTickTime);
            _currentTick += 1;

            if (_currentTick == info.nTicks) {
                _currentAbility->onAbilityEnd(getSource(), target, currentTickTime);
                _currentAbility = nullptr;
                _nextFreeGCD = currentTickTime + Second(1e-6) + getDelayAfterChanneled();
                _currentTick = 0;
                break;
            }

        } break;
        default:
            break;
        }
    } else {
        auto id = getNextAbility();
        auto abl = getAbility(id);
        CHECK(abl);
        auto &&info = abl->getInfo();
        SIM_INFO("[ROTATION] Time : {}, Casting ability [{} : {}]", time.getValue(), detail::getAbilityName(id), id);
        auto afs = getAllFinalStats(*abl, getSource(), target);
        if (afs.size()) {
            _abilityAlacrityAmount = afs[0].alacrity;
            _abilityCastTickTime = info.time / (1 + _abilityAlacrityAmount);
        }
        _abilityStartTime = _nextFreeGCD;
        switch (info.type) {
        case AbilityCastType::Cast:
            _currentAbility = abl;
            _nextFreeGCD +=
                std::max(Second(std::ceil(10 * Second(1.5).getValue() / (1 + _abilityAlacrityAmount)) / 10.0),
                         _abilityCastTickTime);
            _nextFreeGCDForInstant = _nextFreeGCD;
            break;
        case AbilityCastType::Channeled: {
            _currentAbility = abl;
            auto hits = getHits(*_currentAbility, afs, target);
            applyDamageToTarget(hits, getSource(), target, _nextFreeGCD);
            _currentAbility->onAbilityHitTarget(hits, getSource(), target, _nextFreeGCD);
            _source->spendEnergy(_currentAbility->getInfo().energyCost, _nextFreeGCD);
            _currentTick = 1;
            auto abilityCooldown = abl->getCooldownIsAffectedByAlacrity()
                                       ? abl->getCooldown() / (1.0 + _abilityAlacrityAmount)
                                       : abl->getCooldown();
            getSource()->setAbilityCooldown(abl->getId(), abilityCooldown + _nextFreeGCD);
            _nextFreeGCDForInstant +=
                Second(std::ceil(10 * Second(1.5).getValue() / (1 + _abilityAlacrityAmount)) / 10.0);
            _nextFreeGCD += Second(std::ceil(10 * Second(1.5).getValue() / (1 + _abilityAlacrityAmount)) / 10.0);

            break;
        }
        case AbilityCastType::Instant: {
            auto hits = getHits(*abl, afs, target);
            applyDamageToTarget(hits, getSource(), target, _nextFreeGCD);
            abl->onAbilityHitTarget(hits, getSource(), target, _nextFreeGCD);
            _source->spendEnergy(abl->getInfo().energyCost, _nextFreeGCD);
            abl->onAbilityEnd(getSource(), target, _nextFreeGCD);
            auto abilityCooldown = abl->getCooldownIsAffectedByAlacrity()
                                       ? abl->getCooldown() / (1.0 + _abilityAlacrityAmount)
                                       : abl->getCooldown();
            getSource()->setAbilityCooldown(abl->getId(), abilityCooldown + _nextFreeGCD);
            _nextFreeGCDForInstant = _nextFreeGCD + _MinTimeAfterInstant;
            _nextFreeGCD =
                _nextFreeGCD + Second(std::ceil(10 * Second(1.5).getValue() / (1 + _abilityAlacrityAmount)) / 10.0);
            break;
        }
        case AbilityCastType::OffGCD: {
            DamageHits hits = abl->getCoefficients().empty() ? DamageHits{} : getHits(*abl, afs, target);
            auto castTime = std::max(time, _nextFreeGCDForInstant);
            applyDamageToTarget(hits, getSource(), target, castTime);
            abl->onAbilityHitTarget(hits, getSource(), target, castTime);
            _source->spendEnergy(abl->getInfo().energyCost, castTime);
            abl->onAbilityEnd(getSource(), target, castTime);
            auto abilityCooldown = abl->getCooldownIsAffectedByAlacrity()
                                       ? abl->getCooldown() / (1.0 + _abilityAlacrityAmount)
                                       : abl->getCooldown();
            getSource()->setAbilityCooldown(abl->getId(), abilityCooldown + castTime);
            _nextFreeGCDForInstant = castTime + _MinTimeAfterInstant;
            _nextFreeGCD = std::max(_nextFreeGCD, _nextFreeGCDForInstant);
        }
        default:
            break;
        }
    }
}
} // namespace Simulator
