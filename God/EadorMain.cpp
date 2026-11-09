// Entry point of the native build. The game code is compiled once with
// NO_EADOR_MAIN into eador_core, which the unit tests link as well.
int EadorMain(int argc, char **argv);

int main(int argc, char **argv)
{
	return EadorMain(argc, argv);
}
