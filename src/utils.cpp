#include "utils.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

int levelthumbs::getQualityMultiplier(){
    return 4 / CCDirector::sharedDirector()->getContentScaleFactor();
}

CCGLProgram* getProgram(float strength)
{
    std::string vert = R"(
        attribute vec4 a_position;
        attribute vec2 a_texCoord;
        varying vec2 v_texCoord;

        void main() {
            gl_Position = CC_PMatrix * CC_MVMatrix * a_position;
            v_texCoord = a_texCoord;
        }
    )";
    
    // https://www.shadertoy.com/view/fsV3R3
    std::string frag = R"(
        #ifdef GL_ES
        precision mediump float;
        #endif

        const float pi = 3.14159265359;
        const int samples = 35;
        const float sigma = float(samples) * 0.25;

        // Constants for the Gaussian function
        const float sigma2 = 2.0 * sigma * sigma;
        const float pisigma2 = pi * sigma2;

        varying vec2 v_texCoord;
        uniform sampler2D u_texture; // Texture
        uniform vec2 u_resolution;   // Screen resolution

        float gaussian(vec2 i) {
            float top = exp(-((i.x * i.x) + (i.y * i.y)) / sigma2);
            return top / pisigma2;
        }

        vec3 blur(sampler2D sp, vec2 uv, vec2 scale) {
            vec2 offset;
            float weight = gaussian(offset);
            vec3 col = texture2D(sp, uv).rgb * weight;
            float accum = weight;

            for (int x = 0; x <= samples / 2; ++x) {
                for (int y = 1; y <= samples / 2; ++y) {
                    offset = vec2(x, y);
                    weight = gaussian(offset);
                    col += texture2D(sp, uv + scale * offset).rgb * weight;
                    accum += weight;

                    col += texture2D(sp, uv - scale * offset).rgb * weight;
                    accum += weight;

                    offset = vec2(-y, x);
                    col += texture2D(sp, uv + scale * offset).rgb * weight;
                    accum += weight;

                    col += texture2D(sp, uv - scale * offset).rgb * weight;
                    accum += weight;
                }
            }
            
            return col / accum;
        }

        void main() {
            vec2 ps = vec2(1.0) / u_resolution;
            vec2 uv = v_texCoord;

            vec3 resultColor = blur(u_texture, uv, ps);
            gl_FragColor = vec4(resultColor, 1.0);
        }
    )";

    auto program = new CCGLProgram();
    program->initWithVertexShaderByteArray(vert.c_str(), frag.c_str());
    program->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
    program->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
    program->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
    
    program->retain();
    program->link();
    program->updateUniforms();

    GLint resolutionLoc = program->getUniformLocationForName("u_resolution");
    GLint textureLoc = program->getUniformLocationForName("u_texture");

    float screenWidth = CCDirector::get()->getWinSize().width * strength;
    float screenHeight = CCDirector::get()->getWinSize().height * strength;

    program->use();
    program->setUniformsForBuiltins();
    program->setUniformLocationWith2f(resolutionLoc, screenWidth, screenHeight);
    program->setUniformLocationWith1i(textureLoc, 0); // Texture unit 0

    return program;
}

CCSprite* levelthumbs::createBlurredBG(CCImage* image)
{
    CCTexture2D* texture = new CCTexture2D();
    texture->initWithImage(image);

    float v = 255.0f - (Mod::get()->getSettingValue<double>("level-info-layer-bg-darken") * 255.0f);

    log::info("v: {}", v);

    auto spr = CCSprite::createWithTexture(texture);
    spr->setScale(CCDirector::get()->getWinSize().height / spr->getContentHeight());
    spr->setColor(ccc3(v, v, v));

    texture->release();

    auto blurAmount = Mod::get()->getSettingValue<double>("level-info-layer-bg-blur");

    if (!Mod::get()->getSettingValue<bool>("level-info-layer-bg-blur-enabled"))
        return spr;

    auto rtex = CCRenderTexture::create(roundf(spr->getScaledContentWidth()), roundf(spr->getScaledContentHeight()), kCCTexture2DPixelFormat_RGBA8888);
    auto pgram = getProgram(blurAmount);

    rtex->beginWithClear(0, 0, 0, 0);

    spr->setShaderProgram(pgram);
    spr->setScaleY(spr->getScaleY() * -1);
    spr->setPosition(CCDirector::get()->getWinSize() / 2);

    spr->visit();

    rtex->end();

    auto spr2 = CCSprite::createWithTexture(rtex->getSprite()->getTexture());
    spr2->setColor(ccc3(v, v, v));

    CC_SAFE_DELETE(rtex);
    CC_SAFE_DELETE(spr);
    CC_SAFE_DELETE(pgram);

    return spr2;
}