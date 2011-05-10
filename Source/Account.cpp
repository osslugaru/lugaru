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

#include "Account.h"
#include "binio.h"
#include <fstream>
#include "MacCompatibility.h"
#include "string.h"

using namespace std;

extern bool debugmode;

vector<Account*> Account::accounts = vector<Account*>();

Account::Account(string n) {
	name = string(n);
	difficulty = 0;
	progress = 0;
	points = 0;
	memset(highscore, 0, sizeof(highscore));
	memset(fasttime, 0, sizeof(fasttime));
	memset(unlocked, 0, sizeof(unlocked));
	
	currentCampaign = "main";
}

Account* Account::add(string name) {
	accounts.push_back(new Account(name));
	return accounts.back();
}

Account* Account::get(int i) {
	
	if((i>=0)&&(i<accounts.size())) {
		return accounts[i];
	} else
		return NULL;
}

void Account::destroy(int i) {
	accounts.erase(accounts.begin()+i);
}
Account* Account::destroy(Account* a) {
	for(int i=0; i<accounts.size(); i++) {
		if(accounts[i]==a) {
			accounts.erase(accounts.begin()+i);
			return NULL;
		}
	}
	printf("Unexpected error : User %s not found %d\n",a->getName(),a);
	return accounts.front();
}

int Account::getDifficulty() {
	return difficulty;
}

void Account::endGame() {
	campaignProgress[currentCampaign].choices.clear();
	campaignProgress[currentCampaign].score=0;
	campaignProgress[currentCampaign].time=0;
}

void Account::winCampaignLevel(int choice, float score, float time) {
	campaignProgress[currentCampaign].choices.push_back(choice);
	setCampaignScore(campaignProgress[currentCampaign].score+score);
	campaignProgress[currentCampaign].time = time;
}

void Account::winLevel(int level, float score, float time) {
	if(!debugmode) {
		if(score>highscore[level])
			highscore[level]=score;
		if(time<fasttime[level]||fasttime[level]==0)
			fasttime[level]=time;
	}
	if(progress<level+1)
		progress=level+1;
}

Account* Account::loadFile(string filename) {
	FILE *tfile;
	int numaccounts;
	int accountactive;
	int j;
	
	tfile=fopen(ConvertFileName(filename.c_str()), "rb" );
	
	if(tfile)
	{
		funpackf(tfile, "Bi", &numaccounts);
		funpackf(tfile, "Bi", &accountactive);
		printf("number of accounts %d\n",numaccounts);
		for(int i=0;i<numaccounts;i++)
		{
			printf("loading account %d/%d\n",i,numaccounts);
			Account* acc = new Account();
			funpackf(tfile, "Bi", &(acc->difficulty));
			funpackf(tfile, "Bi", &(acc->progress));
			int nbCampaigns;
			funpackf(tfile, "Bi", &nbCampaigns);
			printf("loading %d campaign progress info\n",nbCampaigns);
			
			for(int k=0;k<nbCampaigns;++k) {
				string campaignName = "";
				int t;
				char c;
				funpackf(tfile, "Bi",  &t);
				for(j=0;j<t;j++)
				{
					funpackf(tfile, "Bb",  &c);
					campaignName.append(1,c);
				}
				printf("loading %s campaign progress info\n",campaignName.c_str());
				funpackf(tfile, "Bf", &(acc->campaignProgress[campaignName].time));
				funpackf(tfile, "Bf", &(acc->campaignProgress[campaignName].score));
				funpackf(tfile, "Bf", &(acc->campaignProgress[campaignName].fasttime));
				funpackf(tfile, "Bf", &(acc->campaignProgress[campaignName].highscore));
				int campaignchoicesmade,campaignchoice;
				funpackf(tfile, "Bi", &campaignchoicesmade);
				for(j=0;j<campaignchoicesmade;j++)
				{
					funpackf(tfile, "Bi", &campaignchoice);
					if (campaignchoice >= 10) // what is that for?
					{
						campaignchoice = 0;
					}
					acc->campaignProgress[campaignName].choices.push_back(campaignchoice);
				}
			}
			
			funpackf(tfile, "Bf", &(acc->points));
			for(j=0;j<50;j++)
			{
				funpackf(tfile, "Bf", &(acc->highscore[j]));
				funpackf(tfile, "Bf", &(acc->fasttime[j]));
			}
			for(j=0;j<60;j++)
			{
				funpackf(tfile, "Bb",  &(acc->unlocked[j]));
			}
			int temp;
			char ctemp;
			funpackf(tfile, "Bi",  &temp);
			for(j=0;j<temp;j++)
			{
				funpackf(tfile, "Bb",  &ctemp);
				acc->name.append(1,ctemp);
			}
			if(!strcmp(acc->name.c_str(),""))
				acc->name="Lugaru Player"; // no empty player name security.
			accounts.push_back(acc);
		}

		fclose(tfile);
		return get(accountactive);
	} else {
		printf("filenotfound\n");
		return NULL;
	}
}

void Account::saveFile(string filename, Account* accountactive) {
	FILE *tfile;
	int j;
	
	tfile=fopen(ConvertFileName(filename.c_str(), "wb"), "wb" );
	if(tfile)
	{
		printf("writing %d accounts :\n",getNbAccounts());
		fpackf(tfile, "Bi", getNbAccounts());
		fpackf(tfile, "Bi", indice(accountactive));
		
		for(int i=0;i<getNbAccounts();i++)
		{
			Account* a = Account::get(i);
			printf("writing account %d/%d (%s)\n",i+1,getNbAccounts(),a->getName());
			fpackf(tfile, "Bi", a->difficulty);
			fpackf(tfile, "Bi", a->progress);
			fpackf(tfile, "Bi", a->campaignProgress.size());
			
			map<string,campaign_progress_t>::const_iterator it;
			for( it=a->campaignProgress.begin(); it!= a->campaignProgress.end(); ++it) {
				fpackf(tfile, "Bi",  it->first.size());
				for(j=0;j<it->first.size();j++)
				{
					fpackf(tfile, "Bb",  it->first[j]);
				}
				fpackf(tfile, "Bf", it->second.time);
				fpackf(tfile, "Bf", it->second.score);
				fpackf(tfile, "Bf", it->second.fasttime);
				fpackf(tfile, "Bf", it->second.highscore);
				fpackf(tfile, "Bi", it->second.choices.size());
				for(j=0;j<it->second.choices.size();j++)
				{
					fpackf(tfile, "Bi", it->second.choices[j]);
				}
			}
			
			fpackf(tfile, "Bf", a->points);
			for(j=0;j<50;j++)
			{
				fpackf(tfile, "Bf", a->highscore[j]);
				fpackf(tfile, "Bf", a->fasttime[j]);
			}
			for(j=0;j<60;j++)
			{
				fpackf(tfile, "Bb",  a->unlocked[j]);
			}
			fpackf(tfile, "Bi",  a->name.size());
			for(j=0;j<a->name.size();j++)
			{
				fpackf(tfile, "Bb",  a->name[j]);
			}
		}

		fclose(tfile);
	}
}

int Account::indice(Account* a) {
	for(int i=0; i < accounts.size(); i++) {
		if(accounts[i]==a)
			return i;
	}
	return -1;
}
