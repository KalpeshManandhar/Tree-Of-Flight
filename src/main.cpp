#include <iostream>
#include "graph.h"
#include "file.h"
#include "throwaway.h"
#include "renderers.hpp"

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
    while (!Context::poll_events_and_decide_quit()){

        Context::init_rendering(Color::silver);
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
                path.empty();
                cost = ports.Dijkstra(start, end, &path);
            }

            
            ImGui::End();
            

        }

        
        while (auto port = ports.nodes.iterate()){
            Vec2 b = { port->data->data.x, port->data->data.y };
            if (port->data == start) {

                Context::Circle{
                    .center = to_screen({b.x, b.y}) ,
                    .radius = 10,
                    .color = Color::red
                }.draw();

            }
            else if (port->data == end)
                Context::Circle{
                    .center = to_screen({b.x, b.y}),
                    .radius = 10,
                    .color = Color::lime
                }.draw();
            else
                Context::Circle{
                    .center = to_screen({b.x, b.y}),
                    .radius = 10,
                    .color = Color::green
                }.draw();

          
        }

        //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        while (auto to = path.iterate()) {
            if (to->next)
                Context::Line{
                    .pos1 = to_screen({ to->data->data.x ,to->data->data.y }),
                    .pos2 = to_screen({ to->next->data->data.x ,to->next->data->data.y  }),
                    .color = Color::red,
                    .line_width = 3
                }.draw();
        }

        Context::finish_rendering();
    }
    Context::clean();
    return 0;
}

