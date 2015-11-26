#include "Specter/View.hpp"
#include "Specter/ViewSystem.hpp"

namespace Specter
{

void View::System::init(boo::GLDataFactory* factory)
{
    static const char* VS =
    "#version 330\n"
    "layout(location=0) in vec3 posIn;\n"
    "layout(location=1) in vec4 colorIn;\n"
    SPECTER_VIEW_VERT_BLOCK_GLSL
    "struct VertToFrag\n"
    "{\n"
    "    vec4 color;\n"
    "};\n"
    "out VertToFrag vtf;\n"
    "void main()\n"
    "{\n"
    "    vtf.color = colorIn;\n"
    "    gl_Position = mv * vec4(posIn[gl_VertexID], 1.0);\n"
    "}\n";

    static const char* FS =
    "#version 330\n"
    "struct VertToFrag\n"
    "{\n"
    "    vec4 color;\n"
    "};\n"
    "in VertToFrag vtf;\n"
    "layout(location=0) out vec4 colorOut;\n"
    "void main()\n"
    "{\n"
    "    colorOut = vtf.color;\n"
    "}\n";

    static const char* BlockNames[] = {"SpecterViewBlock"};

    m_solidShader = factory->newShaderPipeline(VS, FS, 0, nullptr, 1, BlockNames,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               false, false, false);
}

View::View(ViewSystem& system)
{
    m_bgVertBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex,
                                       sizeof(Zeus::CVector3f), 4);

    m_bgInstBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex,
                                       sizeof(Zeus::CColor), 1);

    m_specterVertBlock =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Uniform,
                                       sizeof(VertexBlock), 1);

    if (!system.m_viewSystem.m_vtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_bgVertBuf, nullptr, boo::VertexSemantic::Position4, 0},
            {m_bgInstBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
        };
        m_bgVtxFmt = system.m_factory->newVertexFormat(2, vdescs);
        m_bgShaderBinding =
        system.m_factory->newShaderDataBinding(system.m_viewSystem.m_solidShader, m_bgVtxFmt,
                                               m_bgVertBuf, m_bgInstBuf, nullptr, 1,
                                               (boo::IGraphicsBuffer**)&m_specterVertBlock,
                                               0, nullptr);
    }

}

void View::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    bindScissor(gfxQ);
    int pendingSlot = 1 << gfxQ->pendingDynamicSlot();
    if ((m_validDynamicSlots & pendingSlot) == 0)
    {
        m_bgVertBuf->load(m_bgRect, sizeof(Zeus::CVector3f) * 4);
        m_bgInstBuf->load(&m_bgColor, sizeof(Zeus::CColor));
        m_validDynamicSlots |= pendingSlot;
    }
    gfxQ->draw(0, 4);
}

}
