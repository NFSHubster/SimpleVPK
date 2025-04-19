#include <vita2d.h>

#include "../main.hpp"


#define maxImageHeight 272


class Details {
	public:
		void draw(SharedData &sharedData, unsigned int button);
		void free();

	private:
		string longDescription;
		int lastNum = 0;
		int imageCycles = 0;
		int cycleCounter = 0;
		vita2d_texture *navbar1 = vita2d_load_PNG_file("ux0:app/SIMPLEVPK/resources/navbar1.png");
		vita2d_texture *navbar2 = vita2d_load_PNG_file("ux0:app/SIMPLEVPK/resources/navbar2.png");
		vita2d_texture *navbar3 = vita2d_load_PNG_file("ux0:app/SIMPLEVPK/resources/navbar3.png");
		vita2d_texture *navbar4 = vita2d_load_PNG_file("ux0:app/SIMPLEVPK/resources/navbar4.png");
};
