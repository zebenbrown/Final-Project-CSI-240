#define SDL_MAIN_HANDLED true
#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <map>

#include <SDL.h>
#include <SDL_image.h>
//#include <SDL_mixer.h>
//#include <SDL_ttf.h>


#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>




using json = nlohmann::json;

struct BaseRunning{
    std::string CS, PO, SB;
};

struct Fielding{
    std::string Passed_Ball, Outfield_Assists, Errors, Pickoffs;
};

struct Hitting{
    std::string BB, AB, H, IBB, HR, RBI, TB, Triple, GIDP, Double, R, SF, AVG, SAC, HBP, SO;
};

struct Pitching{
    std::string BB, Balk, Wild_Pitch, Flyouts, Blown_Save, Loss, H, HR, ER, Strikes, WHIP, Groundouts, R, Innings_Pitched, Save, Batters_Faced, SO, Win, Hold, Pitches;
};

void from_json(const json& j, BaseRunning& br){
    j.at("CS").get_to(br.CS);
    j.at("PO").get_to(br.PO);
    j.at("SB").get_to(br.SB);
}

void from_json(const json& j, Fielding& fielding){
    j.at("Passed Ball").get_to(fielding.Passed_Ball);
    j.at("Outfield Assists").get_to(fielding.Outfield_Assists);
    j.at("E").get_to(fielding.Errors);
    j.at("Pickoffs").get_to(fielding.Pickoffs);
}

void from_json(const json& j, Hitting& hitting){
    j.at("BB").get_to(hitting.BB);
    j.at("AB").get_to(hitting.AB);
    j.at("H").get_to(hitting.H);
    j.at("IBB").get_to(hitting.IBB);
    j.at("HR").get_to(hitting.RBI);
    j.at("TB").get_to(hitting.TB);
    j.at("3B").get_to(hitting.Triple);
    j.at("GIDP").get_to(hitting.GIDP);
    j.at("2B").get_to(hitting.Double);
    j.at("R").get_to(hitting.R);
    j.at("avg").get_to(hitting.AVG);
    j.at("SAC").get_to(hitting.HBP);
    j.at("SO").get_to(hitting.SO);
}

void from_json(const json& j, Pitching& pitching){
    j.at("BB").get_to(pitching.BB);
    j.at("Balk").get_to(pitching.Balk);
    j.at("Wild Pitch").get_to(pitching.Wild_Pitch);
    j.at("Flyouts").get_to(pitching.Flyouts);
    j.at("BlownSave").get_to(pitching.Blown_Save);
    j.at("Loss").get_to(pitching.Loss);
    j.at("H").get_to(pitching.H);
    j.at("HR").get_to(pitching.HR);
    j.at("ER").get_to(pitching.ER);
    j.at("Strikes").get_to(pitching.ER);
    j.at("WHIP").get_to(pitching.WHIP);
    j.at("Groundouts").get_to(pitching.Groundouts);
    j.at("R").get_to(pitching.R);
    j.at("InningsPitched").get_to(pitching.Innings_Pitched);
    j.at("Save").get_to(pitching.Save);
    j.at("Batters Faced").get_to(pitching.Batters_Faced);
    j.at("SO").get_to(pitching.SO);
    j.at("Win").get_to(pitching.Win);
    j.at("Hold").get_to(pitching.Hold);
    j.at("Pitches").get_to(pitching.Pitches);
}

struct BaseRunning_Stats{
    BaseRunning baseRunning;

    BaseRunning_Stats(const nlohmann::json& js){
        baseRunning = js["BaseRunning"];
    }
};

struct Fielding_Stats{
    Fielding fielding;

    Fielding_Stats(const nlohmann::json& js){
        fielding = js["Fielding"];
    }
};

struct Hitting_Stats{
    Hitting hitting;

    Hitting_Stats(const nlohmann::json& js){
        hitting = js["Hitting"];
    }
};

struct Pitching_Stats{
    Pitching pitching;

