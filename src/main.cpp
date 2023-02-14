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
                    .center = {b.x, b.y},
                    .radius = 30,
                    .color = Color::red
                }.draw();

            }
            else if (port->data == end)
                Context::Circle{
                    .center = {b.x,b.y},
                    .radius = 30,
                    .color = Color::lime
                }.draw();
            else
                Context::Circle{
                    .center = {b.x,b.y},
                    .radius = 30,
                    .color = Color::black
                }.draw();

          
        }

        //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        while (auto to = path.iterate()) {
            if (to->next)
                Context::Line{
                    .pos1 = { to->data->data.x,to->data->data.y },
                    .pos2 = { to->next->data->data.x,to->next->data->data.y },
                    .color = Color::red,
                    .line_width = 3
                }.draw();
        }

        Context::finish_rendering();
    }
    Context::clean();
    return 0;
}

