void entry(void)
{
	extern int main(int argc, const char **argv);
	const char *name[] = {
		"doom",
#if 0
		"-playdemo",
		"DEMO1",
#endif
	};
	main(sizeof (name)/sizeof (*name), name);
	for (;;);
}
