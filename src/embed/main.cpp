#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <gmodule.h>
#include <sys/stat.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>

FILE* open_or_exit(const char* fname, const char* mode)
{
  FILE* f = fopen(fname, mode);
  if (f == NULL) {
    perror(fname);
    exit(EXIT_FAILURE);
  }
  return f;
}

int main(int argc, char** argv)
{
  bool usegmodule = true;
  if (argc < 3) {
    fprintf(stderr, "USAGE: %s {sym} {rsrc}\n\n"
        "  Creates {sym}.c from the contents of {rsrc} fed into the module extension function of glib\n",
        argv[0]);
    return EXIT_FAILURE;
  }
  if(argc == 4)
  {
    std::string arg(argv[3]);
    if(arg.compare("-nomodule") == 0) usegmodule = false;
  }

  boost::filesystem::path libpath(argv[2]);

  const gchar* curr = boost::filesystem::system_complete(boost::filesystem::current_path()).c_str();

  const char* sym = argv[1];

  char symfile[256];
  snprintf(symfile, sizeof(symfile), "%s.c", sym);

  const char* path;
  if(usegmodule)
    path = (libpath.remove_filename()/boost::filesystem::path(g_module_build_path(curr, libpath.filename().c_str())).filename()).c_str();
  else
    path = (libpath.remove_filename()/boost::filesystem::path(libpath.filename().c_str())).filename().c_str();

  std::cout << "Encoding " << path << " into " << symfile << std::endl;

  FILE* in = open_or_exit(path, "r");

  FILE* out = open_or_exit(symfile,"w");
  fprintf(out, "#pragma GCC diagnostic ignored \"-Woverflow\"\n");
  fprintf(out, "#include <stdlib.h>\n");
  fprintf(out, "const char %s[] = {\n", sym);

  struct stat st;
  stat(path, &st);
  off_t size = st.st_size;

  char buf[size];
  size_t nread = 0;
  size_t linecount = 0;
  do {
    nread = fread(buf, 1, sizeof(buf), in);
    size_t i;
    for (i=0; i < nread; i++) {
      fprintf(out, "0x%02x,", buf[i]);
      if (++linecount == 10) { fprintf(out, "\n"); linecount = 0; }
    }
  } while (nread > 0);
  if (linecount > 0) fprintf(out, "\n");
  fprintf(out, "};\n");
  fprintf(out, "const size_t %s_len = sizeof(%s);\n\n",sym,sym);

  fclose(in);
  fclose(out);

  return EXIT_SUCCESS;
}
