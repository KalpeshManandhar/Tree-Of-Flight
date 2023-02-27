#ifdef NDEBUG 
#ifdef _WIN32 || _WIN64 
#define main WinMain
#endif
#endif


#include <iostream>
#include <chrono>

#include "graph.h"
#include "file.h"
#include "throwaway.h"
#include "renderers.hpp"



class Timer
{
private:
    // Type aliases to make accessing nested type easier
    using Clock = std::chrono::steady_clock;
    using Second = std::chrono::duration<double, std::ratio<1> >;

    std::chrono::time_point<Clock> m_beg{ Clock::now() };

public:
    void reset()
    {
        m_beg = Clock::now();
    }

    double elapsed() const
    {
        return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
    }
};


using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Pos = glm::vec<2, float>;
using Mat = glm::mat<3, 3, float>;

Pos operator %(Pos base, Pos divisor) {
    Pos res;
    res.x = (((int)base.x % (int)divisor.x) + (int)divisor.x) % (int)divisor.x;
    res.y = (((int)base.y % (int)divisor.y) + (int)divisor.y) % (int)divisor.y;
    res.x += base.x - floor(base.x);
    res.y += base.y - floor(base.y);
    return res;
}

Pos operator %(Pos base, double divisor) {
    return base % Pos{ divisor,divisor };
}

std::ostream& operator<<(std::ostream& os, Pos pos) {
    os << " ( " << pos.x << " , " << pos.y << " ) ";
    return os;
}

Mat get_rot_mat(double angle) {
    return Mat{
        {cos(angle), sin(angle), 0.0},
        { -sin(angle),cos(angle),0.0 },
        { 0.0,0.0,1.0 }
    };
}
Mat get_delta_mat(Pos delta) {
    return Mat{
        {1.0, 0.0, 0.0},
        { 0.0,1.0,0.0 },
        { delta.x,delta.y,1.0 }
    };
}
Mat get_scale_mat(Pos scale) {
    return Mat{
        {scale.x, 0.0, 0.0},
        { 0.0,scale.y,0.0 },
        { 0.0,0.0,1.0 }
    };
}
Mat get_scale_mat(double scale) {
    return get_scale_mat({ scale,scale });
}

Pos transform(Mat mat, Pos coor) {
    Pos res = mat * Vec3{coor.x, coor.y, 1.0};
    return Pos{res.x, res.y};
}

Vec2 transform_vec(Mat mat, Vec2 coor) {
    Vec3 res = mat * Vec3{coor.x, coor.y, 0.0};
    return Vec2{res.x, res.y};
}


struct Airport{
    const char *name, *abv;
    Vec2 pos;
    uint32_t flights;
};

uint32_t Euclidean(GraphNode<Airport>*start, GraphNode<Airport>*end){
    return((uint32_t)glm::distance(start->data.pos, end->data.pos));
}

uint32_t TaxiCab(GraphNode<Airport>*start, GraphNode<Airport>*end){
    return((uint32_t)(glm::abs(start->data.pos.x - end->data.pos.x)+glm::abs(start->data.pos.y - end->data.pos.y)));
}

uint32_t zeroHeuristic(GraphNode<Airport> *start, GraphNode<Airport> *end){
    return(0);
}