    Pitching_Stats(const nlohmann::json& js){
        pitching = js["Pitching"];
    }
};

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __EMSCRIPTEN__
EM_JS(int, canvas_get_width, (), {
return canvas.width;
});

EM_JS(int, canvas_get_height, (), {
return canvas.height;
});
#endif

int main(int argc, char* argv[]) {
    // Unused argc, argv
    (void) argc;
    (void) argv;

    //database where the key is the PlayerID and the value is the json
    std::map<std::string, json> playerMap, rosterMap;
    std::vector<std::string> allIDs;

    std::ofstream PlayerIdOut;
    PlayerIdOut.open("players.txt", std::ios::out);
    if (!PlayerIdOut.is_open()) {
        std::cout << "players.txt failed to open" << std::endl;
        exit(1);
    }
    std::ifstream PlayerIdIn;
    PlayerIdIn.open("players.txt", std::ios::in);
    if (!PlayerIdIn.is_open()) {
        std::cout << "players.txt failed to open" << std::endl;
        exit(2);
    }

    std::ofstream ARI_rosterOut;
    ARI_rosterOut.open("Arizona Diamondbacks.txt", std::ios::out);
    if (!ARI_rosterOut.is_open()) {
        std::cout << "players.txt failed to open" << std::endl;
        exit(1);
    }

    json js;
    cpr::Response player_list = cpr::Get(
            cpr::Url{"https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBPlayerList"},
            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
            cpr::Header{
                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
            });
    js = json::parse(player_list.text);

    if (player_list.status_code != 200) {
        for (auto player: js["body"]) {
            std::string name = player["longName"];
            std::string playerID = player["playerID"];
            std::string team = player["team"];
            std::string teamID = player["teamID"];
            std::string line;
            std::cin.ignore();
            getline(PlayerIdIn, line);
        }
    } else {

        for (auto player: js["body"]) {
            //std::cout << player << std::endl;
            std::string pid = player["playerID"];
            playerMap[pid] = player;
            allIDs.push_back(pid);
        }

        PlayerIdOut.close();


        player_list.status_code;                  // 200
        player_list.header["content-type"];       // application/json; charset=utf-8
        player_list.text;                         // JSON text string
        std::cout << "Player List: " << player_list.status_code << std::endl;
        cpr::Response ARI_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=ARI&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });
        json ps;
        ps = json::parse(ARI_roster.text);
        //std::cout << ps["body"]["roster"] << std::endl;


//        for (auto ARI: ps["body"]["roster"]) {
//            //std::string college = ARI["college"];
//            std::string jersey_number = ARI["jerseyNum"];
//            std::string batting_handness = ARI["bat"];
//            std::string position = ARI["pos"];
//            std::string height = ARI["height"];
//            std::string weight = ARI["weight"];
//            std::string throwing_handness = ARI["throw"];
//            //std::string birthday = ARI["bDay"];
//            std::string name = ARI["longName"];
//
//
//            ARI_rosterOut << name << " Position: " << position << " Throws: " << throwing_handness << " Bats: "
//                          << batting_handness << " Jersey Number:" << jersey_number << " Height: " << height
//                          << " Weight: " << weight << std::endl;
//        }

        for (auto ARI: ps["body"]["roster"]) {
            std::string pid = ARI["playerID"];
            rosterMap[pid] = ARI;
            BaseRunning_Stats baseRunningStats = BaseRunning_Stats((rosterMap[pid]));
        }
            ARI_roster.status_code;
            std::cout << ARI_roster.status_code << std::endl;

            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
                printf("Error: %s\n", SDL_GetError());
                return -1;
            }

            auto width = 1280;
            auto height = 720;
#ifdef __EMSCRIPTEN__
            width = canvas_get_width();
            height = canvas_get_height();
