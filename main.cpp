#include <raylib.h>

using namespace std;
typedef long long ll;

#define VAL(x) cout << #x << ": " << x << '\n';
#define FOR(I, n) for(ll I=0;I<n;I++)

constexpr ll W = 800;
constexpr ll H = 600;

void update() {}
void draw()
{
	BeginDrawing();

		ClearBackground(BLACK);

	EndDrawing();
}

int main()
{
	InitWindow(W, H, "Jira Doom");

	SetTargetFPS(60);
	
	while (!WindowShouldClose())
	{
		update();
		draw();
	}

	CloseWindow();
}

