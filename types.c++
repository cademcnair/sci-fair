#include <unordered_map>
#include <string>
using namespace std;

namespace Wordle {
  class Model {
  public:
    unordered_map<int, double> variables;
    long long total_tries;
  };
}