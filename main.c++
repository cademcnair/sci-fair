#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include "generation.c++"

int main() {
  Wordle::Model m(unordered_map<int, double>{
    {1, 20.0}, {2, 10.0}, {3, 2.0},
    {4, 4.0}, {5, 2.0}, {6, 5.0},
    {7, 5.0}, {8, 3.0}, {9, 1.5}
  });
  m.is_marked = true;
  m.gen = -1;
  m.num = -1;
  unordered_set<string> words;
  const int w_words_amount = 2308;
  ifstream words_stream("wordle.txt");
  for (int i = 0; i < 2308; ++i) {
    string word; words_stream >> word;
    words.insert(word);
  }
  int custom; cout << "Custom (0=yes, 1=no) "; cin >> custom;
  if (custom == 0) {
    cout << "Generation will be one, models will be (int) ";
    int models_tot; cin >> models_tot;
    vector<Model> models = Generational::load_gen(1, models_tot);
    Generational::run_gen(models, words);
    return 0;
  }
  cout << "Current generation: " << endl;
  int curr_gen; cin >> curr_gen;
  double tot_var = 2.5;
  auto blank_window = [](){
    return unordered_map<int, double>{
      {1, 1}, {2, 1}, {3, 1}, {4, 1},
      {5, 1}, {6, 1}, {7, 1}, {8, 1}, {9, 1}
    };
  };
  while (true) {
    // tot_var += 0.03;
    vector<Model> models;
    int t = 0;
    if (curr_gen != 0) {
      unordered_map<int, double> using_window = blank_window();
      vector<Model> top = Generational::load_best(curr_gen, 100, 10);
      vector<Model> bottom = Generational::load_best(curr_gen, 100, -10);
      for (int i = 1; i < 10; ++i) {
        double avg_top = 0, avg_bottom = 0;
        for (int ii = 0; ii < 10; ++ii) {
          avg_top += top[ii].variables[i];
          avg_bottom += bottom[ii].variables[i];
        }
        avg_top /= 10; avg_bottom /= 10;
        using_window[i] = avg_top / avg_bottom;
      }
      for (auto best : top) {
        int num_models = ++t < 6 ? 15 : 5;
        for (auto new_model : Generational::gen_models(best, num_models, tot_var, using_window)) {
          models.push_back(new_model);
        }
      }
    } else {
      for (auto new_model : Generational::gen_models(m, 100, 1, blank_window())) {
        models.push_back(new_model);
      }
    }
    cout << "Training " << models.size() << " models" << endl;
    Generational::save_gen(models, ++curr_gen);
    Generational::run_gen(Generational::load_gen(curr_gen, 100), words);
  }
  // cout << Wordle::simulate_all(m, words, 8, true) << endl;
  // Generational::save_gen(Generational::gen_models(m, 100, 1.5), 1);
  // Generational::run_gen(Generational::load_gen(1, 100), words);

  return 0;
}