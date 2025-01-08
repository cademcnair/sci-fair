#include "model.c++"
#include <algorithm>
#include <fstream>
#include <vector>
#include <thread>
#include <cmath>
using namespace std;
using namespace Wordle;

namespace Generational {
  typedef vector<pair<double, Model>> results;
  void log_gen(vector<Model> models) {
    for (auto model : models) {
      for (auto setting : model.variables) {
        cout << setting.first << " " << setting.second << endl;
      }
    }
  }
  vector<Model> load_gen(int gen, int models) {
    vector<Model> to_return;
    for (int i = 0; i < models; ++i) {
      ifstream input("models/model-"+to_string(gen)+"-"+to_string(i+1));
      unordered_map<int, double> settings;
      for (int ii = 0; ii < 10; ++ii) {
        int f; double l; 
        input >> f;
        input >> l;
        settings[f] = l;
      }
      to_return.push_back(Model(settings, gen, i+1));
      input.close();
    }
    return to_return;
  }
  vector<pair<double, int>> info_best(int gen, int total, int amount) {
    vector<Model> models = load_gen(gen, total);
    vector<pair<double, int>> results;
    for (int i = 0; i < total; ++i) {
      ifstream input("results/easy/model-"+to_string(gen)+"-"+to_string(i+1));
      double _; input >> _;
      results.push_back(make_pair(_, i));
      input.close();
    }
    sort(results.begin(), results.end());
    if (results.size() > amount) results.resize(amount);
    return results;
  }
  vector<Model> load_best(int gen, int total, int amount) {
    vector<Model> models = load_gen(gen, total);
    auto results = info_best(gen, total, amount);
    vector<Model> to_return;
    for (int i = 0; i < abs(amount); ++i) {
      cout << results[i].second << "(" << results[i].first << "); ";
      if (amount > 0) to_return.push_back(models[results[i].second]);
      else to_return.push_back(models[results[results.size() - i - 1].second]);
    }
    cout << endl;
    log_gen(to_return);
    return to_return;
  }
  results run_gen(vector<Model> models, unordered_set<string> words, int start_after = -1, int max_threads = GEN_THREADS, int sim_all_threads = MODEL_THREADS) {
    results to_return; int i = -1;
    vector<thread> threads;
    for (auto model : models) {
      ++i;
      if (i <= start_after) continue;
      threads.push_back(thread(Wordle::simulate_all, model, words, sim_all_threads, false));
      if (threads.size() == max_threads) {
        for (auto& t : threads) t.join(); 
        threads.clear();
      }
    }
    for (auto& t : threads) t.join();
    return to_return;
  }
  void save_gen(vector<Model> models, int gen) {
    for (int i = 0; i < models.size(); ++i) {
      ofstream output("models/model-"+to_string(gen)+"-"+to_string(i+1));
      for (auto setting : models[i].variables) {
        output << setting.first << " " << setting.second << endl;
      }
    }
  }
  vector<Model> gen_models(Model sample, int amount, double variation, unordered_map<int, double> window) {
    vector<Model> to_return{sample};
    int quota = INT32_MAX/2;
    for (int i = 0; i < amount-1; ++i) {
      double percent = (double(rand())/INT32_MAX)*(1/variation);
      unordered_map<int, double> model;
      for (int ii = 1; ii < 10; ++ii) {
        double curr = sample.variables[ii];
        double w = (min(window[ii], 2.0) - 1)/2;
        curr += curr * (percent * (rand() > quota ? 1 : -1) + w);
        model[ii] = curr;
      }
      to_return.push_back(Model(model));
    }
    return to_return;
  }

}