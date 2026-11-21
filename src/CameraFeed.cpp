#include "CameraFeed.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

namespace
{
using Vec = sf::Vector2<double>;
constexpr unsigned width = 888, height = 468;
constexpr double roomHeight = 32.0;
constexpr double pi = 3.141592653589793;
constexpr double inf = std::numeric_limits<double>::infinity();
double cross(Vec a, Vec b) { return a.x*b.y - a.y*b.x; }
double dot(Vec a, Vec b) { return a.x*b.x + a.y*b.y; }
double fraction(double value) { return value - std::floor(value); }

struct Hit { double depth=inf, along=0; int wall=-1; };

Hit cast(const BuildingMap& map, Vec origin, Vec ray)
{
    Hit hit;
    for (std::size_t i=0; i<map.walls.size(); ++i)
    {
        const auto& w=map.walls[i];
        Vec a{w.x1,w.y1}, segment=Vec{w.x2,w.y2}-a;
        const double det=cross(ray,segment);
        if (std::abs(det)<1e-9) continue;
        Vec delta=a-origin;
        const double depth=cross(delta,segment)/det, u=cross(delta,ray)/det;
        if (depth>1e-6 && depth<hit.depth && u>=0 && u<=1)
            hit={depth,u*std::hypot(segment.x,segment.y),static_cast<int>(i)};
    }
    return hit;
}

struct Lamp
{
    Vec position;
    Vec direction;
    std::array<double,512> reach{};
};

struct Stage
{
    int wall=-1;
    Vec centre{}, tangent{}, normal{};
    double halfWidth=0;
};

struct Decor
{
    Stage stage;
    std::vector<Lamp> lamps;
    enum class FixtureKind { Duct, Cabinet, Tabletop, Seat, Frame, Arcade, Bench, Shelf, Drawers, PartsTray };
    struct Fixture
    {
        Vec lo,hi;
        double bottom,top;
        FixtureKind kind=FixtureKind::Duct;
        int artwork=0;
        bool facesWest=false;
    };
    std::vector<Fixture> fixtures;
    struct TableFootprint { Vec lo,hi; };
    std::vector<TableFootprint> tables;
    struct Part { Vec position; double bottom,width; unsigned tile; };
    std::vector<Part> parts;
};

void addWorkshopFixture(Decor& decor, Vec lo, Vec hi, bool rack)
{
    using Kind = Decor::FixtureKind;
    decor.tables.push_back({lo,hi});
    if (rack)
    {
        for (double top : {3.,12.,21.})
            decor.fixtures.push_back({lo,hi,top-0.65,top,Kind::Shelf});
    }
    else
    {
        decor.fixtures.push_back({lo,hi,9.9,11.,Kind::Bench});
        decor.fixtures.push_back({lo+Vec{0.8,1.3},{hi.x-1,lo.y+8.6},1,9.9,Kind::Drawers});
    }
    for (double x : {lo.x+0.46,hi.x-0.46})
        for (double y : {lo.y+0.46,hi.y-0.46})
            decor.fixtures.push_back({{x-0.43,y-0.43},{x+0.43,y+0.43},0,rack?24.0:9.9,Kind::Frame});
}

void addChair(Decor& decor, Vec centre, double facing)
{
    using Kind = Decor::FixtureKind;
    auto part = [&](double x0, double y0, double x1, double y1,
                    double bottom, double top, Kind kind) {
        decor.fixtures.push_back({centre + Vec{x0,y0}, centre + Vec{x1,y1}, bottom,top,kind});
    };
    part(-2.9,-2.7,2.9,2.7,5.0,5.8,Kind::Seat);
    const double back = 2.45*facing;
    part(-2.9,back-0.3,2.9,back+0.3,8.2,12.3,Kind::Seat);
    for (double x : {-2.45, 2.45})
        for (double y : {-2.45, 2.45})
            part(x-0.22,y-0.22,x+0.22,y+0.22,0,y*facing>0 ? 11.8 : 5.1,Kind::Frame);
}

void addTable(Decor& decor, Vec lo, Vec hi, int seats)
{
    using Kind = Decor::FixtureKind;
    decor.tables.push_back({lo,hi});
    decor.fixtures.push_back({lo,hi,9.5,10.4,Kind::Tabletop});
    for (double x : {lo.x+1.6,hi.x-1.6})
        for (double y : {lo.y+1.5,hi.y-1.5})
            decor.fixtures.push_back({{x-0.42,y-0.42},{x+0.42,y+0.42},0,9.5,Kind::Frame});
    const double spacing = (hi.x-lo.x)/seats;
    for (int seat=0; seat<seats; ++seat)
    {
        const double x=lo.x+(seat+0.5)*spacing;
        addChair(decor,{x,lo.y-3.5},-1);
        addChair(decor,{x,hi.y+3.5},1);
    }
}

constexpr double arcadeHeight=25.0;

void addArcade(Decor& decor, Vec back, bool west, int artwork)
{
    constexpr double halfWidth=5.7;
    auto section=[&](double bottom,double top,double depth) {
        const Vec lo=west ? Vec{back.x-depth,back.y-halfWidth} : Vec{back.x-halfWidth,back.y};
        const Vec hi=west ? Vec{back.x,back.y+halfWidth} : Vec{back.x+halfWidth,back.y+depth};
        decor.fixtures.push_back({lo,hi,bottom,top,Decor::FixtureKind::Arcade,artwork,west});
    };
    section(0,9.8,7.2);
    section(9.8,12.2,8.2);
    section(12.2,20.7,5.9);
    section(20.7,arcadeHeight,7.3);
}

Decor loadDecor(const BuildingMap& map)
{
    Decor decor;
    std::ifstream file("assets/maps/camera-decor.txt");
    std::string line;
    while (std::getline(file,line))
    {
        std::istringstream row(line);
        std::string kind;
        Vec p;
        double value;
        if (!(row>>kind)) continue;
        if (kind=="workbench" || kind=="rack")
        {
            Vec lo,hi;
            if (row>>lo.x>>lo.y>>hi.x>>hi.y && hi.x-lo.x>=9 && hi.y-lo.y>=10)
                addWorkshopFixture(decor,lo,hi,kind=="rack");
            continue;
        }
        if (kind=="part")
        {
            Decor::Part part{};
            if (row>>part.position.x>>part.position.y>>part.bottom>>part.width>>part.tile &&
                std::isfinite(part.position.x) && std::isfinite(part.position.y) &&
                part.bottom>=0 && part.bottom<roomHeight && part.width>0 && part.width<=32 && part.tile<4)
                decor.parts.push_back(part);
            continue;
        }
        if (kind=="arcade")
        {
            std::string direction;
            int artwork;
            if (row>>p.x>>p.y>>direction>>artwork && std::isfinite(p.x) && std::isfinite(p.y) &&
                (direction=="west" || direction=="south") && artwork>=0 && artwork<3)
            {
                const bool west=direction=="west";
                addArcade(decor,p,west,artwork);
                const Vec facing=west ? Vec{-1,0} : Vec{0,1};
                Lamp light{p+facing*8.4,facing};
                for (std::size_t i=0;i<light.reach.size();++i)
                {
                    const double angle=2*pi*i/light.reach.size()-pi;
                    light.reach[i]=cast(map,light.position,{std::cos(angle),std::sin(angle)}).depth;
                }
                decor.lamps.push_back(light);
            }
            continue;
        }
        if (kind=="table")
        {
            Vec lo, hi;
            int seats;
            if (row>>lo.x>>lo.y>>hi.x>>hi.y>>seats && seats>=1 && seats<=8 &&
                hi.x-lo.x>=7.0*seats && hi.y-lo.y>=6.0)
                addTable(decor,lo,hi,seats);
            continue;
        }
        if (kind=="duct" || kind=="cabinet")
        {
            Decor::Fixture box;
            if (kind=="cabinet") box.kind=Decor::FixtureKind::Cabinet;
            if (row>>box.lo.x>>box.lo.y>>box.hi.x>>box.hi.y>>box.bottom>>box.top &&
                box.lo.x<box.hi.x && box.lo.y<box.hi.y &&
                box.bottom>=0 && box.bottom<box.top && box.top<=roomHeight)
                decor.fixtures.push_back(box);
            continue;
        }
        if (!(row>>p.x>>p.y>>value) || !std::isfinite(p.x) ||
            !std::isfinite(p.y) || !std::isfinite(value) || value<=0) continue;
        if (kind=="backdrop")
        {
            for (std::size_t i=0; i<map.walls.size(); ++i)
            {
                const auto& w=map.walls[i];
                Vec a{w.x1,w.y1}, delta=Vec{w.x2,w.y2}-a;
                const double length=std::hypot(delta.x,delta.y);
                if (length<value) continue;
                const Vec tangent=delta/length, normal{-tangent.y,tangent.x};
                const double along=dot(p-a,tangent);
                if (std::abs(dot(p-a,normal))>1 || along<value/2 || along>length-value/2) continue;
                decor.stage={static_cast<int>(i),a+tangent*along,tangent,normal,value/2};
                break;
            }
        }
    }
    for (auto& part:decor.parts)
    {
        double support=0, error=part.bottom;
        for (const auto& f:decor.fixtures)
        {
            if (f.kind!=Decor::FixtureKind::Bench && f.kind!=Decor::FixtureKind::Shelf &&
                f.kind!=Decor::FixtureKind::Tabletop) continue;
            if (part.position.x<f.lo.x || part.position.x>f.hi.x ||
                part.position.y<f.lo.y || part.position.y>f.hi.y) continue;
            const double gap=std::abs(f.top-part.bottom);
            if (gap<error) { error=gap; support=f.top; }
        }
        part.bottom=support;
        if (part.tile==1)
        {
            const Vec half{part.width*0.34,part.width*0.50};
            decor.fixtures.push_back({part.position-half,part.position+half,support,
                                      support+0.31*part.width,Decor::FixtureKind::PartsTray});
        }
    }
    return decor;
}

enum class Surface { Plaster, Floor, Ceiling, Curtain };

sf::Color sample(const sf::Image& atlas, Surface material, double u, double v)
{
    const auto size=atlas.getSize();
    const unsigned w=size.x/2, h=size.y/2;
    const unsigned x=static_cast<unsigned>(fraction(u)*(w-2))+1;
    const unsigned y=static_cast<unsigned>(fraction(v)*(h-2))+1;
    const unsigned tile=static_cast<unsigned>(material);
    return atlas.getPixel({(tile%2)*w+x,(tile/2)*h+y});
}


sf::Color cabinetFinish(const Decor::Fixture& box, Vec p, double z, double& faceLight)
{
    static sf::Image panel;
    static const bool loaded = panel.loadFromFile("assets/images/switchboard.png");
    const Vec size = box.hi - box.lo;
    const bool wide = size.x >= size.y;
    const double vertical = std::clamp((z-box.bottom)/(box.top-box.bottom), 0.0, 1.0);
    const bool front = wide ? std::abs(p.y-box.hi.y)<0.001 : std::abs(p.x-box.lo.x)<0.001;
    if (vertical < 0.045) return {37,36,32};
    if (!loaded) return {74,73,66};
    double u = wide ? (p.x-box.lo.x)/size.x : (p.y-box.lo.y)/size.y;
    double v = (1.0-vertical)/0.955;
    if (!front)
    {
        u = 0.04 + 0.09*fraction((p.x+p.y)/18.0);
        v = 0.57 + 0.11*fraction((p.y+z)/22.0);
        faceLight = vertical > 0.999 ? 1.03 : 0.66;
    }
    const auto pixels = panel.getSize();
    return panel.getPixel({
        static_cast<unsigned>(std::clamp(u,0.0,1.0)*(pixels.x-1)),
        static_cast<unsigned>(std::clamp(v,0.0,1.0)*(pixels.y-1))
    });
}

double arcadeSpill(const Decor& decor, Vec p)
{
    double result=0;
    for (const auto& lamp:decor.lamps)
    {
        const Vec delta=p-lamp.position;
        const double dist2=dot(delta,delta);
        if (dist2>=28*28 || dot(delta,lamp.direction)<-0.8) continue;
        
        const auto bin=static_cast<std::size_t>((std::atan2(delta.y,delta.x)+pi)*
                              (lamp.reach.size()/(2*pi))+0.5) % lamp.reach.size();
        const double limit=lamp.reach[bin]+0.8;
        if (dist2>limit*limit) continue;
        const double edge=1-dist2/(28*28);
        result+=0.85*edge*edge/(1+dist2/64);
    }
    return result;
}
 
bool clipAxis(double p,double d,double lo,double hi,double& entry,double& exit)
{
    if (std::abs(d)<1e-10) return p>=lo && p<=hi;
    double a=(lo-p)/d,b=(hi-p)/d;
    if (a>b) std::swap(a,b);
    entry=std::max(entry,a); exit=std::min(exit,b);
    return exit>=entry;
}

sf::Color cctvColor(sf::Color texel,double infrared,double visible)
{
    const double luma=0.30*texel.r+0.56*texel.g+0.14*texel.b;
    auto channel=[&](double c,double balance) {
        return static_cast<std::uint8_t>(std::clamp(
            2.0+luma*infrared*balance+(0.40*luma+0.60*c)*visible,0.0,250.0));
    };
    return {channel(texel.r,0.87),channel(texel.g,1.00),channel(texel.b,0.92)};
}

sf::Color furnitureFinish(const Decor::Fixture& item, Vec p, double z, const sf::Image& atlas)
{
    using Kind = Decor::FixtureKind;
    const bool top = std::abs(z-item.top)<0.001;
    if (item.kind==Kind::Tabletop && top)
    {
        static sf::Image cloth;
        static const bool ready=cloth.loadFromFile("assets/images/dining-table.png");
        if (ready)
        {
            const Vec size=item.hi-item.lo;
            const auto pixels=cloth.getSize();
            return cloth.getPixel({
                static_cast<unsigned>(std::clamp((p.x-item.lo.x)/size.x,0.0,1.0)*(pixels.x-1)),
                static_cast<unsigned>(std::clamp((p.y-item.lo.y)/size.y,0.0,1.0)*(pixels.y-1))
            });
        }
    }
    const auto grain=sample(atlas,Surface::Curtain,(p.x+p.y)/18,
                            0.86+0.12*fraction((top?p.y:z)/4));
    if (item.kind==Kind::Bench) return {static_cast<std::uint8_t>(53+grain.r*0.72),
        static_cast<std::uint8_t>(36+grain.g*0.61),static_cast<std::uint8_t>(24+grain.b*0.55)};
    if (item.kind==Kind::Shelf || item.kind==Kind::Drawers)
    {
        double value=64+grain.r*0.36;
        if (item.kind==Kind::Drawers && std::abs(p.x-item.lo.x)<0.001)
        {
            const double seam=fraction((z-item.bottom)/2.25);
            if (seam<0.095) value*=0.42;
            if (seam>0.65 && seam<0.80 && std::abs(p.y-(item.lo.y+item.hi.y)/2)<1.65) value+=53;
        }
        return {static_cast<std::uint8_t>(value*0.81),static_cast<std::uint8_t>(value),
                static_cast<std::uint8_t>(value*0.90)};
    }
    if (item.kind==Kind::Frame)
    {
        const auto value=static_cast<std::uint8_t>(39+grain.r*0.50);
        return {value,value,static_cast<std::uint8_t>(value*0.94)};
    }
    if (item.kind==Kind::Tabletop) return {106,77,54};
    return {static_cast<std::uint8_t>(58+grain.r*0.65),
            static_cast<std::uint8_t>(26+grain.g*0.39),
            static_cast<std::uint8_t>(24+grain.b*0.34)};
}

sf::Color arcadeFinish(const Decor::Fixture& item,Vec p,double z,const sf::Image& materials,double& emission)
{
    const double v=std::clamp(1.0-z/arcadeHeight,0.0,1.0);
    const double u=item.facesWest ? (p.y-item.lo.y)/(item.hi.y-item.lo.y)
                                  : (p.x-item.lo.x)/(item.hi.x-item.lo.x);
    const bool front=item.facesWest ? std::abs(p.x-item.lo.x)<0.001 : std::abs(p.y-item.hi.y)<0.001;
    const bool deck=std::abs(z-12.2)<0.001;
    if (front || deck)
    {
        static sf::Image art;
        static const bool loaded=art.loadFromFile("assets/images/arcade-fronts.png");
        if (loaded)
        {
            const auto size=art.getSize();
            const unsigned strip=size.x/3;
            const double inset=item.facesWest ? p.x-item.lo.x : item.hi.y-p.y;
            const double textureV=deck ? 0.59-0.07*std::clamp(inset/2.3,0.0,1.0) : v;
            const sf::Color pixel=art.getPixel({
                item.artwork*strip+static_cast<unsigned>(std::clamp(u,0.0,1.0)*(strip-1)),
                static_cast<unsigned>(textureV*(size.y-1))
            });
            if (front && (v<0.16 || (v>0.20 && v<0.51 && u>0.1 && u<0.9)))
                emission=0.95;
            return pixel;
        }
    }
    const double inset=item.facesWest ? p.x-item.lo.x : item.hi.y-p.y;
    const auto grain=sample(materials,Surface::Curtain,inset/15,0.86+v*0.12);
    const double trim=inset<0.25 ? 1.65 : 0.68;
    return {static_cast<std::uint8_t>((28+grain.r*0.41)*trim),
            static_cast<std::uint8_t>((21+grain.g*0.34)*trim),
            static_cast<std::uint8_t>((22+grain.b*0.36)*trim)};
}

struct PartAtlas
{
    sf::Image image;
    struct Crop { unsigned left=0,top=0,width=0,height=0; };
    std::array<Crop,4> crops{};

