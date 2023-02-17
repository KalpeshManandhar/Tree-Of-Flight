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

int main() {
    char* buffer = loadFileToBuffer("./data/airports.csv");
    Graph<Airport> ports;
    int cursor = 0;
    while (buffer[cursor]) {
        Airport a;
        a.abv = parseString_fixedLength(buffer, cursor, 3);
        a.name = parseStringDelimited(buffer, cursor);
        a.x = getRandom();
        a.y = getRandom();
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


    LinkedList<GraphNode<Airport>*> path;
    
    GraphNode<Airport> noSelection;
    noSelection.data.name = "--------";
    noSelection.data.abv = "";
    GraphNode<Airport> *start = &noSelection, *end= &noSelection;
    uint32_t cost = 0;

    Context::init();
    Context::set_window_title("Tree of Flights");
    Context::set_window_icon("aeroplane.png");
    //For panning{ moving? } features and zooming
    glm::vec2 pannedAmt = { 0.f,0.f };
    float zoomAmt = 1.f;

    Context::cursor_move_callback = [&](double delx, double dely) {
        if (Context::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_2)) {
            pannedAmt.x += delx;
            pannedAmt.y += dely;
        }
    };

    Context::scroll_callback = [&](double xoff, double yoff) {
        zoomAmt *= pow(1.05, yoff);
    };

    //Converts given coordinate to screen, i.e , applies pan and zoom
    auto to_screen = [&](glm::vec2 pos) {
        return (pos  - Context::get_real_dim() * 0.5f) * zoomAmt + Context::get_real_dim() * 0.5f + pannedAmt;
    };
    //Does reverse of above
    auto to_world = [&](glm::vec2 pos) {
        return (pos - pannedAmt - Context::get_real_dim() * 0.5f) / zoomAmt + Context::get_real_dim() * 0.5f ;
    };

    bool show = true;
    bool extend = false;
    int selection= 0;
    const char *options[] = {"Dijkstra", "A*", "Kruskal"};
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
        fly_tex = Context::default_texture;
    }

    

    while (!Context::poll_events_and_decide_quit()){
        double f_time = f_timer.elapsed();
        f_timer.reset();
        Context::init_rendering(Color::silver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.3f, 0.3f, 0.3f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
            
        {
            int windowFlags = 0;
            windowFlags = windowFlags | ImGuiWindowFlags_AlwaysAutoResize;
            ImGui::Begin("Hello!",0, windowFlags);
            ImGui::SetWindowFontScale(1.1);
            ImGui::Checkbox("Extend", &extend);
            if (extend){
                ImGui::Text("Hmm boo!");
            }

            ImGui::Combo("Using?", &selection, options, sizeof(options)/sizeof(*options),-1);
            ImGui::Text("Current: %d ",selection);
            ImGui::Text("From: \t%s %s\t",start->data.name, start->data.abv);
            ImGui::Text("To:   \t%s %s\t",end->data.name, end->data.abv);
            ImGui::Text("Path cost: %u",cost);
            ImGui::NewLine();
            if (ImGui::Button("Change start")){
                start = ports.nodes.search(getRandom()%ports.size)->data;
                cost = 0;
            }
            if (ImGui::Button("Change end")){
                end = ports.nodes.search(getRandom()%ports.size)->data;
                cost = 0;
            }

            if (ImGui::Button("Find Path!")){
                curr_path = 0;
                path.empty();
                cost = ports.Dijkstra(start, end, &path);
            }

            
            ImGui::End();
            

        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        
        while (auto port = ports.nodes.iterate()){
            Vec2 b = { port->data->data.x, port->data->data.y };
            if (port->data == start) {

                Context::Circle{
                    .center = to_screen({b.x, b.y}) ,
                    .radius = 25,
                    .color = Color::yellow
                }.draw(rest_tex);

            }
            else if (port->data == end)
                Context::Circle{
                    .center = to_screen({b.x, b.y}),
                    .radius = 25,
                    .color = Color::lime
                }.draw(rest_tex);
            else
                Context::Circle{
                    .center = to_screen({b.x, b.y}),
                    .radius = 25,
                    .color = Color::green
                }.draw(rest_tex);

          
        }

        int visit_number = curr_path;
        float leftover = curr_path - visit_number;
        path.iterator = nullptr;
        while (auto to = path.iterate()) {
            if (to->next) {
                if (visit_number > 0) {
                    Context::Line{
                        .pos1 = to_screen({ to->data->data.x ,to->data->data.y }),
                        .pos2 = to_screen({ to->next->data->data.x ,to->next->data->data.y  }),
                        .color = Color::olive,
                        .line_width = 3
                    }.draw();
                }
                else if (visit_number == 0) {
                    glm::vec2 pos1{ to->data->data.x, to->data->data.y };
                    glm::vec2 pos2{ to->next->data->data.x, to->next->data->data.y };
                    glm::vec2 mid = pos1 * (1.f - leftover) + pos2 * leftover;
                    Context::Line{
                        .pos1 = to_screen(pos1),
                        .pos2 = to_screen(mid),
                        .color = Color::olive,
                        .line_width = 3
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
            }
        }
                    curr_path += path_rate * f_time;
        Context::finish_rendering();
    }
    Context::clean();
    return 0;
}

