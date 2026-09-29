#include "AppPaths.h"
#include "BreadboardView.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {
using namespace sf;
constexpr float pi = 3.14159265358979323846f;
const Color paper(220, 214, 195), ink(105, 102, 90);
const Color blue(30, 108, 153), teal(15, 153, 155), yellow(225, 192, 60);
const Color red(187, 58, 51), black(46, 51, 49);
const Vector2f resetCenter(535, 723), irqCenter(728, 723), nmiCenter(832, 723);

void rect(RenderTarget& t, float x, float y, float w, float h, Color c) {
    RectangleShape s({w,h}); s.setPosition(x,y); s.setFillColor(c); t.draw(s);
}
void rounded(RenderTarget& t, float x,float y,float w,float h,float r,Color c) {
    ConvexShape s(36);
    const Vector2f centers[]={{x+w-r,y+r},{x+w-r,y+h-r},{x+r,y+h-r},{x+r,y+r}};
    unsigned n=0;
    for(int corner=0;corner<4;++corner) for(int step=0;step<=8;++step) {
        float a=(corner-1+step/8.f)*pi/2;
        s.setPoint(n++,centers[corner]+Vector2f(std::cos(a)*r,std::sin(a)*r));
    }
    s.setFillColor(c);t.draw(s);
}
void circle(RenderTarget& t,Vector2f p,float r,Color c) {
    CircleShape s(r,32);s.setPosition(p-Vector2f(r,r));s.setFillColor(c);t.draw(s);
}
void text(RenderTarget& t,const Font& font,const std::string& label,float x,float y,
          unsigned size,Color c) {
    Text s(label,font,size);s.setPosition(x,y);s.setFillColor(c);t.draw(s);
}
void quad(VertexArray& v,float x,float y,float w,float h,Color c) {
    v.append(Vertex({x,y},c));v.append(Vertex({x+w,y},c));
    v.append(Vertex({x+w,y+h},c));v.append(Vertex({x,y+h},c));
}
void stroke(RenderTarget& t,const std::vector<Vector2f>& points,float width,Color color,
            Vector2f offset={0,0}) {
    VertexArray strip(TriangleStrip);
    for(size_t i=0;i<points.size();++i) {
        Vector2f tangent=points[std::min(i+1,points.size()-1)]-points[i?i-1:0];
        float length=std::sqrt(tangent.x*tangent.x+tangent.y*tangent.y);
        if(length<.001f) continue;
        Vector2f normal(-tangent.y/length*width/2,tangent.x/length*width/2);
        strip.append(Vertex(points[i]+offset+normal,color));
        strip.append(Vertex(points[i]+offset-normal,color));
    }
    t.draw(strip);
}
void wire(RenderTarget& t,Vector2f a,Vector2f b,Vector2f c,Vector2f d,Color color,
          float width=4) {
    std::vector<Vector2f> points;
    for(int i=0;i<=48;++i) {
        float u=i/48.f,v=1-u;
        points.push_back(a*(v*v*v)+b*(3*v*v*u)+c*(3*v*u*u)+d*(u*u*u));
    }
    stroke(t,points,width+3,Color(0,0,0,45),{1,3});
    stroke(t,points,width+1,Color(color.r/2,color.g/2,color.b/2));
    stroke(t,points,width,color);
    stroke(t,points,1,Color(255,255,255,65),{-.6f,-.7f});
    circle(t,a,1.3f,Color(206,206,182));circle(t,d,1.3f,Color(206,206,182));
}
void breadboard(RenderTarget& t,const Font& f,float y,unsigned number) {
    rounded(t,49,y+5,1102,230,7,Color(0,0,0,50));
    rounded(t,50,y,1100,230,5,Color(181,175,157));
    rounded(t,52,y,1096,226,4,paper);
    rect(t,57,y+2,1085,2,Color(246,240,219));
    rounded(t,85,y+103,1030,11,3,Color(189,182,163));
    rect(t,89,y+105,1022,3,Color(174,168,151));
    // Power rails are visibly separated from the two five-hole terminal strips.
    for(float rail:{14.f,192.f}) {
        rect(t,80,y+rail,1044,1.5f,Color(183,80,68));
        rect(t,80,y+rail+28,1044,1.5f,Color(68,111,152));
        text(t,f,"+",61,y+rail-9,17,red);
        text(t,f,"-",61,y+rail+16,17,blue);
    }
    VertexArray holes(Quads);
    for(int col=0;col<64;++col) {
        float x=96+col*16.f;
        for(int row=0;row<10;++row) {
            float hy=y+50+row*11.f+(row>=5?20:0);
            quad(holes,x-3,hy-3,6,6,Color(239,232,210));
            quad(holes,x-2,hy-2,4,4,Color(104,103,91));
            quad(holes,x-1,hy-1,3,2,Color(50,57,53));
        }
        if(col%6!=5) for(float rail:{24.f,34.f,202.f,212.f}) {
            quad(holes,x-3,y+rail-3,6,6,Color(239,232,210));
            quad(holes,x-2,y+rail-2,4,4,Color(67,73,64));
        }
        if(col%5==4) {
            text(t,f,std::to_string(col+1),x-5,y+39,8,ink);
            text(t,f,std::to_string(col+1),x-5,y+171,8,ink);
        }
    }
    t.draw(holes);
    for(int row=0;row<10;++row) {
        float hy=y+44+row*11.f+(row>=5?20:0);
        text(t,f,std::string(1,char('a'+row)),75,hy,9,ink);
    }
    text(t,f,"BB"+std::to_string(number),1115,y+102,10,ink);
}
void chip(RenderTarget& t,const Font& f,float x,float y,float w,float h,int pins,
          const std::string& name,const std::string& role) {
    rounded(t,x+2,y+5,w,h,4,Color(0,0,0,75));
    float pitch=(w-24)/(pins/2-1);
    for(int i=0;i<pins/2;++i) {
        float px=x+12+i*pitch;
        for(float py:{y-12,y+h}) {
            rect(t,px-3,py,7,13,Color(97,100,94));
            rect(t,px-2,py,4,13,Color(191,189,169));
            rect(t,px-2,py,1,11,Color(233,225,199));
        }
    }
    rounded(t,x,y,w,h,4,Color(22,26,26));
    rect(t,x+5,y+2,w-10,1,Color(59,61,56));
    rect(t,x+5,y+h-3,w-10,2,Color(12,17,17));
    circle(t,{x,y+h/2},7,Color(9,13,14));
    circle(t,{x+15,y+h-12},3,Color(49,51,44));
    text(t,f,name,x+30,y+12,16,Color(188,183,151));
    text(t,f,role,x+30,y+36,10,Color(112,121,108));
}
void capacitor(RenderTarget& t,Vector2f p,float r=13) {
    rect(t,p.x-6,p.y-24,2,32,Color(185,180,161));
    rect(t,p.x+5,p.y-24,2,32,Color(185,180,161));
    circle(t,p+Vector2f(2,4),r+2,Color(0,0,0,65));
    circle(t,p,r,Color(25,35,35));
    circle(t,p-Vector2f(1,2),r-3,Color(160,171,164));
    circle(t,p-Vector2f(1,2),r-5,Color(196,201,182));
    rect(t,p.x-r+5,p.y-3,r*2-10,1,Color(117,131,125));
    rect(t,p.x-1,p.y-r+4,1,r*2-9,Color(117,131,125));
}
void resistor(RenderTarget& t,Vector2f p) {
    rect(t,p.x-29,p.y-1,58,2,Color(151,151,135));
    rounded(t,p.x-15,p.y-5,30,10,4,Color(191,169,120));
    rect(t,p.x-10,p.y-5,3,10,Color(111,63,43));
    rect(t,p.x-3,p.y-5,3,10,Color(35,36,31));
    rect(t,p.x+4,p.y-5,3,10,Color(176,62,37));
    rect(t,p.x+10,p.y-5,2,10,Color(186,156,64));
}
void button(RenderTarget& t,Vector2f p,bool down=false) {
    for(float dx:{-21.f,18.f}) rect(t,p.x+dx,p.y-5,3,10,Color(162,162,145));
    rounded(t,p.x-19,p.y-19,38,38,3,Color(39,46,44));
    rounded(t,p.x-16,p.y-16,32,32,2,Color(159,163,144));
    for(float dx:{-12.f,12.f})for(float dy:{-12.f,12.f})
        circle(t,p+Vector2f(dx,dy),2,Color(65,72,63));
    circle(t,p+Vector2f(0,2),11,Color(40,46,42));
    circle(t,p,down?9:11,down?Color(34,40,38):Color(62,68,60));
    if(!down) circle(t,p-Vector2f(2,3),5,Color(73,79,68));
}
void oscillator(RenderTarget& t,const Font& f,float x,float y,const std::string& title) {
    for(float dx:{8.f,94.f})for(float dy:{-6.f,60.f})
        rect(t,x+dx,y+dy,3,10,Color(176,173,151));
    rounded(t,x+2,y+4,108,65,8,Color(0,0,0,55));
    rounded(t,x,y,108,65,8,Color(116,121,113));
    rounded(t,x+2,y+2,104,60,7,Color(195,199,177));
    rounded(t,x+5,y+5,98,53,5,Color(162,171,158));
    rect(t,x+12,y+6,85,1,Color(228,225,201));
    text(t,f,title,x+15,y+12,12,Color(55,65,59));
}
void makeBoard(RenderTarget& t,const Font& f) {
    t.clear(Color(28,34,35));
    text(t,f,"6502 / BREADBOARD COMPUTER",50,20,23,Color(227,229,214));
    text(t,f,"W65C02 CPU   /   W65C22 VIA   /   32K ROM   /   16K MAPPED RAM",51,52,12,Color(146,162,152));
    struct Legend { const char* label; Color color; };
    const Legend legend[]={{"ADDRESS",blue},{"DATA",teal},{"CONTROL",yellow},{"5V",red},{"GND",black}};
    float lx=735;
    for(const auto& item:legend) {
        circle(t,{lx,46},4,item.color);
        text(t,f,item.label,lx+9,38,10,Color(172,184,172));
        lx+=item.label[0]=='A'?103:80;
    }
    breadboard(t,f,91,1);breadboard(t,f,328,2);breadboard(t,f,565,3);

    // Power distribution between the three physical breadboards.
    for(float y:{91.f,328.f}) {
        wire(t,{1120,y+202},{1143,y+211},{1143,y+253},{1120,y+261},red,4);
        wire(t,{1104,y+212},{1120,y+223},{1120,y+263},{1104,y+271},black,4);
    }
    wire(t,{96,115},{57,119},{58,142},{96,160},red,5);
    wire(t,{96,125},{66,130},{68,153},{112,160},black,5);
    // Address and data fanout into ROM and RAM, kept above the packages.
    for(int i=0;i<14;++i) {
        wire(t,{160+i*16.f,190},{240+i*20.f,109+i*3.f},
               {615+i*16.f,117+i*2.f},{615+i*16.f,190},blue,3.5f);
        wire(t,{615+i*16.f,190},{660+i*17.f,139-i*2.f},
               {899+i*16.f,127-i*2.f},{899+i*16.f,190},blue,3.5f);
    }
    for(int i=0;i<8;++i) {
        wire(t,{348+i*16.f,280},{480+i*15.f,327-i*3.f},
               {719+i*14.f,336-i*3.f},{719+i*14.f,280},teal,4);
        wire(t,{719+i*14.f,280},{801+i*12.f,311+i*3.f},
               {996+i*14.f,319+i*3.f},{996+i*14.f,280},teal,4);
    }
    // Bundle between CPU bus and VIA, bowed around the chip bodies.
    for(int i=0;i<18;++i)
        wire(t,{160+i*16.f,190},{529+i*6.f,122+i*3.f},
               {624+i*3.f,547-i*3.f},{218+i*16.f,516},i<10?blue:teal,4);

    chip(t,f,145,202,330,66,40,"W65C02S","CPU  /  WESTERN DESIGN CENTER");
    chip(t,f,603,202,234,66,28,"AT28C256","EEPROM  /  8000-FFFF");
    chip(t,f,883,202,234,66,28,"HM62256","SRAM  /  0000-3FFF");
    chip(t,f,207,438,330,66,40,"W65C22S","VERSATILE INTERFACE ADAPTER  /  6000");
    chip(t,f,96,444,93,50,14,""," ");
    text(t,f,"74HC00",107,453,12,Color(182,180,150));
    text(t,f,"DECODE",108,474,9,Color(117,128,112));

    // Chip-select, clock, reset and power jumpers.
    wire(t,{112,432},{126,362},{532,369},{523,426},yellow,4);
    wire(t,{144,506},{145,538},{196,539},{219,516},yellow,4);
    wire(t,{128,506},{163,582},{175,641},{193,674},yellow,4);
    wire(t,{160,432},{128,332},{118,304},{160,280},yellow,4);
    wire(t,{112,162},{103,172},{124,185},{157,190},red,4);
    wire(t,{112,301},{117,311},{167,305},{169,280},black,4);
    wire(t,{592,162},{575,173},{574,191},{615,190},red,4);
    wire(t,{864,162},{855,173},{860,190},{899,190},red,4);
    wire(t,{608,303},{608,312},{658,320},{655,280},black,4);
    wire(t,{880,303},{884,317},{925,322},{931,280},black,4);
    wire(t,{208,352},{190,365},{195,411},{219,426},red,4);
    wire(t,{208,540},{191,539},{191,526},{235,516},black,4);
    wire(t,{519,724},{368,658},{138,381},{128,173},yellow,4);
    wire(t,{551,724},{584,731},{586,770},{592,777},black,4);
    wire(t,{712,724},{688,678},{496,622},{489,516},yellow,3.5f);
    wire(t,{816,724},{790,644},{465,583},{457,516},yellow,3.5f);
    wire(t,{744,724},{766,741},{768,777},{768,777},black,3.5f);
    wire(t,{848,724},{864,747},{864,777},{864,777},black,3.5f);

    capacitor(t,{114,150});capacitor(t,{842,151});
    capacitor(t,{179,388});capacitor(t,{577,383});capacitor(t,{173,627});
    resistor(t,{152,171});resistor(t,{546,625});resistor(t,{564,377});
    oscillator(t,f,126,680,"OSCILLATOR");
    oscillator(t,f,289,680,"20.0000 MHz");
    text(t,f,"SPARE",310,716,11,Color(62,73,64));
    wire(t,{134,674},{112,647},{114,604},{128,589},red,4);
    wire(t,{134,750},{113,760},{114,777},{128,777},black,4);
    wire(t,{220,674},{249,627},{420,646},{457,426},yellow,4);
    button(t,resetCenter);button(t,irqCenter);button(t,nmiCenter);
    text(t,f,"RESET / R",503,756,12,ink);
    text(t,f,"IRQ / I",705,756,12,ink);
    text(t,f,"NMI / N",807,756,12,ink);
    text(t,f,"CLOCK",155,757,11,ink);
    text(t,f,"5V",1011,743,11,ink);
    circle(t,{1020,722},10,Color(62,85,58));
    circle(t,{1020,722},7,Color(109,188,94));
    circle(t,{1018,720},3,Color(191,235,144));

    // Serial expansion on the spare section of the third breadboard.
    chip(t,f,598,611,255,61,28,"W65C51N","ACIA  /  5000-5003");
    chip(t,f,927,613,160,57,16,"MAX232","RS-232 LINE DRIVER");
    wire(t,{619,599},{590,563},{557,551},{521,516},teal,3);
    wire(t,{650,599},{644,569},{656,557},{672,540},red,3);
    wire(t,{681,684},{650,700},{659,770},{672,777},black,3);
    wire(t,{833,684},{857,694},{885,686},{942,684},yellow,3);
    wire(t,{817,599},{854,586},{888,591},{958,601},teal,3);
    text(t,f,"SERIAL TERMINAL  /  TX + RX",887,691,11,ink);

    // The front-panel status is sampled from the existing CPU and VIA.
    rounded(t,50,819,1100,60,6,Color(22,28,30));
    text(t,f,"Click the board buttons or use R / I / N",51,884,10,Color(132,150,143));
}
void makeLcdWires(RenderTarget& t) {
    t.clear(Color::Transparent);
    // These leads terminate at the LCD header, not over the active glass.
    for(int i=0;i<8;++i) {
        float endX=625+.65f*(112+(7+i)*27);
        wire(t,{383+i*16.f,516},{605+i*5.f,547-i*6.f},
             {586+i*7.f,321-i*3.f},{endX,406},teal,4);
    }
    for(int i=0;i<3;++i) {
        float endX=625+.65f*(112+(4+i)*27);
        wire(t,{489+i*16.f,426},{558+i*8.f,345-i*8.f},
             {697+i*18.f,346-i*8.f},{endX,406},yellow,3.5f);
    }
    wire(t,{640,352},{657,361},{685,361},{715,406},red,3.5f);
    wire(t,{624,362},{648,374},{670,374},{698,406},black,3.5f);
    wire(t,{579,377},{620,380},{698,354},{733,406},black,3);

}
std::string hex(unsigned value,int width) {
    std::ostringstream s;s<<std::uppercase<<std::hex<<std::setfill('0')<<std::setw(width)<<value;return s.str();
}
std::string frequency(double hz) {
    std::ostringstream s;s<<std::setprecision(4)<<(hz>=1e6?hz/1e6:hz/1e3)<<(hz>=1e6?" MHz":" kHz");return s.str();
}
} // namespace