#endif

            // Setup window
            SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
            SDL_Window *window = SDL_CreateWindow("Baseball Stats Program", SDL_WINDOWPOS_CENTERED,
                                                  SDL_WINDOWPOS_CENTERED, width, height, window_flags);

            if (!window) {
                std::cout << "Window could not be created!" << std::endl
                          << "SDL_Error: " << SDL_GetError() << std::endl;
                abort();
            }

            // Setup SDL_Renderer instance
            SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
            if (renderer == nullptr) {
                SDL_Log("Error creating SDL_Renderer!");
                abort();
            }
            //SDL_RendererInfo info;
            //SDL_GetRendererInfo(renderer, &info);
            //SDL_Log("Current SDL_Renderer: %s", info.name);

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            (void) io;
            //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsClassic();

            // Setup Platform/Renderer backends
            ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
            ImGui_ImplSDLRenderer_Init(renderer);

            // Load Fonts
            // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
            // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
            // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
            // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
            // - Read 'docs/FONTS.md' for more instructions and details.
            // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
            //io.Fonts->AddFontDefault();

            //io.Fonts->AddFontFromFileTTF("/Users/zebenbrown/Library/Fonts/commando.ttf", 12.0f);
            io.Fonts->Build();
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
            //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
            //IM_ASSERT(font != NULL);

            // Our state
            bool show_demo_window = false;
            bool show_another_window = false;
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

            // Main loop
            bool done = false;

            // Declare rect of square
            SDL_Rect squareRect;

            // Square dimensions: Half of the min(SCREEN_WIDTH, SCREEN_HEIGHT)
            squareRect.w = std::min(width, height) / 2;
            squareRect.h = std::min(width, height) / 2;

            // Square position: In the middle of the screen
            squareRect.x = width / 2 - squareRect.w / 2 - squareRect.w;
            squareRect.y = height / 2 - squareRect.h / 2;

            // Event loop
            while (!done) {
                // Poll and handle events (inputs, window resize, etc.)
                // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
                // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
                // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
                // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
                SDL_Event event;

                while (SDL_PollEvent(&event)) {
                    ImGui_ImplSDL2_ProcessEvent(&event);
                    if (event.type == SDL_QUIT)
                        done = true;
                    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                        event.window.windowID == SDL_GetWindowID(window))
                        done = true;
                }

                // Start the Dear ImGui frame
                ImGui_ImplSDLRenderer_NewFrame();
                ImGui_ImplSDL2_NewFrame();
                ImGui::NewFrame();

                // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
                if (show_demo_window)
                    ImGui::ShowDemoWindow(&show_demo_window);

                // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
                {
                    static float f = 0.0f;
                    static int counter = 0;

                    ImGui::Begin("Welcome To Baseball Stats Program", nullptr, ImGuiWindowFlags_AlwaysAutoResize |
                                                                               ImGuiWindowFlags_NoMove);                          // Create a window called "Hello, world!" and append into it.
                    //ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                    ImGui::Checkbox("Demo Window",
                                    &show_demo_window);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Another Window", &show_another_window);


                    //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                    //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                    //if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    //counter++;
                    //ImGui::SameLine();
                    //ImGui::Text("counter = %d", counter);

                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                                ImGui::GetIO().Framerate);
                    ImGui::End();
                }

                // 3. Show another simple window.
                if (show_another_window) {
                    ImGui::Begin("Another Window",
                                 &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                    ImGui::Text("Hello from another window!");
                    if (ImGui::Button("Close Me"))
                        show_another_window = false;
                    ImGui::End();
                }


                // Rendering
                ImGui::Render();

                SDL_SetRenderDrawColor(renderer, (Uint8) (clear_color.x * 255), (Uint8) (clear_color.y * 255),
                                       (Uint8) (clear_color.z * 255), (Uint8) (clear_color.w * 255));
                SDL_RenderClear(renderer);

                // todo: add your game logic here to be drawn before the ui rendering
                // Set renderer color red to draw the square
                //SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
                // Draw filled square
                //SDL_RenderFillRect(renderer, &squareRect);

                // present ui on top of your drawings
                ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
                SDL_RenderPresent(renderer);

                SDL_Delay(0);
            }

            // Cleanup
            ImGui_ImplSDLRenderer_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();

            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();

            return 0;
        }
    }