    PartAtlas()
    {
        if (!image.loadFromFile("assets/images/workshop-parts.png")) return;
        const auto size=image.getSize();
        for (unsigned tile=0; tile<4; ++tile)
        {
            const unsigned x0=(tile%2)*(size.x/2);
            const unsigned split=tile%2==0 ? size.y*58/100 : size.y/2;
            const unsigned y0=tile<2 ? 0 : split;
            const unsigned y1=tile<2 ? split : size.y;
            unsigned left=size.x,top=size.y,right=0,bottom=0;
            for (unsigned y=y0; y<y1; ++y)
                for (unsigned x=x0; x<x0+size.x/2; ++x)
                    if (image.getPixel({x,y}).a>=64)
                    {
                        left=std::min(left,x); right=std::max(right,x);
                        top=std::min(top,y); bottom=std::max(bottom,y);
                    }
            if (left<=right && top<=bottom) crops[tile]={left,top,right-left+1,bottom-top+1};
        }
    }
};

sf::Color trayFinish(const Decor::Fixture& tray,Vec p,double z,const PartAtlas& art)
{
    const Vec size=tray.hi-tray.lo;
    const double u=std::clamp((p.y-tray.lo.y)/size.y,0.,1.);
    const double v=std::clamp((tray.hi.x-p.x)/size.x,0.,1.);
    if (std::abs(tray.top-z)<0.001)
    {
        if (std::min({u,1-u,v,1-v})<0.028) return {111,84,70};
        const Vec back=Vec{695,250}*(1-u)+Vec{989,142}*u;
        const Vec front=Vec{899,477}*(1-u)+Vec{1206,339}*u;
        const Vec uv=back*(1-v)+front*v;
        const auto sizePx=art.image.getSize();
        return art.image.getPixel({static_cast<unsigned>(uv.x*sizePx.x/1254),
                                   static_cast<unsigned>(uv.y*sizePx.y/1254)});
    }
    const double t=(z-tray.bottom)/(tray.top-tray.bottom);
    const double along=std::abs(p.x-tray.lo.x)<0.001 || std::abs(p.x-tray.hi.x)<0.001 ? u : v;
    const auto sizePx=art.image.getSize();
    const auto rust=art.image.getPixel({static_cast<unsigned>((922+228*along)*sizePx.x/1254),
                                static_cast<unsigned>((492-83*along+34*(1-t))*sizePx.y/1254)});
    return rust;
}

void shadeSupport(const Decor& decor,const PartAtlas* art,Vec p,double z,
                  sf::Color& texel,double& gain)
{
    for (const auto& part:decor.parts)
    {
        if (std::abs(part.bottom-z)>0.001) continue;
        const Vec d=(p-part.position)/part.width;
        const double r=d.x*d.x*10+d.y*d.y*3.4;
        if (r<1) gain*=0.46+0.54*r*(2-r);
        if (part.tile!=2 || !art) continue;
        const auto& crop=art->crops[part.tile];
        if (!crop.width || !crop.height) continue;
        const double u=0.5+d.y,v=0.5-d.x*crop.width/crop.height;
        if (u<0 || u>=1 || v<0 || v>=1) continue;
        const auto metal=art->image.getPixel({crop.left+static_cast<unsigned>(u*crop.width),
                                              crop.top+static_cast<unsigned>(v*crop.height)});
        if (metal.a>=96) { texel=metal; gain=0.59; }
    }
}

sf::Image renderRoom(const BuildingMap& map,const BuildingMap::Camera& camera,
                     double heading,double fov,double eyeHeight,double horizonRatio,const sf::Image& atlas)
{
    const Decor decor=loadDecor(map);
    const Stage& stage=decor.stage;
    const Vec origin{camera.x,camera.y};
    const double angle=std::remainder(heading,360.0)*pi/180.0;
    const Vec forward{std::cos(angle),std::sin(angle)},right{-forward.y,forward.x};
    const double focal=width/(2*std::tan(fov*pi/360.0)), horizon=height*horizonRatio;
    struct ProjectedPart
    {
        double depth,left,top,width,height,spill;
        PartAtlas::Crop crop;
    };
    std::vector<ProjectedPart> props;
    const PartAtlas* art=nullptr;
    if (!decor.parts.empty())
    {
        static const PartAtlas loaded;
        art=&loaded;
        for (const auto& part:decor.parts)
        {
            const Vec offset=part.position-origin;
            const double depth=dot(offset,forward);
            const auto crop=art->crops[part.tile];
            if (part.tile==1 || part.tile==2 || depth<=0.01 || crop.width==0) continue;
            const double w=focal*part.width/depth, h=w*crop.height/crop.width;
            const double left=width/2.0+focal*dot(offset,right)/depth-w/2;
            constexpr std::array<double,4> foot{0.80,0.91,1.0,0.93};
            const double top=horizon+focal*(eyeHeight-part.bottom)/depth-h*foot[part.tile];
            if (left>=width || left+w<=0 || top>=height || top+h<=0) continue;
            props.push_back({depth,left,top,w,h,arcadeSpill(decor,part.position),crop});
        }
    }
    sf::Image frame({width,height},{11,12,13});
    for (unsigned x=0; x<width; ++x)
    {
        const double screenX=(x+0.5)/width-0.5;
        const Vec ray=forward+right*(screenX*width/focal);
        const Hit hit=cast(map,origin,ray);
        const Vec wallPoint=hit.wall>=0 ? origin+ray*hit.depth : origin;
        const double wallPool=arcadeSpill(decor,wallPoint);
        double face=1;
        if (hit.wall>=0)
        {
            const auto& w=map.walls[hit.wall];
            const double dx=double(w.x2)-w.x1, dy=double(w.y2)-w.y1;
            face=0.80+0.20*std::abs(dx)/std::hypot(dx,dy);
        }
        const Vec local=origin-stage.centre;
        const double curtainU=dot(wallPoint-stage.centre,stage.tangent);
        const bool curtain=stage.wall>=0 && hit.wall==stage.wall &&
            dot(local,stage.normal)>0 && std::abs(curtainU)<stage.halfWidth;
        double entry=0,exit=inf;
        const bool platform=stage.wall>=0 &&
            clipAxis(dot(local,stage.tangent),dot(ray,stage.tangent),
                     -stage.halfWidth,stage.halfWidth,entry,exit) &&
            clipAxis(dot(local,stage.normal),dot(ray,stage.normal),0,8,entry,exit);

        struct FixtureSpan { const Decor::Fixture* fixture; double near,far; };
        std::vector<FixtureSpan> spans;
        for (const auto& fixture:decor.fixtures)
        {
            double near=0,far=hit.depth;
            if (clipAxis(origin.x,ray.x,fixture.lo.x,fixture.hi.x,near,far) &&
                clipAxis(origin.y,ray.y,fixture.lo.y,fixture.hi.y,near,far))
                spans.push_back({&fixture,near,far});
        }
        for (unsigned y=0; y<height; ++y)
        {
            const double slope=(horizon-y-0.5)/focal;
            const bool ceiling=slope>0;
            const double plane=std::abs(slope)>1e-8 ?
                (ceiling ? roomHeight-eyeHeight : -eyeHeight)/slope : inf;
            double depth=std::min(hit.depth,plane);
            if (!std::isfinite(depth)) continue;
            sf::Color texel;
            double gain=1,spill=0,emission=0;
            const double deck=slope<0 ? (3.0-eyeHeight)/slope : inf;
            const bool seesDeck=platform && deck>=entry && deck<=exit && deck<depth;
            const double entryZ=eyeHeight+slope*entry;
            if (seesDeck || (platform && entry>0 && entry<depth && entryZ>=0 && entryZ<=3))
            {
                depth=seesDeck ? deck : entry;
                const Vec p=origin+ray*depth;
                const double along=dot(p-stage.centre,stage.tangent);
                const double across=dot(p-stage.centre,stage.normal);
                const double grainV=seesDeck ? fraction(across/8) : 1-entryZ/3;
                texel=sample(atlas,Surface::Curtain,along/32,0.86+0.13*grainV);
                gain=(seesDeck ? 0.86 : 0.59)*0.46;
                spill=arcadeSpill(decor,p);
            }
            else if (hit.depth<plane)
            {
                const double z=eyeHeight+slope*depth;
                const double v=std::clamp(1-z/roomHeight,0.0,0.99999);
                texel=curtain ? sample(atlas,Surface::Curtain,0.5+curtainU/(2*stage.halfWidth),v)
                              : sample(atlas,Surface::Plaster,hit.along/34,v);
                const double contact=std::clamp(std::min(z,roomHeight-z)/4,0.0,1.0);
                gain=0.34*face*(0.63+0.37*contact);
                spill=wallPool*face;
            }
            else
            {
                const Vec p=origin+ray*depth;
                if (ceiling)
                {
                    texel=sample(atlas,Surface::Ceiling,p.x/36,0.03+0.36*fraction(p.y/19));
                    gain=0.24;
                }
                else
                {
                    texel=sample(atlas,Surface::Floor,p.x/33,p.y/33);
                    gain=0.32;
                    spill=arcadeSpill(decor,p)*0.72;
                    for (const auto& table:decor.tables)
                    {
                        const double dx=std::max({table.lo.x-p.x,0.0,p.x-table.hi.x});
                        const double dy=std::max({table.lo.y-p.y,0.0,p.y-table.hi.y});
                        gain*=0.58+0.42*std::min(1.0,(dx*dx+dy*dy)/16.0);
                    }
                    shadeSupport(decor,art,p,0,texel,gain);
                }
            }
            for (const auto& span:spans)
            {
                const auto& fixture=*span.fixture;
                double near=span.near,far=span.far;
                if (!clipAxis(eyeHeight,slope,fixture.bottom,fixture.top,near,far) ||
                    near<=0 || near>=depth) continue;
                depth=near;
                const Vec p=origin+ray*depth;
                const double z=eyeHeight+slope*depth;
                emission=0;
                spill=arcadeSpill(decor,p);
                if (fixture.kind==Decor::FixtureKind::PartsTray && art && art->crops[1].width)
                {
                    texel=trayFinish(fixture,p,z,*art);
                    gain=std::abs(z-fixture.top)<0.001 ? 0.65 : 0.41;
                    continue;
                }
                if (fixture.kind==Decor::FixtureKind::Arcade)
                {
                    gain=0.43;
                    texel=arcadeFinish(fixture,p,z,atlas,emission);
                    continue;
                }
                if (fixture.kind!=Decor::FixtureKind::Duct && fixture.kind!=Decor::FixtureKind::Cabinet)
                {
                    texel=furnitureFinish(fixture,p,z,atlas);
                    const bool top=std::abs(z-fixture.top)<0.001;
                    const bool side=std::abs(p.x-fixture.lo.x)<0.001 || std::abs(p.x-fixture.hi.x)<0.001;
                    const double faceLight=top ? 1.04 : (side ? 0.72 : 0.84);
                    gain=faceLight*0.48;
                    if (top) shadeSupport(decor,art,p,z,texel,gain);
                    continue;
                }
                if (fixture.kind==Decor::FixtureKind::Cabinet)
                {
                    double faceLight=1.0;
                    texel=cabinetFinish(fixture,p,z,faceLight);
                    gain=faceLight*0.52;
                    continue;
                }
                const auto& duct=fixture;
                const bool underside=std::abs(z-duct.bottom)<0.001;
                const bool acrossX=(duct.hi.x-duct.lo.x)>(duct.hi.y-duct.lo.y);
                const double along=acrossX?p.x:p.y;
                const double across=acrossX?p.y:p.x;
                const double endDistance=std::min(along-(acrossX?duct.lo.x:duct.lo.y),
                                                 (acrossX?duct.hi.x:duct.hi.y)-along);
                const double sideDistance=std::min(across-(acrossX?duct.lo.y:duct.lo.x),
                                                  (acrossX?duct.hi.y:duct.hi.x)-across);
                const bool endCap=!underside && endDistance<0.001;
                const auto grain=sample(atlas,Surface::Plaster,(endCap?across:along)/10,
                                       0.18+0.38*fraction((underside?across:z)/9));
                double metal=44+0.23*grain.r;
                if (endCap && sideDistance>0.36 && z>duct.bottom+0.4 && z<duct.top-0.4)
                    metal*=fraction((z-duct.bottom)/0.59)<0.48 ? 0.28 : 0.88;
                const double seam=fraction(along/14);
                if (!endCap && seam<0.045) metal*=seam<0.018 ? 1.38 : 0.69;
                if (!underside && z-duct.bottom<0.12) metal*=1.36;
                texel={static_cast<std::uint8_t>(metal),static_cast<std::uint8_t>(metal*0.98),
                       static_cast<std::uint8_t>(metal*0.95)};
                gain=(underside?0.57:0.9)*0.5;
            }
            for (const auto& part:props)
            {
                if (part.depth>=depth) continue;
                const double u=(x+0.5-part.left)/part.width,v=(y+0.5-part.top)/part.height;
                if (u<0 || v<0 || u>=1 || v>=1) continue;
                const sf::Color pixel=art->image.getPixel({
                    part.crop.left+static_cast<unsigned>(u*part.crop.width),
                    part.crop.top+static_cast<unsigned>(v*part.crop.height)
                });
                if (pixel.a<96) continue;
                texel=pixel; depth=part.depth; gain=0.59; emission=0; spill=part.spill;
            }
            const double screenY=(y+0.5)/height-0.5;
            const double beam=1/(1+5.8*screenX*screenX+3.6*screenY*screenY);
            const double infrared=0.24+2.45*beam*beam/(1+depth*depth/2900);
            frame.setPixel({x,y},cctvColor(texel,gain*infrared,emission+spill));
        }
    }
    return frame;
}
}

