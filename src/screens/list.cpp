#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "list.hpp"

#include <sstream>
#include <iostream>
#include <psp2/kernel/processmgr.h>

#include "../utils/filesystem.hpp"
#include "../utils/search.hpp"
#include "../net/download.hpp"

uint64_t lastJoystickScrollTime = 0;
int minCooldown = 50000;    // 50ms
int maxCooldown = 300000;   // 300ms

vector<string> split(string strToSplit, char delimeter) {
	stringstream ss(strToSplit);
	string item;
	vector<string> splittedStrings;

	while (getline(ss, item, delimeter))
		splittedStrings.push_back(item);

	return splittedStrings;
}

std::string extractNameFromURL(const std::string& url) {
    // Find the position of the last '/' in the URL
    size_t lastSlashPos = url.find_last_of('/');
    
    // If a '/' exists, return the substring from the position after the last '/'
    if (lastSlashPos != std::string::npos) {
        return url.substr(lastSlashPos + 1);
    }

    // If no '/' exists, return the URL as-is (it might be the name itself)
    return url;
}

void List::draw(SharedData &sharedData, unsigned int button) {
	
	if (updateImeDialog() == IME_DIALOG_RESULT_FINISHED) {
		searchResult = string((char *)getImeDialogInputTextUTF8());

		if (searchResult == "" || searchResult == "*")
			sharedData.vpks = sharedData.original;
		else
			sharedData.vpks = filterJson(searchResult, sharedData.original);
	}

	arrayLength = static_cast<int>(sharedData.vpks.size());
	scrollPercent = 504.0 / (arrayLength * 85);   
	scrollThumbHeight = 504 * scrollPercent;

	scrollThumbY = scrollY * scrollPercent;

	if (button == NULL) {
		scrollDelay = 0; 
		scrollStage = 0;
	}

	if (scrollDelay >= 0)
		scrollDelay--;

	vita2d_draw_rectangle(950, scrollThumbY, 10, scrollThumbHeight, RGBA8(0,0,0,150));

	if (sharedData.cursorY * 85 > scrollY + 374) scrollY += 85;
	if (sharedData.cursorY * 85 < scrollY) scrollY -= 85;

	// special iterator member functions for objects
	for (int i = 0; i < arrayLength; i++) {

		//if it isn't visible, skip render
		if ((i * 85) - scrollY > 544) break;
		if ((i * 85) + 80 - scrollY < 0) continue;

		vita2d_font_draw_textf(sharedData.font, 20, ((i * 85) + 48 - scrollY), WHITE, 32, "%s %s", sharedData.vpks[i]["name"].get<string>().c_str(), sharedData.vpks[i]["version"].get<string>().c_str());
		vita2d_font_draw_textf(sharedData.font, 20, ((i * 85) + 80 - scrollY), LGREY, 28,    "%s", sharedData.vpks[i]["description"].get<string>().c_str());
	}

	vita2d_draw_rectangle(10, ((sharedData.cursorY * 85) + 14 - scrollY), 940, 80, RGBA8(0,0,0,80));

	vita2d_draw_texture(navbar0, 0, 504);

	if (scrollDelay <= 1) {
		if (scrollDelay == 0)
			scrollStage = 0;

		uint64_t currentTime = sceKernelGetProcessTimeWide();  // in microseconds
        int pressure = abs(sharedData.leftStickY - 128);
        if (pressure > 30) {
            // Map pressure to cooldown: higher pressure = faster scrolling
            int scrollCooldown = maxCooldown - ((pressure - 30) * (maxCooldown - minCooldown)) / (127 - 30);
            
            if (currentTime - lastJoystickScrollTime >= scrollCooldown) {
                if (sharedData.leftStickY < 100 && sharedData.cursorY > 0) {
                    sharedData.cursorY--;
                    lastJoystickScrollTime = currentTime;
                } else if (sharedData.leftStickY > 155 && sharedData.cursorY < arrayLength - 1) {
                    sharedData.cursorY++;
                    lastJoystickScrollTime = currentTime;
                }
            }
        }
		
		switch (button) {
			case SCE_CTRL_CROSS:
				if (sharedData.blockCross)
					break;
				
				sharedData.initDetail = true;
				sharedData.blockCross = true;
				sharedData.screenshots.clear();

				if (!sharedData.liteMode && sharedData.vpks[sharedData.cursorY]["screenshots"].get<string>() != "") {
					if (!sharedData.screenshots.empty()) {
						for (int i = 0; i < sharedData.screenshots.size(); i++)
							if (sharedData.screenshots[i])
								vita2d_free_texture(sharedData.screenshots[i]);

						sharedData.screenshots.clear();
					}

					subPaths = split(sharedData.vpks[sharedData.cursorY]["screenshots"].get<string>().c_str(), ';');

					for (string subPath : subPaths) {
						Filesystem::mkDir("ux0:data/SimpleVPK/screenshots");
						vita2d_texture *img;
						string img_file = ("ux0:data/SimpleVPK/screenshots/" + extractNameFromURL(subPath));
						curlDownload(subPath.c_str(), img_file.c_str());

						img = vita2d_load_PNG_file(img_file.c_str());
						if (!img) img = vita2d_load_JPEG_file(img_file.c_str());
						if (!img) img = vita2d_load_BMP_file(img_file.c_str());

						if (img) sharedData.screenshots.push_back(img);
					}
				}

				sharedData.scene = 1;
				break;
			case SCE_CTRL_TRIANGLE:
				initImeDialog("Search for an application", "", 28);

				sharedData.cursorY = 0;
				break;
			case SCE_CTRL_DOWN:
				if (sharedData.cursorY >= arrayLength - 1)
					break;

				if (scrollStage <= 10) {
					if (scrollDelay == 1)
						scrollStage++;
					scrollDelay = 8;
				} else {
					scrollDelay = 3;
				}
				
				sharedData.cursorY++;
				break;
			case SCE_CTRL_UP:
				if (sharedData.cursorY <= 0)
					break;

				if (scrollStage <= 10) {
					if (scrollDelay == 1)
						scrollStage++;
					scrollDelay = 8;
				} else {
					scrollDelay = 3;
				}
				
				sharedData.cursorY--;
				break;
		}
	}
}

void List::free() {
	vita2d_free_texture(navbar0);
}
