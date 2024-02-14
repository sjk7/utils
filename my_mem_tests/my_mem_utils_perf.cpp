
#include "../../utils/my_timing.hpp"
#include "../../utils/my_utils.hpp"
#include "../../utils/my_memory_utils.hpp"
#include <vector>
#include <string>
#include <cstdio>


using namespace std;
using namespace my;


std::vector<std::string> make_random_strings(size_t howMany) {
  std::vector<std::string> strings;
  printf("Making %zu random strings ...\n", howMany);
  strings.reserve(howMany);
  for (size_t i = 0; i < howMany; ++i) {
    strings.push_back(my::utils::strings::random_string(32));
  }
  printf("Making %zu random strings, complete.\n", howMany);
  return strings;
}

int64_t test_arena(const std::vector<std::string> &v, Arena** a) {
  int64_t ret = 0;
  my::stopwatch sw("Testing Arena ...");

   auto l = (size_t)0;
  for (const auto &s : v) {
       l = s.size();
      volatile char *ptr = (char *)(*a)->alloc(l + 1);
      if (!ptr){
          printf("Arena out of space. Only %zu bytes free. Resetting it.\n", (*a)->space());
          //delete (*a);
          //(*a) = new Arena();
          (*a)->reset();
        continue;
    }
      if (ptr)
      memcpy((char*)ptr, s.c_str(), l + 1);
    ret += l;
  };

  printf("space() in Arena = %zu\n", (*a)->space());
  return ret;
}
int64_t test_malloc(const std::vector<std::string> &v) {

  int64_t ret = 0;
  my::stopwatch sw("Testing malloc (no free for fairness) ...");
  char *firstptr = 0;
  for (const auto &s : v) {               // NOLINT

      const auto l = s.size();
       volatile char *ptr = (char *)malloc(l + 1); // NOLINT
      memcpy((void*)ptr, s.c_str(), l + 1);
       ret -= l;
    if (firstptr == 0)
           firstptr = (char*)ptr;
  }
  // free(firstptr);
  return ret;
}

int main() {
    puts("App started. Please wait ...");
  auto v = make_random_strings(3'000'000);
    Arena* a;
    a  = new Arena();
  int64_t ret = 0;
  for (int i = 0; i < 10; ++i) {

      ret = test_arena(v,&a);
    printf("ret a: %zu\n", (size_t)ret);

      ret -= test_malloc(v);
    printf("ret b: %zu\n", (size_t)ret);
      ret -= test_arena(v,&a);
    printf("ret c: %zu\n", (size_t)ret);
      ret = test_malloc(v);
    printf("ret d: %zu\n", (size_t)ret);
  }
  if (a) delete a;
  printf("All done!\n");
  return (int)ret;

  /*/
  Arena myArena;
  printf("Arena capacity() is %zu bytes\n", myArena.capacity());
  assert(myArena.capacity() > 0);

  size_t old_free = myArena.space();
  char* strptr = (char*)myArena.alloc(256);
  size_t new_free = myArena.space();
  assert(old_free - new_free == 256);
  assert(myArena.size() == 256);
  constexpr const char* greet = "Hello, nobba!";
  memcpy(strptr, greet, strlen(greet));

  assert(strptr);
  puts(strptr);
  /*/
  return 0;
}
