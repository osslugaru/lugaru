/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010 - Côme <MCMic> BERNIGAUD

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _Account_H_
#define _Account_H_

#include <vector>
#include <string>
#include <map>
#include <fstream>

struct CampaignProgress {
    float highscore;
    float fasttime;
    float score;
    float time;
    std::vector<int> choices;
    CampaignProgress() {
        highscore = 0;
        fasttime = 0;
        score = 0;
        time = 0;
    }
};

class Account
{
public:
    static void destroy(int i);
    static Account* destroy(Account* a);
    static Account* add(std::string name);
    static Account* get(int i);
    static Account* loadFile(std::string filename);
    static void saveFile(std::string filename, Account* accountactive);
    static int indice(Account* a);

    void endGame();
    void winCampaignLevel(int choice, float score, float time);
    void winLevel(int level, float score, float time);

    // getter and setters
    int getDifficulty();
    void setDifficulty(int i) {
        difficulty = i;
    };
    const char* getName() {
        return name.c_str();
    };
    float getCampaignScore() {
        return campaignProgress[currentCampaign].score;
    };
    int getCampaignChoicesMade() {
        return campaignProgress[currentCampaign].choices.size();
    };
    int getCampaignChoice(int i) {
        return campaignProgress[currentCampaign].choices[i];
    };
    void setCampaignScore(int s) {
        campaignProgress[currentCampaign].score = s;
        if (s > campaignProgress[currentCampaign].highscore)
            campaignProgress[currentCampaign].highscore = s;
    };
    void setCampaignFinalTime(float t) {
        campaignProgress[currentCampaign].time = t;
        if ((t < campaignProgress[currentCampaign].fasttime) || ((campaignProgress[currentCampaign].fasttime == 0) && (t != 0)))
            campaignProgress[currentCampaign].fasttime = t;
    };
    float getCampaignFasttime() {
        return campaignProgress[currentCampaign].fasttime;
    };
    void resetFasttime() {
        campaignProgress[currentCampaign].fasttime = 0;
    };
    float getCampaignHighScore() {
        return campaignProgress[currentCampaign].highscore;
    };
    float getHighScore(int i) {
        return highscore[i];
    };
    float getFastTime(int i) {
        return fasttime[i];
    };
    int getProgress() {
        return progress;
    };
    std::string getCurrentCampaign() {
        return currentCampaign;
    };
    void setCurrentCampaign(std::string name);

    static int getNbAccounts() {
        return accounts.size();
    };
private:
    Account(std::string n = "");
    int difficulty;
    int progress; // progress in challenge levels
    float points;
    float highscore[50];
    float fasttime[50];
    bool unlocked[60];
    std::string name;

    std::string currentCampaign;
    std::map<std::string, CampaignProgress> campaignProgress;

    //statics
    static std::vector<Account*> accounts;
};

#endif
