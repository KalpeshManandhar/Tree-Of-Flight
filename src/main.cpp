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

struct Airport{
    const char *name, *abv;
    Vec2 pos;
    uint32_t flights;
};



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
    while (auto port= ports.nodes.iterate()) {
        for (int i = 0;i < port->data->data.flights;i++) {
            int j = getRandom()%ports.size;
            auto to = ports.nodes.search(j);
            ports.addEdge(port->data, to->data);
        }
    }


    LinkedList<GraphEdge<Airport>*> path;
    
    GraphNode<Airport> noSelection;
    noSelection.data.name = "--------";
    noSelection.data.abv = "";
    GraphNode<Airport> *start = &noSelection, *end= &noSelection;
    uint32_t cost = 0;

    

    Context::init();
    ImGui::StyleColorsDark();
    ImGui::GetStyle().Alpha = 0.75;
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF("comic.ttf", 25);

    Context::set_window_title("Tree of Flights");
    Context::set_window_icon("aeroplane.png");
    //For panning{ moving? } features and zooming
    glm::vec2 pannedAmt = { Context::get_real_dim().x*0.43f,0.f};
    glm::vec2 zoomAmt = { 1.6f,0.8f };
    bool windowHover = false;

    Context::cursor_move_callback = [&](double delx, double dely) {
        if (Context::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_1) && Context::is_key_pressed(GLFW_KEY_SPACE)) {
            pannedAmt.x += delx/zoomAmt.x;
            pannedAmt.y += dely/zoomAmt.y;
        }
    };

    Context::scroll_callback = [&](double xoff, double yoff) {
        if(!windowHover)
            zoomAmt *= pow(1.05, yoff);
    };

    //Converts given coordinate to screen, i.e , applies pan and zoom
    auto to_screen = [&](glm::vec2 pos) {
        return (pos + pannedAmt - Context::get_real_dim() * 0.5f) * zoomAmt + Context::get_real_dim() * 0.5f ;
    };
    //Does reverse of above
    auto to_world = [&](glm::vec2 pos) {
        return (pos - Context::get_real_dim() * 0.5f) / zoomAmt + Context::get_real_dim() * 0.5f - pannedAmt;
    };

    bool show = true;
    bool animations = false;
    int selection= 0;
    const char *options[] = {"Dijkstra", "A*", "BFS", "DFS"};
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
    glm::vec2 back_scale = { 1.f / zoomAmt.x, 1.f / zoomAmt.y };
    back_scale *= 1.2f;
    glm::vec2 back_pan = Context::get_real_dim() * 0.5f - pannedAmt*0.5f;

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


        windowHover = false;
        f_timer.reset();
       
        Context::init_rendering(Color::silver);

        Context::Rectangle{
            .center = to_screen(back_pan),
            .size = {nepal_tex.width, nepal_tex.height},
            .color = Color::white,
            .scale = zoomAmt * back_scale
        }.draw(nepal_tex);
        // imgui window
        {
            int windowFlags = 0;
            windowFlags = windowFlags | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
            ImGui::Begin("Hello!",0, windowFlags);
            ImGui::SetWindowFontScale(1.2);

            windowHover |= ImGui::IsWindowHovered();

            ImGui::Combo("Using?", &selection, options, sizeof(options)/sizeof(*options),-1);
            ImGui::Text("Current: %d ",selection);
            ImGui::Text("From: \t%s %s\t",start->data.name, start->data.abv);
            ImGui::Text("To:   \t%s %s\t",end->data.name, end->data.abv);
            ImGui::Checkbox("Animations", &animations);
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
                path.empty();
                Timer timer;
                timer.reset();
                switch (selection){
                case 0:     cost = ports.Dijkstra(start, end, &path);break;
                case 1:     cost = ports.AStar(start, end,&zeroHeuristic ,&path);break;
                case 2:     cost = ports.BreadthFirstSearch(start, end, &path);break;
                case 3:     cost = ports.DepthFirstSearch(start, end, &path);break;
                
                default:    break;
                }
                timediff = timer.elapsed();
            }
            ImGui::NewLine();
            ImGui::Text("Time taken to find path: %0.4lf ms", timediff*1000);
            ImGui::NewLine();
            ImGui::Text("Path cost: %u",cost);
            if(ImGui::CollapsingHeader("Path Details",ImGuiTreeNodeFlags_Framed)){
                windowHover |= ImGui::IsItemHovered();
                if(!path.isEmpty()){
                    // int tableFlags = 0;
                    int tableFlags = ImGuiTableFlags_PadOuterX|ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_RowBg;
                    ImGui::SetWindowFontScale(1.2);
                    if (ImGui::BeginTable("split", 4, tableFlags)){
                        windowHover |= ImGui::IsItemHovered();

                        ImGui::TableSetupColumn("   FROM   ");
                        ImGui::TableSetupColumn(" ");
                        ImGui::TableSetupColumn("   TO   ");
                        ImGui::TableSetupColumn("   COST   ");
                        ImGui::TableHeadersRow();

                        auto st = start;
                        while(auto edge = path.iterate()){
                            auto to = edge->data->to;
                            ImGui::TableNextColumn(); ImGui::Text("   %s   ", st->data.abv);
                            ImGui::TableNextColumn(); ImGui::Text(" -- ");
                            ImGui::TableNextColumn(); ImGui::Text("   %s   ", to->data.abv);
                            ImGui::TableNextColumn(); ImGui::Text("   %d   ", edge->data->weight);
                            // ImGui::TableNextRow();
                            st = to;
                        }
                        ImGui::EndTable();
                    }
                }
            }

            
            ImGui::End();
            
        }


        // show names above selected nodes
        {
            if (start != &noSelection) {
                int windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ;
                ImGui::Begin("Start", NULL, windowFlags);
                // imgui uses top left as (0,0)
                glm::vec2 pos = to_screen({ start->data.pos.x,start->data.pos.y });
                pos += glm::vec2{ -17.5,60 };
                pos.y = Context::get_real_dim().y - pos.y;
                ImGui::SetWindowPos({ pos.x, pos.y }); 
                ImGui::Text("%s", start->data.abv);
                ImGui::End();
            }
            if (end != &noSelection) {
                int windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
                ImGui::Begin("End", NULL, windowFlags);
                // imgui uses top left as (0,0)
                glm::vec2 pos = to_screen({ end->data.pos.x,end->data.pos.y });
                pos += glm::vec2{ -17.5,60 };
                pos.y = Context::get_real_dim().y - pos.y;
                ImGui::SetWindowPos({ pos.x, pos.y });
                ImGui::Text("%s", end->data.abv);
                ImGui::End();
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
        path.iterator = nullptr;
        GraphNode<Airport>* st = start;
        while (auto edge = path.iterate()) {
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


                    break;
                }
                else
                    break;
              
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
        curr_path += path_rate * f_time;

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

