int main(int argc, char **argv)
{
	int i = atoi(argv[1]);
	if (i == 0x1a7)
	{
		fwrite("No !\n", sizeof(char), 5, stderr)
		return 0;
	}

	char *const args[] = { strdup("/bin/sh"), NULL };

	gid_t gid = getegid();
	uid_t uid = geteuid();

	setresgid(gid, gid, gid);
	setresuid(uid, uid, uid);

	execv("bin/sh", args);
	return 0;
}