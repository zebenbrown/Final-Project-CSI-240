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
    j.at("Outfield assists").get_to(fielding.Outfield_Assists);
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
        baseRunning = js["stats"]["BaseRunning"];
    }
};

struct Fielding_Stats{
    Fielding fielding;

    Fielding_Stats(const nlohmann::json& js){
        fielding = js["stats"]["Fielding"];
    }
};

struct Hitting_Stats{
    Hitting hitting;

    Hitting_Stats(const nlohmann::json& js){
        hitting = js["stats"]["Hitting"];
    }
};

struct Pitching_Stats{
    Pitching pitching;

    Pitching_Stats(const nlohmann::json& js){
        pitching = js["stats"]["Pitching"];
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
    std::map<std::string, json> playerMap, rosterMap, teamMap;
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
        js = json::parse(ARI_roster.text);


        for (auto ARI: js["body"]["roster"]) {
            //std::string college = ARI["college"];
            std::string jersey_number = ARI["jerseyNum"];
            std::string batting_handness = ARI["bat"];
            std::string position = ARI["pos"];
            std::string height = ARI["height"];
            std::string weight = ARI["weight"];
            std::string throwing_handness = ARI["throw"];
            //std::string birthday = ARI["bDay"];
            std::string name = ARI["longName"];
//            if (position == "P" && !ARI["stats"].empty()) {
//                float WHIP = ARI["stats"]["Pitching"]["WHIP"];
//            }
//            else{
//                float hits = ARI["stats"]["Hitting"]["H"];
//        }

//            ARI_rosterOut << name << " Position: " << position << " Throws: " << throwing_handness << " Bats: "
//                          << batting_handness << " Jersey Number:" << jersey_number << " Height: " << height
//                          << " Weight: " << weight << "Hits: " << std::endl;
        }

        for (auto ARI: js["body"]["roster"]) {
            std::string pid = ARI["playerID"];
            std::string teamID = ARI["teamID"];
            teamMap[teamID] = ARI;
            rosterMap[pid] = ARI;
            if (rosterMap[pid].contains("stats") && rosterMap[pid]["stats"].contains("Pitching") && !rosterMap[pid]["stats"].empty()) {
                //BaseRunning_Stats baseRunningStats = BaseRunning_Stats(rosterMap[pid]);
                //Fielding_Stats fieldingStats = Fielding_Stats(rosterMap[pid]);
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }
            ARI_roster.status_code;
            std::cout << "ARI Roster Status Code " << ARI_roster.status_code << std::endl;

        cpr::Response COL_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=COL&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

            js = json::parse(COL_roster.text);

            for (auto COL: js["body"]["roster"]) {
            std::string pid = COL["playerID"];
            std::string teamID = COL["teamID"];
            rosterMap[pid] = COL;
            teamMap[teamID] = COL;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }
            std::cout << "COL Roster Status Code " << COL_roster.status_code << std::endl;

            cpr::Response LAD_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=LAD&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(LAD_roster.text);

        for (auto LAD: js["body"]["roster"]) {
            std::string pid = LAD["playerID"];
            std::string teamID = LAD["teamID"];
            teamMap[teamID] = LAD;
            rosterMap[pid] = LAD;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }
        std::cout << "LAD Roster Status Code " << LAD_roster.status_code << std::endl;

        cpr::Response SF_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=SF&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(SF_roster.text);

        for (auto SF: js["body"]["roster"]) {
            std::string pid = SF["playerID"];
            std::string teamID = SF["teamID"];
            rosterMap[pid] = SF;
            teamMap[teamID] = SF;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "SF Roster Status Code " << SF_roster.status_code << std::endl;

        cpr::Response SD_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=SD&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(SD_roster.text);

        for (auto SD: js["body"]["roster"]) {
            std::string pid = SD["playerID"];
            std::string teamID = SD["teamID"];
            teamMap[teamID] = SD;
            rosterMap[pid] = SD;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "SD Roster Status Code " << SD_roster.status_code << std::endl;


        cpr::Response CHC_roster = cpr::Get(cpr::Url{
                                                   "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=CHC&getStats=true"},
                                           cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                           cpr::Header{
                                                   {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                   {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                           });

        js = json::parse(CHC_roster.text);

        for (auto CHC: js["body"]["roster"]) {
            std::string pid = CHC["playerID"];
            std::string teamID = CHC["teamID"];
            teamMap[teamID] = CHC;
            rosterMap[pid] = CHC;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "CHC Roster Status Code " << CHC_roster.status_code << std::endl;

        cpr::Response STL_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=STL&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(STL_roster.text);

        for (auto STL: js["body"]["roster"]) {
            std::string pid = STL["playerID"];
            std::string teamID = STL["teamID"];
            teamMap[teamID] = STL;
            rosterMap[pid] = STL;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "STL Roster Status Code " << STL_roster.status_code << std::endl;

        cpr::Response CIN_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=CIN&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(CIN_roster.text);

        for (auto CIN: js["body"]["roster"]) {
            std::string pid = CIN["playerID"];
            std::string teamID = CIN["teamID"];
            teamMap[teamID] = CIN;
            rosterMap[pid] = CIN;

            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "CIN Roster Status Code " << CIN_roster.status_code << std::endl;

        cpr::Response PIT_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=PIT&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(PIT_roster.text);

        for (auto PIT: js["body"]["roster"]) {
            std::string pid = PIT["playerID"];
            std::string teamID = PIT["teamID"];
            teamMap[teamID] = PIT;
            rosterMap[pid] = PIT;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "PIT Roster Status Code " << PIT_roster.status_code << std::endl;

        cpr::Response MIL_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=MIL&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(MIL_roster.text);

        for (auto MIL: js["body"]["roster"]) {
            std::string pid = MIL["playerID"];
            std::string teamID = MIL["teamID"];
            teamMap[teamID] = MIL;
            rosterMap[pid] = MIL;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "MIL Roster Status Code " << MIL_roster.status_code << std::endl;

        cpr::Response PHI_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=PHI&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(PHI_roster.text);

        for (auto PHI: js["body"]["roster"]) {
            std::string pid = PHI["playerID"];
            std::string teamID = PHI["teamID"];
            teamMap[teamID] = PHI;
            rosterMap[pid] = PHI;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "PHI Roster Status Code " << PHI_roster.status_code << std::endl;

        cpr::Response NYM_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=NYM&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(NYM_roster.text);

        for (auto NYM: js["body"]["roster"]) {
            std::string pid = NYM["playerID"];
            std::string teamID = NYM["teamID"];
            teamMap[teamID] = NYM;
            rosterMap[pid] = NYM;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "NYM Roster Status Code " << NYM_roster.status_code << std::endl;

        cpr::Response ATL_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=ATL&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(ATL_roster.text);

        for (auto ATL: js["body"]["roster"]) {
            std::string pid = ATL["playerID"];
            std::string teamID = ATL["teamID"];
            teamMap[teamID] = ATL;
            rosterMap[pid] = ATL;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "ATL Roster Status Code " << ATL_roster.status_code << std::endl;

        cpr::Response MIA_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=MIA&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(MIA_roster.text);

        for (auto MIA: js["body"]["roster"]) {
            std::string pid = MIA["playerID"];
            std::string teamID = MIA["teamID"];
            teamMap[teamID] = MIA;
            rosterMap[pid] = MIA;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "MIA Roster Status Code " << MIA_roster.status_code << std::endl;

        cpr::Response WSH_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=WSH&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(WSH_roster.text);

        for (auto WSH: js["body"]["roster"]) {
            std::string pid = WSH["playerID"];
            std::string teamID = WSH["teamID"];
            teamMap[teamID] = WSH;
            rosterMap[pid] = WSH;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "WSH Roster Status Code " << WSH_roster.status_code << std::endl;

        cpr::Response SEA_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=SEA&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(SEA_roster.text);

        for (auto SEA: js["body"]["roster"]) {
            std::string pid = SEA["playerID"];
            std::string teamID = SEA["teamID"];
            teamMap[teamID] = SEA;
            rosterMap[pid] = SEA;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "SEA Roster Status Code " << SEA_roster.status_code << std::endl;

        cpr::Response HOU_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=HOU&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(HOU_roster.text);

        for (auto HOU: js["body"]["roster"]) {
            std::string pid = HOU["playerID"];
            std::string teamID = HOU["teamID"];
            teamMap[teamID] = HOU;
            rosterMap[pid] = HOU;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "HOU Roster Status Code " << HOU_roster.status_code << std::endl;

        cpr::Response LAA_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=LAA&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(LAA_roster.text);

        for (auto LAA: js["body"]["roster"]) {
            std::string pid = LAA["playerID"];
            std::string teamID = LAA["teamID"];
            teamMap[teamID] = LAA;
            rosterMap[pid] = LAA;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "LAA Roster Status Code " << LAA_roster.status_code << std::endl;

        cpr::Response OAK_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=OAK&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(OAK_roster.text);

        for (auto OAK: js["body"]["roster"]) {
            std::string pid = OAK["playerID"];
            std::string teamID = OAK["teamID"];
            teamMap[teamID] = OAK;
            rosterMap[pid] = OAK;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "OAK Roster Status Code " << OAK_roster.status_code << std::endl;

        cpr::Response TEX_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=TEX&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(TEX_roster.text);

        for (auto TEX: js["body"]["roster"]) {
            std::string pid = TEX["playerID"];
            std::string teamID = TEX["teamID"];
            teamMap[teamID] = TEX;
            rosterMap[pid] = TEX;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "TEX Roster Status Code " << TEX_roster.status_code << std::endl;

        cpr::Response CHW_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=CHW&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(CHW_roster.text);

        for (auto CHW: js["body"]["roster"]) {
            std::string pid = CHW["playerID"];
            std::string teamID = CHW["teamID"];
            teamMap[teamID] = CHW;
            rosterMap[pid] = CHW;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "CHW Roster Status Code " << CHW_roster.status_code << std::endl;

        cpr::Response DET_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=DET&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(DET_roster.text);

        for (auto DET: js["body"]["roster"]) {
            std::string pid = DET["playerID"];
            std::string teamID = DET["teamID"];
            teamMap[teamID] = DET;
            rosterMap[pid] = DET;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "DET Roster Status Code " << DET_roster.status_code << std::endl;

        cpr::Response CLE_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=CLE&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(CLE_roster.text);

        for (auto CLE: js["body"]["roster"]) {
            std::string pid = CLE["playerID"];
            std::string teamID = CLE["teamID"];
            teamMap[teamID] = CLE;
            rosterMap[pid] = CLE;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "CLE Roster Status Code " << CLE_roster.status_code << std::endl;

        cpr::Response MIN_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=MIN&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(MIN_roster.text);

        for (auto MIN: js["body"]["roster"]) {
            std::string pid = MIN["playerID"];
            std::string teamID = MIN["teamID"];
            teamMap[teamID] = MIN;
            rosterMap[pid] = MIN;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "MIN Roster Status Code " << MIN_roster.status_code << std::endl;

        cpr::Response KC_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=KC&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(KC_roster.text);

        for (auto KC: js["body"]["roster"]) {
            std::string pid = KC["playerID"];
            std::string teamID = KC["teamID"];
            teamMap[teamID] = KC;
            rosterMap[pid] = KC;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "KC Roster Status Code " << KC_roster.status_code << std::endl;

        cpr::Response NYY_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=NYY&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(NYY_roster.text);

        for (auto NYY: js["body"]["roster"]) {
            std::string pid = NYY["playerID"];
            std::string teamID = NYY["teamID"];
            teamMap[teamID] = NYY;
            rosterMap[pid] = NYY;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "NYY Roster Status Code " << NYY_roster.status_code << std::endl;

        cpr::Response BAL_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=BAL&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(BAL_roster.text);

        for (auto BAL: js["body"]["roster"]) {
            std::string pid = BAL["playerID"];
            std::string teamID = BAL["teamID"];
            teamMap[teamID] = BAL;
            rosterMap[pid] = BAL;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "BAL Roster Status Code " << BAL_roster.status_code << std::endl;

        cpr::Response TB_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=TB&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(TB_roster.text);

        for (auto TB: js["body"]["roster"]) {
            std::string pid = TB["playerID"];
            std::string teamID = TB["teamID"];
            teamMap[teamID] = TB;
            rosterMap[pid] = TB;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "TB Roster Status Code " << TB_roster.status_code << std::endl;

        cpr::Response BOS_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=BOS&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(BOS_roster.text);

        for (auto BOS: js["body"]["roster"]) {
            std::string pid = BOS["playerID"];
            std::string teamID = BOS["teamID"];
            teamMap[teamID] = BOS;
            rosterMap[pid] = BOS;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "BOS Roster Status Code " << BOS_roster.status_code << std::endl;

        cpr::Response TOR_roster = cpr::Get(cpr::Url{
                                                    "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=TOR&getStats=true"},
                                            cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                            cpr::Header{
                                                    {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                    {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                            });

        js = json::parse(TOR_roster.text);

        for (auto TOR: js["body"]["roster"]) {
            std::string pid = TOR["playerID"];
            std::string teamID = TOR["teamID"];
            teamMap[teamID] = TOR;
            rosterMap[pid] = TOR;
            if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
                Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
            }

            else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
                Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
            }
        }

        std::cout << "TOR Roster Status Code " << TOR_roster.status_code << std::endl;
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
                    ImGui::Begin("Roster/Stats",
                                 &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)


                    if (ImGui::CollapsingHeader("Teams"))
                    {
                        for (auto [teamID, teamRoster] : teamMap){
                            std::string tname= teamRoster["team"];
//                            ImGui::Text(tname.c_str());c_str
                            if (ImGui::CollapsingHeader(tname.c_str())){
                                std::string tid = teamRoster["teamID"];
                                ImGui::Text(tid.c_str());
                            }

                        }

//                        ImGui::Text("NL West");
//                        if (ImGui::CollapsingHeader("Arizona Diamondbacks"))
//                        {
//                             ImGui::Text(teamMap["1"]);
//                        }
//                        ImGui::CollapsingHeader("Colorado Rockies");
//                        ImGui::CollapsingHeader("Los Angeles Dodgers");
//                        ImGui::CollapsingHeader("San Fransisco Giants");
//                        ImGui::CollapsingHeader("San Diego Padres");
//
//                        ImGui::Text("NL Central");
//                        ImGui::CollapsingHeader("Chicago Cubs");
//                        ImGui::CollapsingHeader("Cincinnati Reds");
//                        ImGui::CollapsingHeader("Pittsburgh Pirates");
//                        ImGui::CollapsingHeader("Milwaukee Brewers");
//                        ImGui::CollapsingHeader("St. Louis Cardinals");
//
//                        ImGui::Text("NL East");
//                        ImGui::CollapsingHeader("Atlanta Braves");
//                        ImGui::CollapsingHeader("Miami Marlins");
//                        ImGui::CollapsingHeader("New York Mets");
//                        ImGui::CollapsingHeader("Philadelphia Phillies");
//                        ImGui::CollapsingHeader("Washington Nationals");
//
//                        ImGui::Text("AL West");
//                        ImGui::CollapsingHeader("Houston Astros");
//                        ImGui::CollapsingHeader("Los Angeles Angels");
//                        ImGui::CollapsingHeader("Seattle Mariners");
//                        ImGui::CollapsingHeader("Texas Rangers");
//                        ImGui::CollapsingHeader("Oakland Athletics");
//
//                        ImGui::Text("AL Central");
//                        ImGui::CollapsingHeader("Chicago White Sox");
//                        ImGui::CollapsingHeader("Cleveland Guardians");
//                        ImGui::CollapsingHeader("Detroit Tigers");
//                        ImGui::CollapsingHeader("Kansas City Royals");
//                        ImGui::CollapsingHeader("Minnesota Twins");
//
//                        ImGui::Text("AL East");
//                        ImGui::CollapsingHeader("Baltimore Orioles");
//                        ImGui::CollapsingHeader("Boston Red Sox");
//                        ImGui::CollapsingHeader("New York Yankees");
//                        ImGui::CollapsingHeader("Tampa Bay Rays");
//                        ImGui::CollapsingHeader("Toronto Blue Jays");
//
//                        ImGui::Text("");
                    }

                    if (ImGui::CollapsingHeader("Pitching Stats"))
                    {
                        ImGui::Text("Leaders");
                        ImGui::CollapsingHeader("ERA Leaders");
                        ImGui::CollapsingHeader("Strikeout Leaders");
                        ImGui::CollapsingHeader("Walk Leaders");
                        ImGui::Text("");
                    }

                    if (ImGui::CollapsingHeader("Hitting Stats"))
                    {
                        ImGui::CollapsingHeader("Batting Average Leaders");
                        ImGui::CollapsingHeader("On Base Percentage Leaders");
                        ImGui::CollapsingHeader("Slugging Percentage");
                        ImGui::CollapsingHeader("OPS Leaders");
                        ImGui::CollapsingHeader("Hits Leaders");
                        ImGui::CollapsingHeader("Doubles Leaders");
                        ImGui::CollapsingHeader("Triples Leaders");
                        ImGui::CollapsingHeader("Home Run Leaders");
                        ImGui::CollapsingHeader("RBI Leaders");
                        ImGui::CollapsingHeader("Walk Leaders");
                        ImGui::CollapsingHeader("Strikeout Leaders");
                        ImGui::Text("");
                    }


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