#include <iostream>
#include "headers/graph.h"
#include "headers/file.h"

#include "headers/throwaway.h"


#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer.h"

#include "SDL.h"



int main(int argc, char**argv){

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

    
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0){
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // imgui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    SDL_Window* window = SDL_CreateWindow(
        "LMAO", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        1280, 720, 
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_ACCELERATED);
    if (!renderer){
        SDL_Log("Error creating SDL_Renderer!");
        return 0;
    }

    // init imgui for sdl w/sdl_renderer
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);


    ImVec4 bgColor = {123,123,123,45};
    bool show = true;
    bool extend = false;
    int selection= 0;
    const char *options[] = {"Dijkstra", "A*", "Kruskal"};
    while (true){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT){
                return(0);
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                return(0);
            }
        }
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();



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

        
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        SDL_RenderClear(renderer);

        while (auto port = ports.nodes.iterate()){
            Vec2 b = { port->data->data.x, port->data->data.y };
            if (port->data == start) {
                renderHighlightPoint(renderer, &b, { 255,0,0 });
            }
            else if (port->data == end)
                renderHighlightPoint(renderer, &b, { 0,255,0 });
            else
                renderHighlightPoint(renderer, &b);
            while (auto to = port->data->neighbours.iterate()) {
                auto c = to->data.to;
                Vec2 d = { c->data.x, c->data.y };
                drawLine(renderer, b, d);
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        while (auto to = path.iterate()) {
            if (to->next)
                drawLine(renderer, { to->data->data.x,to->data->data.y }, { to->next->data->data.x,to->next->data->data.y }, {255,0,0});
        }


        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }
}