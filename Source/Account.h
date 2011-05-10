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
#include <fstream>


class Account {
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
		void setDifficulty(int i) { difficulty = i; };
		const char* getName() { return name.c_str(); };
		float getCampaignScore() { return campaignscore; };
		int getCampaignChoicesMade() { return campaignchoices.size(); };
		int getCampaignChoice(int i) { return campaignchoices[i]; };
		void setCampaignScore(int s) {
			campaignscore=s;
			if(s>campaignhighscore)
				campaignhighscore=s;
		};
		void setCampaignFinalTime(float t) {
				campaigntime = t;
				if((t<campaignfasttime) || (campaignfasttime==0) && (t!=0))
					campaignfasttime = t;
		};
		float getCampaignFasttime() { return campaignfasttime; };
		void resetFasttime() { campaignfasttime = 0; };
		float getCampaignHighScore() { return campaignhighscore; };
		float getHighScore(int i) { return highscore[i]; };
		float getFastTime(int i) { return fasttime[i]; };
		int getProgress() { return progress; };
		
		static int getNbAccounts() { return accounts.size(); };
	private:
		Account(std::string n="");
		int difficulty;
		int progress;
		float points;
		float highscore[50];
		float fasttime[50];
		bool unlocked[60];
		std::string name;
		float campaignhighscore;
		float campaignfasttime;
		float campaignscore;
		float campaigntime;
		std::vector<int> campaignchoices;
	
	//statics
		static std::vector<Account*> accounts;
};

#endif
