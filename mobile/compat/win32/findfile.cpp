// findfile.cpp — _findfirst/_findnext/_findclose (POSIX dizin okuma + joker eşleşmesi).
#include <io.h>

#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>

#include <string>
#include <vector>

namespace
{
struct FindState
{
	std::vector<std::string> names;
	std::string dir;
	size_t index = 0;
};

void Fill(FindState* st, _finddata_t* fi)
{
	const std::string& n = st->names[st->index];
	std::memset(fi, 0, sizeof(*fi));
	std::snprintf(fi->name, sizeof(fi->name), "%s", n.c_str());
	struct stat sb {};
	if (stat((st->dir + "/" + n).c_str(), &sb) == 0)
	{
		fi->size       = (unsigned long) sb.st_size;
		fi->time_write = sb.st_mtime;
		fi->time_create = sb.st_ctime;
		fi->time_access = sb.st_atime;
		if (S_ISDIR(sb.st_mode))
			fi->attrib |= _A_SUBDIR;
	}
}
} // namespace

intptr_t _findfirst(const char* pattern, _finddata_t* fi)
{
	if (!pattern || !fi)
		return -1;
	std::string pat = pattern;
	for (char& c : pat)
		if (c == '\\') c = '/';
	std::string dir = ".", file = pat;
	auto slash = pat.find_last_of('/');
	if (slash != std::string::npos)
	{
		dir  = pat.substr(0, slash);
		file = pat.substr(slash + 1);
		if (dir.empty()) dir = "/";
	}
	DIR* d = opendir(dir.c_str());
	if (!d)
		return -1;
	auto* st = new FindState();
	st->dir  = dir;
	while (dirent* e = readdir(d))
	{
		if (std::strcmp(e->d_name, ".") == 0 || std::strcmp(e->d_name, "..") == 0)
			continue;
		if (fnmatch(file.c_str(), e->d_name, FNM_CASEFOLD) == 0)
			st->names.push_back(e->d_name);
	}
	closedir(d);
	if (st->names.empty())
	{
		delete st;
		return -1;
	}
	Fill(st, fi);
	return reinterpret_cast<intptr_t>(st);
}

int _findnext(intptr_t handle, _finddata_t* fi)
{
	if (handle == 0 || handle == -1 || !fi)
		return -1;
	auto* st = reinterpret_cast<FindState*>(handle);
	if (st->index + 1 >= st->names.size())
		return -1;
	++st->index;
	Fill(st, fi);
	return 0;
}

int _findclose(intptr_t handle)
{
	if (handle == 0 || handle == -1)
		return -1;
	delete reinterpret_cast<FindState*>(handle);
	return 0;
}
