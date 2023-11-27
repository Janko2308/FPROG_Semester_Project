#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <regex>
#include <numeric>
#include <map>
#include <unordered_map>
#include <functional>
#include <optional>

auto calculateDistances = [](const std::unordered_map<std::string, int>& occurences) {
    std::map<std::string, std::vector<int>> distances;

    // for each entry in occurences, initialize a vector dist, the size is the count of the word
    // count of the word is the value of how many times the word appeared
    // fill the vector with 0, 1, 2, 3, ..., count - 1, representing indices of the word
    std::for_each(occurences.begin(), occurences.end(), [&](const auto& entry) {
        const std::string& word = entry.first;
        const int count = entry.second;

        std::vector<int>& dist = distances[word];
        dist.resize(count);

        std::iota(dist.begin(), dist.end(), 0);
    });

    // for each entry in occurences, retrieve the vector dist from the distances map
    // transform the values in dist, with respective indices, converting the distance vector
    // to a vector of distances
    std::for_each(occurences.begin(), occurences.end(), [&](const auto& entry) {
        const std::string& word = entry.first;

        std::vector<int>& dist = distances[word];
        int index = 0;
        std::transform(dist.begin(), dist.end(), dist.begin(), [&index](int) { return index++; });
    });

    return distances;
};

/// @brief Pure function to calculate the density of a word in a chapter
/// @param occurrences A map of words to their counts
/// @param totalWordsInChapter The total number of words in the chapter
/// @return The density of the word in the chapter
auto calculateDensity = [](const std::unordered_map<std::string, int>& occurrences, int totalWordsInChapter) {
    double totalOccurrences = std::accumulate(occurrences.begin(), occurrences.end(), 0,
        [](const int previous, const std::pair<std::string, int>& p) { return previous + p.second; });
    return totalWordsInChapter > 0 ? totalOccurrences / totalWordsInChapter : 0.0;
};

/// @brief Pure function to count occurences of words in a word list
/// @param words The list of words to count
/// @return A map of words to their counts
auto countOccurences = [](const std::vector<std::string>& words) {
    // Map step: Transform words into pairs of (word, 1)
    // As such all pairs are initialized with a count of 1
    auto map = [](const std::string& word) {
        return std::make_pair(word, 1);
    };

    // Transform the words into pairs
    std::vector<std::pair<std::string, int>> pairs;
    std::transform(words.begin(), words.end(), std::back_inserter(pairs), map);

    // Reduce step: Reduce the pairs into a map of words to their counts
    auto reduce = [](std::unordered_map<std::string, int>& result, const std::pair<std::string, int>& pair) {
        result[pair.first] += pair.second;
    };

    // Iterate over each element in the pairs vector and reduce them using reduce function
    // as such updating the counts of words in the result map.
    std::unordered_map<std::string, int> result;
    std::for_each(pairs.begin(), pairs.end(), std::bind(reduce, std::ref(result), std::placeholders::_1));

    return result;
};

/// @brief Pure function to filter words from a word list
/// @param wordList The list of all words to filter
/// @param filterList The list of words to filter out
/// @return The filtered list of words
auto filterWords = [](const std::vector<std::string>& filterList) {
    return [filterList](const std::vector<std::string>& wordList) {
        std::vector<std::string> result;

        // if the word from wordList is in filterList, copy it to result
        std::copy_if(wordList.begin(), wordList.end(), std::back_inserter(result), [&filterList](const std::string& word) {
            return std::find(filterList.begin(), filterList.end(), word) != filterList.end();
        });

        return result;
    };
};



// Pure function to read files
/// @brief Read file contents into a string
/// @param fileName The name of the file to read
/// @return The contents of the file
auto readFile = [](const std::string& fileName) -> std::optional<std::string> {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        return std::nullopt;
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
};

/// @brief Tokenize the input text
/// @param optionalInputText The input text to tokenize
/// @return A vector of tokens
auto tokenize = [](const std::optional<std::string>& optionalInputText) -> std::vector<std::string> {
    if (!optionalInputText) {
        return {}; // Return an empty vector if there's no input text
    }

    const std::string& inputText = *optionalInputText;
    // Replace "CHAPTER <number>" with "CHAPTER_<number>"
    std::regex chapterPattern(R"(CHAPTER (\d+))");
    std::string processedText = std::regex_replace(inputText, chapterPattern, "CHAPTER_$1");
    
    std::istringstream stream(processedText);
    const std::vector<std::string> tokens((std::istream_iterator<std::string>(stream)), std::istream_iterator<std::string>());

    // Map step: Transform tokens
    const auto filteredTokens = [&tokens]() {
        std::vector<std::string> result;
        std::transform(tokens.begin(), tokens.end(), std::back_inserter(result),
                       [](const std::string& token) {
                           std::string filtered;
                           std::copy_if(token.begin(), token.end(), std::back_inserter(filtered),
                                        [](char c) { return std::isalpha(c) || std::isdigit(c) || c == '_'; });
                           return filtered;
                       });
        return result;
    }();

    // Reduce step: Filter out empty tokens
    const auto nonEmptyTokens = [&filteredTokens]() {
        std::vector<std::string> result;
        std::copy_if(filteredTokens.begin(), filteredTokens.end(), std::back_inserter(result),
                     [](const std::string& token) { return !token.empty(); });
        return result;
    }();

    return nonEmptyTokens;
};

