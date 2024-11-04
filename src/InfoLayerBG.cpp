#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "utils.hpp"
#include "ImageCache.hpp"

using namespace geode::prelude;

class $modify (LevelInfoLayer)
{
    bool init(GJGameLevel* level, bool challenge)
    {
        if (!LevelInfoLayer::init(level, challenge))
            return false;

        if (!ImageCache::get()->getImage(fmt::format("thumb-{}", (int)m_level->m_levelID)))
            return true;

        if (Mod::get()->getSettingValue<bool>("level-info-layer-bg"))
        {
            auto blueBG = getChildByID("background");

            auto bg = levelthumbs::createBlurredBG(ImageCache::get()->getImage(fmt::format("thumb-{}", (int)m_level->m_levelID)));
            bg->setID("background"_spr);
            bg->setPosition(CCDirector::get()->getWinSize() / 2);

            this->insertAfter(bg, blueBG);
        }

        return true;
    }
};