int main() {

    char* buffer = loadFileToBuffer("data/airports.csv");
    Graph<Airport> ports;
    int cursor = 0;
    while (buffer[cursor]) {
        Airport a;
        a.abv = parseString_fixedLength(buffer, cursor, 3);
        a.name = parseStringDelimited(buffer, cursor);
        a.pos.x = getRandom();
        a.pos.y = getRandom();
        a.flights = getRandom() % 2 +1;
        ports.addNode(newGraphNode(a));
    }
    uint32_t maxWt = 0;
    uint32_t minWt = UINT32_MAX;
    while (auto port= ports.nodes.iterate()) {
        for (int i = 0;i < port->data->data.flights;i++) {
            int j = getRandom()%ports.size;
            auto to = ports.nodes.search(j);
            uint32_t wt = getRandom() % 4 + 1;
            if (wt > maxWt)
                maxWt = wt;
            if (wt < minWt)
                minWt = wt;
            ports.addEdge(port->data, to->data,wt );
        }
    }


    Path<Airport> path;
    
    GraphNode<Airport> noSelection;
    noSelection.data.name = "--------";
    noSelection.data.abv = "";
    GraphNode<Airport> *start = &noSelection, *end= &noSelection;
    uint32_t cost = 0;

    

    Context::init();
    ImGui::StyleColorsDark();
    ImGui::GetStyle().Alpha = 0.75;
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF("comic.ttf", 18);

    Context::set_window_title("Tree of Flights");
    Context::set_window_icon("aeroplane.png");


    Pos world_scale = { 1.0,1.0 };
    Pos anchor_world = { 0.0,0.0 };
    Pos anchor_screen = Context::get_real_dim() * 0.5f;
    bool is_gui_hover = false;

    auto get_to_scr_mat = [&]() {

        Mat mat = get_scale_mat(1.0);

        //Translate back by world anchor
        mat = get_delta_mat(-anchor_world) * mat;

        //Scaling
        mat = get_scale_mat(world_scale) * mat;

        //Converting back to screen size
        mat = get_scale_mat(Context::get_real_dim() * 0.5f) * mat;

        //Translating back to anchor screen
        mat = get_delta_mat(anchor_screen) * mat;

        //Translating to center of screen
        //mat = get_delta_mat(graph_size * 0.5) * mat;

        return mat;

    };

    auto to_world = [&](Pos pos) {
        auto mat = glm::inverse(get_to_scr_mat());
        return transform(mat, pos);
    };

    auto to_screen = [&](Pos pos) {
        auto mat = get_to_scr_mat();
        return transform(mat, pos);
    };

    auto is_in_graph = [&](Pos pos) {
        return (pos.x >= 0 && pos.y >= 0 &&
            pos.x <= Context::get_real_dim().x &&
            pos.y <= Context::get_real_dim().y);
    };
    Context::cursor_move_callback = [&](double dx, double dy) {
        if (is_in_graph(Context::get_mouse_pos()) && !is_gui_hover
            && Context::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_1)) {
            anchor_screen += Pos{ dx, dy };
        }
    };
    Context::scroll_callback = [&](double dx, double dy) {
        if (is_in_graph(Context::get_mouse_pos()) && !is_gui_hover) {
            anchor_world = to_world(Context::get_mouse_pos());
            anchor_screen = Context::get_mouse_pos();
            world_scale *= pow(1.1, dy);
        }
    };



    // bool show = true;
    bool animations = false;
    int selection= 0;
    int heuristicSelected= 0;
    const char *algoOptions[] = {"Dijkstra", "A*", "BFS", "DFS"};
    const char *heuristicOptions[] = {"Zero", "Euclidean distance", "TaxiCab distance"};
    Timer f_timer;
    f_timer.reset();

    //Rate at which the path will be revealed
    double path_rate = 0.25;
    //A number representing where, and at which path we currently are
    //Integer path represents the path number and floating part represents fraction
    double curr_path = 0;
    //Plane foto
    Context::Texture fly_tex;
    if (!Context::createTexture("plane.png", fly_tex)) {
        std::cerr << "Couldnot load plane png" << std::endl;
        fly_tex = Context::default_texture;
    }
    Context::Texture rest_tex;
    if (!Context::createTexture("vert_plane.png", rest_tex)) {
        std::cerr << "Couldnot load resting plane png" << std::endl;
        rest_tex = Context::default_texture;
    }
    Context::Texture nepal_tex;
    if (!Context::createTexture("nepal_map.png", nepal_tex)) {
        std::cerr << "Couldnot load nepal map png" << std::endl;
        nepal_tex = Context::default_texture;
    }

    //These are to adjust aspect ratio and offset for background image
    glm::vec2 back_scale = { 1.f / world_scale.x, 1.f / world_scale.y };
    back_scale *= 1.2f;
    glm::vec2 back_pan = Context::get_real_dim() * 0.5f;

