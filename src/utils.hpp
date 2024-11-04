#pragma once

#include <Geode/Geode.hpp>

namespace levelthumbs{
    int getQualityMultiplier();
    cocos2d::CCSprite* createBlurredBG(cocos2d::CCImage* image);
}