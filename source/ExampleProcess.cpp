#include <windows.h>
#include <iostream>
#include <time.h>

int main (int argc, const char* argv[])
{
	{
		clock_t start = clock ();
		for (int i = 0; i < argc; i++) {
			OutputDebugString (argv[i]);
			OutputDebugString ("\n");
		}
		for (int i = 0; i < 100000; i++) {
			char text[1024];
			sprintf_s (text, 1024, "%d\n", i);
			OutputDebugString (text);
		}
		clock_t end = clock ();
		std::cout << "Lot of OutputDebugString: " << (float) (end - start) << std::endl;
	}

	{
		clock_t start = clock ();
		double x = 0.0;
		for (int i = 0; i < 10000000; i++) {
			x += sqrt ((double) i);
		}
		clock_t end = clock ();
		std::cout << "Lot of calculation: " << (float) (end - start) << std::endl;
	}
	
	return 0;
}
