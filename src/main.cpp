#ifdef NDEBUG 
#ifdef _WIN32 || _WIN64 
#define main WinMain
#endif
#endif

#include <fstream>

#include "timers.hpp"

#include "graph.h"
#include "file.h"
#include "throwaway.h"
#include "renderers.hpp"
#include "math.h"

#define DataFilePath "./data/airportdata.csv"
#define FlightFilePath "./data/flights.csv"
#define MapTextureFile "./asia.png"


#define LONGITUDE_MIN 55
#define LONGITUDE_MAX 155
#define LATITUDE_MIN -25
#define LATITUDE_MAX 60

#define INITIAL_WORLD_SCALE 0.001

std::ostream& operator<<(std::ostream& os, Pos pos) {
    os << " ( " << pos.x << " , " << pos.y << " ) ";
    return os;
}


struct Airport{
    const char *name, *abv, *country;
    Vec2 pos;
    float latitude, longitude;
    uint32_t flights;
};
using Node = GraphNode<Airport>;


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

    Graph<Airport> ports;
    char* buffer = loadFileToBuffer(DataFilePath);
    int cursor = 0;
    while (buffer[cursor]) {
        Airport a;
        a.country = parseStringDelimited(buffer, cursor,',');
        a.name = parseStringDelimited(buffer, cursor,',');
        a.abv = parseString_fixedLength(buffer, cursor, 3);
        a.latitude = parseFloat(buffer, cursor);
        a.longitude = parseFloat(buffer,cursor);
        a.pos.x = a.longitude;
        a.pos.y = a.latitude;
        a.flights = getRandom() % 2 +1;
        auto newNode = newGraphNode(a);
        ports.addNode(newNode);
    }


    uint32_t maxWt = 0;
    uint32_t minWt = UINT32_MAX;
    // flights
    {
        char *flights = loadFileToBuffer(FlightFilePath);
        int curs = 0;
        GraphNode<Airport> *currentAirport = NULL;
        Timer t;
        Timer t1;
        t1.reset();
        int i = 0;
        while(flights[curs]){
            if (flights[curs] == '#'){
                const char *from = parseString_fixedLength(flights, ++curs, 3);
                
                auto found = ports.nodes.search(
                    [&](ListNode<Node *> *node, int)->bool{
                        return (strcmp(from, node->data->data.abv) == 0);
                    }
                );
                printf("Search time %d: %lf\n", i++, t.elapsed());
                if (found){
                    currentAirport = found->data;
                }
            }
            else if (flights[curs] == '.'){
                const char *to = parseString_fixedLength(flights, ++curs, 3);

                auto found = ports.nodes.search(
                    [&](ListNode<Node*>* node, int)->bool {
                        return (strcmp(to, node->data->data.abv) == 0);
                    }
                );
                printf("Search time %d: %lf\n", i++, t.elapsed());
                if (found) {
                    bool isRepeated = false;
                    while (auto toNodes = currentAirport->neighbours.iterate()){
                        if (found->data == toNodes->data.to)
                            isRepeated = true;
                    }
                    // only add edge if connection previously doesnt exist
                    if (!isRepeated){
                        uint32_t wt = getRandom() % 4 + 1;
                        if (wt > maxWt)
                            maxWt = wt;
                        if (wt < minWt)
                            minWt = wt;
                        ports.addEdge(currentAirport, found->data, wt);
                        currentAirport->data.flights++;
                    }
                }
            }
            curs++;
        }
        delete[] flights;
    }

    // the found path
    Path<Airport> path;        

    // placeholder values when no start/end airports
    GraphNode<Airport> noSelection;     
    noSelection.data.name = "--------";
    noSelection.data.abv = "";

    // start and end airports for pathfinding
    GraphNode<Airport> *start = &noSelection, *end= &noSelection;

    // currently selected airport
    GraphNode<Airport> *currentAirport = NULL;

    // cost of the returned path
    uint32_t cost = 0;

    //Log file setup
    std::ofstream log_file("log_file.log", std::ios::app | std::ios::out);
    //log_file << LOG_FILE_DATE_TIME;

    Context::init();
    ImGui::StyleColorsDark();
    ImGui::GetStyle().Alpha = 0.75;
    // ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF("comic.ttf", 18);

    Context::set_window_title("Tree of Flight");
    // Context::set_window_icon("aeroplane.png");


    Pos world_scale = { 0.019,0.019*2.0};
    Pos anchor_world = { 0.0,0.0 };
    Pos anchor_screen = { 0.f,0.f };
    bool is_gui_hover = false;



    // animations off/on
    bool animations = false;

    // selected algorithm/heuristic indexes + options
    int algoSelected= 0;
    const char *algoOptions[] = {"Dijkstra", "A*", "BFS", "DFS"};
    int heuristicSelected= 0;
    const char *heuristicOptions[] = {"Zero", "Euclidean distance", "Manhattan distance"};
    
    
    

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
    Context::Texture map_texture;
    if (!Context::createTexture(MapTextureFile, map_texture)) {
        std::cerr << "Couldnot load map png" << std::endl;
        map_texture = Context::default_texture;
    }

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

    auto is_in_screen = [&](Pos pos) {
        return (pos.x >= 0 && pos.y >= 0 &&
            pos.x <= Context::get_real_dim().x &&
            pos.y <= Context::get_real_dim().y);
    };
    bool mouse_dragged = false;
    




    //These are to adjust aspect ratio and offset for background image
    Vec2 map_size{ LONGITUDE_MAX - LONGITUDE_MIN, LATITUDE_MAX - LATITUDE_MIN };
    Vec2 map_center = Vec2{ LONGITUDE_MAX + LONGITUDE_MIN , LATITUDE_MAX + LATITUDE_MIN};
    map_center /= 2.0;


    Context::cursor_move_callback = [&](double dx, double dy) {
        if (is_in_screen(Context::get_mouse_pos()) && !is_gui_hover
            && Context::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_1)) {
            anchor_screen += Pos{ dx, dy };
            mouse_dragged = true;
        }
        else
            mouse_dragged = false;
    };

    Context::scroll_callback = [&](double dx, double dy) {
        if (is_in_screen(Context::get_mouse_pos()) && !is_gui_hover) {
            anchor_world = to_world(Context::get_mouse_pos());
            anchor_screen = Context::get_mouse_pos();
            world_scale *= pow(1.1, dy);
        }
    };

    // callback for mouseclick: selects an airport as currentAirport
    Context::mouse_click_callback = [&](int button, int action, int mods) {
        if (!is_gui_hover && !mouse_dragged && action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_1 && !Context::is_key_pressed(GLFW_KEY_SPACE)) {
            Vec2 mpos = Context::get_mouse_pos();
            
            float search_radius = 25;
            auto node = ports.nodes.search(
                [&](ListNode<GraphNode<Airport>*>* node, int)->bool {
                    return glm::distance(mpos,
                        to_screen({ node->data->data.pos.x,node->data->data.pos.y })) < search_radius;
                }
            );
            if (node)
                currentAirport = node->data;
        }
        mouse_dragged = false;
    };

    Timer f_timer;
    f_timer.reset();

    //Timer purpose analyzer
    Analyzer alz = MAKE_ANALYZER(General_Analyzer);


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

        // [BACKGROUND]
        // map
        Context::Rectangle{
            .center = to_screen(map_center),
            .size = transform_vec(get_to_scr_mat(), map_size),
            .color = Color::aqua
        }.draw(map_texture);

        // [UI]
        // imgui window
        {
            int windowFlags = 0;
            windowFlags = windowFlags | ImGuiWindowFlags_AlwaysAutoResize;
            ImGui::Begin("Flight Controls",0, windowFlags);
            ImGui::SetWindowFontScale(1.5);

            is_gui_hover |= ImGui::IsWindowHovered();
            is_gui_hover |= ImGui::IsAnyItemHovered();

            //General debug info, remove later
            {
                Pos mpos = Context::get_mouse_pos();
                Pos mwld = to_world(mpos);
                using namespace std;
                std::string msg;
                msg += "Screen mouse position : " + to_string(mpos.x) + " , " + to_string(mpos.y);
                msg += "\n";
                msg += "World mouse position : " + to_string(mwld.x) + " , " + to_string(mwld.y);
                msg += "\n";
                ImGui::Text("%s", msg.c_str());

            }


            // currently selected node
            if (ImGui::CollapsingHeader("Currently selected", ImGuiTreeNodeFlags_Framed)){
                if (currentAirport){
                    ImGui::Text("Name:    \t%s", currentAirport->data.name);
                    ImGui::Text("Country: \t%s", currentAirport->data.country);
                    ImGui::Text("Code:    \t%s", currentAirport->data.abv);
                    
                    if (ImGui::Button("Set as start")){
                        start = currentAirport;
                    }
                    ImGui::SameLine(0, 15);
                    if (ImGui::Button("Set as end")){
                        end = currentAirport;
                    }

                    if(ImGui::CollapsingHeader("Flights",ImGuiTreeNodeFlags_Framed)){
                        int tableFlags = ImGuiTableFlags_PadOuterX|ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_RowBg;
                        if (ImGui::BeginTable("flight", 4, tableFlags)){

                            ImGui::TableSetupColumn("   FROM   ");
                            ImGui::TableSetupColumn(" ");
                            ImGui::TableSetupColumn("   TO   ");
                            ImGui::TableSetupColumn("   COST   ");
                            ImGui::TableHeadersRow();

                            while(auto edge = currentAirport->neighbours.iterate()){
                                auto flight = edge->data;
                                auto to = flight.to;
                                ImGui::TableNextColumn(); ImGui::Text("    %s   ", currentAirport->data.abv);
                                ImGui::TableNextColumn(); ImGui::Text(" -- ");
                                ImGui::TableNextColumn(); ImGui::Text("    %s   ", to->data.abv);
                                ImGui::TableNextColumn(); ImGui::Text("    %d   ", flight.weight);
                                // ImGui::TableNextRow();
                            }
                            ImGui::EndTable();
                        }
                    }
                }
                ImGui::NewLine();
            } 
            ImGui::NewLine();


            {

                ImGui::CollapsingHeader("Algorithm details", ImGuiTreeNodeFlags_Framed);
                ImGui::Combo("Using?", &algoSelected, algoOptions, sizeof(algoOptions)/sizeof(*algoOptions),-1);
                if(algoSelected == 1)
                    ImGui::Combo("Heuristic?", &heuristicSelected, heuristicOptions, sizeof(heuristicOptions)/sizeof(*heuristicOptions), -1);
                ImGui::Text("From: \t%s %s\t",start->data.name, start->data.abv);
                ImGui::Text("To:   \t%s %s\t",end->data.name, end->data.abv);
                ImGui::Checkbox("Animations", &animations);
                if (animations) {
                    float speed = path_rate;
                    ImGui::SliderFloat("Adjust Speed", &speed, 0.1f, 2.f);
                    path_rate = speed;
                }
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

                switch (algoSelected){
                case 0:     cost = ports.Dijkstra(start, end, &path);           break;
                case 1:     cost = ports.AStar(start, end, heuristicFunc,&path);break;
                case 2:     cost = ports.BreadthFirstSearch(start, end, &path); break;
                case 3:     cost = ports.DepthFirstSearch(start, end, &path);   break;
                default:    break;
                }
                timediff = timer.elapsed();
            }
            ImGui::NewLine();
            

            // details of found path
            if(ImGui::CollapsingHeader("Path Details",ImGuiTreeNodeFlags_Framed)){
                ImGui::Text("Time taken to find path: %0.4lf ms", timediff*1000);
                ImGui::NewLine();
                ImGui::SetWindowFontScale(1.8);
                ImGui::Text("Path cost: \t %u",cost);
                
                if(!path.edges.isEmpty()){
                    // int tableFlags = 0;
                    int tableFlags = ImGuiTableFlags_PadOuterX|ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit|ImGuiTableFlags_RowBg;
                    ImGui::SetWindowFontScale(1.5);
                    if (ImGui::BeginTable("split", 4, tableFlags)){

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


            if (currentAirport && currentAirport != start && currentAirport != end){
                showAirportName(&currentAirport->data);
            }
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

        auto to_scr_mat = get_to_scr_mat();

        while (auto port = ports.nodes.iterate()) {
            while (auto edge = port->data->neighbours.iterate()) {
                auto to = edge->data.to;

                Vec2 pos1 = transform(to_scr_mat, { port->data->data.pos.x, port->data->data.pos.y });
                Vec2 pos2 = transform(to_scr_mat, { to->data.pos.x, to->data.pos.y });

                if (!is_in_screen(pos1) && !is_in_screen(pos2))
                    continue;

                Context::Line{
                    pos1,
                    pos2,
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
                    Vec2 pos1{ st->data.pos.x, st->data.pos.y };
                    Vec2 pos2{ to->data.pos.x, to->data.pos.y };
                    Vec2 mid = pos1 * (1.f - leftover) + pos2 * leftover;
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
            Vec2 b = transform(to_scr_mat, { port->data->data.pos.x, port->data->data.pos.y });
            if (!is_in_screen(b))
                continue;
            if (port->data == start) {
                Context::Circle{
                    .center = b ,
                    .radius = 25,
                    .color = Color::red2
                }.draw(rest_tex);
            }
            else if (port->data == end)
                Context::Circle{
                    .center = b,
                    .radius = 25,
                    .color = Color::emerald
            }.draw(rest_tex);
            else if (port->data == currentAirport)
                Context::Circle{
                    .center = b,
                    .radius = 25,
                    .color = Color::aqua
            }.draw(rest_tex);
            else
                Context::Circle{
                    .center = b,
                    .radius = 23,
                    .color = Color::blackOlive
            }.draw(rest_tex);

        }


        Context::finish_rendering();

        alz.loop();

    }

    log_file << alz;

    Context::clean();
    return 0;
}

