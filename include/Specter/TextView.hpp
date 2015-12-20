#ifndef SPECTER_TEXTVIEW_HPP
#define SPECTER_TEXTVIEW_HPP

#include "View.hpp"
#include <boo/graphicsdev/GL.hpp>
#include <boo/graphicsdev/D3D.hpp>
#include <boo/graphicsdev/Metal.hpp>

#include "FontCache.hpp"

namespace Specter
{
class ViewResources;

class TextView : public View
{
public:
    enum class Alignment
    {
        Left,
        Center,
        Right
    };

private:
    size_t m_capacity;
    boo::IGraphicsBufferD* m_glyphBuf;
    boo::IVertexFormat* m_vtxFmt = nullptr; /* OpenGL only */
    boo::IShaderDataBinding* m_shaderBinding;
    const FontAtlas& m_fontAtlas;
    Alignment m_align;
    bool m_valid = false;
    int m_width = 0;

    friend class MultiLineTextView;
    static int DoKern(FT_Pos val, const FontAtlas& atlas);

public:
    class Resources
    {
        friend class ViewResources;
        friend class TextView;
        friend class MultiLineTextView;
        FontCache* m_fcache = nullptr;
        boo::IShaderPipeline* m_regular = nullptr;
        boo::IShaderPipeline* m_subpixel = nullptr;
        boo::IVertexFormat* m_vtxFmt = nullptr; /* Not OpenGL */

        void init(boo::GLDataFactory* factory, FontCache* fcache);
#if _WIN32
        void init(boo::ID3DDataFactory* factory, FontCache* fcache);
#elif BOO_HAS_METAL
        void init(boo::MetalDataFactory* factory, FontCache* fcache);
#endif
    };

    TextView(ViewResources& res, View& parentView, const FontAtlas& font, Alignment align=Alignment::Left, size_t capacity=256);
    TextView(ViewResources& res, View& parentView, FontTag font, Alignment align=Alignment::Left, size_t capacity=256);

    struct RenderGlyph
    {
        Zeus::CVector3f m_pos[4];
        Zeus::CMatrix4f m_mv;
        Zeus::CVector3f m_uv[4];
        Zeus::CColor m_color;

        RenderGlyph(int& adv, const FontAtlas::Glyph& glyph, const Zeus::CColor& defaultColor);
    };
    struct RenderGlyphInfo
    {
        uint32_t m_char;
        std::pair<int,int> m_dims;
        int m_adv;
        bool m_space = false;

        RenderGlyphInfo(uint32_t ch, int width, int height, int adv)
        : m_char(ch), m_dims(width, height), m_adv(adv), m_space(iswspace(ch)) {}
    };
    std::vector<RenderGlyph>& accessGlyphs() {return m_glyphs;}
    const std::vector<RenderGlyph>& accessGlyphs() const {return m_glyphs;}
    void updateGlyphs() {m_valid = false;}

    void typesetGlyphs(const std::string& str,
                       const Zeus::CColor& defaultColor=Zeus::CColor::skWhite);
    void typesetGlyphs(const std::wstring& str,
                       const Zeus::CColor& defaultColor=Zeus::CColor::skWhite);

    void colorGlyphs(const Zeus::CColor& newColor);
    void colorGlyphsTypeOn(const Zeus::CColor& newColor, float startInterval=0.2, float fadeTime=0.5);
    void think();

    void resized(const boo::SWindowRect &rootView, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    int nominalWidth() const {return m_width;}
    int nominalHeight() const {return m_fontAtlas.FT_LineHeight() >> 6;}

    std::pair<int,int> queryGlyphDimensions(size_t pos) const;
    size_t reverseSelectGlyph(int x) const;
    int queryReverseAdvance(size_t idx) const;
    std::pair<size_t,size_t> queryWholeWordRange(size_t idx) const;

private:
    std::vector<RenderGlyph> m_glyphs;
    std::vector<RenderGlyphInfo> m_glyphInfo;
};

}

#endif // SPECTER_TEXTVIEW_HPP