bool loadCameraFeed(sf::Texture& texture,const BuildingMap& map,const BuildingMap::Camera& camera)
{
    std::istringstream source(camera.imagePath);
    std::string kind,extra;
    source>>kind;
    if (kind!="view") return texture.loadFromFile(camera.imagePath);
    double heading=0,fov=0;
    double eyeHeight=21.0,horizonRatio=0.40;
    bool valid=static_cast<bool>(source>>heading>>fov);
    if (valid && !(source>>std::ws).eof())
        valid=static_cast<bool>(source>>eyeHeight>>horizonRatio) && !(source>>extra);
    if (!valid || !(eyeHeight>0 && eyeHeight<roomHeight) || !(horizonRatio>0 && horizonRatio<1) || !std::isfinite(heading) ||
        !std::isfinite(fov) || fov<1 || fov>=179 ||
        !std::isfinite(camera.x) || !std::isfinite(camera.y))
    {
        std::cerr<<"Invalid camera view: "<<camera.imagePath<<'\n';
        return false;
    }
    static sf::Image atlas;
    static const bool loaded=atlas.loadFromFile("assets/images/cctv-materials.png");
    if (!loaded || atlas.getSize().x<8 || atlas.getSize().y<8) return false;
    return texture.loadFromImage(renderRoom(map,camera,heading,fov,eyeHeight,horizonRatio,atlas));
}
