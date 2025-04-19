#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "list.hpp"

#include <sstream>
#include <iostream>

#include "../utils/filesystem.hpp"
#include "../utils/search.hpp"
#include "../net/download.hpp"


vector<string> split(string strToSplit, char delimeter) {
	stringstream ss(strToSplit);
	string item;
	vector<string> splittedStrings;

	while (getline(ss, item, delimeter))
		splittedStrings.push_back(item);

	return splittedStrings;
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
						Filesystem::mkDir("ux0:data/Easy_VPK/screenshots");
						curlDownload((PARENT_URL + subPath).c_str(), ("ux0:data/Easy_VPK/" + subPath).c_str());

						vita2d_texture *img;
						string img_file = ("ux0:data/Easy_VPK/" + subPath);

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