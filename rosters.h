//
// Created by Zeben Brown  on 4/18/24.
//
#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

using json = nlohmann::json;

void GetRosters(std::string teamName);

extern std::map<std::string, json> rosterMap;
// players in a team
extern std::map<std::string, std::vector<json>> TeamMap;
//std::vector<std::string> allIDs;

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

struct BasicPlayerData {
    std::string playerID, Name, Team, Position;
};