#ifdef  NDEBUG
    //Loading time
    const double load_time = 3.0;
    bool loading = true;
#endif //  NDEBUG
    while (!Context::poll_events_and_decide_quit()){
        double f_time = f_timer.elapsed();
#ifdef NDEBUG
        if (loading) {
            if (f_time > load_time)
                loading = false;
            float diag_len = glm::length(Context::get_real_dim());
            float ratio = Context::get_real_dim().x / (fly_tex.width * 10.f);
            diag_len -= fly_tex.width * ratio;
            float curr_len = fly_tex.width * ratio * 0.5 + diag_len * f_time / load_time;
            Context::init_rendering(Color::blackOlive);
            Context::Rectangle{
                .center = glm::normalize(Context::get_real_dim()) * curr_len,
                .size = glm::vec2{fly_tex.width,fly_tex.height}*ratio,
                .color = Color::white,
                .rotate = atan2f(Context::get_real_dim().y,Context::get_real_dim().x)
            }.draw(fly_tex);
            int windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove; 
            ImGui::Begin("", NULL, windowFlags);
            // imgui uses top left as (0,0)
            glm::vec2 pos = Context::get_real_dim() * glm::vec2{ 0.33f,0.66f };
            ImGui::SetWindowPos({ pos.x, pos.y });
            ImGui::Text("LOADING..."); 
            ImGui::End();
            Context::finish_rendering();
            continue;
        }
#endif


        is_gui_hover = false;
        f_timer.reset();
       
        Context::init_rendering(Color::silver);

        Context::Rectangle{
            .center = to_screen(back_pan),
            .size = {nepal_tex.width, nepal_tex.height},
            .color = Color::white,
            .scale = world_scale * back_scale
        }.draw(nepal_tex);
        // imgui window
        {
            int windowFlags = 0;
            windowFlags = windowFlags | ImGuiWindowFlags_AlwaysAutoResize;
            ImGui::Begin("Hello!",0, windowFlags);
            ImGui::SetWindowFontScale(1.5);

            is_gui_hover |= ImGui::IsWindowHovered();

            ImGui::Combo("Using?", &selection, algoOptions, sizeof(algoOptions)/sizeof(*algoOptions),-1);
            if(selection == 1)
                ImGui::Combo("Heuristic?", &heuristicSelected, heuristicOptions, sizeof(heuristicOptions)/sizeof(*heuristicOptions), -1);
            ImGui::Text("Current: %d ",selection);
            ImGui::Text("From: \t%s %s\t",start->data.name, start->data.abv);
            ImGui::Text("To:   \t%s %s\t",end->data.name, end->data.abv);
            ImGui::Checkbox("Animations", &animations);
            if (animations) {
                float speed = path_rate;
                ImGui::SliderFloat("Adjust Speed", &speed, 0.1f, 2.f);
                path_rate = speed;
            }
            ImGui::NewLine();

            if (ImGui::Button("Select start")){
                //Change the mouse click callback to set start node to clicked mouse position
                Context::mouse_click_callback = [&](int button, int action, int mods) {
                    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1 && !Context::is_key_pressed(GLFW_KEY_SPACE)) {
                        glm::vec2 mpos = Context::get_mouse_pos();
                        mpos = to_world(mpos);
                        float search_radius = 25;
                        auto node = ports.nodes.search(
                            [&](ListNode<GraphNode<Airport>*>* node, int)->bool {
                                return glm::distance(mpos, { node->data->data.pos.x,node->data->data.pos.y }) < search_radius;
                            }
                        );
                        if (node)
                            start = node->data;
                    }
                };
            }
            if (ImGui::Button("Select end")){
                //Change the mouse click callback to set start node to clicked mouse position
                Context::mouse_click_callback = [&](int button, int action, int mods) {
                    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1 && !Context::is_key_pressed(GLFW_KEY_SPACE)) {
                        glm::vec2 mpos = Context::get_mouse_pos();
                        mpos = to_world(mpos);
                        float search_radius = 25;
                        auto node = ports.nodes.search(
                            [&](ListNode<GraphNode<Airport>*>* node, int)->bool {
                                return glm::distance(mpos, { node->data->data.pos.x,node->data->data.pos.y }) < search_radius;
                            }
                        );
                        if (node)
                            end = node->data;
                    }
                };
            }
            static double timediff = 0;
            if (ImGui::Button("Find Path!")){
                curr_path = 0;
                path.edges.empty();
                Timer timer;
                timer.reset();

                uint32_t (*heuristicFunc)(GraphNode<Airport>*,GraphNode<Airport>*)=NULL;
                switch (heuristicSelected){
                case 0:     heuristicFunc = &zeroHeuristic;    break;
                case 1:     heuristicFunc = &Euclidean;        break;
                case 2:     heuristicFunc = &TaxiCab;          break;
                default:    break;
                }

                switch (selection){
                case 0:     cost = ports.Dijkstra(start, end, &path);           break;
                case 1:     cost = ports.AStar(start, end, heuristicFunc,&path);break;
                case 2:     cost = ports.BreadthFirstSearch(start, end, &path); break;
                case 3:     cost = ports.DepthFirstSearch(start, end, &path);   break;
                default:    break;
                }
                timediff = timer.elapsed();
            }
            ImGui::NewLine();
            ImGui::Text("Time taken to find path: %0.4lf ms", timediff*1000);
            ImGui::NewLine();
            ImGui::Text("Path cost: %u",cost);
            uint32_t dist = 0; 
            if(ImGui::CollapsingHeader("Path Details",ImGuiTreeNodeFlags_Framed)){
                is_gui_hover |= ImGui::IsItemHovered();
                if(!path.edges.isEmpty()){
                    // int tableFlags = 0;
                    int tableFlags = ImGuiTableFlags_PadOuterX|ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_RowBg;
                    ImGui::SetWindowFontScale(1.5);
                    if (ImGui::BeginTable("split", 4, tableFlags)){
                        is_gui_hover |= ImGui::IsItemHovered();

                        ImGui::TableSetupColumn("   FROM   ");
                        ImGui::TableSetupColumn(" ");
                        ImGui::TableSetupColumn("   TO   ");
                        ImGui::TableSetupColumn("   COST   ");
                        ImGui::TableHeadersRow();

                        auto st = path.start;
                        while(auto edge = path.edges.iterate()){
                            auto to = edge->data->to;
                            ImGui::TableNextColumn(); ImGui::Text("    %s   ", st->data.abv);
                            ImGui::TableNextColumn(); ImGui::Text(" -- ");
                            ImGui::TableNextColumn(); ImGui::Text("    %s   ", to->data.abv);
                            ImGui::TableNextColumn(); ImGui::Text("    %d   ", edge->data->weight);
                            // ImGui::TableNextRow();
                            dist += glm::distance(st->data.pos, to->data.pos);
                            st = to;
                        }
                        ImGui::EndTable();
                    }
                }
            }
            // ImGui::Text("Euclidean distance covered: %u", dist);

            
            ImGui::End();
            
        }


        // show names above selected nodes
        {
            auto showAirportName = [=](Airport *a){
                int windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ;
                ImGui::Begin(a->abv, NULL, windowFlags);
                // imgui uses top left as (0,0)
                glm::vec2 pos = to_screen({ a->pos.x,a->pos.y });
                pos += glm::vec2{ -17.5,60 };
                pos.y = Context::get_real_dim().y - pos.y;
                ImGui::SetWindowPos({ pos.x, pos.y }); 
                ImGui::Text("%s", a->abv);
                ImGui::End();
            };


            if (start != &noSelection) {
                showAirportName(&start->data);
            }
            if (end != &noSelection) {
                showAirportName(&end->data);
            }
            if (!path.edges.isEmpty()){
                while (auto edge = path.edges.iterate()){
                    auto to = edge->data->to;
                    if (to != end)
                        showAirportName(&to->data);
                }
            }
        }


        //First draw lines
        while (auto port = ports.nodes.iterate()) {

            while (auto edge = port->data->neighbours.iterate()) {
                auto to = edge->data.to;
                Context::Line{
                    to_screen({port->data->data.pos.x, port->data->data.pos.y}),
                    to_screen({to->data.pos.x, to->data.pos.y}),
                    Color::gray,
                    1
                }.draw();
            }
        }


        int visit_number = curr_path;
        float leftover = curr_path - visit_number;
        path.edges.iterator = nullptr;
        GraphNode<Airport>* st = path.start;
        while (auto edge = path.edges.iterate()) {
            auto to = edge->data->to;
            // if (to->next) {
                if (visit_number > 0 || !animations) {
                    Context::Circle{
                        .center = to_screen({st->data.pos.x,st->data.pos.y}),
                        .radius = (st == start || st == end)?25.0f:20.0f,
                        .color = Color::orange2
                    }.draw();
                    Context::Line{
                        .pos1 = to_screen({ st->data.pos.x ,st->data.pos.y }),
                        .pos2 = to_screen({ to->data.pos.x ,to->data.pos.y }),
                        .color = Color::pale,
                        .line_width = 6
                    }.draw();
                }
                else if (visit_number == 0) {
                    // glm::vec2 pos1{ to->data->data.x, to->data->data.y };
                    // glm::vec2 pos2{ to->next->data->data.x, to->next->data->data.y };
                    glm::vec2 pos1{ st->data.pos.x, st->data.pos.y };
                    glm::vec2 pos2{ to->data.pos.x, to->data.pos.y };
                    glm::vec2 mid = pos1 * (1.f - leftover) + pos2 * leftover;
                    Context::Circle{
                        .center = to_screen(pos1),
                        .radius = (st == start || st == end)?25.0f:20.0f,
                        .color = Color::orange2
                    }.draw();
                    Context::Line{
                        .pos1 = to_screen(pos1),
                        .pos2 = to_screen(mid),
                        .color = Color::pale,
                        .line_width = 6
                    }.draw();
                    pos1 = pos2 - pos1;
                    float fac = 1.f;
                    if (pos1.x < 0.f)
                        fac = -1.f;
                    Context::Rectangle{
                        .center = to_screen(mid),
                        .size = {60,35 * fac},
                        .color = Color::white,
                        .rotate = atan2f(pos1.y,pos1.x)
                    }.draw(fly_tex);
                    curr_path += ( maxWt *2.0 - edge->data->weight ) * path_rate * f_time / (maxWt*2.0 - minWt);

                }              
                visit_number--;
            // }
            // else {
            //     Context::Circle{
            //         .center = to_screen({to->data->data.pos.x,to->data->data.pos.y}),
            //         .radius = 25,
            //         .color = Color::orange2
            //     }.draw();
            // }
                st = to;
                if (st == end) {
                    Context::Circle{
                        .center = to_screen({st->data.pos.x,st->data.pos.y}),
                        .radius = 25.0f,
                        .color = Color::orange2
                    }.draw();
                }
        }

        //Now draw icons

        while (auto port = ports.nodes.iterate()){
            Vec2 b = { port->data->data.pos.x, port->data->data.pos.y };
            if (port->data == start) {
                Context::Circle{
                    .center = to_screen({b.x, b.y}) ,
                    .radius = 25,
                    .color = Color::red2
                }.draw(rest_tex);
            }
            else if (port->data == end)
                Context::Circle{
                    .center = to_screen({b.x, b.y}),
                    .radius = 25,
                    .color = Color::emerald
            }.draw(rest_tex);
            else
                Context::Circle{
                    .center = to_screen({b.x, b.y}),
                    .radius = 23,
                    .color = Color::blackOlive
            }.draw(rest_tex);

        }


        Context::finish_rendering();
    }
    Context::clean();
    return 0;
}

