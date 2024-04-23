//
// Created by Zeben Brown  on 4/18/24.
//
#include "rosters.h"
using json = nlohmann::json;

std::map<std::string, json> rosterMap, playerMap;
// players in a team
std::map<std::string, std::vector<json>> teamMap;
std::vector<std::string> allIDs;


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

struct Basic_Information{
    std::string name, position, playerID;
};

void from_json(const json& j, Basic_Information& basicInformation){
    j.at("longName").get_to(basicInformation.name);
    j.at("pos").get_to(basicInformation.position);
    j.at("playerID").get_to(basicInformation.playerID);
}

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

struct Basic_Player_Information{
    Basic_Information basicInformation;

    Basic_Player_Information(const nlohmann::json& js){
        basicInformation = js["roster"];
    }
};

void GetRosters(std::string teamName){
    cpr::Response team_roster = cpr::Get(cpr::Url{
                                                "https://tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com/getMLBTeamRoster?teamAbv=" + teamName + "&getStats=true"},
                                        cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                                        cpr::Header{
                                                {"X-RapidAPI-Key",  "45163bd802msh2c9c15fa6c4660dp18f643jsn931c847bc680"},
                                                {"X-RapidAPI-Host", "tank01-mlb-live-in-game-real-time-statistics.p.rapidapi.com"}
                                        });

    json js = json::parse(team_roster.text);

    for (auto teams: js["body"]["roster"]) {
        std::string pid = teams["playerID"];
        std::string teamID = teams["teamID"];
        rosterMap[pid] = teams;
        teamMap[teamID].push_back(teams);
        if(rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Pitching")) {
            Pitching_Stats pitchingStats = Pitching_Stats(rosterMap[pid]);
        }

        else if (rosterMap[pid].contains("stats") && !rosterMap[pid]["stats"].empty() && rosterMap[pid]["stats"].contains("Hitting")){
            Hitting_Stats hittingStats = Hitting_Stats(rosterMap[pid]);
        }
    }
}