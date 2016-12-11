/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2016 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Account.h"
#include "binio.h"
#include <fstream>
#include "MacCompatibility.h"
#include "string.h"
#include <iostream>

using namespace std;

extern bool devtools;

vector<Account*> Account::accounts = vector<Account*>();
int Account::i_active = -1;

Account::Account(const string& name) : name(name), campaignProgress()
{
    difficulty = 0;
    progress = 0;
    points = 0;
    memset(highscore, 0, sizeof(highscore));
    memset(fasttime, 0, sizeof(fasttime));
    memset(unlocked, 0, sizeof(unlocked));

    setCurrentCampaign("main");
}

void Account::setCurrentCampaign(const string& name)
{
    currentCampaign = name;
}

void Account::add(const string& name)
{
    accounts.push_back(new Account(name));
    i_active = accounts.size() - 1;
}

Account* Account::get(int i)
{
    return accounts.at(i);
}

int Account::getNbAccounts()
{
    return accounts.size();
}

bool Account::hasActive()
{
    return (i_active >= 0);
}

Account& Account::active()
{
    return *(accounts.at(i_active));
}

void Account::setActive(int i)
{
    if ((i >= 0) && (i < int(accounts.size()))) {
        i_active = i;
    } else {
        cerr << "Tried to set active account to " << i << " but there is not such account" << endl;
        i_active = -1;
    }
}

void Account::destroyActive()
{
    if ((i_active >= 0) && (i_active < int(accounts.size()))) {
        accounts.erase(accounts.begin() + i_active);
        i_active = -1;
    } else {
        cerr << "Tried to destroy active account " << i_active << " but there is not such account" << endl;
        i_active = -1;
    }
}

int Account::getDifficulty()
{
    return difficulty;
}

void Account::endGame()
{
    campaignProgress[currentCampaign].choices.clear();
    campaignProgress[currentCampaign].score = 0;
    campaignProgress[currentCampaign].time = 0;
}

void Account::winCampaignLevel(int choice, float score, float time)
{
    campaignProgress[currentCampaign].choices.push_back(choice);
    setCampaignScore(campaignProgress[currentCampaign].score + score);
    campaignProgress[currentCampaign].time = time;
}

void Account::winLevel(int level, float score, float time)
{
    if (!devtools) {
        if (score > highscore[level])
            highscore[level] = score;
        if (time < fasttime[level] || fasttime[level] == 0)
            fasttime[level] = time;
    }
    if (progress < level + 1)
        progress = level + 1;
}

