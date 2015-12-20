#include <LogVisor/LogVisor.hpp>
#include "Specter/SplitView.hpp"
#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::SplitView");

void SplitView::Resources::init(boo::IGraphicsDataFactory* factory, const ThemeData& theme)
{
    static const Zeus::RGBA32 tex[3] =
    {
        {0,0,0,64},
        {0,0,0,255},
        {255,255,255,64}
    };
    m_shadingTex = factory->newStaticTexture(3, 1, 1, boo::TextureFormat::RGBA8, tex, 12);
}

SplitView::SplitView(ViewResources& system, View& parentView, Axis axis)
: View(system, parentView), m_axis(axis)
{
    m_splitBlockBuf = system.m_factory->newDynamicBuffer(boo::BufferUse::Uniform, sizeof(ViewBlock), 1);
    m_splitVertsBuf = system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(TexShaderVert), 4);

    if (!system.m_viewRes.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_splitVertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_splitVertsBuf, nullptr, boo::VertexSemantic::UV4}
        };
        m_splitVtxFmt = system.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_splitBlockBuf};
        boo::ITexture* texs[] = {system.m_splitRes.m_shadingTex};
        m_splitShaderBinding = system.m_factory->newShaderDataBinding(system.m_viewRes.m_texShader,
                                                                      m_splitVtxFmt, m_splitVertsBuf, nullptr,
                                                                      nullptr, 1, bufs, 1, texs);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_splitBlockBuf};
        boo::ITexture* texs[] = {system.m_splitRes.m_shadingTex};
        m_splitShaderBinding = system.m_factory->newShaderDataBinding(system.m_viewRes.m_texShader,
                                                                      system.m_viewRes.m_texVtxFmt,
                                                                      m_splitVertsBuf, nullptr,
                                                                      nullptr, 1, bufs, 1, texs);
    }

    commitResources(system);
}

View* SplitView::setContentView(int slot, View* view)
{
    if (slot < 0 || slot > 1)
        Log.report(LogVisor::FatalError, "out-of-range slot to RootView::SplitView::setContentView");
    View* ret = m_views[slot].m_view;
    m_views[slot].m_view = view;
    m_views[slot].m_mouseDown = 0;
    m_views[slot].m_mouseIn = false;
    updateSize();
    return ret;
}

void SplitView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_axis == Axis::Horizontal)
    {
        int slidePx = subRect().size[1] * m_slide;
        if (abs(int(coord.pixel[1] + 2) - slidePx) < 4)
        {
            if (button == boo::EMouseButton::Primary)
            {
                m_dragging = true;
                setSlide((coord.pixel[1] + 2) / float(subRect().size[1]));
            }
            else if (button == boo::EMouseButton::Secondary)
            {
                // TODO: Split menu
            }
            return;
        }
    }
    else if (m_axis == Axis::Vertical)
    {
        int slidePx = subRect().size[0] * m_slide;
        if (abs(int(coord.pixel[0] + 2) - slidePx) < 4)
        {
            if (button == boo::EMouseButton::Primary)
            {
                m_dragging = true;
                setSlide((coord.pixel[0] + 2) / float(subRect().size[0]));
            }
            else if (button == boo::EMouseButton::Secondary)
            {
                // TODO: Split menu
            }
            return;
        }
    }
    m_views[0].mouseDown(coord, button, mod);
    m_views[1].mouseDown(coord, button, mod);
}

void SplitView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (button == boo::EMouseButton::Primary)
        m_dragging = false;
    m_views[0].mouseUp(coord, button, mod);
    m_views[1].mouseUp(coord, button, mod);
}

void SplitView::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_axis == Axis::Horizontal)
    {
        if (m_dragging)
            setSlide((coord.pixel[1] + 2) / float(subRect().size[1]));
        int slidePx = subRect().size[1] * m_slide;
        if (abs(int(coord.pixel[1] + 2) - slidePx) < 4)
            rootView().window()->setCursor(boo::EMouseCursor::VerticalArrow);
        else
            rootView().window()->setCursor(boo::EMouseCursor::Pointer);
    }
    else if (m_axis == Axis::Vertical)
    {
        if (m_dragging)
            setSlide((coord.pixel[0] + 2) / float(subRect().size[0]));
        int slidePx = subRect().size[0] * m_slide;
        if (abs(int(coord.pixel[0] + 2) - slidePx) < 4)
            rootView().window()->setCursor(boo::EMouseCursor::HorizontalArrow);
        else
            rootView().window()->setCursor(boo::EMouseCursor::Pointer);
    }

    m_views[0].mouseMove(coord);
    m_views[1].mouseMove(coord);
}

void SplitView::mouseEnter(const boo::SWindowCoord& coord)
{
    m_views[0].mouseEnter(coord);
    m_views[1].mouseEnter(coord);
}

void SplitView::mouseLeave(const boo::SWindowCoord& coord)
{
    m_views[0].mouseLeave(coord);
    m_views[1].mouseLeave(coord);
}

void SplitView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    if (m_axis == Axis::Horizontal)
    {
        boo::SWindowRect ssub = sub;
        ssub.size[1] *= m_slide;
        if (m_views[0].m_view)
            m_views[0].m_view->resized(root, ssub);
        ssub.location[1] += ssub.size[1];
        ssub.size[1] = sub.size[1] - ssub.size[1];
        if (m_views[1].m_view)
            m_views[1].m_view->resized(root, ssub);
        ssub.location[1] -= 1;
        m_splitBlock.setViewRect(root, ssub);
        setHorizontalVerts(ssub.size[0]);
    }
    else if (m_axis == Axis::Vertical)
    {
        boo::SWindowRect ssub = sub;
        ssub.size[0] *= m_slide;
        if (m_views[0].m_view)
            m_views[0].m_view->resized(root, ssub);
        ssub.location[0] += ssub.size[0];
        ssub.size[0] = sub.size[0] - ssub.size[0];
        if (m_views[1].m_view)
            m_views[1].m_view->resized(root, ssub);
        ssub.location[0] -= 1;
        m_splitBlock.setViewRect(root, ssub);
        setVerticalVerts(ssub.size[1]);
    }
    m_splitBlockBuf->load(&m_splitBlock, sizeof(ViewBlock));
    m_splitVertsBuf->load(m_splitVerts, sizeof(TexShaderVert) * 4);
}

void SplitView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    if (m_views[0].m_view)
        m_views[0].m_view->draw(gfxQ);
    if (m_views[1].m_view)
        m_views[1].m_view->draw(gfxQ);
    gfxQ->setShaderDataBinding(m_splitShaderBinding);
    gfxQ->draw(0, 4);

}

}

