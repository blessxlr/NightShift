#include "FedorModel.hpp"

#include <SFML/Graphics/Image.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace
{
constexpr int width = 192, height = 288;
constexpr float pi = 3.14159265f;
constexpr float scale = 27.f;
struct V { float x, y, z; };
V operator+(V a,V b) { return {a.x+b.x,a.y+b.y,a.z+b.z}; }
V operator-(V a,V b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
V operator*(V p,float scale) { return {p.x*scale,p.y*scale,p.z*scale}; }
V cross(V a,V b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
float dot(V a,V b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
V unit(V p)
{
    return p * (1.f / std::sqrt(std::max(dot(p, p), 1e-12f)));
}
V rotateZ(V p, float cosine, float sine)
{
    return {p.x * cosine - p.y * sine, p.x * sine + p.y * cosine, p.z};
}
V rotateX(V p,float c,float s) { return {p.x,p.y*c-p.z*s,p.y*s+p.z*c}; }
V rotateY(V p,float c,float s) { return {p.x*c+p.z*s,p.y,p.z*c-p.x*s}; }

enum Part { core, head, jaw, leftArm, rightArm, leftLeg, rightLeg, leftShin, rightShin, leftEar, rightEar };
enum class ConceptSurface { None, Enamel, Snout, Patch, Glass, Cloth, TarnishedBrass };
struct Material { V base; float metal, corrosion, emission; ConceptSurface surface = ConceptSurface::None; };
constexpr Material shell{{73,78,77},0.89f,0.87f,0.f};
constexpr Material plate{{99,107,106},0.85f,0.94f,0.f};
constexpr Material muzzle{{111,105,92},0.64f,0.73f,0.f};
constexpr Material dark{{29,33,33},0.34f,0.26f,0.f};
constexpr Material rubber{{13,15,16},0.08f,0.f,0.f};
constexpr Material steel{{118,132,135},0.94f,0.24f,0.f};
constexpr Material tooth{{150,144,120},0.26f,0.34f,0.f};
constexpr Material eye{{108,145,145},0.53f,0.f,0.29f};
constexpr Material wire{{79,35,24},0.2f,0.f,0.f};
constexpr Material rabbitShell{{83,78,98},0.85f,0.63f,0.f};
constexpr Material rabbitArmor{{109,98,114},0.78f,0.70f,0.f};
constexpr Material rabbitMuzzle{{143,127,109},0.64f,0.45f,0.f};
constexpr Material rabbitEye{{172,169,134},0.44f,0.f,0.26f};
constexpr Material henShell{{158,123,64},0.77f,0.51f,0.f};
constexpr Material henArmor{{188,151,83},0.72f,0.46f,0.f};
constexpr Material henBeak{{176,108,43},0.65f,0.30f,0.f};
constexpr Material henCrest{{115,50,46},0.66f,0.41f,0.f};
constexpr Material henEye{{197,128,48},0.46f,0.f,0.28f};
struct Vertex { V point, normal; };
struct JointVertex { Vertex surface; Part part; };
struct Face { std::size_t a,b,c; Material paint; };
struct Mesh
{
    std::vector<JointVertex> vertices;
    std::vector<Face> faces;
};

void addFoxyHead(Mesh& mesh, bool closeup)
{
    constexpr Material enamel{{138, 53, 30}, 0.79f, 0.39f, 0.f, ConceptSurface::Enamel};
    constexpr Material copper{{166, 76, 44}, 0.83f, 0.41f, 0.f, ConceptSurface::Enamel};
    constexpr Material sand{{169, 130, 88}, 0.65f, 0.35f, 0.f, ConceptSurface::Snout};
    constexpr Material leather{{49, 39, 34}, 0.14f, 0.18f, 0.f, ConceptSurface::Patch};
    constexpr Material lens{{136, 125, 102}, 0.95f, 0.02f, 0.18f, ConceptSurface::Glass};
    struct Ring { V center; float width, thickness; };
    const int sides = closeup ? 40 : 20;

    const auto loft = [&](Part part, Material material, V lateral, V vertical,
                          std::initializer_list<Ring> rings) {
        const auto start = mesh.vertices.size();
        const std::vector<Ring> sections(rings);
        const auto onRing = [&](std::size_t j, float c, float s) {
            const auto& ring = sections[j];
            return ring.center + lateral * (c * ring.width) + vertical * (s * ring.thickness);
        };
        for (std::size_t j = 0; j < sections.size(); ++j)
        {
            for (int i = 0; i < sides; ++i)
            {
                const float theta = i * (2.f * pi / sides);
                const float c = std::cos(theta), s = std::sin(theta);
                const V tangent = lateral * (-s * sections[j].width) + vertical * (c * sections[j].thickness);
                const V length = onRing(std::min(j + 1, sections.size() - 1), c, s) -
                                 onRing(j ? j - 1 : j, c, s);
                V normal = unit(cross(tangent, length));
                if (dot(normal, lateral * c + vertical * s) < 0.f)
                    normal = normal * -1.f;
                mesh.vertices.push_back({{onRing(j, c, s), normal}, part});
            }
        }
        const auto triangle = [&](std::size_t a, std::size_t b, std::size_t c) {
            const auto& v = mesh.vertices;
            if (dot(cross(v[b].surface.point - v[a].surface.point,
                          v[c].surface.point - v[a].surface.point), v[a].surface.normal) < 0.f)
                std::swap(b, c);
            mesh.faces.push_back({a, b, c, material});
        };
        for (std::size_t j = 1; j < sections.size(); ++j)
            for (int i = 0; i < sides; ++i)
            {
                const auto a = start + (j - 1) * sides + i;
                const auto b = start + (j - 1) * sides + (i + 1) % sides;
                triangle(a, b, a + sides);
                triangle(b, b + sides, a + sides);
            }
    };
    const V x{1,0,0}, y{0,1,0}, z{0,0,1};
    loft(head, enamel, x, z, {{{0,7.43f,0},0.69f,0.61f},{{0,7.78f,0},1.13f,0.83f},
        {{0,8.40f,-0.03f},1.23f,0.94f},{{0,8.90f,-0.04f},1.07f,0.84f},
        {{0,9.17f,-0.03f},0.76f,0.60f},{{0,9.31f,-0.06f},0.02f,0.02f}});
    for (float side : {-1.f, 1.f})
    {
        const Part ear = side < 0.f ? leftEar : rightEar;
        const float tip = side < 0.f ? 10.69f : 10.58f;
        loft(ear, copper, x, z, {{{side*0.76f,8.92f,-0.09f},0.26f,0.19f},
            {{side*1.02f,9.40f,-0.11f},0.47f,0.25f},{{side*1.22f,9.83f,-0.17f},0.40f,0.19f},
            {{side*1.45f,10.29f,-0.21f},0.22f,0.12f},{{side*1.64f,tip,-0.24f},0.012f,0.018f}});
        loft(ear, leather, x, z, {{{side*0.96f,9.22f,0.10f},0.10f,0.045f},
            {{side*1.09f,9.49f,0.09f},0.32f,0.078f},{{side*1.28f,9.90f,0.003f},0.26f,0.072f},
            {{side*1.56f,tip-0.19f,-0.12f},0.012f,0.02f}});
        for (int layer = 0; layer < 2; ++layer)
        {
            const float h = 7.64f + layer * 0.33f;
            loft(head, layer ? copper : enamel, y,z, {{{side*0.88f,h,0.17f},0.24f,0.51f},
                {{side*1.25f,h-0.09f,0.08f},0.23f,0.39f},
                {{side*(1.70f-layer*0.18f),h+0.09f,-0.13f},0.02f,0.018f}});
        }
        for (int i = 0; i < 5; ++i)
        {
            const float depth = 0.75f + 0.25f*i;
            const float offset = side*(0.54f - i*0.052f);
            const float root = 7.78f - 0.033f * i;
            const float tipY = root - (i == 3 ? 0.39f : 0.24f);
            loft(head, tooth, x,z, {{{offset,root,depth},0.105f,0.11f},
                {{offset,root-0.08f,depth},0.095f,0.10f},{{offset*0.94f,tipY,depth-0.05f},0.008f,0.008f}});
            loft(jaw, tooth, x,z, {{{offset,7.10f,depth-0.11f},0.094f,0.09f},
                {{offset*0.96f,7.25f+(i%3)*0.04f,depth-0.08f},0.009f,0.009f}});
        }
    }
    loft(jaw, enamel, x,y, {{{0,7.19f,-0.16f},0.78f,0.39f},{{0,7.02f,0.49f},0.75f,0.27f},
        {{0,6.96f,1.29f},0.56f,0.18f},{{0,7.01f,1.84f},0.31f,0.14f},{{0,7.06f,1.96f},0.02f,0.02f}});
    loft(jaw, rubber, x,y, {{{0,7.13f,0.46f},0.56f,0.16f},{{0,7.1f,1.21f},0.49f,0.10f},
        {{0,7.11f,1.83f},0.24f,0.05f},{{0,7.11f,1.90f},0.02f,0.01f}});
    for (int i = 0; i < 4; ++i)
    {
        const float t = i/4.f;
        const float radius = 0.71f - t*0.39f;
        const float h = 8.03f - 0.22f*t;
        const float depth = 0.70f + 1.48f*t;
        loft(head, sand, x,y, {{{0,h,depth},radius*0.96f,radius*0.76f},
            {{0,h,depth+0.04f},radius,radius*0.80f},
            {{0,h-0.06f,depth+0.36f},radius-0.069f,(radius-0.075f)*0.80f},
            {{0,h-0.06f,depth+0.38f},radius-0.080f,(radius-0.084f)*0.78f}});
    }
    loft(head, rubber, x,y, {{{0,7.84f,2.10f},0.36f,0.23f},{{0,7.85f,2.23f},0.40f,0.24f},
        {{0,7.82f,2.36f},0.33f,0.17f},{{0,7.81f,2.41f},0.02f,0.02f}});
    loft(head, dark, x,y, {{{0.57f,8.43f,0.55f},0.46f,0.45f},{{0.57f,8.43f,0.91f},0.44f,0.42f},
        {{0.57f,8.43f,1.01f},0.36f,0.34f}});
    loft(head, lens, x,y, {{{0.57f,8.43f,0.89f},0.32f,0.30f},{{0.57f,8.43f,1.06f},0.30f,0.28f},
        {{0.57f,8.43f,1.19f},0.17f,0.16f},{{0.57f,8.43f,1.22f},0.01f,0.01f}});
    loft(head, leather, x,y, {{{-0.54f,8.48f,0.70f},0.55f,0.43f},{{-0.54f,8.47f,0.94f},0.52f,0.41f},
        {{-0.54f,8.45f,1.06f},0.43f,0.32f},{{-0.54f,8.45f,1.10f},0.01f,0.01f}});
    for (int tuft = -1; tuft <= 1; ++tuft)
        loft(head, copper, x,z, {{{tuft*0.15f,9.25f,0.05f},0.071f,0.081f},
            {{tuft*0.22f,9.50f,-0.02f},0.049f,0.055f},{{tuft*0.34f,9.66f+(tuft==0?0.13f:0.f),-0.12f},0.008f,0.008f}});
}

Mesh makeAnimatronic(bool closeup, AnimatronicKind kind)
{
    Mesh mesh;
    const bool rabbit = kind == AnimatronicKind::Bonny;
    const bool hen = kind == AnimatronicKind::Chica;
    const bool fox = kind == AnimatronicKind::Foxy;
    const bool golden = kind == AnimatronicKind::GoldenFedor;
    constexpr Material foxShell{{140,53,31},0.83f,0.40f,0.f, ConceptSurface::Enamel};
    constexpr Material foxArmor{{165,73,45},0.80f,0.42f,0.f, ConceptSurface::Enamel};
    constexpr Material brass{{143,111,54},0.84f,0.64f,0.f,ConceptSurface::TarnishedBrass};
    constexpr Material wornBrass{{162,124,65},0.80f,0.60f,0.f,ConceptSurface::TarnishedBrass};
    const Material& shellMaterial = golden ? brass : shell;
    const Material& plateMaterial = golden ? wornBrass : plate;
    const Material& muzzleMaterial = golden ? wornBrass : muzzle;
    const Material bodyShell = fox ? foxShell : hen ? henShell : rabbit ? rabbitShell : shellMaterial;
    const Material armor = fox ? foxArmor : hen ? henArmor : rabbit ? rabbitArmor : plateMaterial;
    const auto solid = [&](Part part, V at, V size, Material color, float roundness = 0.85f,
                           float tilt = 0.f) {
        const bool detail = std::max({size.x, size.y, size.z}) < 0.29f;
        const int rings = detail ? 4 : closeup ? 18 : 9;
        const int sides = detail ? 8 : closeup ? 32 : 16;
        const float cosine = std::cos(tilt);
        const float sine = std::sin(tilt);
        const auto rounded=[&](float v) {
            return std::copysign(std::pow(std::abs(v),roundness),v);
        };
        const auto point=[&](int r,int i) {
            const float latitude = -pi / 2.f + pi * r / rings;
            const float longitude = 2.f * pi * i / sides;
            const float level = std::max(0.f, std::cos(latitude));
            const V sphere{level * std::cos(longitude), std::sin(latitude), level * std::sin(longitude)};
            const float warp = detail ? 1.f : 1.f + 0.048f *
                std::sin(sphere.x * 10.f + sphere.y * 5.f + at.y) *
                std::sin(sphere.z * 9.f - sphere.y * 12.f + at.x);
            const V position = V{size.x * rounded(sphere.x), size.y * rounded(sphere.y),
                                 size.z * rounded(sphere.z)} * warp;
            const auto normalComponent = [roundness](float v, float radius) {
                return std::copysign(std::pow(std::abs(v), 2.f - roundness), v) / radius;
            };
            const V normal = unit({normalComponent(sphere.x, size.x), normalComponent(sphere.y, size.y),
                                   normalComponent(sphere.z, size.z)});
            return Vertex{at + rotateZ(position, cosine, sine), rotateZ(normal, cosine, sine)};
        };
        const std::size_t start = mesh.vertices.size();
        for (int r = 0; r <= rings; ++r)
            for (int i = 0; i < sides; ++i)
                mesh.vertices.push_back({point(r, i), part});
        for (int r = 0; r < rings; ++r)
            for (int i = 0; i < sides; ++i)
            {
                const std::size_t here = start + r * sides;
                const std::size_t next = (i + 1) % sides;
                if (r > 0)
                    mesh.faces.push_back({here + i, here + sides + i, here + next, color});
                if (r + 1 < rings)
                    mesh.faces.push_back({here + sides + i, here + sides + next, here + next, color});
            }
    };

    const auto bolt = [&](Part part, V at, float radius) {
        solid(part, at, {radius * 1.34f, radius * 1.34f, 0.048f}, dark, 0.64f);
        solid(part, at + V{0,0,0.035f}, {radius, radius, 0.048f}, steel, 0.53f);
        solid(part, at + V{0,0,0.078f}, {radius * 0.66f, radius * 0.18f, 0.011f}, dark, 0.58f, -0.5f);
    };

    solid(core,{0,5.50f,-0.13f},{rabbit ? 1.58f : 1.72f,1.40f,0.97f},bodyShell,0.79f);
    solid(core,{0,5.35f,0.72f},{1.39f,0.98f,0.24f},dark,0.65f);
    solid(core,{0.05f,5.70f,0.80f},{1.34f,0.54f,0.32f},armor,0.71f,-0.09f);
    solid(core,{-0.03f,5.00f,0.70f},{1.19f,0.41f,0.30f},armor,0.73f,0.05f);
    solid(core,{0,4.47f,-0.02f},{1.30f,0.60f,0.70f},dark,0.57f);
    solid(core,{0.19f,4.46f,0.59f},{1.02f,0.40f,0.35f},bodyShell,0.70f);
    solid(core,{0,6.69f,0.25f},{0.51f,0.60f,0.57f},dark,0.53f);
    solid(core,{0,5.76f,-0.87f},{0.53f,0.93f,0.43f},dark,0.55f);
    for (int ring = 0; ring < 4; ++ring)
    {
        solid(core,{0,6.39f + ring * 0.16f,0.30f},{0.43f,0.072f,0.46f},steel,0.72f);
        solid(core,{0,5.12f + ring * 0.42f,-1.17f},{0.50f,0.075f,0.17f},steel,0.58f);
    }
    for(float side : {-1.f,1.f})
    {
        const Part arm=side<0?leftArm:rightArm;
        const Part leg=side<0?leftLeg:rightLeg;
        const Part shin=side<0?leftShin:rightShin;
        const float x=side*2.16f, hip=side*0.84f;
        solid(arm,{side*1.82f,5.97f,-0.04f},{0.58f,0.65f,0.64f},dark,0.83f);
        solid(arm,{side*2.00f,6.13f,0.06f},{0.67f,0.62f,0.70f},armor,0.69f,side * 0.28f);
        solid(arm,{x,5.41f,-0.10f},{0.47f,0.65f,0.48f},bodyShell,0.81f,side * -0.07f);
        solid(arm,{x,4.79f,-0.02f},{0.38f,0.34f,0.36f},dark);
        solid(arm,{x,4.81f,0.31f},{0.24f,0.23f,0.11f},steel,0.80f);
        solid(arm,{x,4.11f,0.09f},{0.47f,0.63f,0.48f},armor,0.52f,-side * 0.15f);
        solid(arm,{x+side*0.12f,4.08f,0.46f},{0.22f,0.48f,0.14f},shellMaterial,0.52f,-side * 0.16f);
        bolt(arm,{x - side*0.20f,4.35f,0.53f},0.068f);
        bolt(arm,{x + side*0.27f,3.88f,0.51f},0.060f);
        bolt(arm,{side*2.18f,6.37f,0.69f},0.083f);
        solid(arm,{x,3.48f,0.14f},{0.29f,0.22f,0.29f},dark,0.60f);
        solid(arm,{x,3.26f,0.20f},{0.42f,0.36f,0.35f},shellMaterial,0.68f,-side * 0.12f);
        solid(arm,{x-side*0.36f,3.19f,0.37f},{0.16f,0.29f,0.15f},plateMaterial,0.72f,side * 0.27f);
        for (int finger = 0; finger < 4; ++finger)
        {
            const float digit = x + (finger - 1.5f) * 0.19f;
            const float end = 2.89f + 0.05f * std::abs(finger - 1);
            solid(arm,{digit,end + 0.08f,0.25f},{0.091f,0.24f,0.16f},plateMaterial,0.54f);
            solid(arm,{digit,end-0.16f,0.30f},{0.087f,0.14f,0.16f},steel,0.64f);
        }
        solid(leg,{hip,3.47f,0},{0.66f,0.94f,0.63f},bodyShell,0.67f);
        solid(leg,{hip,3.45f,0.48f},{0.45f,0.61f,0.19f},armor,0.56f);
        solid(shin,{hip,2.37f,0},{0.45f,0.39f,0.44f},steel);
        solid(shin,{hip,2.37f,0.36f},{0.41f,0.29f,0.20f},plateMaterial,0.60f);
        solid(shin,{hip,1.42f,0.02f},{0.54f,0.84f,0.49f},bodyShell,0.50f);
        solid(shin,{hip,1.43f,0.44f},{0.34f,0.63f,0.13f},armor,0.55f,side * 0.10f);
        bolt(shin,{hip + side*0.17f,1.79f,0.57f},0.079f);
        bolt(shin,{hip - side*0.17f,1.08f,0.55f},0.064f);
        solid(shin,{hip,0.60f,0.09f},{0.35f,0.28f,0.37f},steel);
        solid(shin,{hip,0.33f,0.36f},{0.69f,0.32f,0.85f},hen ? henArmor : plateMaterial,0.58f);
        for(int toe=-1;toe<=1;++toe)
            solid(shin,{hip+toe*0.33f,0.24f,0.96f},{0.17f,0.21f,hen ? 0.43f : 0.31f},
                  hen ? henBeak : muzzleMaterial,0.67f);
        if (kind == AnimatronicKind::Fedor || golden)
        {
            const float earY = side < 0.f ? 8.96f : 9.21f;
            solid(head,{side*1.31f,earY - 0.23f,-0.11f},{0.24f,0.45f,0.22f},steel,0.68f,side * -0.31f);
            solid(head,{side*1.47f,earY,-0.09f},{0.44f,0.39f,0.27f},plateMaterial,0.66f,side * -0.41f);
            solid(head,{side*1.49f,earY,0.13f},{0.31f,0.27f,0.10f},dark,0.77f,side * -0.41f);
            solid(head,{side*1.02f,7.66f,0.24f},{0.41f,0.46f,0.49f},shellMaterial,0.75f,side * 0.29f);
        }
        solid(jaw,{side*1.05f,7.20f,0.41f},{0.18f,0.38f,0.28f},steel,0.85f,side * 0.25f);
        solid(jaw,{side*1.13f,7.24f,0.53f},{0.17f,0.16f,0.09f},dark);
        for (const auto fastener : {V{side*0.94f,5.92f,1.02f}, V{side*1.20f,5.64f,0.98f},
                                V{side*0.91f,4.91f,0.91f}})
            bolt(core,fastener,0.070f);
    }
    if (fox)
    {
        for (auto& vertex : mesh.vertices)
        {
            const float waist = std::clamp(std::abs(vertex.surface.point.y - 4.2f), 0.f, 1.f);
            const float factor = 0.68f + 0.18f * waist;
            vertex.surface.point.x *= factor;
            vertex.surface.normal.x /= factor;
            vertex.surface.normal = unit(vertex.surface.normal);
        }
        addFoxyHead(mesh, closeup);
        constexpr Material fabric{{83,43,31},0.08f,0.11f,0.f,ConceptSurface::Cloth};
        const auto collarStart = mesh.vertices.size();
        constexpr int pleats = 58;
        for (int row = 0; row < 4; ++row)
            for (int i = 0; i < pleats; ++i)
            {
                const float a = 2.f*pi*i/pleats;
                const float fold = 0.065f*std::sin(8.f*a) + 0.035f*std::cos(15.f*a);
                const float spread = row / 3.f;
                const float hem = row == 3 ? 0.35f*std::abs(std::sin(i*1.48f)) : 0.f;
                const float sag = (0.38f + 0.32f*std::cos(a-0.5f))*spread;
                const V v{(0.63f+spread*0.78f+fold)*std::cos(a),
                          6.99f-spread*0.56f-sag-hem,
                          (0.56f+spread*0.52f+fold)*std::sin(a)};
                mesh.vertices.push_back({{v,unit({std::cos(a),0.48f,std::sin(a)})},core});
            }
        for (int row=0;row<3;++row)
            for (int i=0;i<pleats;++i)
            {
                const auto a=collarStart+row*pleats+i, b=collarStart+row*pleats+(i+1)%pleats;
                mesh.faces.push_back({a,b,a+pleats,fabric});
                mesh.faces.push_back({b,b+pleats,a+pleats,fabric});
            }
        solid(head,{0,7.42f,0.44f},{0.65f,0.44f,0.44f},rubber,0.8f);
        solid(jaw,{0,7.08f,1.12f},{0.47f,0.12f,0.63f},rubber,0.74f);
        constexpr Material strap{{48,37,31},0.18f,0.11f,0.f,ConceptSurface::Patch};
        for (int i = 0; i < 28; ++i)
        {
            const float t = i / 27.f;
            const float a = -0.48f + 2.60f*t;
            const float h = 8.55f + 0.65f*std::sin(t*pi);
            const float crown = 1.f - 0.24f*std::sin(t*pi);
            solid(head,{1.23f*crown*std::sin(a),h,0.96f*crown*std::cos(a)},
                  {0.113f,0.081f,0.10f},strap,0.81f,0.33f);
        }
        solid(head,{0.63f,8.86f,0.79f},{0.38f,0.10f,0.12f},foxArmor,0.70f,-0.16f);
        solid(head,{0.59f,8.67f,0.91f},{0.38f,0.105f,0.13f},foxShell,0.8f,-0.16f);
        solid(head,{0.59f,8.18f,0.90f},{0.33f,0.075f,0.10f},rubber,0.82f,-0.03f);
        solid(head,{0.03f,8.97f,0.72f},{0.032f,0.29f,0.033f},dark,0.7f,-0.32f);
        solid(head,{-0.99f,8.43f,0.36f},{0.41f,0.092f,0.23f},rubber,0.7f,-0.12f);
        for (float side : {-1.f,1.f})
        {
            bolt(head,{side*0.99f,8.33f,0.70f},0.057f);
            bolt(head,{side*0.80f,8.97f,0.54f},0.046f);
            bolt(head,{side*0.77f,9.24f,0.17f},0.078f);
            solid(head,{side*0.20f,7.84f,2.36f},{0.088f,0.052f,0.025f},rubber);
            for (int pore=0; pore<3; ++pore)
                solid(head,{side*(0.34f+pore*0.056f),7.89f+(pore%2)*0.07f,2.1f-pore*0.19f},
                      {0.028f,0.035f,0.027f},dark);
        }
        return mesh;
    }
    if (hen)
    {
        solid(head,{0,8.09f,-0.03f},{1.39f,1.02f,0.95f},henShell,0.77f);
        solid(head,{0,8.55f,0.61f},{1.05f,0.25f,0.41f},henArmor,0.65f,-0.033f);
        solid(head,{0.04f,8.72f,0.95f},{0.023f,0.18f,0.024f},dark,0.55f,0.17f);
        solid(head,{-0.42f,9.13f,0.13f},{0.29f,0.41f,0.22f},henCrest,0.56f,0.33f);
        solid(head,{-0.04f,9.39f,0.07f},{0.38f,0.56f,0.24f},henCrest,0.60f,0.16f);
        solid(head,{0.36f,9.18f,-0.01f},{0.31f,0.47f,0.24f},henCrest,0.59f,-0.23f);
        bolt(head,{-0.39f,8.93f,0.36f},0.055f);
        bolt(head,{-0.05f,9.04f,0.31f},0.070f);
        bolt(head,{0.32f,8.94f,0.27f},0.055f);

        solid(head,{0,7.79f,1.02f},{0.83f,0.27f,0.57f},henBeak,0.55f,-0.025f);
        solid(head,{0,7.84f,1.40f},{0.43f,0.21f,0.29f},henBeak,0.58f);
        solid(head,{0,7.44f,0.95f},{0.77f,0.18f,0.38f},rubber,0.61f);
        solid(head,{0,7.30f,0.55f},{0.62f,0.29f,0.30f},dark,0.67f);
        solid(jaw,{0,7.21f,0.95f},{0.83f,0.17f,0.55f},henBeak,0.53f,-0.038f);
        solid(jaw,{0,7.13f,0.63f},{0.77f,0.25f,0.43f},henShell,0.62f);
        for (float side : {-1.f, 1.f})
        {
            solid(head,{side*1.04f,7.83f,0.40f},{0.43f,0.38f,0.46f},henArmor,0.58f,side * 0.37f);
            solid(head,{side*1.09f,7.59f,0.57f},{0.25f,0.17f,0.29f},henShell,0.59f,side*0.37f);
            solid(head,{side*0.54f,8.27f,0.84f},{0.37f,0.32f,0.23f},dark,0.72f);
            solid(head,{side*0.54f,8.29f,0.98f},{0.27f,0.26f,0.15f},rubber,0.86f);
            solid(head,{side*0.54f,8.28f,1.10f},{0.126f,0.137f,0.08f},henEye);
            solid(head,{side*0.54f+0.03f,8.30f,1.17f},{0.039f,0.039f,0.015f},dark);
            solid(head,{side*0.24f,7.94f,1.61f},{0.065f,0.041f,0.018f},dark,0.73f);
            solid(jaw,{side*0.66f,7.36f,0.92f},{0.11f,0.17f,0.11f},steel,0.72f);
            bolt(head,{side*1.19f,7.97f,0.74f},0.058f);
        }
        for (int vent = -1; vent <= 1; ++vent)
            solid(core,{vent*0.29f,5.61f,1.13f},{0.079f,0.16f,0.023f},dark,0.60f,0.13f);
        return mesh;
    }
    if (rabbit)
    {
        solid(head,{0,8.15f,-0.07f},{1.26f,1.03f,0.90f},rabbitShell,0.74f);
        solid(head,{0,8.67f,0.54f},{0.99f,0.35f,0.36f},rabbitArmor,0.71f);
        solid(head,{0,8.74f,0.86f},{0.026f,0.31f,0.025f},dark,0.56f,-0.07f);
        solid(head,{0,7.80f,0.95f},{0.84f,0.30f,0.41f},rabbitMuzzle,0.69f);
        solid(head,{0,7.95f,1.27f},{0.23f,0.12f,0.19f},dark,0.62f);
        solid(head,{0,7.77f,1.35f},{0.017f,0.16f,0.019f},dark,0.54f);
        solid(head,{0,7.27f,0.65f},{0.73f,0.26f,0.34f},rubber,0.66f);
        solid(jaw,{0,7.02f,0.75f},{0.97f,0.25f,0.53f},rabbitShell,0.65f);
        solid(jaw,{0,7.13f,1.04f},{0.74f,0.13f,0.22f},dark,0.62f);
        for (float side : {-1.f, 1.f})
        {
            const Part ear = side < 0.f ? leftEar : rightEar;
            const float earTilt = side < 0.f ? 0.08f : -0.24f;
            const float earTop = side < 0.f ? 10.88f : 11.02f;
            solid(ear,{side*0.63f,9.20f,0.02f},{0.20f,0.44f,0.23f},steel,0.59f,earTilt);
            solid(ear,{side*0.77f,9.87f,-0.02f},{0.32f,0.67f,0.22f},rabbitArmor,0.50f,earTilt);
            solid(ear,{side*0.77f,9.87f,0.18f},{0.21f,0.48f,0.09f},dark,0.62f,earTilt);
            solid(ear,{side*0.89f,10.46f,0.01f},{0.26f,0.14f,0.21f},steel,0.61f,earTilt);
            const float tipTilt = earTilt + (side < 0.f ? 0.38f : -0.06f);
            solid(ear,{side*1.01f,earTop,-0.07f},{0.30f,0.56f,0.18f},rabbitArmor,0.62f,tipTilt);
            solid(ear,{side*1.01f,earTop,0.09f},{0.18f,0.40f,0.084f},dark,0.66f,tipTilt);
            bolt(ear,{side*0.84f,10.41f,0.23f},0.062f);
            bolt(ear,{side*0.68f,9.42f,0.21f},0.060f);
            solid(head,{side*0.96f,7.76f,0.34f},{0.38f,0.31f,0.51f},rabbitArmor,0.64f,side * 0.18f);
            solid(head,{side*0.53f,8.22f,0.77f},{0.38f,0.36f,0.23f},dark,0.67f);
            solid(head,{side*0.53f,8.22f,0.94f},{0.28f,0.26f,0.13f},rubber,0.90f);
            solid(head,{side*0.53f,8.24f,1.035f},{0.166f,0.161f,0.115f},rabbitEye);
            solid(head,{side*0.53f+0.024f,8.25f,1.144f},{0.041f,0.043f,0.010f},dark);
            solid(head,{side*0.10f,7.49f,1.31f},{0.090f,0.15f,0.10f},tooth,0.49f,side * 0.04f);
            solid(head,{side*0.31f,7.51f,1.24f},{0.086f,0.100f,0.086f},tooth,0.49f);
            solid(jaw,{side*0.21f,7.22f,1.22f},{0.085f,0.090f,0.08f},tooth,0.49f);
            bolt(head,{side*0.72f,7.81f,1.16f},0.049f);
        }
        for (unsigned slot = 0; slot < 3; ++slot)
            solid(core,{0.40f,5.78f - 0.17f * slot,1.10f},{0.31f,0.035f,0.020f},dark,0.63f,-0.08f);
        return mesh;
    }
    solid(head,{0.01f,8.16f,-0.01f},{1.33f,1.04f,1.03f},shellMaterial,0.86f);
    solid(head,{-0.46f,8.72f,0.57f},{0.75f,0.43f,0.41f},plateMaterial,0.78f,-0.14f);
    solid(head,{0.56f,8.69f,0.53f},{0.69f,0.44f,0.38f},shellMaterial,0.81f,0.07f);
    solid(head,{-0.14f,8.81f,0.93f},{0.028f,0.26f,0.025f},dark,0.57f,0.39f);
    solid(head,{-0.23f,8.99f,0.87f},{0.024f,0.16f,0.045f},dark,0.59f,-0.32f);
    solid(head,{0,7.73f,1.03f},{0.89f,0.30f,0.48f},muzzleMaterial,0.63f,0.034f);
    for(float side : {-1.f,1.f})
    {
        solid(head,{side*0.56f,8.20f,0.78f},{0.42f,0.35f,0.25f},dark,0.79f,side * 0.19f);
        solid(head,{side*0.57f,8.18f,0.86f},{0.31f,0.27f,0.25f},rubber,0.87f,side * 0.14f);
        solid(head,{side*0.62f,8.53f,0.78f},{0.49f,0.14f,0.19f},plateMaterial,0.60f,side * 0.28f);
        bolt(head,{side*0.97f,8.53f,0.94f},0.058f);
        solid(head,{side*0.60f,7.74f,1.46f},{0.052f,0.048f,0.033f},dark);
        solid(head,{side*0.47f,7.60f,1.48f},{0.064f,0.040f,0.032f},dark);
    }
    if (!golden)
    {
        solid(head,{0.54f,8.19f,1.02f},{0.214f,0.212f,0.17f},steel);
        solid(head,{0.55f,8.19f,1.156f},{0.132f,0.138f,0.075f},eye);
        solid(head,{0.562f,8.196f,1.229f},{0.045f,0.046f,0.011f},dark);
        solid(head,{0.59f,8.237f,1.220f},{0.034f,0.028f,0.019f},tooth);
    }
    else
    {
        solid(head,{0.57f,8.18f,1.059f},{0.20f,0.14f,0.105f},rubber);
        solid(head,{0.61f,8.18f,1.165f},{0.029f,0.023f,0.014f},eye);
    }
    solid(head,{-0.62f,8.09f,1.13f},{0.051f,0.14f,0.034f},wire,0.72f,-0.46f);
    solid(head,{-0.72f,8.14f,1.08f},{0.027f,0.16f,0.040f},steel,0.71f,0.76f);
    solid(head,{0,7.94f,1.38f},{0.28f,0.13f,0.24f},dark,0.62f,-0.035f);
    solid(head,{0.02f,7.77f,1.50f},{0.018f,0.105f,0.020f},dark,0.61f,0.12f);
    solid(head,{0,7.31f,0.65f},{0.85f,0.23f,0.34f},rubber,0.76f);
    solid(head,{0,7.05f,0.37f},{0.35f,0.42f,0.28f},dark,0.79f);
    solid(head,{-0.64f,7.19f,0.70f},{0.084f,0.37f,0.091f},wire,0.64f,-0.31f);
    solid(jaw,{0,7.02f,0.81f},{1.03f,0.25f,0.60f},shellMaterial,0.74f,-0.027f);
    solid(jaw,{0,7.15f,1.07f},{0.79f,0.14f,0.41f},dark,0.65f);
    for (int d = -3; d <= 3; ++d)
    {
        const float x = d * 0.224f;
        const float row = 1.409f - 0.029f * d * d;
        const float toothHeight = d == -2 ? 0.071f : 0.103f - 0.009f * std::abs(d);
        solid(head,{x,7.45f + 0.014f * std::abs(d),row},{0.085f,toothHeight,0.108f},tooth,0.47f,d*0.03f);
        solid(jaw,{x,7.25f,row-0.049f},{0.084f,0.089f,0.102f},tooth,0.50f,-d*0.037f);
    }
    return mesh;
}

float area(V a,V b,float x,float y)
{
    return (b.x-a.x)*(y-a.y)-(b.y-a.y)*(x-a.x);
}

std::uint32_t hash(int x, int y, int z)
{
    auto bits = static_cast<std::uint32_t>(x) * 0x9e3779b1u ^
                static_cast<std::uint32_t>(y) * 0x68e31da4u ^
                static_cast<std::uint32_t>(z) * 0x855c3f19u;
    bits ^= bits >> 16;
    bits *= 0x7feb352du;
    bits ^= bits >> 15;
    return bits;
}

float noiseAt(int x, int y, int z, int period, int step)
{
    const auto weight = [step](int n) {
        const float t = static_cast<float>(n % step) / step;
        return t * t * (3.f - 2.f * t);
    };
    const auto sample = [period](int a, int b, int c) {
        return static_cast<float>(hash(a % period, b % period, c % period) & 255u) / 255.f;
    };
    const auto blend = [](float a, float b, float t) { return a + (b - a) * t; };
    const int ix = x / step, iy = y / step, iz = z / step;
    const float tx = weight(x), ty = weight(y), tz = weight(z);
    const auto layer = [&](int d) {
        return blend(blend(sample(ix, iy, d), sample(ix + 1, iy, d), tx),
                     blend(sample(ix, iy + 1, d), sample(ix + 1, iy + 1, d), tx), ty);
    };
    return blend(layer(iz), layer(iz + 1), tz);
}

struct Finish
{
    float oxidized;
    float grain;
    float scuff;
};

const Finish& finishAt(V p)
{
    constexpr int size = 64;
    static const auto details = [] {
        std::vector<Finish> surface(size * size * size);
        for (int z = 0; z < size; ++z)
            for (int y = 0; y < size; ++y)
                for (int x = 0; x < size; ++x)
                {
                    const float cloud = noiseAt(x, y, z, 4, 16);
                    const float speckle = noiseAt(x, y, z, 8, 8);
                    const float pits = noiseAt(x, y, z, 32, 2);
                    const auto grit = static_cast<float>(hash(x, y, z) & 255u) / 255.f;
                    const float crust = std::clamp(
                        (cloud * 0.59f + speckle * 0.28f + pits * 0.13f - 0.36f) * 2.45f,
                        0.f, 1.f);
                    const float scuff = std::max(0.f, (grit - 0.78f) * 4.5f) * (1.f - crust);
                    surface[(z * size + y) * size + x] = {crust, pits * 0.65f + grit * 0.35f, scuff};
                }
        return surface;
    }();
    const auto coord = [](float v) {
        return static_cast<unsigned>(static_cast<int>((v + 18.f) * 37.f)) & (size - 1);
    };
    return details[(coord(p.z) * size + coord(p.y)) * size + coord(p.x)];
}

class MetalTexture
{
public:
    MetalTexture()
    {
        sf::Image image;
        if (image.loadFromFile("assets/images/fedor-cast-steel.png"))
        {
            const auto size = image.getSize();
            const auto* source = image.getPixelsPtr();
            texels_.resize(textureSize * textureSize);
            for (unsigned y = 0; y < textureSize; ++y)
                for (unsigned x = 0; x < textureSize; ++x)
                {
                    const auto* texel = source + 4u * (y * size.y / textureSize * size.x +
                                                       x * size.x / textureSize);
                    texels_[y * textureSize + x] = {texel[0], texel[1], texel[2]};
                }
        }
    }

    bool ready() const
    {
        return !texels_.empty();
    }

    V color(V point, V normal) const
    {
        const float x = normal.x * normal.x;
        const float y = normal.y * normal.y;
        const float z = normal.z * normal.z;
        const V weight{x * x, y * y, z * z};
        return (pixel(point.z, point.y) * weight.x +
                pixel(point.x, point.z) * weight.y +
                pixel(point.x, point.y) * weight.z) *
            (1.f / std::max(weight.x + weight.y + weight.z, 1e-8f));
    }

private:
    static constexpr unsigned textureSize = 1024;

    static unsigned coordinate(float value)
    {
        constexpr float texelsPerUnit = 112.f;
        const auto position = static_cast<unsigned>((value + 16.f) * texelsPerUnit);
        const unsigned folded = position & (2 * textureSize - 1);
        return folded < textureSize ? folded : 2 * textureSize - folded - 1;
    }

    V pixel(float horizontal, float vertical) const
    {
        const auto texel = texels_[coordinate(vertical) * textureSize + coordinate(horizontal)];
        return {float(texel.r), float(texel.g), float(texel.b)};
    }

    std::vector<sf::Color> texels_;
};

class ConceptFinish
{
public:
    ConceptFinish()
    {
        if (art_.loadFromFile("assets/images/animatronics-lineup.png") &&
            art_.getSize() == sf::Vector2u{2172,724})
            pixels_ = art_.getPixelsPtr();
    }

    V sample(ConceptSurface layer, V p, V base) const
    {
        if (!pixels_)
            return base;
        const auto tile = [](float n) { return 1.f - std::abs(2.f*(n-std::floor(n))-1.f); };
        const float u = tile(0.61f*p.x + 0.37f*p.z);
        const float v = tile(0.47f*p.y + 0.22f*p.z);
        float x = 1439.f + 74.f*u, y = 231.f + 43.f*v;
        switch (layer)
        {
            case ConceptSurface::Glass:
                x = 1586.f + 66.f*(p.x-0.57f);
                y = 321.f - 67.f*(p.y-8.43f);
                break;
            case ConceptSurface::Patch:
                x = 1446.f + 48.f*u; y = 292.f + 47.f*v;
                break;
            case ConceptSurface::Snout:
                x = 1525.f - 55.f*(p.z-0.78f) + 64.f*p.x;
                y = 371.f + 47.f*(p.z-0.78f) - 56.f*(p.y-8.03f);
                break;
            case ConceptSurface::Cloth:
                x = 1584.f + 45.f*u; y = 609.f + 53.f*v;
                break;
            case ConceptSurface::TarnishedBrass:
                x = 1968.f + 54.f*u; y = 149.f + 73.f*v;
                break;
            default: break;
        }
        const auto* texel = pixels_ + 4*(static_cast<int>(y)*2172 + static_cast<int>(x));
        const V color{float(texel[0]),float(texel[1]),float(texel[2])};
        if (layer == ConceptSurface::TarnishedBrass)
            return V{color.x * 0.96f, color.y * 0.95f, color.z * 0.76f} + base * 0.29f;
        return color * 1.22f + base * 0.21f;
    }

private:
    sf::Image art_;
    const std::uint8_t* pixels_ = nullptr;
};

V litMetal(const Material& material, V normal, const Finish& finish,
           const MetalTexture& texture, V position, V surfaceNormal)
{
    float rustAmount = material.corrosion * finish.oxidized;
    const V steelTone = material.base * (0.77f + 0.47f * finish.grain);
    const V rustTone{99.f + 38.f * finish.grain, 53.f + 15.f * finish.grain, 28.f + 10.f * finish.grain};
    V albedo = steelTone * (1.f - rustAmount) + rustTone * rustAmount;
    if (material.corrosion > 0.20f && texture.ready())
    {
        const auto photo = texture.color(position, surfaceNormal);
        const float oxidation = std::clamp((photo.x - photo.z) / 32.f, 0.f, 1.f);
        const float photoWeight = std::min(0.94f, material.corrosion);
        const V naturalTone = photo * 1.10f + V{8.f, 10.f, 12.f};
        albedo = albedo * (1.f - photoWeight) + naturalTone * photoWeight;
        rustAmount = 0.34f * rustAmount + 0.66f * oxidation;
    }
    if (material.surface != ConceptSurface::None)
    {
        static const ConceptFinish finishArt;
        albedo = finishArt.sample(material.surface, position, albedo);
    }
    const float lambert = std::max(0.f, dot(normal, {-0.50f, 0.73f, 0.46f}));
    const float fill = std::max(0.f, dot(normal, {0.83f, 0.27f, -0.49f}));
    const float rim = std::max(0.f, dot(normal, {-0.83f, 0.18f, -0.53f}));
    const float reflection = std::max(0.f, dot(normal, {-0.32f, 0.46f, 0.83f}));
    const float square = reflection * reflection;
    const float broad = square * square * square * square;
    const float specular = material.metal * (1.f - rustAmount * 0.83f) *
                           (broad * (34.f + 24.f * finish.scuff) + broad * broad * 41.f);
    const float diffuse = 0.19f + 0.73f * lambert + 0.25f * fill;
    const float metalGleam = specular + finish.scuff * material.metal * lambert * 18.f;
    return albedo * (diffuse + material.emission) +
           V{0.78f, 0.96f, 1.15f} * (metalGleam + material.metal * rim * 29.f) +
           V{20.f, 26.f, 29.f} * (fill * material.metal);
}

void paintFace(const Vertex& first, const Vertex& second, const Vertex& third,
               const Vertex& surfaceA, const Vertex& surfaceB, const Vertex& surfaceC,
               const Material& material, const MetalTexture& texture, int resolution, float gameplayScale, float portraitOrigin,
               std::vector<float>& depth, std::vector<std::uint8_t>& pixels)
{
    const V planeNormal = cross(second.point - first.point, third.point - first.point);
    if (planeNormal.z <= 0.f)
        return;
    const int canvasWidth = width * resolution;
    const int canvasHeight = height * resolution;
    const auto project = [resolution, gameplayScale, portraitOrigin](V p) {
        if (resolution == 3)
            return V{(width / 2.f + p.x * scale * 1.26f) * resolution,
                     (portraitOrigin - (p.y - 7.88f) * scale * 1.26f) * resolution, p.z};
        return V{(width / 2.f + p.x * gameplayScale) * resolution,
                 (height - 4.f - p.y * gameplayScale) * resolution, p.z};
    };
    const V a = project(first.point);
    const V b = project(second.point);
    const V c = project(third.point);
    const float determinant = area(a, b, c.x, c.y);
    if (std::abs(determinant) < 1e-6f)
        return;
    const int x0 = std::max(0, static_cast<int>(std::floor(std::min({a.x, b.x, c.x}))));
    const int y0 = std::max(0, static_cast<int>(std::floor(std::min({a.y, b.y, c.y}))));
    const int x1 = std::min(canvasWidth - 1, static_cast<int>(std::ceil(std::max({a.x, b.x, c.x}))));
    const int y1 = std::min(canvasHeight - 1, static_cast<int>(std::ceil(std::max({a.y, b.y, c.y}))));
    const float inverse = 1.f / determinant;
    const V step = (c - b) * inverse;
    const V secondStep = (a - c) * inverse;
    float rowA = area(b, c, x0 + 0.5f, y0 + 0.5f) * inverse;
    float rowB = area(c, a, x0 + 0.5f, y0 + 0.5f) * inverse;
    for (int y = y0; y <= y1; ++y)
    {
        float wa = rowA;
        float wb = rowB;
        rowA += step.x;
        rowB += secondStep.x;
        for (int x = x0; x <= x1; ++x, wa -= step.y, wb -= secondStep.y)
        {
            const float wc = 1.f - wa - wb;
            if (wa < -1e-5f || wb < -1e-5f || wc < -1e-5f)
                continue;
            const float z = wa * a.z + wb * b.z + wc * c.z;
            const int cell = y * canvasWidth + x;
            if (z <= depth[cell])
                continue;
            depth[cell] = z;

            const auto position = surfaceA.point * wa + surfaceB.point * wb + surfaceC.point * wc;
            const auto normal = first.normal * wa + second.normal * wb + third.normal * wc;
            const auto surfaceNormal = surfaceA.normal * wa + surfaceB.normal * wb + surfaceC.normal * wc;
            const V color = litMetal(material, normal, finishAt(position), texture, position, surfaceNormal);
            const auto channel = [](float value) {
                return static_cast<std::uint8_t>(std::clamp(value, 0.f, 255.f));
            };
            pixels[4 * cell] = channel(color.x);
            pixels[4 * cell + 1] = channel(color.y);
            pixels[4 * cell + 2] = channel(color.z);
            pixels[4 * cell + 3] = 255;
        }
    }
}

void shadeRecesses(int resolution, const std::vector<float>& depth, std::vector<std::uint8_t>& pixels)
{
    const int columns = width * resolution;
    const int rows = height * resolution;
    const int reach = 5 * resolution;
    const std::array<int, 8> samples{reach, -reach, reach * columns, -reach * columns,
        reach * (columns + 1), -reach * (columns + 1), reach * (columns - 1), -reach * (columns - 1)};
    for (int y = reach; y < rows - reach; ++y)
        for (int x = reach; x < columns - reach; ++x)
        {
            const int pixel = y * columns + x;
            if (pixels[4 * pixel + 3] == 0)
                continue;
            const float z = depth[pixel];
            float coverage = 0.f;
            for (std::size_t sample = 0; sample < 4; ++sample)
            {
                const float step = depth[pixel + samples[sample]] - z;
                coverage += std::max(0.f, 1.f - std::abs(step - 0.43f) * 2.65f);
            }
            const float shadow = 1.f - coverage * 0.085f;
            for (unsigned channel = 0; channel < 3; ++channel)
                pixels[4 * pixel + channel] = static_cast<std::uint8_t>(pixels[4 * pixel + channel] * shadow);
        }
}
}

FedorModel::FedorModel(bool closeup, AnimatronicKind kind)
    : texture_({width * (closeup ? 3u : kind == AnimatronicKind::GoldenFedor ? 2u : 1u),
                height * (closeup ? 3u : kind == AnimatronicKind::GoldenFedor ? 2u : 1u)}),
      kind_(kind),
      resolutionScale_(static_cast<int>(texture_.getSize().x / width)),
      pixels_(width * height * resolutionScale_ * resolutionScale_ * 4),
      depth_(width * height * resolutionScale_ * resolutionScale_)
{
    texture_.setSmooth(true);
}

const sf::Texture& FedorModel::render(FedorPose pose,float viewAngle,float attack)
{
    const int angleKey=static_cast<int>(std::round(viewAngle*36.f/pi));
    const int phaseKey=static_cast<int>(std::round(pose.phase*12.f/pi));
    const int strideKey=static_cast<int>(std::round(pose.stride*10.f));
    const int attackKey=static_cast<int>(std::round(attack*24.f));
    const int restKey = static_cast<int>(std::round(std::clamp(pose.rest, 0.f, 1.f) * 24.f));
    if(restKey==lastRest_ && angleKey==lastAngle_ && phaseKey==lastPhase_ &&
       strideKey==lastStride_ && attackKey==lastAttack_) return texture_;
    lastRest_ = restKey;
    lastAngle_=angleKey; lastPhase_=phaseKey; lastStride_=strideKey; lastAttack_=attackKey;
    std::fill(pixels_.begin(),pixels_.end(),0);
    std::fill(depth_.begin(),depth_.end(),-std::numeric_limits<float>::infinity());

    const float angle=angleKey*pi/36.f, phase=phaseKey*pi/12.f;
    const float stride=strideKey/10.f, lunge=attackKey/24.f;
    const float swing=std::sin(phase)*stride;
    const float camCos=std::cos(angle), camSin=std::sin(angle);
    const bool fox = kind_ == AnimatronicKind::Foxy;
    const bool golden = kind_ == AnimatronicKind::GoldenFedor;
    const float rest = restKey / 24.f;
    std::array<float,11> rotations{lunge*0.16f, swing*0.08f-lunge*0.20f,
        swing*0.08f+lunge*0.77f, -swing*0.40f-lunge*0.78f, swing*0.40f-lunge*0.92f,
        swing*0.39f, -swing*0.39f, std::max(swing,0.f)*0.48f, std::max(-swing,0.f)*0.48f,
        swing*0.10f-lunge*0.19f, -swing*0.078f+lunge*0.13f};
    if (fox)
    {
        rotations[head] -= 0.37f * rest;
        rotations[jaw] -= 0.16f * (1.f - lunge);
        rotations[leftArm] -= 0.98f * rest + 0.18f * stride;
        rotations[rightArm] -= 0.86f * rest + 0.20f * stride;
        rotations[leftLeg] -= 0.82f * rest;
        rotations[rightLeg] -= 0.98f * rest;
        rotations[leftShin] += 1.69f * rest;
        rotations[rightShin] += 1.74f * rest;
    }
    if (golden)
    {
        rotations[head] += 0.18f;
        rotations[jaw] += 0.19f;
        rotations[leftEar] -= 0.22f;
        rotations[rightEar] += 0.17f;
        if (resolutionScale_ != 3)
        {
            rotations[leftArm] -= 0.68f;
            rotations[rightArm] -= 0.77f;
            rotations[leftLeg] = -1.48f;
            rotations[rightLeg] = -1.32f;
            rotations[leftShin] = -0.03f;
            rotations[rightShin] = -0.14f;
        }
    }
    constexpr std::array<V,11> pivots{{{0,5,0},{0,7.18f,0.10f},{0,7.22f,0.26f},
        {-1.83f,6.02f,0},{1.83f,6.02f,0},{-0.84f,4.22f,0},{0.84f,4.22f,0},
        {-0.84f,2.37f,0},{0.84f,2.37f,0},{-0.63f,8.97f,0.02f},{0.63f,8.97f,0.02f}}};
    std::array<float,11> cosine{},sine{};
    for(std::size_t i=0;i<rotations.size();++i)
    {
        cosine[i]=std::cos(rotations[i]);
        sine[i]=std::sin(rotations[i]);
    }
    const float bob=std::abs(swing)*0.11f;
    const auto pivot=[&](V p,Part part) {
        return rotateX(p-pivots[part],cosine[part],sine[part])+pivots[part];
    };
    const bool seated = golden && resolutionScale_ != 3;
    const float bodyPitch = fox ? rest * 1.16f + stride * 0.16f : 0.f;
    const float pitchCos = std::cos(bodyPitch), pitchSin = std::sin(bodyPitch);
    const auto transform = [&](Vertex vertex, Part part) {
        const auto articulate = [&](Part joint) {
            vertex.point = pivot(vertex.point, joint);
            vertex.normal = rotateX(vertex.normal, cosine[joint], sine[joint]);
        };
        articulate(part);
        if (part == leftShin)
            articulate(leftLeg);
        if (part == rightShin)
            articulate(rightLeg);
        const bool ear = part == leftEar || part == rightEar;
        if (part == jaw || ear)
            articulate(head);
        if (golden && (part == head || part == jaw || ear))
        {
            const V centre{0.f, 7.18f, 0.10f};
            constexpr float rollCos = 0.969f;
            constexpr float rollSin = -0.247f;
            vertex.point = rotateZ(vertex.point - centre, rollCos, rollSin) + centre;
            vertex.normal = rotateZ(vertex.normal, rollCos, rollSin);
        }
        if (part <= rightArm || ear)
            vertex.point.y += bob;
        if (fox)
        {
            vertex.point = rotateX(vertex.point - V{0,4.2f,0}, pitchCos,pitchSin) + V{0,4.2f,0};
            vertex.normal = rotateX(vertex.normal,pitchCos,pitchSin);
            vertex.point.z -= 1.4f * rest;
        }
        vertex.point = rotateY(vertex.point, camCos, camSin);
        vertex.normal = rotateY(vertex.normal, camCos, camSin);
        if (seated)
        {
            vertex.point = rotateX(vertex.point, 0.947f, 0.321f);
            vertex.normal = rotateX(vertex.normal, 0.947f, 0.321f);
        }
        return vertex;
    };
    const bool rabbit = kind_ == AnimatronicKind::Bonny;
    const bool hen = kind_ == AnimatronicKind::Chica;
    const auto& mesh = [&]() -> const Mesh& {
        if (fox)
        {
            if (resolutionScale_ == 3)
            {
                static const auto closeFox = makeAnimatronic(true, AnimatronicKind::Foxy);
                return closeFox;
            }
            static const auto foxMesh = makeAnimatronic(false, AnimatronicKind::Foxy);
            return foxMesh;
        }
        if (hen)
        {
            if (resolutionScale_ == 3)
            {
                static const auto chicaCloseup = makeAnimatronic(true, AnimatronicKind::Chica);
                return chicaCloseup;
            }
            static const auto chica = makeAnimatronic(false, AnimatronicKind::Chica);
            return chica;
        }
        if (golden)
        {
            if (resolutionScale_ == 3)
            {
                static const auto closeGolden = makeAnimatronic(true, AnimatronicKind::GoldenFedor);
                return closeGolden;
            }
            static const auto seatedGolden = makeAnimatronic(true, AnimatronicKind::GoldenFedor);
            return seatedGolden;
        }
        if (rabbit)
        {
            if (resolutionScale_ == 3)
            {
                static const auto bonnyCloseup = makeAnimatronic(true, AnimatronicKind::Bonny);
                return bonnyCloseup;
            }
            static const auto bonny = makeAnimatronic(false, AnimatronicKind::Bonny);
            return bonny;
        }
        if (resolutionScale_ == 3)
        {
            static const auto closeup = makeAnimatronic(true, AnimatronicKind::Fedor);
            return closeup;
        }
        static const auto gameplay = makeAnimatronic(false, AnimatronicKind::Fedor);
        return gameplay;
    }();
    std::vector<Vertex> posed;
    posed.reserve(mesh.vertices.size());
    for (const auto& vertex : mesh.vertices)
        posed.push_back(transform(vertex.surface, vertex.part));

    if ((fox && resolutionScale_ == 1) || seated)
    {
        const auto floor = std::min_element(posed.begin(), posed.end(),
            [](const Vertex& a, const Vertex& b) { return a.point.y < b.point.y; })->point.y;
        for (auto& vertex : posed)
            vertex.point.y -= floor;
    }
    static const MetalTexture metal;
    const float gameplayScale = fox ? scale * (0.94f - 0.16f * rest) : rabbit ? scale * 0.86f : hen ? scale * 0.96f : scale;
    for (const auto& face : mesh.faces)
        paintFace(posed[face.a], posed[face.b], posed[face.c], mesh.vertices[face.a].surface,
                  mesh.vertices[face.b].surface, mesh.vertices[face.c].surface,
                  face.paint, metal, resolutionScale_, gameplayScale, fox ? 103.f : 71.f, depth_, pixels_);

    shadeRecesses(resolutionScale_, depth_, pixels_);
    texture_.update(pixels_.data());
    return texture_;
}