/// @brief Split the tokens by chapter
/// @param tokens The tokens to split
/// @return A map of chapter numbers to their tokens
auto splitByChapter = [](const std::vector<std::string>& tokens) {
    std::map<int, std::vector<std::string>> chapters;
    std::regex chapterPattern(R"(CHAPTER_\d+)");
    int chapterIndex = 0;

    // Use std::for_each to iterate over the tokens
    std::for_each(tokens.begin(), tokens.end(), [&chapters, &chapterIndex, &chapterPattern](const std::string& token) {
        if (std::regex_match(token, chapterPattern)) {
            // Start a new chapter
            chapterIndex++;
        } else {
            // Add token to the current chapter's vector
            chapters[chapterIndex].push_back(token);
        }
    });

    // If the first token is not a chapter and chapterIndex is still 0, remove the entry.
    if (chapterIndex == 0) {
        chapters.erase(chapterIndex);
    }

    return chapters;
};

TEST_CASE("calculateDistances with empty input") {
    std::unordered_map<std::string, int> emptyMap;
    auto result = calculateDistances(emptyMap);

    CHECK(result.empty());
}

TEST_CASE("calculateDistances with non-empty input") {
    std::unordered_map<std::string, int> inputMap = {
        {"apple", 3},
        {"orange", 2},
        {"banana", 4}
    };
    auto result = calculateDistances(inputMap);

    CHECK(result.size() == inputMap.size());

    // Check distances for each word
    CHECK(result["apple"] == std::vector<int>{0, 1, 2});
    CHECK(result["orange"] == std::vector<int>{0, 1});
    CHECK(result["banana"] == std::vector<int>{0, 1, 2, 3});
}

TEST_CASE("calculateDensity with empty occurrences") {
    std::unordered_map<std::string, int> emptyMap;
    int totalWords = 100;  // Some arbitrary total words
    auto result = calculateDensity(emptyMap, totalWords);

    CHECK(result == 0.0);
}

TEST_CASE("calculateDensity with non-empty occurrences") {
    std::unordered_map<std::string, int> occurrences = {
        {"apple", 3},
        {"orange", 2},
        {"banana", 4}
    };
    int totalWords = 50;  // Some arbitrary total words
    auto result = calculateDensity(occurrences, totalWords);

    double expectedDensity = (3 + 2 + 4) / static_cast<double>(totalWords);

    CHECK(result == doctest::Approx(expectedDensity));
}

TEST_CASE("countOccurrences with empty input") {
    std::vector<std::string> emptyWords;
    auto result = countOccurences(emptyWords);

    CHECK(result.empty());
}

TEST_CASE("countOccurrences with non-empty input") {
    std::vector<std::string> words = {"apple", "orange", "banana", "apple", "banana"};
    auto result = countOccurences(words);

    CHECK(result.size() == 3);  // There are three unique words

    CHECK(result["apple"] == 2);
    CHECK(result["orange"] == 1);
    CHECK(result["banana"] == 2);
}

TEST_CASE("filterWords with empty filterList") {
    std::vector<std::string> emptyFilterList;
    auto filterFunction = filterWords(emptyFilterList);

    std::vector<std::string> wordList = {"apple", "orange", "banana"};
    auto result = filterFunction(wordList);

    CHECK(result.empty());
}

TEST_CASE("filterWords with non-empty filterList") {
    std::vector<std::string> filterList = {"apple", "banana"};
    auto filterFunction = filterWords(filterList);

    std::vector<std::string> wordList = {"apple", "orange", "banana", "grape"};
    auto result = filterFunction(wordList);

    CHECK(result.size() == 2);  // "apple" and "banana" are in the filterList

    CHECK(result[0] == "apple");
    CHECK(result[1] == "banana");
}

TEST_CASE("tokenize with empty optionalInputText") {
    std::optional<std::string> emptyInputText = std::nullopt;
    auto result = tokenize(emptyInputText);

    CHECK(result.empty());
}

TEST_CASE("tokenize with non-empty optionalInputText") {
    std::optional<std::string> inputText = "CHAPTER 1 The Quick Brown Fox Jumps Over the Lazy Dog";
    auto result = tokenize(inputText);

    CHECK(result.size() == 10);  // 10 tokens in the example sentence

    CHECK(result[0] == "CHAPTER_1");
    CHECK(result[1] == "The");
    CHECK(result[2] == "Quick");
    CHECK(result[3] == "Brown");
    CHECK(result[4] == "Fox");
    CHECK(result[5] == "Jumps");
    CHECK(result[6] == "Over");
    CHECK(result[7] == "the");
    CHECK(result[8] == "Lazy");
    CHECK(result[9] == "Dog");
}

