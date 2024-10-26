#include <iostream>
#include <fstream>
#include "model.c++"

int main() {
  Wordle::Model m(unordered_map<int, double>{
    {1, 1.0}, {2, 5.0}, {3, 2.0},
    {4, 1.5}, {5, 1.0}, {6, 1.0},
    {7, 1.0}, {8, 0.0}, {9, 0.0}
  });
  unordered_set<string> w_words, words;
  const int words_amount = 8066, w_words_amount = 2308;
  ifstream words_stream("wordle.txt");
  for (int i = 0; i < 2308; ++i) {
    string word; words_stream >> word;
    words.insert(word);
  }
  ifstream w_words_stream("words.txt");
  for (int i = 0; i < 8066; ++i) {
    string word; w_words_stream >> word;
    w_words.insert(word);
  }
  Wordle::letter_freq letters{
    unordered_map<char, int>{},
    unordered_map<char, int>{},
    unordered_map<char, int>{},
    unordered_map<char, int>{},
    unordered_map<char, int>{}
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
  for (auto play : m.simulate_game("brass", w_words, words, letters, known)) cout << play << endl;
  cout << "---------------------" << endl;
  for (auto play : m.simulate_game("shock", w_words, words, letters, known)) cout << play << endl;
  return 0;
}