void Account::loadFile(string filename)
{
    FILE *tfile;
    int numaccounts;
    int iactive;
    errno = 0;

    tfile = fopen(filename.c_str(), "rb" );

    if (tfile) {
        funpackf(tfile, "Bi", &numaccounts);
        funpackf(tfile, "Bi", &iactive);
        printf("number of accounts %d\n", numaccounts);
        for (int i = 0; i < numaccounts; i++) {
            printf("loading account %d/%d\n", i, numaccounts);
            Account* acc = new Account();
            funpackf(tfile, "Bi", &(acc->difficulty));
            funpackf(tfile, "Bi", &(acc->progress));
            int nbCampaigns;
            funpackf(tfile, "Bi", &nbCampaigns);

            for (int k = 0; k < nbCampaigns; ++k) {
                string campaignName = "";
                int t;
                char c;
                funpackf(tfile, "Bi",  &t);
                for (int j = 0; j < t; j++) {
                    funpackf(tfile, "Bb",  &c);
                    campaignName.append(1, c);
                }
                funpackf(tfile, "Bf", &(acc->campaignProgress[campaignName].time));
                funpackf(tfile, "Bf", &(acc->campaignProgress[campaignName].score));
                funpackf(tfile, "Bf", &(acc->campaignProgress[campaignName].fasttime));
                funpackf(tfile, "Bf", &(acc->campaignProgress[campaignName].highscore));
                int campaignchoicesmade, campaignchoice;
                funpackf(tfile, "Bi", &campaignchoicesmade);
                for (int j = 0; j < campaignchoicesmade; j++) {
                    funpackf(tfile, "Bi", &campaignchoice);
                    if (campaignchoice >= 10) { // what is that for?
                        campaignchoice = 0;
                    }
                    acc->campaignProgress[campaignName].choices.push_back(campaignchoice);
                }
            }

            acc->currentCampaign = "";
            int t;
            char c;
            funpackf(tfile, "Bi",  &t);
            for (int i = 0; i < t; i++) {
                funpackf(tfile, "Bb",  &c);
                acc->currentCampaign.append(1, c);
            }

            funpackf(tfile, "Bf", &(acc->points));
            for (int i = 0; i < 50; i++) {
                funpackf(tfile, "Bf", &(acc->highscore[i]));
                funpackf(tfile, "Bf", &(acc->fasttime[i]));
            }
            for (int i = 0; i < 60; i++) {
                funpackf(tfile, "Bb",  &(acc->unlocked[i]));
            }
            int temp;
            char ctemp;
            funpackf(tfile, "Bi",  &temp);
            for (int i = 0; i < temp; i++) {
                funpackf(tfile, "Bb",  &ctemp);
                acc->name.append(1, ctemp);
            }
            if (!strcmp(acc->name.c_str(), ""))
                acc->name = "Lugaru Player"; // no empty player name security.
            accounts.push_back(acc);
        }

        fclose(tfile);
        setActive(iactive);
    } else {
        perror(("Couldn't load users from " + filename).c_str());
        i_active = -1;
    }
}

void Account::saveFile(string filename)
{
    FILE *tfile;
    errno = 0;

    tfile = fopen(filename.c_str(), "wb" );
    if (tfile) {
        fpackf(tfile, "Bi", getNbAccounts());
        fpackf(tfile, "Bi", i_active);

        for (int i = 0; i < getNbAccounts(); i++) {
            Account* a = Account::get(i);
            printf("writing account %d/%d (%s)\n", i + 1, getNbAccounts(), a->getName().c_str());
            fpackf(tfile, "Bi", a->difficulty);
            fpackf(tfile, "Bi", a->progress);
            fpackf(tfile, "Bi", a->campaignProgress.size());

            map<string, CampaignProgress>::const_iterator it;
            for (it = a->campaignProgress.begin(); it != a->campaignProgress.end(); ++it) {
                fpackf(tfile, "Bi",  it->first.size());
                for (unsigned j = 0; j < it->first.size(); j++) {
                    fpackf(tfile, "Bb",  it->first[j]);
                }
                fpackf(tfile, "Bf", it->second.time);
                fpackf(tfile, "Bf", it->second.score);
                fpackf(tfile, "Bf", it->second.fasttime);
                fpackf(tfile, "Bf", it->second.highscore);
                fpackf(tfile, "Bi", it->second.choices.size());
                for (unsigned j = 0; j < it->second.choices.size(); j++) {
                    fpackf(tfile, "Bi", it->second.choices[j]);
                }
            }

            fpackf(tfile, "Bi", a->getCurrentCampaign().size());
            for (unsigned j = 0; j < a->getCurrentCampaign().size(); j++) {
                fpackf(tfile, "Bb", a->getCurrentCampaign()[j]);
            }

            fpackf(tfile, "Bf", a->points);
            for (unsigned j = 0; j < 50; j++) {
                fpackf(tfile, "Bf", a->highscore[j]);
                fpackf(tfile, "Bf", a->fasttime[j]);
            }
            for (unsigned j = 0; j < 60; j++) {
                fpackf(tfile, "Bb",  a->unlocked[j]);
            }
            fpackf(tfile, "Bi",  a->name.size());
            for (unsigned j = 0; j < a->name.size(); j++) {
                fpackf(tfile, "Bb",  a->name[j]);
            }
        }

        fclose(tfile);
    } else {
        perror(("Couldn't save users in " + filename).c_str());
    }
}
