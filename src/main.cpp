#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1920, 1200, OF_WINDOW);
	ofRunApp(new testApp());
}
