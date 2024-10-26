#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

namespace Wordle {
  typedef unordered_map<char, int> letter_freq_item;
  typedef unordered_map<char, unordered_set<char>> guess_info_item;
  typedef vector<letter_freq_item> letter_freq;
  typedef vector<guess_info_item> guess_info;
  class Model {
    double score_word(string word, letter_freq letters, guess_info known, bool wordle_word) {
      double total = wordle_word ? variables[8] : 0;
      unordered_set<char> used_points;
      for (int i = 0; i < 5; ++i) {
        auto this_guess = known[i];
        char character = word[i];
        int points_source = 0;
        double to_add = 0;
        if (this_guess['n'].contains(character)) continue;
        // points for green letters
        if (this_guess['g'].contains(character)) {
          double value = variables[0];
          if (value > to_add) {
            points_source = 0;
            to_add = value;
          }
        }
        bool has_used_char = used_points.contains(character);
        // points for yellow letters
        if (this_guess['y'].contains(character)) {
          double value = variables[has_used_char ? 2 : 1];
          if (value > to_add) {
            points_source = 1;
            to_add = value;
          }
        }
        // points for letter frequency
        double this_freq = letters[i][character] / letters[i]['_'];
        double dilution_factor = has_used_char ? 7 : 6;
        this_freq = this_freq + (1-this_freq)/dilution_factor;
        double points_freq = variables[has_used_char ? 5 : 4] * this_freq;
        if (points_freq > to_add) {
          points_source = 2;
          to_add = points_freq;
        }
        if (points_source != 0) used_points.insert(character);
        total += to_add;
      }
      return total;
    }
    string play_word(unordered_set<string> pos_words, unordered_set<string> all_words, letter_freq letters, guess_info known) {
      if (pos_words.size() == 1) return *pos_words.begin();
      // this is a forced random selection
      string best_option = ""; double most_points = 0;
      for (auto word : all_words) {
        double score = score_word(word, letters, known, pos_words.contains(word));
        if (score > most_points) {
          best_option = word;
          most_points = score;
        }
      }
      return best_option;
    }
  public:
    vector<string> simulate_game(string ans, unordered_set<string> words, unordered_set<string> all_words, letter_freq letters, guess_info known) {
      unordered_set<char> ans_chars{ans[0], ans[1], ans[2], ans[3], ans[4]};
      vector<string> guess_list{};
      while (true) {
        string playing = play_word(words, all_words, letters, known);
        guess_list.push_back(playing);
        if (playing == ans) break;
        if (words.contains(playing))
          words.erase(playing);
        all_words.erase(playing);
        // gathering information for settings
        for (int i = 0; i < 5; ++i) {
          char character = playing[i];
          if (ans_chars.contains(character)) {
            if (ans[i] == character) {
              known[i]['g'].insert(character);
              known[i]['y'].clear();
            } else {
              known[i]['n'].insert(character);
              for (int ii = 0; ii < 5; ++ii) {
                if (ii == i) continue;
                if (!known[5].contains(character))
                  known[5][character] = unordered_set<char>{'0'};
                else known[5][character].insert('0'+known[5][character].size());
                if (known[ii]['g'].empty())
                  known[ii]['y'].insert(character);
              }
            }
          } else for (int ii = 0; ii < 5; ++ii) {
            if (known[5].contains(character))
              known[5][character].insert('S');
            known[ii]['n'].insert(character);
          }
        }
        auto it = words.begin();
        // deleting impossible words from list
        while (it != words.end()) {
          unordered_map<char, int> characters;
          string word = *it;
          bool deleting = false;
          // taking off the easy ones: basing off greens/grays
          for (int i = 0; i < 5; ++i) {
            auto letter_info = known[i];
            char character = word[i];
            if (characters.contains(character))
              ++characters[character];
            else characters[character] = 1;
            if (letter_info['n'].contains(character)) {
              deleting = true; break;
            }
            if (!letter_info['g'].empty()) {
              if (*letter_info['g'].begin() != character) {
                deleting = true; break;
              }
            }
          }
          if (deleting) {
            for (int i = 0; i < 5; ++i) {
              --letters[i][word[i]];
              --letters[i]['_'];
            }
            auto temp_it = it;
            advance(it, 1);
            words.erase(temp_it);
            continue;
          }
          // harder: taking off based off the yellows
          unordered_set<char> in_characters{word[0], word[1], word[2], word[3], word[4]};
          for (auto character : in_characters) {
            if (!known[5].contains(character)) continue;
            bool strict = known[5][character].contains('S');
            int amount = known[5][character].size()-int(strict);
            int word_amount = characters[character];
            if (amount > word_amount) {
              deleting = true; break;
            }
            if (strict) if (amount != word_amount) {
              deleting = true; break;
            }
          }
          if (deleting) {
            for (int i = 0; i < 5; ++i) {
              --letters[i][word[i]];
              --letters[i]['_'];
            }
            auto temp_it = it;
            advance(it, 1);
            words.erase(temp_it);
            continue;
          }
          advance(it, 1);
        }
      }
      return guess_list;
    }
    Model(unordered_map<int, double> c_variables) {
      variables = c_variables;
    }
    unordered_map<int, double> variables;
    long long total_tries = 0;
  };
}