BreadboardView::BreadboardView() {
    if(!font.loadFromFile(AppPaths::asset("sansation.ttf").string()))
        throw std::runtime_error("Could not load the breadboard label font");
    if(!background.create(Width,Height))
        throw std::runtime_error("Could not create the breadboard render texture");
    makeBoard(background,font);background.display();
    if(!lcdWires.create(Width,Height))
        throw std::runtime_error("Could not create the LCD wiring texture");
    makeLcdWires(lcdWires);lcdWires.display();
}

void BreadboardView::draw(sf::RenderTarget& target,const Snapshot& state,Button pressed) {
    target.draw(sf::Sprite(background.getTexture()));
    sf::RenderStates placement;
    placement.transform.translate(625,365).scale(.65f,.65f);
    lcd.draw(target,state.pixels,state.lcdWidth,state.lcdHeight,placement);
    target.draw(sf::Sprite(lcdWires.getTexture()));

    if(pressed!=Button::Released)
        button(target,pressed==Button::Reset?resetCenter:pressed==Button::Irq?irqCenter:nmiCenter,true);
    text(target,font,frequency(state.frequencyHz),141,716,13,Color(51,67,59));
    const char* mode=!state.started?"READY / PRESS R":state.stopped?"STOPPED":state.paused?"PAUSED":state.waiting?"WAITING":"RUNNING";
    circle(target,{73,847},4,state.started&&!state.stopped?Color(116,189,135):Color(176,152,87));
    text(target,font,mode,85,836,13,Color(191,211,196));
    text(target,font,"PC  "+hex(state.pc,4),288,836,15,Color(217,224,211));
    text(target,font,"A "+hex(state.a,2)+"   X "+hex(state.x,2)+"   Y "+hex(state.y,2),430,836,15,Color(174,193,184));
    text(target,font,"PA "+hex(state.pa,2)+"   PB "+hex(state.pb,2),683,836,15,Color(125,199,194));
    text(target,font,state.irq?"IRQ LOW":"IRQ HIGH",905,836,13,state.irq?yellow:Color(133,156,144));
    text(target,font,frequency(state.frequencyHz),1040,836,13,Color(174,193,184));
}

BreadboardView::Button BreadboardView::hitTest(sf::Vector2f point) {
    for(auto entry:{std::make_pair(Button::Reset,resetCenter),std::make_pair(Button::Irq,irqCenter),std::make_pair(Button::Nmi,nmiCenter)}) {
        Vector2f d=point-entry.second;
        if(std::abs(d.x)<=24 && std::abs(d.y)<=24) return entry.first;
    }
    return Button::Released;
}

sf::Vector2f BreadboardView::boardPoint(sf::Vector2i pixel,sf::Vector2u size) {
    if(!size.x || !size.y) return {-1,-1};
    return {pixel.x*float(Width)/size.x,pixel.y*float(Height)/size.y};
}
