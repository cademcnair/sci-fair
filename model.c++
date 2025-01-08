#include "mode.c++"
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <set>
using namespace std;

namespace Wordle {
  auto total_tries = unordered_map<int, unordered_map<int, long long>>{};
  typedef unordered_map<char, int> letter_freq_item;
  typedef unordered_map<char, unordered_set<char>> guess_info_item;
  typedef vector<letter_freq_item> letter_freq;
  typedef vector<guess_info_item> guess_info;
  class Model {
  public:
    Model(unordered_map<int, double> c_variables) {
      variables = c_variables;
      is_marked = false;
      results = set<pair<string, double>>{};
    }
    Model(unordered_map<int, double> c_variables, int c_gen, int c_num) {
      variables = c_variables;
      is_marked = true;
      gen = c_gen;
      num = c_num;
      results = set<pair<string, double>>{};
    }
    unordered_map<int, double> variables;
    set<pair<string, double>> results;
    int gen, num;
    bool is_marked;
  };
  unordered_set<char> commons{'t', 'n', 's', 'r', 'l', 'c'};
  unordered_set<char> vowels{'a', 'e', 'i', 'o', 'u'};
  double score_word(Model m, string word, letter_freq letters, guess_info known) {
    auto variables = m.variables;
    double total = 0;
    unordered_set<char> used_points;
    for (int i = 0; i < 5; ++i) {
      auto this_guess = known[i];
      char character = word[i];
      int points_source = 0;
      double to_add = 0;
      if (this_guess['n'].contains(character)) continue;
      // points for green letters
      if (this_guess['g'].contains(character)) continue;
      bool has_used_char = used_points.contains(character);
      // points for yellow letters
      if (this_guess['y'].contains(character)) {
        double value = variables[has_used_char ? 2 : 1];
        if (value > to_add) {
          points_source = 1;
          to_add = value;
        }
      }

      bool is_common = commons.contains(character);
      bool is_vowel = vowels.contains(character);
      // points for letter frequency
      double this_freq = (letters[i][character] / letters[i]['_']);
      double x = variables[has_used_char ? 7 : 6];
      this_freq = this_freq + (pow(x, this_freq * this_freq)*0.1);
      double points_freq = variables[has_used_char ? 5 : 4] * this_freq;
      if (is_common) points_freq *= variables[9];
      if (is_vowel) points_freq *= variables[3];
      if (points_freq > to_add) {
        points_source = 2;
        to_add = points_freq;
      }

      double all_freq = (letters[5][character] / letters[5]['_']);
      all_freq = all_freq + (pow(x, all_freq * all_freq)*0.1);
      double all_points = variables[8] * (has_used_char ? (variables[5]/variables[4]) : 1) * all_freq;
      if (is_common) all_points *= variables[9];
      if (is_vowel) all_points *= variables[3];
      if (all_points > to_add) {
        points_source = 3;
        to_add = all_points;
      }
      if (points_source != 0) used_points.insert(character);
      total += to_add;
    }
    return total;
  }
  string play_word(Model m, unordered_set<string> pos_words, letter_freq letters, guess_info known, string ans) {
    if (pos_words.size() == 1) return *pos_words.begin();
    // this is a forced random selection
    set<string> top; double most_points = 0;
    for (auto word : pos_words) {
      double score = score_word(m, word, letters, known);
      if (score > most_points) {
        top = set<string>{word};
        most_points = score;
      }
      if (score == most_points) {
        top.insert(word);
      }
    }
    return *top.begin();
  }
  vector<string> simulate_game(Model m, string ans, unordered_set<string> g_words, letter_freq letters, guess_info known, ofstream *output = 0x0, int *t = 0x0) {
    unordered_set<string> words;
    for (auto word : g_words) words.insert(word);
    unordered_set<char> ans_chars{ans[0], ans[1], ans[2], ans[3], ans[4]};
    unordered_map<char, int> ans_chars_amounts;
    for (auto ans_char : ans_chars) ans_chars_amounts[ans_char] = 0;
    for (auto character : ans) ++ans_chars_amounts[character];
    vector<string> guess_list{};
    int total = 0;
    while (true) {
      if (guess_list.size() == 7 && MODE != 1) break;
      // cout << "-----" << endl;
      // for (auto guesses : guess_list) {
      //   cout << guesses << endl;
      // }
      // cout << words.size() << endl;
      // cout << words.contains("yearn") << endl;
      // cout << "-----" << endl;
      string playing = play_word(m, words, letters, known, ans);
      guess_list.push_back(playing);
      if (words.size() == 0) {
        cout << "!! " << ans << " " << guess_list.size() << " " << guess_list[guess_list.size() - 2] << endl;
        break;
      }
      if (playing == ans) break;
      words.erase(playing);
      // gathering information for settings
      unordered_set<char> playing_chars;
      unordered_map<char, int> yellows;
      unordered_map<char, int> found;
      for (int i = 0; i < 5; ++i) {
        playing_chars.insert(playing[i]);
        yellows[playing[i]] = 0;
        found[playing[i]] = 0;
      }
      for (auto character : playing_chars) known[5][character] = unordered_set<char>{};
      for (int i = 0; i < 5; ++i) {
        char character = playing[i];
        ++found[character];
        if (ans_chars.contains(character)) {
          if (ans[i] == character) {
            known[i]['g'].insert(character);
            known[i]['y'].clear();
          } else {
            known[i]['n'].insert(character);
            known[i]['y'].erase(character);
            for (int ii = 0; ii < 5; ++ii) {
              if (ii == i) continue;
              if (known[ii]['g'].empty())
                known[ii]['y'].insert(character);
            }
            ++yellows[character];
          }
        } else for (int ii = 0; ii < 5; ++ii) {
          known[5][character].insert('S');
          known[ii]['n'].insert(character);
        }
      }
      for (auto character : playing_chars) {
        for (int i = 0; i < yellows[character]; ++i) {
          known[5][character].insert('0'+i);
        }
      }
      auto it = words.begin();
      // deleting impossible words from list
      while (it != words.end()) {
        unordered_map<char, int> characters;
        string word = *it;
        bool deleting = false;
        // bool is_logging = word == "yearn";
        // taking off the easy ones: basing off greens/grays
        for (int i = 0; i < 5; ++i) {
          auto letter_info = known[i];
          char character = word[i];
          if (characters.contains(character))
            ++characters[character];
          else characters[character] = 1;
          if (letter_info['n'].contains(character)) {
            // if (is_logging) cout << "a1" << endl;
            deleting = true; 
            break;
          }
          if (!letter_info['g'].empty()) {
            if (*letter_info['g'].begin() != character) {
              // if (is_logging) cout << "a2" << endl;
              deleting = true;
              break;
            }
          }
        }
        if (deleting) {
          for (int i = 0; i < 5; ++i) {
            --letters[i][word[i]];
            --letters[5][word[i]];
            --letters[i]['_'];
          }
          letters[5]['_'] -= 5;
          auto temp_it = it;
          advance(it, 1);
          words.erase(temp_it);
          continue;
        }
        advance(it, 1);
      }
    }
    // total_tries[m.gen][m.num] += guess_list.size();
    int num_value = 0;
    if (MODE == 1) num_value = guess_list.size();
    if (MODE == 2) num_value = guess_list.size() > 6 ? 100 : guess_list.size();
    if (MODE == 3) num_value = min(int(guess_list.size()), 6);
    if (t != 0x0) *t += num_value;
    if (output != 0x0) (*output) << ans << " " << num_value << endl;
    return guess_list;
  }
  double simulate_all(Model m, unordered_set<string> words, int m_threads, bool logging = false) {
    double ans = 0;
    Wordle::letter_freq letters{
      unordered_map<char, int>{{'_', 2308},{'a', 140},{'b', 173},{'c', 198},{'d', 111},{'e', 71},{'f', 135},{'g', 115},{'h', 69},{'i', 34},{'j', 20},{'k', 20},{'l', 87},{'m', 107},{'n', 37},{'o', 41},{'p', 141},{'q', 23},{'r', 105},{'s', 365},{'t', 149},{'u', 33},{'v', 43},{'w', 82},{'y', 6},{'z', 3}},
      unordered_map<char, int>{{'_', 2308},{'a', 304},{'b', 16},{'c', 40},{'d', 20},{'e', 241},{'f', 8},{'g', 11},{'h', 144},{'i', 201},{'j', 2},{'k', 10},{'l', 200},{'m', 37},{'n', 87},{'o', 279},{'p', 61},{'q', 5},{'r', 267},{'s', 16},{'t', 77},{'u', 185},{'v', 15},{'w', 44},{'x', 14},{'y', 22},{'z', 2}},
      unordered_map<char, int>{{'_', 2308},{'a', 306},{'b', 55},{'c', 56},{'d', 75},{'e', 177},{'f', 25},{'g', 67},{'h', 9},{'i', 266},{'j', 3},{'k', 12},{'l', 112},{'m', 61},{'n', 137},{'o', 243},{'p', 57},{'q', 1},{'r', 163},{'s', 80},{'t', 111},{'u', 165},{'v', 49},{'w', 26},{'x', 12},{'y', 29},{'z', 11}},
      unordered_map<char, int>{{'_', 2308},{'a', 162},{'b', 24},{'c', 150},{'d', 69},{'e', 317},{'f', 35},{'g', 76},{'h', 28},{'i', 158},{'j', 2},{'k', 55},{'l', 162},{'m', 68},{'n', 182},{'o', 132},{'p', 50},{'r', 150},{'s', 171},{'t', 139},{'u', 82},{'v', 45},{'w', 25},{'x', 3},{'y', 3},{'z', 20}},
      unordered_map<char, int>{{'_', 2308},{'a', 63},{'b', 11},{'c', 31},{'d', 118},{'e', 422},{'f', 26},{'g', 41},{'h', 137},{'i', 11},{'k', 113},{'l', 155},{'m', 42},{'n', 130},{'o', 58},{'p', 56},{'r', 211},{'s', 36},{'t', 253},{'u', 1},{'w', 17},{'x', 8},{'y', 364},{'z', 4}},
      unordered_map<char, int>{{'_', 11540},{'a', 975},{'b', 279},{'c', 475},{'d', 393},{'e', 1228},{'f', 229},{'g', 310},{'h', 387},{'i', 670},{'j', 27},{'k', 210},{'l', 716},{'m', 315},{'n', 573},{'o', 753},{'p', 365},{'q', 29},{'r', 896},{'s', 668},{'t', 729},{'u', 466},{'v', 152},{'w', 194},{'y', 424},{'z', 40},{'x', 37}}
    };
    Wordle::guess_info known{
      unordered_map<char, unordered_set<char>>{
        {'g', unordered_set<char>{}},
        {'y', unordered_set<char>{}},
        {'n', unordered_set<char>{}},
      }, unordered_map<char, unordered_set<char>>{
        {'g', unordered_set<char>{}},
        {'y', unordered_set<char>{}},
        {'n', unordered_set<char>{}},
      }, unordered_map<char, unordered_set<char>>{
        {'g', unordered_set<char>{}},
        {'y', unordered_set<char>{}},
        {'n', unordered_set<char>{}},
      }, unordered_map<char, unordered_set<char>>{
        {'g', unordered_set<char>{}},
        {'y', unordered_set<char>{}},
        {'n', unordered_set<char>{}},
      }, unordered_map<char, unordered_set<char>>{
        {'g', unordered_set<char>{}},
        {'y', unordered_set<char>{}},
        {'n', unordered_set<char>{}},
      }, unordered_map<char, unordered_set<char>>{}
    };
    ofstream *output = 0x0;
    if (m.is_marked) {
      output = new ofstream("results/model-"+to_string(m.gen)+"-"+to_string(m.num));
    }
    vector<thread> threads{};
    int *total = new int(0);
    int total_num = 0;
    for (auto ans : words) {
      if (m_threads != 1) {
        threads.push_back(thread(Wordle::simulate_game, m, ans, words, letters, known, output, total));
        if (logging) ++total_num;
        if (threads.size() == m_threads) {
          for (auto& t : threads) t.join(); 
          threads.clear();
          // if (logging) cout << total_num << endl;
        }
      } else Wordle::simulate_game(m, ans, words, letters, known, output, total);
    }
    for (auto& t : threads) t.join();
    ans = (double)*total; 
    try{delete total;}catch(exception e){}
    if (m.is_marked) {
      *output << "TOTAL " << ans / words.size() << endl;
      ofstream easy_output("results/easy/model-"+to_string(m.gen)+"-"+to_string(m.num));
      easy_output << ans / words.size() << endl; easy_output.close();
    }
    output->close();
    return ans / words.size();
  }
}