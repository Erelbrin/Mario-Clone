#include <SFML/Graphics.hpp>
#include "GameEngine.hpp"

int main(int argc, char* argv[])
{
	GameEngine g("assets.txt");
	g.run();

	return 0;
}